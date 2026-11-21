#include "pocket_internals.h"

PKAtom *pk_generic_el(Pocket lisp, PKAtom *atom, size_t index) {
    switch (atom->header.type) {
        case PK_ATOMTY_CONS: {
            return pk_atom_list_el(lisp, atom, index);
        }
        case PK_ATOMTY_STRING: {
            return (PKAtom *)pk_atom_string_el(lisp, (PKAtomString *)atom, index);
        }
        case PK_ATOMTY_ARRAY: {
            return (PKAtom *)pk_atom_array_el(lisp, (PKAtomArray *)atom, index);
        }
        case PK_ATOMTY_BLOB: {
            return pk_atom_blob_el(lisp, (PKAtomBlob *)atom, index);
        }
        default: {
            pk_error_type4(lisp, atom, PK_TYPE_CONS, PK_TYPE_STRING, PK_TYPE_ARRAY, PK_TYPE_BLOB);
            return lisp->cache.nil;
        }
    }
}

bool pk_generic_is_element(Pocket lisp, PKAtom *atom, PKAtom *element) {
    switch (atom->header.type) {
        case PK_ATOMTY_NIL: {
            return false;
        }
        case PK_ATOMTY_CONS: {
            return pk_atom_list_is_element(lisp, atom, element);
        }
        case PK_ATOMTY_ARRAY: {
            return pk_atom_array_is_element(lisp, (PKAtomArray *)atom, element);
        }
        default: {
            pk_error_type3(lisp, atom, PK_TYPE_NIL, PK_TYPE_CONS, PK_TYPE_ARRAY);
            return 0;
        }
    }
}

size_t pk_generic_length(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_NIL: {
            return 0;
        }
        case PK_ATOMTY_CONS: {
            return pk_atom_list_length(lisp, atom);
        }
        case PK_ATOMTY_STRING: {
            return pk_atom_string_length(lisp, (PKAtomString *)atom);
        }
        case PK_ATOMTY_ARRAY: {
            return pk_atom_array_length(lisp, (PKAtomArray *)atom);
        }
        case PK_ATOMTY_BLOB: {
            return pk_atom_blob_length(lisp, (PKAtomBlob *)atom);
        }
        default: {
            pk_error_type4(lisp, atom, PK_TYPE_NIL, PK_TYPE_CONS, PK_TYPE_STRING, PK_TYPE_ARRAY);
            return 0;
        }
    }
}

PKAtom *pk_generic_to_list(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_NIL: {
            return lisp->cache.nil;
        }
        case PK_ATOMTY_CONS: {
            return atom;
        }
        case PK_ATOMTY_ARRAY: {
            PKAtoms atoms = pk_array_slice(&atom->array.atoms);
            return pk_atom_list(lisp, atoms);
        }
        default: {
            pk_error_type3(lisp, atom, PK_TYPE_NIL, PK_TYPE_CONS, PK_TYPE_ARRAY);
            return lisp->cache.nil;
        }
    }
}

PKAtom *pk_generic_cat(Pocket lisp, PKAtom *atom, PKAtoms atoms) {
    switch (atom->header.type) {
        case PK_ATOMTY_NIL: {
            return lisp->cache.nil;
        }
        case PK_ATOMTY_CONS: {
            return pk_atom_list_cat(lisp, atoms);
        }
        case PK_ATOMTY_STRING: {
            return (PKAtom *)pk_atom_string_cat(lisp, atoms);
        }
        case PK_ATOMTY_BLOB: {
            return (PKAtom *)pk_atom_blob_cat(lisp, atoms);
        }
        default: {
            pk_error_type4(lisp, atom, PK_TYPE_NIL, PK_TYPE_CONS, PK_TYPE_STRING, PK_TYPE_BLOB);
            return lisp->cache.nil;
        }
    }
}

PKAtom *pk_generic_reverse(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_NIL: {
            return lisp->cache.nil;
        }
        case PK_ATOMTY_STRING: {
            return (PKAtom *)pk_atom_string_reverse(lisp, atom->string.str);
        }
        case PK_ATOMTY_CONS: {
            return pk_atom_list_reverse(lisp, atom);
        }
        case PK_ATOMTY_BLOB: {
            return (PKAtom *)pk_atom_blob_reverse(lisp, atom->blob.bytes);
        }
        default: {
            pk_error_type4(lisp, atom, PK_TYPE_NIL, PK_TYPE_CONS, PK_TYPE_STRING, PK_TYPE_BLOB);
            return lisp->cache.nil;
        }
    }
}

