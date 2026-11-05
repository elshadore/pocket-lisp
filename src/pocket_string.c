#include "pocket_internals.h"

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

PKAtomChar *pk_atom_char(Pocket lisp, char c) {
    PKAtomChar *result = (PKAtomChar *)pk_atom_alloc(lisp);

    result->header.type = PK_ATOMTY_CHARACTER;
    result->c = c;

    return result;
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

PKAtomString *pk_atom_string_cat_(PKWriter *w, Pocket lisp, PKAtoms atoms) {
    for (size_t i = 0; i < atoms.length; ++i) {
        PKAtom *atom = atoms.e[i];
        if (pk_atom_is_string(atom)) {
            pk_writer_string(lisp, w, atom->string.str);
        } else if (pk_atom_is_character(atom)) {
            pk_writer_char(lisp, w, atom->character.c);
        } else if (pk_atom_is_string_array(atom)) {
            pk_writer_string(lisp, w, pk_writer_slice(&atom->string_array.string));
        } else {
            pk_writer_atom(lisp, w, atom);
        }
    }
    return pk_atom_string(lisp, pk_writer_slice(w));
}
    
typedef struct PKAtomStringCatData_ {
    PKWriter w;
    PKAtoms atoms;
} PKAtomStringCatData;

void pk_atom_string_cat_body(void *user_closure, Pocket lisp) {
    PKAtomStringCatData *data = (PKAtomStringCatData *)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_atom_string_cat_(&data->w, lisp, data->atoms));
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

    PKAtom *out = pk_stack_head(lisp);
    pk_pop(lisp);
    return (PKAtomString *)out;
}

char pk_atom_string_el(Pocket lisp, PKAtomString *string, size_t index) {
    if (index >= string->str.length) {
        pk_errorf(lisp, "index: %zu, longer than the string length: %zu", index, string->str.length);
        return '\0';
    }
    return string->str.length[index];
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
