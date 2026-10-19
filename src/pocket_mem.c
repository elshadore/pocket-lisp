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
    
    return (PKAtom *)result;
}

void pk_atom_free(Pocket lisp, PKAtom *atom) {
    pk_u8 type = atom->header.type;
    if (type == PK_ATOMTY_FREE) {
        return;
    }
    switch (type) {
        case PK_ATOMTY_STRING: {
            pk_free(lisp, atom->string.str.c);
            break;
        }
        case PK_ATOMTY_KEYWORD: {
            pk_free(lisp, atom->keyword.id.c);
            break;
        }
        case PK_ATOMTY_LFUN: {
            pk_free(lisp, atom->lfun.bc.e);
            pk_free(lisp, atom->lfun.atoms.e);
            break;
        }
        case PK_ATOMTY_LMACRO: {
            pk_free(lisp, atom->lmacro.bc.e);
            pk_free(lisp, atom->lmacro.atoms.e);
            break;
        }
        case PK_ATOMTY_SYMBOL: {
            pk_slotmap_deinit(lisp, &atom->symbol.slots);
            pk_hashmap_deinit(lisp, &atom->symbol.methods);
            break;
        }
        case PK_ATOMTY_LOBJECT: {
            pk_atoms_free(lisp, &atom->lobject.slots);
            break;
        }
        case PK_ATOMTY_COBJECT: {
            if (((PKAtomSymbol *)atom->cobject.type)->type_type == PK_TYPETY_C_OWNED) {
                pk_free(lisp, atom->cobject.data);
            }
            break;
        }
        case PK_ATOMTY_CONS: {
            break;
        }
        default: break;
    }
    atom->header = (PKAtomHeader) {
        .type = PK_ATOMTY_FREE,
        .flags = PK_ATOMFLAG_NONE,
    };
    atom->free.next = lisp->free;
    lisp->free = (PKAtomFree *)atom;
}
