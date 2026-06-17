#include "pocket_internals.h"

Pocket pk_init(void *user_closure, PKAllocFn alloc, PKPrintFn print) {
    Pocket lisp = (alloc)(user_closure, NULL, 0, sizeof(struct PocketLispMachine_));
    if (lisp == NULL) {
        return NULL;
    }
    *lisp = (struct PocketLispMachine_) {
        .user_env = user_closure,
        .alloc = alloc,
        .stack = (PKStack){0},
        .free = NULL,
        .pool = NULL,
        .print = print,
        .cache = (PKCache){0},
    };
    lisp->cache = (PKCache) {
        .nil = pk_atom_nil(lisp),
        .t = pk_atom_symbol_interned(lisp, pkstr("t")),
        .nilsym = pk_atom_symbol_interned(lisp, pkstr("nil")),
        .lambda = pk_atom_symbol_interned(lisp, pkstr("lambda")),
        .quote = pk_atom_symbol_interned(lisp, pkstr("quote")),
        .if_sym = pk_atom_symbol_interned(lisp, pkstr("if")),
        .empty_string = pk_atom_string_nomemcpy(lisp, pkstr("")),
    };

    pk_env_set(lisp, PKEnvTy_Var, lisp->cache.t, (PKAtom *)lisp->cache.t);
    pk_env_set(lisp, PKEnvTy_Var, lisp->cache.nilsym, lisp->cache.nil);

    pk_load_std(lisp);

    return lisp;
}

void pk_deinit(Pocket lisp) {
    if (lisp == NULL) {
        return;
    }

    pk_symtable_deinit(lisp, &lisp->vars);
    pk_symtable_deinit(lisp, &lisp->funs);

    if (lisp->stack.e != NULL) {
        pk_free(lisp, lisp->stack.e, lisp->stack.capacity * sizeof(PKAtom *));
    }
    
    if (lisp->frames.e != NULL) {
        pk_free(lisp, lisp->frames.e, lisp->frames.capacity * sizeof(PKFrame));
    }

    if (lisp->lets.e != NULL) {
        pk_free(lisp, lisp->lets.e, lisp->lets.capacity * sizeof(PKLet));
    }

    if (lisp->intern.e != NULL) {
        pk_free(lisp, lisp->intern.e, lisp->intern.capacity * sizeof(PKAtomSymbol *));
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
    pk_push(lisp, (PKAtom *)pk_atom_int(lisp, integer));
}

void pk_push_float(Pocket lisp, float floater) {
    pk_push(lisp, (PKAtom *)pk_atom_float(lisp, floater));
}

void pk_push_string(Pocket lisp, PKString string) {
    pk_push(lisp, (PKAtom *)pk_atom_string(lisp, string));
}

void pk_push_cstr(Pocket lisp, char *cstr) {
    pk_push_string(lisp, pk_string_from_cstr(cstr));
}

void pk_push_nstr(Pocket lisp, char *str, size_t length) {
    pk_push_string(lisp, pk_string_new(str, length));
}

void pk_push_symbol(Pocket lisp, PKString symbol) {
    pk_push(lisp, (PKAtom *)pk_atom_symbol_interned(lisp, symbol));
}

void pk_push_csym(Pocket lisp, char *cstr) {
    pk_push_symbol(lisp, pk_string_from_cstr(cstr));
}

void pk_push_nsym(Pocket lisp, char *sym, size_t length) {
    pk_push_symbol(lisp, pk_string_new(sym, length));
}

void pk_dupe(Pocket lisp, int stack_pointer) {
    PKAtom *atom = pk_stack_get(lisp, stack_pointer);
    pk_push(lisp, atom);
}

void pk_insert(Pocket lisp, int from, int to) {
    size_t a = pk_sp_index(lisp, from);
    size_t b = pk_sp_index(lisp, to);
    lisp->stack.e[b] = lisp->stack.e[a];
}

void pk_swap(Pocket lisp, int a, int b) {
    size_t ia = pk_sp_index(lisp, a);
    size_t ib = pk_sp_index(lisp, b);
    PKAtom *tmp = lisp->stack.e[ia];
    lisp->stack.e[ia] = lisp->stack.e[ib];
    lisp->stack.e[ib] = tmp;
}

void pk_car(Pocket lisp, int cons) {
    PKAtomCons *c = pk_atom_cast_cons(lisp, pk_stack_get(lisp, cons));
    pk_push(lisp, c->car);
}

void pk_cdr(Pocket lisp, int cons) {
    PKAtomCons *c = pk_atom_cast_cons(lisp, pk_stack_get(lisp, cons));
    pk_push(lisp, c->cdr);
}

void pk_set_car(Pocket lisp, int cons, int new_car) {
    PKAtomCons *c = pk_atom_cast_cons(lisp, pk_stack_get(lisp, cons));
    c->car = pk_stack_get(lisp, new_car);
}

void pk_set_cdr(Pocket lisp, int cons, int new_cdr) {
    PKAtomCons *c = pk_atom_cast_cons(lisp, pk_stack_get(lisp, cons));
    c->cdr = pk_stack_get(lisp, new_cdr);
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

bool pk_eq(Pocket lisp, int lhs, int rhs) {
    PKAtom *a = pk_stack_get(lisp, lhs);
    PKAtom *b = pk_stack_get(lisp, rhs);
    return pk_atom_eq(lisp, a, b);
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

void pk_push_cfunc(Pocket lisp, void *user_closure, PKFn fn, int args, PKArity mode) {
    PKFuncArity arity = { .mode = mode, .args = args };
    pk_push(lisp, (PKAtom *)pk_atom_cfunc(lisp, user_closure, fn, arity));
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
    if (result == NULL) {
        pk_push(lisp, lisp->cache.nil);
    } else {
        pk_push(lisp, (PKAtom *)result);
    }
}

void pk_format(Pocket lisp, int stack_pointer) {
    PKAtom *atom = pk_stack_get(lisp, stack_pointer);
    PKWriter w = pk_writer_init(lisp);
    pk_writer_atom(&w, atom);
    PKAtomString *string = pk_atom_string(lisp, pk_string_new(w.c, w.count));
    pk_push(lisp, (PKAtom *)string);
    pk_writer_deinit(&w);
}
