#include "pocket_internals.h"

typedef struct PKAtomStringCatData_ {
    PKWriter w;
    PKAtoms atoms;
} PKAtomStringCatData;

PKAtomString *pk_atom_string_new_empty(Pocket lisp) {
    PKAtomString *result = (PKAtomString *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_STRING;
    result->str = PK_STR_EMPTY;
    
    return result;
}

PKAtomString *pk_atom_string_nomemcpy(Pocket lisp, PKStr string) {
    if (string.length == 0) {
        return lisp->cache.empty_string;
    }

    PKAtomString *result = (PKAtomString *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_STRING;
    result->str = string;
    
    return result;
}

PKAtomString *pk_atom_string(Pocket lisp, PKStr string) {
    if (string.length == 0) {
        return lisp->cache.empty_string;
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

PKAtomChar *pk_atom_char(Pocket lisp, char c) {
    PKAtomChar *result = (PKAtomChar *)pk_atom_alloc(lisp);

    result->header.type = PK_ATOMTY_CHARACTER;
    result->c = c;

    return result;
}

char pk_atom_str_to_char(Pocket lisp, PKStr string) {
    if (string.length == 0) {
        pk_errors(lisp, pk_str("string is empty"));
    }
    if (string.length > 1) {
        pk_errors(lisp, pk_str("string greater than a single character"));
    }
    return string.c[0];
}

PKAtomChar *pk_atom_string_to_char(Pocket lisp, PKAtomString *string) {
    return pk_atom_char(lisp, pk_atom_str_to_char(lisp, string->str));
}

pk_u8 pk_atom_character_to_byte(Pocket lisp, PKAtomChar *character) {
    (void)lisp;
    return (pk_u8)character->c;
}

PKAtom *pk_atom_string_to_chars(Pocket lisp, PKAtomString *string) {
    return pk_atom_to_list_chars(lisp, string->str);
}

PKAtomBlob *pk_atom_string_to_bytes(Pocket lisp, PKAtomString *string) {
    PKBlob blob = {(pk_u8 *)string->str.c, string->str.length};
    return pk_atom_blob(lisp, blob);
}

PKAtomString *pk_atom_string_alloc(Pocket lisp, size_t length) {
    if (length == 0) {
        return lisp->cache.empty_string;
    }

    PKStr buffer = pk_str_alloc(lisp, length);
    
    return pk_atom_string_nomemcpy(lisp, buffer);
}

PKAtomString *pk_atom_stringc(Pocket lisp, char *cstring) {
    PKStr string = pk_str_from_cstr(cstring);
    return pk_atom_string(lisp, string);
}

void pk_atom_string_cat_body(void *user_closure, Pocket lisp) {
    PKAtomStringCatData *data = (PKAtomStringCatData *)user_closure;
    for (size_t i = 0; i < data->atoms.length; ++i) {
        PKAtom *atom = data->atoms.e[i];
        pk_writer_string(lisp, &data->w, pk_atom_as_string(lisp, atom));
    }
    pk_stack_push(lisp, (PKAtom *)pk_atom_string(lisp, pk_writer_slice(&data->w)));
}

PKAtomString *pk_atom_string_cat(Pocket lisp, PKAtoms atoms) {
    PKAtomStringCatData data = (PKAtomStringCatData) {
        .w = PK_WRITER_EMPTY,
        .atoms = atoms,
    };

    PK_RES result = pk_protect(lisp, &data, pk_atom_string_cat_body, 0);

    pk_writer_deinit(lisp, &data.w);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }

    return (PKAtomString *)pk_stack_head_pop(lisp);
}

PKAtomString *pk_atom_string_reverse(Pocket lisp, PKStr input) {
    PKStr dupe = pk_str_dupe(lisp, input);
    pk_str_reverse_mut(dupe);
    return pk_atom_string_nomemcpy(lisp, input);
}

char pk_atom_string_el(Pocket lisp, PKAtomString *string, size_t index) {
    if (index >= string->str.length) {
        pk_errorf(lisp, "index: %zu, longer than the string length: %zu", index, string->str.length);
        return '\0';
    }
    return string->str.c[index];
}

PKStr pk_atom_as_string(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            return atom->string.str;
        }
        case PK_ATOMTY_CHARACTER: {
            return (PKStr) {
                .c = &atom->character.c,
                .length = 1,
            };
        }
        default: {
            pk_error_type2(lisp, atom, lisp->cache.type_string, lisp->cache.type_character);
            return PK_STR_EMPTY;
        }
    }
}

size_t pk_atom_string_length(Pocket lisp, PKAtomString *string) {
    (void)lisp;
    return string->str.length;
}

bool pk_atom_is_string(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_STRING;
}

bool pk_atom_is_character(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_CHARACTER;
}

PKAtomString *pk_atom_assert_string(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_string(atom)) {
        return (PKAtomString *)atom;
    } else {
        pk_error_type(lisp, atom, lisp->cache.type_string);
        return lisp->cache.empty_string;
    }
}

PKAtomChar *pk_atom_assert_character(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_character(atom)) {
        return (PKAtomChar *)atom;
    } else {
        pk_error_type(lisp, atom, lisp->cache.type_character);
        return (PKAtomChar *)lisp->cache.nil;
    }
}

bool pk_atom_string_cmp(PKAtomString *string, PKStr comp) {
    return pk_str_eq(string->str, comp);
}
