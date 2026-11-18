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
        default: {
            pk_error_type3(lisp, atom, lisp->cache.type_cons, lisp->cache.type_string, lisp->cache.type_array);
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
            pk_error_type3(lisp, atom, lisp->cache.type_nil, lisp->cache.type_cons, lisp->cache.type_array);
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
        default: {
            pk_error_type4(lisp, atom, lisp->cache.type_nil, lisp->cache.type_cons, lisp->cache.type_string, lisp->cache.type_array);
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
            pk_error_type3(lisp, atom, lisp->cache.type_nil, lisp->cache.type_cons, lisp->cache.type_array);
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
        default: {
            pk_error_type3(lisp, atom, lisp->cache.type_nil, lisp->cache.type_cons, lisp->cache.type_string);
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
        default: {
            pk_error_type3(lisp, atom, lisp->cache.type_nil, lisp->cache.type_cons, lisp->cache.type_string);
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
        default: {
            pk_error_type3(lisp, atom, lisp->cache.type_string, lisp->cache.type_symbol, lisp->cache.type_keyword);
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
            pk_error_type3(lisp, atom, lisp->cache.type_string, lisp->cache.type_symbol, lisp->cache.type_keyword);
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
            pk_error_type3(lisp, atom, lisp->cache.type_string, lisp->cache.type_symbol, lisp->cache.type_keyword);
            return (PKAtomKeyword *)lisp->cache.nil;
        }
    }
}
