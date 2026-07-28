#include "pocket_internals.h"

PK_RES pk_init2(Pocket lisp) {
    PKAtom *ignore = NULL;
    
    pk_try(pk_atom_nil_new(lisp, &lisp->cache.nil));
    
    pk_try(pk_atom_symbol_interned(lisp, "error", &lisp->cache.error));
    /* Reset throwing state */
    pk_catch_all(lisp);
    
    pk_try(pk_atom_symbol_interned(lisp, "t", &lisp->cache.t));
    pk_try(pk_atom_symbol_interned(lisp, "nil", &lisp->cache.nil_sym));
    pk_try(pk_atom_symbol_interned(lisp, "lambda", &lisp->cache.lambda));
    pk_try(pk_atom_symbol_interned(lisp, "macro", &lisp->cache.macro));
    pk_try(pk_atom_symbol_interned(lisp, "quote", &lisp->cache.quote));
    pk_try(pk_atom_symbol_interned(lisp, "quasiquote", &lisp->cache.quasiquote));
    pk_try(pk_atom_symbol_interned(lisp, "unquote", &lisp->cache.unquote));
    pk_try(pk_atom_symbol_interned(lisp, "unquote-splice", &lisp->cache.unquote_splice));
    pk_try(pk_atom_symbol_interned(lisp, "string-substitute", &lisp->cache.string_substitute));
    pk_try(pk_atom_symbol_interned(lisp, "progn", &lisp->cache.progn));
    pk_try(pk_atom_symbol_interned(lisp, "while", &lisp->cache.while_sym));
    pk_try(pk_atom_symbol_interned(lisp, "if", &lisp->cache.if_sym));
    pk_try(pk_atom_symbol_interned(lisp, "let", &lisp->cache.let_sym));
    pk_try(pk_atom_symbol_interned(lisp, "let*", &lisp->cache.let_star));
    pk_try(pk_atom_symbol_interned(lisp, "flet", &lisp->cache.flet_sym));
    pk_try(pk_atom_symbol_interned(lisp, "flet*", &lisp->cache.flet_star));
    
    pk_try(pk_atom_stringn_nomemcpy(lisp, "", 0, &lisp->cache.empty_string));

    
    pk_try(pk_env_set(lisp, PKEnvTy_Var, lisp->cache.t, (PKAtom *)lisp->cache.t, &ignore));
    pk_try(pk_env_set(lisp, PKEnvTy_Var, lisp->cache.nil_sym, lisp->cache.nil, &ignore));

    pk_try(pk_load_std(lisp));
    
    return PK_OK;
}

Pocket pk_init(void *user_env, PKAllocFn alloc, PKPrintFn print) {
    Pocket lisp = (alloc)(user_env, NULL, 0, sizeof(struct PocketLispMachine_));
    if (lisp == NULL) {
        return NULL;
    }
    
    lisp->user_env = user_env;
    lisp->alloc = alloc;
    lisp->print = print;

    lisp->stack = pk_atoms_init();
    
    lisp->lets.e = NULL;
    lisp->lets.count = 0;
    lisp->lets.capacity = 0;
    
    lisp->frames.e = NULL;
    lisp->frames.count = 0;
    lisp->frames.capacity = 0;
    
    lisp->intern.e = NULL;
    lisp->intern.count = 0;
    lisp->intern.capacity = 0;
    
    lisp->vars = pk_symtable_init();
    lisp->funs = pk_symtable_init();
    
    lisp->current_frame.stack_offset = 0;
    lisp->current_frame.lets_offset = 0;
    lisp->current_frame.arity = 0;

    lisp->free = NULL;
    lisp->pool = NULL;
    
    if (!pk_init2(lisp)) {
        pk_deinit(lisp);
        return NULL;
    }
    return lisp;
}

void pk_deinit(Pocket lisp) {
    PKPool *pool = NULL;

    if (lisp == NULL) {
        return;
    }

    pk_symtable_deinit(lisp, &lisp->vars);
    pk_symtable_deinit(lisp, &lisp->funs);

    pk_atoms_free(lisp, &lisp->stack);

    if (lisp->frames.e != NULL) {
        pk_free(lisp, lisp->frames.e, lisp->frames.capacity * sizeof(PKFrame));
    }

    if (lisp->lets.e != NULL) {
        pk_free(lisp, lisp->lets.e, lisp->lets.capacity * sizeof(PKLet));
    }

    if (lisp->intern.e != NULL) {
        pk_free(lisp, lisp->intern.e, lisp->intern.capacity * sizeof(PKAtomSymbol *));
    }

    for (pool = lisp->pool; pool != NULL; pool = pool->next) {
        size_t i = 0;
        for (i = 0; i < PK_POOL_MAX; ++i) {
            pk_atom_free(lisp, &pool->e[i]);
        }
    }

    pool = lisp->pool;
    while (pool != NULL) {
        PKPool *next = pool->next;
        pk_free(lisp, pool, sizeof(PKPool));
        pool = next;
    }
   
    pk_free(lisp, lisp, sizeof(struct PocketLispMachine_));
}
