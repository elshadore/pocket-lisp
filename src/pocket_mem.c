#include "pocket_internals.h"

PK_RES pk_pool_expand(Pocket lisp) {
    PKPool *pool;
    PKAtomFree *free;
    size_t i = 0;

    pk_try(pk_talloc(PKPool, lisp, &pool));

    pool->next = lisp->pool;
    for (i = 0; i < (PK_POOL_MAX - 1); ++i) {
        PKAtom *a = &pool->e[i];
        PKAtom *b = &pool->e[i + 1];
        
        a->tag.ty = PKAtomTy_Free;
        a->tag.marked = PK_FALSE;
        a->free.next = (PKAtomFree *)b;
    }
    
    free = (PKAtomFree *)&pool->e[PK_POOL_MAX - 1];
    
    free->tag.ty = PKAtomTy_Free;
    free->tag.marked = PK_FALSE;
    free->next = lisp->free;
    
    lisp->free = (PKAtomFree *)&pool->e[0];
    lisp->pool = pool;
    return PK_OK;
}

PK_RES pk_atom_alloc(Pocket lisp, PKAtom **output) {
    PKAtomFree *result = NULL;
    
    if (lisp->free == NULL) {
        pk_try(pk_pool_expand(lisp));
    }
    
    result = lisp->free;
    lisp->free = result->next;
    
    *output = (PKAtom *)result;
    return PK_OK;
}

void pk_atom_free(Pocket lisp, PKAtom *atom) {
    PKAtomFree *free = NULL;
    /* const char *ident = pk_ident_atomty(atom->tag.ty); */
    /* printf("FREEING: %s\n", ident); */
    switch (atom->tag.ty) {
        case PKAtomTy_Free: {
            return;
        }
        case PKAtomTy_String: {
            pk_string_free(lisp, atom->string.c, atom->string.length);
            break;
        }
        case PKAtomTy_LFunc: {
            /*
            PKAtomLFunc *lfunc = (PKAtomLFunc *)atom;
            pk_free(lisp, lfunc->atoms.e, lfunc->atoms.length * sizeof(PKAtom *));
            pk_free(lisp, lfunc->bc.e, lfunc->bc.length * sizeof(pk_u8));
            */
            break;
        }
        default: {
            break;
        }
    }
    
    free = (PKAtomFree *)atom;
    free->tag.ty = PKAtomTy_Free;
    free->tag.marked = PK_FALSE;
    free->next = lisp->free;
    
    lisp->free = free;
}

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity) {
    if (old_capacity == 0) {
        return init_capacity;
    } else {
        return old_capacity * 2;
    }
}

PK_RES pk_malloc(Pocket lisp, size_t size, void **output) {
    void *result = (lisp->alloc)(lisp->user_env, NULL, 0, size);
    if (result == NULL) {
        return pk_error(lisp);
    }
    *output = result;
    return PK_OK;
}

PK_RES pk_realloc(Pocket lisp, void *ptr, size_t old_size, size_t new_size, void **output) {
    void *result = (lisp->alloc)(lisp->user_env, ptr, old_size, new_size);
    if ((new_size > 0) && (result == NULL)) {
        return pk_error(lisp);
    }
    *output = result;
    return PK_OK;
}

void pk_free(Pocket lisp, void *ptr, size_t size) {
    (void)(lisp->alloc)(lisp->user_env, ptr, size, 0);
}

