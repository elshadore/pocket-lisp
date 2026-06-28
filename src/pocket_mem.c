#include "pocket_internals.h"

PKRes pk_pool_expand(Pocket lisp) {
    PKPool *pool;
    pk_try(pk_talloc(PKPool, lisp, &pool));

    pool->next = lisp->pool;
    for (size_t i = 0; i < (PK_POOL_MAX - 1); ++i) {
        PKAtom *a = &pool->e[i];
        PKAtom *b = &pool->e[i + 1];
        a->free = (PKAtomFree) {
            .tag.ty = PKAtomTy_Free,
            .tag.marked = false,
            .next = (PKAtomFree *)b,
        };
    }
    PKAtomFree *free = (PKAtomFree *)&pool->e[PK_POOL_MAX - 1];
    *free = (PKAtomFree) {
        .tag.ty = PKAtomTy_Free,
        .tag.marked = false,
        .next = lisp->free,
    };
    lisp->free = free;
    lisp->pool = pool;
    return PK_Ok;
}

PKRes pk_atom_alloc(Pocket lisp, PKAtom **output) {
    if (lisp->free == NULL) {
        pk_try(pk_pool_expand(lisp));
    }
    PKAtomFree *result = lisp->free;
    lisp->free = result->next;
    *output = (PKAtom *)result;
    return PK_Ok;
}

void pk_atom_free(Pocket lisp, PKAtom *atom) {
    switch (atom->tag.ty) {
        case PKAtomTy_Free: {
            return;
        }
        case PKAtomTy_String: {
            pk_string_free(lisp, atom->string.lit);
            break;
        }
        default: {
            break;
        }
    }
    PKAtomFree *free = (PKAtomFree *)atom;
    *free = (PKAtomFree) {
        .tag.ty = PKAtomTy_Free,
        .tag.marked = false,
        .next = lisp->free,
    };
    lisp->free = free;
}

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity) {
    if (old_capacity == 0) {
        return init_capacity;
    } else {
        return old_capacity * 2;
    }
}

PKRes pk_malloc(Pocket lisp, size_t size, void **output) {
    void *result = (lisp->alloc)(lisp->user_env, NULL, 0, size);
    if (result == NULL) {
        return pk_error(lisp);
    }
    *output = result;
    return PK_Ok;
}

PKRes pk_realloc(Pocket lisp, void *ptr, size_t old_size, size_t new_size, void **output) {
    void *result = (lisp->alloc)(lisp->user_env, ptr, old_size, new_size);
    if ((new_size > 0) && (result == NULL)) {
        return pk_error(lisp);
    }
    *output = result;
    return PK_Ok;
}

void pk_free(Pocket lisp, void *ptr, size_t size) {
    (void)(lisp->alloc)(lisp->user_env, ptr, size, 0);
}

PKRes pk_error_impl(Pocket lisp, const char *file, int line) {
    (void)lisp;
    fprintf(stderr, "ERROR: %s:%d\n", file, line);
    (void)pk_trace_dump(lisp, "error");
    return PK_Yield;
}
