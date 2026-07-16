#include "pocket_internals.h"

PKRes pk_atom_string(Pocket lisp, PKString string, PKAtomString **output) {
    if (string.length == 0) {
        *output = lisp->cache.empty_string;
        return PK_Ok;
    }
    PKString dupe;
    pk_try(pk_string_dupe(lisp, string, &dupe));
    pk_try(pk_atom_string_nomemcpy(lisp, dupe, output));
    return PK_Ok;
}

PKRes pk_atom_string_nomemcpy(Pocket lisp, PKString string, PKAtomString **output) {
    PKAtom *a;
    pk_try(pk_atom_alloc(lisp, &a));
    PKAtomString *atom = (PKAtomString *)a;

    size_t hash = pk_hash_djb2(string.c, string.length);

    *atom = (PKAtomString) {
        .tag.ty = PKAtomTy_String,
        .tag.marked = false,
        .lit = string,
        .hash = hash,
    };
    *output = atom;
    return PK_Ok;
}

PKRes pk_atom_string_concat(Pocket lisp, PKAtoms strings, PKAtomString **output) {
    size_t length = 0;
    for (size_t i = 0; i < strings.length; ++i) {
        PKAtomString *string = NULL;
        pk_try(pk_atom_cast_string(lisp, strings.e[i], &string));
        length += string->lit.length;
    }
    if (length == 0) {
        *output = lisp->cache.empty_string;
        return PK_Ok;
    }
    
    char *buffer = NULL;
    pk_try(pk_malloc(lisp, length * sizeof(char), (void **)&buffer));

    size_t acc = 0;
    for (size_t i = 0; i < strings.length; ++i) {
        size_t index = pk_index_inv(i, strings.length);
        PKAtomString *string = (PKAtomString *)strings.e[index];
        (void)memcpy(buffer + acc, string->lit.c, string->lit.length);
        acc += string->lit.length;
    }
    
    PKRes result = PK_Yield;
    
    pk_defer(pk_atom_string_nomemcpy(lisp, pk_string_new(buffer, length), output));
    
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

bool pk_atom_string_eq(Pocket lisp, PKAtomString *lhs, PKAtomString *rhs) {
    (void)lisp;
    return pk_string_eq(lhs->lit, rhs->lit);
}

