#include "pocket_internals.h"

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity) {
    if (old_capacity == 0) {
        return init_capacity;
    } else {
        return old_capacity * 2;
    }
}

void *pk_malloc(Pocket lisp, size_t size) {
    void *result = malloc(size);
    if ((result == NULL) && (size > 0)) {
        pk_error_oom(lisp);
    }
    return result;
}

void *pk_realloc(Pocket lisp, void *ptr, size_t new_size) {
    void *result = realloc(ptr, new_size);
    if ((result == NULL) && (new_size > 0)) {
        pk_error_oom(lisp);
    }
    return result;
}

void pk_free(Pocket lisp, void *ptr) {
    (void)lisp;
    free(ptr);
}

void pk_pool_expand(Pocket lisp) {
    PKPool *pool = (PKPool *)pk_malloc(lisp, sizeof(PKPool));
    if (pool == NULL) {
        pk_error_oom(lisp);
    }

    for (size_t i = 0; i < (PK_POOL_MAX - 1); ++i) {
        PKAtom *a = &pool->e[i];
        PKAtom *b = &pool->e[i + 1];
        a->free = (PKAtomFree) {
            .header = (PKAtomHeader) {
                .type = PK_ATOMTY_FREE,
                .flags = PK_ATOMFLAG_NONE,
            },
            .next = (PKAtomFree *)b,
        };
    }

    (pool->e[PK_POOL_MAX - 1]).free = (PKAtomFree) {
        .header = (PKAtomHeader) {
            .type = PK_ATOMTY_FREE,
        },
        .next = NULL,
    };

    pool->next = lisp->pool;
    lisp->pool = pool;
    lisp->free = (PKAtomFree *)&pool->e[0];
}

PKAtom *pk_atom_alloc(Pocket lisp) {
    if (lisp->free == NULL) {
        pk_pool_expand(lisp);
    }
    
    PKAtomFree *result = lisp->free;
    lisp->free = result->next;

    result->header = (PKAtomHeader) {
        .type = PK_ATOMTY_UNDEFINED,
        .flags = PK_ATOMFLAG_NONE,
    };

    return (PKAtom *)result;
}

void pk_atom_free(Pocket lisp, PKAtom *atom) {
    pk_u8 type = atom->header.type;
    if (type == PK_ATOMTY_FREE) {
        return;
    }
    pk_atom_deinit(lisp, atom);
    atom->header = (PKAtomHeader) {
        .type = PK_ATOMTY_FREE,
        .flags = PK_ATOMFLAG_NONE,
    };
    atom->free.next = lisp->free;
    lisp->free = (PKAtomFree *)atom;
}
