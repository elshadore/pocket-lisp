#include "pocket_internals.h"

Pocket pk_init(void *user_closure, PKAllocFn alloc) {
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
    };

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
    pk_push(lisp, pk_make_atom_nil(lisp));
}

void pk_push_t(Pocket lisp) {
    pk_push(lisp, (PKAtom *)pk_make_atom_symbol(lisp, pkstr("t")));
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
