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
