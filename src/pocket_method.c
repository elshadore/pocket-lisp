#include "pocket_internals.h"

PKAtom pk_method_set(PKAtom type, PKAtom symbol, PKAtom method) {
    pk_propagate(pk_typecheck_symbol(type));
    pk_propagate(pk_typecheck_symbol(symbol));
    pk_propagate(pk_typecheck_function(method));

    if (type->symbol.type_type == PK_TYPETY_NOTYPE) {
        return pk_error_message(pk_str("not a type: "), type);
    }

    PKAtom old_method = pk_hashmap_insert(&type->symbol.methods, symbol, method);
    pk_propagate(old_method);

    if (pk_is_undefined(old_method)) {
        return pk_nil();
    } else {
        return old_method;
    }
}

PKAtom pk_method_get_raw(PKAtom type, PKAtom symbol) {
    pk_propagate(pk_typecheck_symbol(type));
    pk_propagate(pk_typecheck_symbol(symbol));

    return pk_hashmap_get(&type->symbol.methods, symbol);
}

PKAtom pk_method_get(PKAtom type, PKAtom symbol) {
    PKAtom result = pk_method_get_raw(type, symbol);
    pk_propagate(result);

    if (pk_is_undefined(result)) {
        return pk_nil();
    } else {
        return result;
    }
}