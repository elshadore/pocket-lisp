#include "pocket_internals.h"

PocketLispMachine PK_CORE = (PocketLispMachine) {
    .pool = NULL,
    .free = NULL,
    .cache = (PKCache){0},
    .initialized = false,
};

PKAtom pk_init2(void) {
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
}
