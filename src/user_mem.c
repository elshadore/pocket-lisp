#include "user_internals.h"

size_t ul_grow_capacity(size_t old_capacity, size_t init_capacity) {
    if (old_capacity == 0) {
        return init_capacity;
    } else {
        return old_capacity * 2;
    }
}

void *ul_malloc(size_t size) {
    return malloc(size);
}

void *ul_realloc(void *ptr, size_t new_size) {
    return realloc(ptr, new_size);
}

void ul_free(void *ptr) {
    free(ptr);
}

ULAtom ul_error(void) {
    abort();
    return NULL;
}
    
ULAtom ul_error_oom(void) {
    abort();
    return NULL;
}

ULAtom ul_pool_expand(void) {
    if (!UL_CORE.initialized) {
        if (!ul_init()) {
            return ul_error_oom();
        }
    }
    
    ULPool *pool = (ULPool *)ul_malloc(sizeof(ULPool));
    if (pool == NULL) {
        return ul_error_oom();
    }

    for (size_t i = 0; i < (UL_POOL_MAX - 1); ++i) {
        ULAtom a = &pool->e[i];
        ULAtom b = &pool->e[i + 1];
        a->free = (ULAtomFree) {
            .header = (ULAtomHeader) {
                .type = UL_ATOMTY_FREE,
                .refcount = 0,
            },
            .next = (ULAtomFree *)b,
        };
    }

    (pool->e[UL_POOL_MAX - 1]).free = (ULAtomFree) {
        .header = (ULAtomHeader) {
            .type = UL_ATOMTY_FREE,
            .refcount = 0,
        },
        .next = NULL,
    };

    pool->next = UL_CORE.pool;
    UL_CORE.pool = pool;
    UL_CORE.free = (ULAtomFree *)&pool->e[0];
    
    return ul_true();
}

ULAtom ul_atom_alloc(void) {
    if (UL_CORE.free == NULL) {
        ul_propagate(ul_pool_expand());
    }
    ULAtomFree *free = UL_CORE.free;
    UL_CORE.free = free->next;
    return (ULAtom)free;
}
