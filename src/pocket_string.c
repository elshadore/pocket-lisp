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
        } else {
            pk_writer_atom(lisp, w, atom);
        }
    }
    return pk_atom_string(lisp, pk_writer_slice(w));
}
    
PKAtomString *pk_atom_string_cat(Pocket lisp, PKAtoms atoms) {
    PKWriter w = PK_WRITER_EMPTY;
    PKAtomString *result = pk_atom_string_cat_(&w, lisp, atoms);
    pk_writer_deinit(lisp, &w);
    return result;
}

bool pk_atom_is_string(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_STRING;
}

PKAtomString *pk_atom_assert_string(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_string(atom)) {
        return (PKAtomString *)atom;
    } else {
        PKStr ty = pk_ident_atomty(atom->header.type);
        pk_errorf(lisp, "pk_atom_assert_string: expected a string, got %.*s", (int)ty.length, ty.c);
        return lisp->cache.empty_string;
    }
}

bool pk_atom_string_cmp(PKAtomString *string, PKStr comp) {
    return pk_str_eq(string->str, comp);
}