PKAtomString *pk_generic_to_string(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            return (PKAtomString *)atom;
        }
        case PK_ATOMTY_SYMBOL: {
            return atom->symbol.name;
        }
        case PK_ATOMTY_KEYWORD: {
            return pk_atom_keyword_to_string(lisp, (PKAtomKeyword *)atom);
        }
        case PK_ATOMTY_BLOB: {
            return pk_atom_blob_to_string(lisp, (PKAtomBlob *)atom);
        }
        default: {
            pk_error_type4(lisp, atom, PK_TYPE_STRING, PK_TYPE_SYMBOL, PK_TYPE_KEYWORD, PK_TYPE_BLOB);
            return lisp->cache.empty_string;
        }
    }
}

PKAtomSymbol *pk_generic_to_symbol(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_SYMBOL: {
            return (PKAtomSymbol *)atom;
        }
        case PK_ATOMTY_KEYWORD: {
            return pk_atom_keyword_to_symbol(lisp, (PKAtomKeyword *)atom);
        }
        case PK_ATOMTY_STRING: {
            return pk_atom_string_to_symbol(lisp, (PKAtomString *)atom);
        }
        default: {
            pk_error_type3(lisp, atom, PK_TYPE_STRING, PK_TYPE_SYMBOL, PK_TYPE_KEYWORD);
            return (PKAtomSymbol *)lisp->cache.nil;
        }
    }
}

PKAtomKeyword *pk_generic_to_keyword(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_SYMBOL: {
            return pk_atom_symbol_to_keyword(lisp, (PKAtomSymbol *)atom);
        }
        case PK_ATOMTY_KEYWORD: {
            return (PKAtomKeyword *)atom;
        }
        case PK_ATOMTY_STRING: {
            return pk_atom_string_to_keyword(lisp, (PKAtomString *)atom);
        }
        default: {
            pk_error_type3(lisp, atom, PK_TYPE_STRING, PK_TYPE_SYMBOL, PK_TYPE_KEYWORD);
            return (PKAtomKeyword *)lisp->cache.nil;
        }
    }
}

PKAtom *pk_generic_property_put(Pocket lisp, PKAtom *pobj, PKAtom *key, PKAtom *value) {
    switch (pobj->header.type) {
        case PK_ATOMTY_SYMBOL: {
            return pk_atom_symbol_put(lisp, (PKAtomSymbol *)pobj, key, value);
        }
        case PK_ATOMTY_CFUN: {
            return pk_atom_cfun_put(lisp, (PKAtomCFun *)pobj, key, value);
        }
        case PK_ATOMTY_CMACRO: {
            return pk_atom_cfun_put(lisp, (PKAtomCFun *)pobj, key, value);
        }
        case PK_ATOMTY_LFUN: {
            return pk_atom_lfun_put(lisp, (PKAtomLFun *)pobj, key, value);
        }
        case PK_ATOMTY_LMACRO: {
            return pk_atom_lfun_put(lisp, (PKAtomLFun *)pobj, key, value);
        }
        case PK_ATOMTY_OBJECT: {
            return pk_atom_object_put(lisp, (PKAtomObject *)pobj, key, value);
        }
        default: {
            pk_error_type4(lisp, pobj, PK_TYPE_SYMBOL, PK_TYPE_FUNCTION, PK_TYPE_MACRO, PK_TYPE_OBJECT);
            return lisp->cache.nil;
        }
    }
}

PKAtom *pk_generic_property_get(Pocket lisp, PKAtom *pobj, PKAtom *key) {
    switch (pobj->header.type) {
        case PK_ATOMTY_SYMBOL: {
            return pk_atom_symbol_get(lisp, (PKAtomSymbol *)pobj, key);
        }
        case PK_ATOMTY_CFUN: {
            return pk_atom_cfun_get(lisp, (PKAtomCFun *)pobj, key);
        }
        case PK_ATOMTY_CMACRO: {
            return pk_atom_cfun_get(lisp, (PKAtomCFun *)pobj, key);
        }
        case PK_ATOMTY_LFUN: {
            return pk_atom_lfun_get(lisp, (PKAtomLFun *)pobj, key);
        }
        case PK_ATOMTY_LMACRO: {
            return pk_atom_lfun_get(lisp, (PKAtomLFun *)pobj, key);
        }
        case PK_ATOMTY_OBJECT: {
            return pk_atom_object_get(lisp, (PKAtomObject *)pobj, key);
        }
        default: {
            pk_error_type4(lisp, pobj, PK_TYPE_SYMBOL, PK_TYPE_FUNCTION, PK_TYPE_MACRO, PK_TYPE_OBJECT);
            return lisp->cache.nil;
        }
    }
}

