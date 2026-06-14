#include "pocket_internals.h"

Pocket pk_init(void *user_closure, PKAllocFn alloc, PKPrintFn print) {
    Pocket lisp = (alloc)(user_closure, NULL, 0, sizeof(struct PocketLispMachine_));
    if (lisp == NULL) {
        return NULL;
    }
    *lisp = (struct PocketLispMachine_) {
        .user_closure = user_closure,
        .alloc = alloc,
        .stack = (PKStack){0},
        .free = NULL,
        .pool = NULL,
        .print = print,
        .cache = (PKCache){0},
    };
    
    lisp->cache.nil = pk_make_atom_nil(lisp);
    lisp->cache.t = pk_make_atom_symbol(lisp, pkstr("t"));

    return lisp;
}

void pk_deinit(Pocket lisp) {
    if (lisp == NULL) {
        return;
    }

    if (lisp->stack.e != NULL) {
        pk_free(lisp, lisp->stack.e, lisp->stack.capacity * sizeof(PKAtom *));
    }

    for (PKPool *pool = lisp->pool; pool != NULL; pool = pool->next) {
        for (size_t i = 0; i < PK_POOL_MAX; ++i) {
            pk_atom_free(lisp, &pool->e[i]);
        }
    }

    PKPool *pool = lisp->pool;
    while (pool != NULL) {
        PKPool *next = pool->next;
        pk_free(lisp, pool, sizeof(PKPool));
        pool = next;
    }

    pk_free(lisp, lisp, sizeof(struct PocketLispMachine_));
}

void pk_push_nil(Pocket lisp) {
    pk_push(lisp, lisp->cache.nil);
}

void pk_push_t(Pocket lisp) {
    pk_push(lisp, (PKAtom *)lisp->cache.t);
}

void pk_push_cond(Pocket lisp, bool cond) {
    if (cond) {
        pk_push_t(lisp);
    } else {
        pk_push_nil(lisp);
    }
}

void pk_push_int(Pocket lisp, int integer) {
    pk_push(lisp, (PKAtom *)pk_make_atom_int(lisp, integer));
}

void pk_push_float(Pocket lisp, float floater) {
    pk_push(lisp, (PKAtom *)pk_make_atom_float(lisp, floater));
}

void pk_push_string(Pocket lisp, PKString string) {
    pk_push(lisp, (PKAtom *)pk_make_atom_string(lisp, string));
}

void pk_push_cstr(Pocket lisp, char *cstr) {
    pk_push_string(lisp, pk_string_from_cstr(cstr));
}

void pk_push_nstr(Pocket lisp, char *str, size_t length) {
    pk_push_string(lisp, pk_string_new(str, length));
}

void pk_push_symbol(Pocket lisp, PKString symbol) {
    pk_push(lisp, (PKAtom *)pk_make_atom_symbol(lisp, symbol));
}

void pk_push_csym(Pocket lisp, char *cstr) {
    pk_push_symbol(lisp, pk_string_from_cstr(cstr));
}

void pk_push_nsym(Pocket lisp, char *sym, size_t length) {
    pk_push_symbol(lisp, pk_string_new(sym, length));
}

void pk_add(Pocket lisp, int lhs, int rhs) {
    PKAtom *a = pk_stack_get(lisp, lhs);
    PKAtom *b = pk_stack_get(lisp, rhs);
    PKAtomNumber *n = pk_number_add(lisp, pk_atom_cast_number(lisp, a), pk_atom_cast_number(lisp, b));
    pk_push(lisp, (PKAtom *)n);
}

void pk_sub(Pocket lisp, int lhs, int rhs) {
    PKAtom *a = pk_stack_get(lisp, lhs);
    PKAtom *b = pk_stack_get(lisp, rhs);
    PKAtomNumber *n = pk_number_sub(lisp, pk_atom_cast_number(lisp, a), pk_atom_cast_number(lisp, b));
    pk_push(lisp, (PKAtom *)n);
}

void pk_mul(Pocket lisp, int lhs, int rhs) {
    PKAtom *a = pk_stack_get(lisp, lhs);
    PKAtom *b = pk_stack_get(lisp, rhs);
    PKAtomNumber *n = pk_number_mul(lisp, pk_atom_cast_number(lisp, a), pk_atom_cast_number(lisp, b));
    pk_push(lisp, (PKAtom *)n);
}

void pk_div(Pocket lisp, int lhs, int rhs) {
    PKAtom *a = pk_stack_get(lisp, lhs);
    PKAtom *b = pk_stack_get(lisp, rhs);
    PKAtomNumber *n = pk_number_div(lisp, pk_atom_cast_number(lisp, a), pk_atom_cast_number(lisp, b));
    pk_push(lisp, (PKAtom *)n);
}

bool pk_gt(Pocket lisp, int lhs, int rhs) {
    PKAtom *a = pk_stack_get(lisp, lhs);
    PKAtom *b = pk_stack_get(lisp, rhs);
    return pk_number_gt(lisp, pk_atom_cast_number(lisp, a), pk_atom_cast_number(lisp, b));
}

bool pk_gte(Pocket lisp, int lhs, int rhs) {
    PKAtom *a = pk_stack_get(lisp, lhs);
    PKAtom *b = pk_stack_get(lisp, rhs);
    return pk_number_gte(lisp, pk_atom_cast_number(lisp, a), pk_atom_cast_number(lisp, b));
}

bool pk_lt(Pocket lisp, int lhs, int rhs) {
    PKAtom *a = pk_stack_get(lisp, lhs);
    PKAtom *b = pk_stack_get(lisp, rhs);
    return pk_number_lt(lisp, pk_atom_cast_number(lisp, a), pk_atom_cast_number(lisp, b));
}

bool pk_lte(Pocket lisp, int lhs, int rhs) {
    PKAtom *a = pk_stack_get(lisp, lhs);
    PKAtom *b = pk_stack_get(lisp, rhs);
    return pk_number_lte(lisp, pk_atom_cast_number(lisp, a), pk_atom_cast_number(lisp, b));
}

int pk_to_int(Pocket lisp, int stack_pointer) {
    PKAtom *atom = pk_stack_get(lisp, stack_pointer);
    return pk_number_to_int(pk_atom_cast_number(lisp, atom));
}

float pk_to_float(Pocket lisp, int stack_pointer) {
    PKAtom *atom = pk_stack_get(lisp, stack_pointer);
    return pk_number_to_float(pk_atom_cast_number(lisp, atom));
}

PKString pk_to_string(Pocket lisp, int stack_pointer) {
    PKAtom *atom = pk_stack_get(lisp, stack_pointer);
    PKAtomString *s = pk_atom_cast_string(lisp, atom);
    return s->lit;
}

void pk_read(Pocket lisp, int stack_pointer) {
    PKAtom *atom = pk_stack_get(lisp, stack_pointer);
    PKAtomString *s = pk_atom_cast_string(lisp, atom);
    pk_read_string(lisp, s->lit);
}

void pk_read_cstr(Pocket lisp, char *cstr) {
    PKString string = pk_string_from_cstr(cstr);
    pk_read_string(lisp, string);
}

void pk_read_nstr(Pocket lisp, char *string, size_t length) {
    pk_read_string(lisp, pk_string_new(string, length));
}

void pk_read_string(Pocket lisp, PKString string) {
    PKAtomCons *result = pk_read_from_string(lisp, string);
    pk_push(lisp, (PKAtom *)result);
}
