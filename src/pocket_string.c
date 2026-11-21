#include "pocket_internals.h"

PKAtomString *pk_atom_string_nomemcpy(Pocket lisp, PKStr string) {
    if (string.length == 0) {
        return &PK_ATOM_EMPTY_STRING;
    }

    PKAtomString *result = (PKAtomString *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_STRING;
    result->str = string;
    
    return result;
}

PKAtomString *pk_atom_string(Pocket lisp, PKStr string) {
    if (string.length == 0) {
        return &PK_ATOM_EMPTY_STRING;
    }

    PKStr dupe = pk_str_dupe(lisp, string);
    
    return pk_atom_string_nomemcpy(lisp, dupe);
}

PKAtomString *pk_atom_string_char(Pocket lisp, char c) {
    PKStr str = (PKStr) {
        .c = &c,
        .length = 1,
    };
    return pk_atom_string(lisp, str);
}

PKAtomSymbol *pk_atom_string_to_symbol(Pocket lisp, PKAtomString *string) {
    return pk_atom_symbol(lisp, string->str);
}

PKAtomKeyword *pk_atom_string_to_keyword(Pocket lisp, PKAtomString *string) {
    return pk_atom_keyword(lisp, string->str);
}

PKAtomString *pk_atom_string_alloc(Pocket lisp, size_t length) {
    if (length == 0) {
        return &PK_ATOM_EMPTY_STRING;
    }

    PKStr buffer = pk_str_alloc(lisp, length);
    
    return pk_atom_string_nomemcpy(lisp, buffer);
}

PKAtomString *pk_atom_stringc(Pocket lisp, char *cstring) {
    PKStr string = pk_str_from_cstr(cstring);
    return pk_atom_string(lisp, string);
}

PKAtomString *pk_atom_string_cat(Pocket lisp, PKAtoms atoms) {
    pk_writer_reset(lisp);
    
    for (size_t i = 0; i < atoms.length; ++i) {
        PKAtomString *string = pk_atom_assert_string(lisp, atoms.e[i]);
        pk_writer_string(lisp, string->str);
    }
    
    PKAtomString *result = pk_atom_string(lisp, pk_writer_str(lisp));
    pk_writer_reset(lisp);
    return result;
}

PKAtomString *pk_atom_string_reverse(Pocket lisp, PKStr input) {
    PKStr dupe = pk_str_dupe(lisp, input);
    pk_str_reverse_mut(dupe);
    return pk_atom_string_nomemcpy(lisp, dupe);
}

PKAtomString *pk_atom_string_el(Pocket lisp, PKAtomString *string, size_t index) {
    if (index >= string->str.length) {
        pk_errorf(lisp, "index: %zu, longer than the string length: %zu", index, string->str.length);
        return &PK_ATOM_EMPTY_STRING;
    }
    PKStr str = (PKStr) {
        .c = &string->str.c[index],
        .length = 1,
    };
    return pk_atom_string(lisp, str);
}

size_t pk_atom_string_length(Pocket lisp, PKAtomString *string) {
    (void)lisp;
    return string->str.length;
}

bool pk_atom_is_character(PKAtom *atom) {
    if (pk_atom_is_string(atom)) {
        return atom->string.str.length == 1;
    } else {
        return false;
    }
}

bool pk_atom_is_string(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_STRING;
}

PKAtomString *pk_atom_assert_string(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_string(atom)) {
        pk_error_type(lisp, atom, PK_TYPE_STRING);
    }
    return (PKAtomString *)atom;
}

bool pk_atom_string_cmp(PKAtomString *string, PKStr comp) {
    return pk_str_eq(string->str, comp);
}
