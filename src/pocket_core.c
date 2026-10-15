#include "pocket_internals.h"

PocketLispMachine PK_CORE = POCKET_LISP_MACHINE_EMPTY;

PKAtom pk_init_symbols(void) {
    PK_CORE.cache.lambda = pk_symbol(pk_str("lambda"));
    pk_propagate(PK_CORE.cache.lambda);
    
    PK_CORE.cache.macro = pk_symbol(pk_str("macro"));
    pk_propagate(PK_CORE.cache.macro);
    
    PK_CORE.cache.progn = pk_symbol(pk_str("progn"));
    pk_propagate(PK_CORE.cache.progn);
    
    PK_CORE.cache.sym_if = pk_symbol(pk_str("if"));
    pk_propagate(PK_CORE.cache.sym_if);
    
    PK_CORE.cache.sym_while = pk_symbol(pk_str("while"));
    pk_propagate(PK_CORE.cache.sym_while);
    
    PK_CORE.cache.sym_and = pk_symbol(pk_str("and"));
    pk_propagate(PK_CORE.cache.sym_and);
    
    PK_CORE.cache.sym_or = pk_symbol(pk_str("or"));
    pk_propagate(PK_CORE.cache.sym_or);
    
    PK_CORE.cache.error = pk_symbol(pk_str("error"));
    pk_propagate(PK_CORE.cache.error);
    
    PK_CORE.cache.throw = pk_symbol(pk_str("throw"));
    pk_propagate(PK_CORE.cache.throw);
    
    PK_CORE.cache.var = pk_symbol(pk_str("var"));
    pk_propagate(PK_CORE.cache.var);
    
    PK_CORE.cache.fvar = pk_symbol(pk_str("fvar"));
    pk_propagate(PK_CORE.cache.fvar);
    
    PK_CORE.cache.let = pk_symbol(pk_str("let"));
    pk_propagate(PK_CORE.cache.let);
    
    PK_CORE.cache.let_star = pk_symbol(pk_str("let*"));
    pk_propagate(PK_CORE.cache.let_star);
    
    PK_CORE.cache.flet = pk_symbol(pk_str("flet"));
    pk_propagate(PK_CORE.cache.flet);
    
    PK_CORE.cache.flet_star = pk_symbol(pk_str("flet*"));
    pk_propagate(PK_CORE.cache.flet_star);
    
    PK_CORE.cache.block = pk_symbol(pk_str("block"));
    pk_propagate(PK_CORE.cache.block);
    
    PK_CORE.cache.apply = pk_symbol(pk_str("apply"));
    pk_propagate(PK_CORE.cache.apply);
    
    PK_CORE.cache.macro_apply = pk_symbol(pk_str("macro-apply"));
    pk_propagate(PK_CORE.cache.macro_apply);
    
    PK_CORE.cache.quote = pk_symbol(pk_str("quote"));
    pk_propagate(PK_CORE.cache.quote);
    
    PK_CORE.cache.quasiquote = pk_symbol(pk_str("quasiquote"));
    pk_propagate(PK_CORE.cache.quasiquote);
    
    PK_CORE.cache.unquote = pk_symbol(pk_str("unquote"));
    pk_propagate(PK_CORE.cache.unquote);
    
    PK_CORE.cache.unquote_splice = pk_symbol(pk_str("unquote-splice"));
    pk_propagate(PK_CORE.cache.unquote_splice);
    
    PK_CORE.cache.string_substitute = pk_symbol(pk_str("string-substitute"));
    pk_propagate(PK_CORE.cache.string_substitute);
    
    return pk_true();
}

PKAtom pk_init2(void) {
    pk_propagate(pk_init_symbols());
    pk_propagate(pk_std_load());
    return pk_true();
}

bool pk_init(void) {
    if (PK_CORE.initialized) {
        return true;
    }
    
    PK_CORE.initialized = true;
    
    if (pk_is_throwing(pk_init2())) {
        pk_deinit();
        return false;
    }
    return true;
}

void pk_deinit_cache(void) {
    pk_ref_dec(PK_CORE.cache.lambda);
    pk_ref_dec(PK_CORE.cache.macro);
    pk_ref_dec(PK_CORE.cache.progn);
    pk_ref_dec(PK_CORE.cache.sym_if);
    pk_ref_dec(PK_CORE.cache.sym_while);
    pk_ref_dec(PK_CORE.cache.sym_and);
    pk_ref_dec(PK_CORE.cache.sym_or);
    pk_ref_dec(PK_CORE.cache.error);
    pk_ref_dec(PK_CORE.cache.throw);
    pk_ref_dec(PK_CORE.cache.var);
    pk_ref_dec(PK_CORE.cache.fvar);
    pk_ref_dec(PK_CORE.cache.let);
    pk_ref_dec(PK_CORE.cache.let_star);
    pk_ref_dec(PK_CORE.cache.flet);
    pk_ref_dec(PK_CORE.cache.flet_star);
    pk_ref_dec(PK_CORE.cache.block);
    pk_ref_dec(PK_CORE.cache.apply);
    pk_ref_dec(PK_CORE.cache.macro_apply);
    pk_ref_dec(PK_CORE.cache.quote);
    pk_ref_dec(PK_CORE.cache.quasiquote);
    pk_ref_dec(PK_CORE.cache.unquote);
    pk_ref_dec(PK_CORE.cache.unquote_splice);
    pk_ref_dec(PK_CORE.cache.string_substitute);
}

void pk_deinit(void) {
    if (!PK_CORE.initialized) {
        return;
    }

    pk_array_deinit(&PK_CORE.stack);
    pk_free(PK_CORE.frames.e);
    pk_free(PK_CORE.lets.e);
    pk_deinit_cache();
    pk_hashmap_deinit(&PK_CORE.symbolizer);

    // Symbol Deinit
    // We don't have an easy to interact with environment yet. Our environment currently
    // is just the symbol objects themselves scattered across the memory pool. This code
    // is essentially an environment deinit function.
    for (PKPool *iter = PK_CORE.pool; iter != NULL; iter = iter->next) {
        for (size_t i = 0; i < PK_POOL_MAX; ++i) {
            PKAtom atom = &iter->e[i];
            if (atom->header.type == PK_ATOMTY_SYMBOL) {
                pk_ref_dec(atom);
            }
        }
    }

    #ifdef POCKET_CFLAG_DEBUG
    pk_debug_leakcheck();
    #endif

    {
        PKPool *iter = PK_CORE.pool;
        while (iter != NULL) {
            PKPool *pool = iter;
            iter = iter->next;
            pk_free(pool);
        }
    }
    
    PK_CORE = POCKET_LISP_MACHINE_EMPTY;
}

