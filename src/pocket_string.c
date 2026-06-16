#include "pocket_internals.h"

PKAtomString *pk_atom_string(Pocket lisp, PKString string) {
    PKString dupe = pk_string_dupe(lisp, string);
    return pk_atom_string_nomemcpy(lisp, dupe);
}

PKAtomString *pk_atom_string_nomemcpy(Pocket lisp, PKString string) {
    PKAtomString *atom = (PKAtomString *)pk_atom_alloc(lisp);
    
    size_t hash = pk_hash_djb2(string.c, string.length);
    
    *atom = (PKAtomString) {
        .tag.ty = PKAtomTy_String,
        .tag.marked = false,
        .lit = string,
        .hash = hash,
    };
    return atom;
}

PKAtomString *pk_atom_cast_string(Pocket lisp, PKAtom *atom) {
    if (atom->tag.ty != PKAtomTy_String) pk_error(lisp);
    return (PKAtomString *)atom;
}

bool pk_atom_string_eq(Pocket lisp, PKAtomString *lhs, PKAtomString *rhs) {
    (void)lisp;
    return pk_string_eq(lhs->lit, rhs->lit);
}
