#include "pocket_internals.h"

PKRes pk_init2(Pocket lisp) {
    pk_try(pk_atom_nil_new(lisp, &lisp->cache.nil));
    
    pk_try(pk_atom_symbol_interned(lisp, pkstr("t"), &lisp->cache.t));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("nil"), &lisp->cache.nil_sym));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("lambda"), &lisp->cache.lambda));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("macro"), &lisp->cache.macro));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("quote"), &lisp->cache.quote));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("quasiquote"), &lisp->cache.quasiquote));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("unquote"), &lisp->cache.unquote));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("unquote-splice"), &lisp->cache.unquote_splice));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("string-substitute"), &lisp->cache.string_substitute));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("progn"), &lisp->cache.progn));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("while"), &lisp->cache.while_sym));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("if"), &lisp->cache.if_sym));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("let"), &lisp->cache.let_sym));
    pk_try(pk_atom_symbol_interned(lisp, pkstr("let*"), &lisp->cache.let_star));
    
    pk_try(pk_atom_string_nomemcpy(lisp, PK_STRING_EMPTY, &lisp->cache.empty_string));

    PKAtom *_ignored;
    pk_try(pk_env_set(lisp, PKEnvTy_Var, lisp->cache.t, (PKAtom *)lisp->cache.t, &_ignored));
    pk_try(pk_env_set(lisp, PKEnvTy_Var, lisp->cache.nil_sym, lisp->cache.nil, &_ignored));

    pk_try(pk_load_std(lisp));
    
    return PK_Ok;
}

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
    if (!pk_init2(lisp)) {
        pk_deinit(lisp);
        return NULL;
    }
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

    pk_arena_deinit_all(lisp);
    
    pk_free(lisp, lisp, sizeof(struct PocketLispMachine_));
}
