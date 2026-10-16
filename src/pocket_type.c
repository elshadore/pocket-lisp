#include "pocket_internals.h"

static PKAtom pk_typecheck_new_type(PKAtom symbol) {
    pk_propagate(pk_typecheck_symbol(symbol));
    if (symbol->symbol.type_type != PK_TYPETY_NOTYPE) {
        return pk_error_message(pk_str("type already defined: "), symbol);
    }
    return pk_true();
}

PKAtom pk_deftype_lisp(PKAtom symbol, PKAtoms slots) {
    pk_propagate(pk_typecheck_new_type(symbol));

    for (size_t i = 0; i < slots.length; ++i) {
        pk_propagate(pk_typecheck_symbol(slots.e[i]));
        for (size_t j = 0; j < i; ++j) {
            if (pk_eq(slots.e[i], slots.e[j])) {
                return pk_error_message(pk_str("slot already defined: "), slots.e[i]);
            }
        }
    }

    pk_propagate(pk_slotmap_reserve(&symbol->symbol.slots, slots.length));

    for (size_t i = 0; i < slots.length; ++i) {
        size_t index = 0;
        pk_propagate(pk_slotmap_insert(&symbol->symbol.slots, slots.e[i], &index));
    }

    symbol->symbol.type_type = PK_TYPETY_LISP;
    symbol->symbol.owned_memory = 0;

    return pk_true();
}

PKAtom pk_deftype_c_owned(PKAtom symbol, size_t size) {
    pk_propagate(pk_typecheck_new_type(symbol));

    symbol->symbol.type_type = PK_TYPETY_C_OWNED;
    symbol->symbol.owned_memory = size;

    return pk_true();
}

PKAtom pk_deftype_c_user(PKAtom symbol) {
    pk_propagate(pk_typecheck_new_type(symbol));

    symbol->symbol.type_type = PK_TYPETY_C_USER;
    symbol->symbol.owned_memory = 0;

    return pk_true();
}
