#include "pocket_internals.h"

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity) {
    if (old_capacity == 0) {
        return init_capacity;
    } else {
        return old_capacity * 2;
    }
}

void *pk_malloc(size_t size) {
    return malloc(size);
}

void *pk_realloc(void *ptr, size_t new_size) {
    return realloc(ptr, new_size);
}

void pk_free(void *ptr) {
    free(ptr);
}

PKAtom pk_error(void) {
    abort();
    return NULL;
}
    
PKAtom pk_error_oom(void) {
    abort();
    return NULL;
}

PKAtom pk_pool_expand(void) {
    if (!PK_CORE.initialized) {
        if (!pk_init()) {
            return pk_error_oom();
        }
    }
    
    PKPool *pool = (PKPool *)pk_malloc(sizeof(PKPool));
    if (pool == NULL) {
        return pk_error_oom();
    }

    for (size_t i = 0; i < (PK_POOL_MAX - 1); ++i) {
        PKAtom a = &pool->e[i];
        PKAtom b = &pool->e[i + 1];
        a->free = (PKAtomFree) {
            .header = (PKAtomHeader) {
                .type = PK_ATOMTY_FREE,
                .refcount = 0,
            },
            .next = (PKAtomFree *)b,
        };
    }

    (pool->e[PK_POOL_MAX - 1]).free = (PKAtomFree) {
        .header = (PKAtomHeader) {
            .type = PK_ATOMTY_FREE,
            .refcount = 0,
        },
        .next = NULL,
    };

    pool->next = PK_CORE.pool;
    PK_CORE.pool = pool;
    PK_CORE.free = (PKAtomFree *)&pool->e[0];
    
    return pk_true();
}

PKAtom pk_atom_alloc(void) {
    if (PK_CORE.free == NULL) {
        pk_propagate(pk_pool_expand());
    }
    PKAtomFree *free = PK_CORE.free;
    PK_CORE.free = free->next;
    return (PKAtom)free;
}
