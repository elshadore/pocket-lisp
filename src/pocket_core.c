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
    
    PK_CORE.cache.error = pk_symbol(pk_str("error"));
    pk_propagate(PK_CORE.cache.error);
    
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
    
    if (pk_is_error(pk_init2())) {
        pk_deinit();
        return false;
    }
    return true;
}

void pk_deinit(void) {
    if (!PK_CORE.initialized) {
        return;
    }
    
    PK_CORE = POCKET_LISP_MACHINE_EMPTY;
}

