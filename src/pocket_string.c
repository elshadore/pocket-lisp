#include "pocket_internals.h"

PKRes pk_atom_string(Pocket lisp, const char *cstr, PKAtomString **output) {
    size_t length = 0;
    length = strlen(cstr);
    return pk_atom_stringn(lisp, cstr, length, output);
}

PKRes pk_atom_stringn(Pocket lisp, const char *string, size_t length, PKAtomString **output) {
    char *dupe = NULL;
    if (length == 0) {
        *output = lisp->cache.empty_string;
        return PK_Ok;
    }
    pk_try(pk_string_dupe(lisp, string, length, &dupe));
    pk_try(pk_atom_stringn_nomemcpy(lisp, dupe, length, output));
    return PK_Ok;
}
    
PKRes pk_atom_stringn_nomemcpy(Pocket lisp, char *c, size_t length, PKAtomString **output) {
    PKAtom *a = NULL;
    size_t hash = 0;
    
    pk_try(pk_atom_alloc(lisp, &a));

    hash = pk_hash_djb2(c, length);

    a->tag.ty = PKAtomTy_String;
    a->string.c = c;
    a->string.length = length;
    a->string.hash = hash;

    *output = (PKAtomString *)a;
    return PK_Ok;
}

PKRes pk_atom_string_concat(Pocket lisp, PKAtoms strings, PKAtomString **output) {
    size_t length = 0;
    size_t i = 0;
    size_t acc = 0;
    char *buffer = NULL;
    PKRes result = PK_Yield;
    
    for (i = 0; i < strings.length; ++i) {
        PKAtomString *string = NULL;
        pk_try(pk_atom_cast_string(lisp, strings.e[i], &string));
        length += string->length;
    }
    if (length == 0) {
        *output = lisp->cache.empty_string;
        return PK_Ok;
    }
    
    pk_try(pk_malloc(lisp, length * sizeof(char), (void **)&buffer));

    for (i = 0; i < strings.length; ++i) {
        size_t index = pk_index_inv(i, strings.length);
        PKAtomString *string = (PKAtomString *)strings.e[index];
        (void)memcpy(buffer + acc, string->c, string->length);
        acc += string->length;
    }
    
    
    pk_defer(pk_atom_stringn_nomemcpy(lisp, buffer, acc, output));
    
    result = PK_Ok;
    
    DEFER:
    if (result == PK_Yield) {
        pk_free(lisp, buffer, length * sizeof(char));
    }
    return result;
}

PKRes pk_atom_cast_string(Pocket lisp, PKAtom *atom, PKAtomString **output) {
    if (atom->tag.ty != PKAtomTy_String) return pk_error(lisp);
    *output = (PKAtomString *)atom;
    return PK_Ok;
}

pk_bool pk_atom_string_eq(Pocket lisp, PKAtomString *lhs, PKAtomString *rhs) {
    (void)lisp;
    return pk_string_eq(lhs->c, lhs->length, rhs->c, rhs->length);
}

PKRes pk_atom_string_slurp(Pocket lisp, PKAtomString *file_path, PKAtomString **output) {
    char *buffer = NULL;
    size_t length = 0;

    pk_try(pk_util_slurpn(lisp, file_path->c, file_path->length, &buffer, &length));
    
    if (!pk_atom_stringn_nomemcpy(lisp, buffer, length, output)) {
        pk_string_free(lisp, buffer, length);
        return PK_Yield;
    }
    
    return PK_Ok;
}