PKAtom *pk_generic_property_remove(Pocket lisp, PKAtom *pobj, PKAtom *key) {
    switch (pobj->header.type) {
        case PK_ATOMTY_SYMBOL: {
            return pk_atom_symbol_remove(lisp, (PKAtomSymbol *)pobj, key);
        }
        case PK_ATOMTY_CFUN: {
            return pk_atom_cfun_remove(lisp, (PKAtomCFun *)pobj, key);
        }
        case PK_ATOMTY_CMACRO: {
            return pk_atom_cfun_remove(lisp, (PKAtomCFun *)pobj, key);
        }
        case PK_ATOMTY_LFUN: {
            return pk_atom_lfun_remove(lisp, (PKAtomLFun *)pobj, key);
        }
        case PK_ATOMTY_LMACRO: {
            return pk_atom_lfun_remove(lisp, (PKAtomLFun *)pobj, key);
        }
        case PK_ATOMTY_OBJECT: {
            return pk_atom_object_remove(lisp, (PKAtomObject *)pobj, key);
        }
        default: {
            pk_error_type4(lisp, pobj, PK_TYPE_SYMBOL, PK_TYPE_FUNCTION, PK_TYPE_MACRO, PK_TYPE_OBJECT);
            return lisp->cache.nil;
        }
    }
}

bool pk_generic_property_is_property(Pocket lisp, PKAtom *pobj, PKAtom *key) {
    switch (pobj->header.type) {
        case PK_ATOMTY_SYMBOL: {
            return pk_atom_symbol_is_property(lisp, (PKAtomSymbol *)pobj, key);
        }
        case PK_ATOMTY_CFUN: {
            return pk_atom_cfun_is_property(lisp, (PKAtomCFun *)pobj, key);
        }
        case PK_ATOMTY_CMACRO: {
            return pk_atom_cfun_is_property(lisp, (PKAtomCFun *)pobj, key);
        }
        case PK_ATOMTY_LFUN: {
            return pk_atom_lfun_is_property(lisp, (PKAtomLFun *)pobj, key);
        }
        case PK_ATOMTY_LMACRO: {
            return pk_atom_lfun_is_property(lisp, (PKAtomLFun *)pobj, key);
        }
        case PK_ATOMTY_OBJECT: {
            return pk_atom_object_is_property(lisp, (PKAtomObject *)pobj, key);
        }
        default: {
            pk_error_type4(lisp, pobj, PK_TYPE_SYMBOL, PK_TYPE_FUNCTION, PK_TYPE_MACRO, PK_TYPE_OBJECT);
            return false;
        }
    }
}

PKAtom *pk_generic_properties(Pocket lisp, PKAtom *pobj) {
    switch (pobj->header.type) {
        case PK_ATOMTY_SYMBOL: {
            return pk_atom_symbol_to_plist(lisp, (PKAtomSymbol *)pobj);
        }
        case PK_ATOMTY_CFUN: {
            return pk_atom_cfun_to_plist(lisp, (PKAtomCFun *)pobj);
        }
        case PK_ATOMTY_CMACRO: {
            return pk_atom_cfun_to_plist(lisp, (PKAtomCFun *)pobj);
        }
        case PK_ATOMTY_LFUN: {
            return pk_atom_lfun_to_plist(lisp, (PKAtomLFun *)pobj);
        }
        case PK_ATOMTY_LMACRO: {
            return pk_atom_lfun_to_plist(lisp, (PKAtomLFun *)pobj);
        }
        case PK_ATOMTY_OBJECT: {
            return pk_atom_object_to_plist(lisp, (PKAtomObject *)pobj);
        }
        default: {
            pk_error_type4(lisp, pobj, PK_TYPE_SYMBOL, PK_TYPE_FUNCTION, PK_TYPE_MACRO, PK_TYPE_OBJECT);
            return lisp->cache.nil;
        }
    }
}
