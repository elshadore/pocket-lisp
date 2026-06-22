#include "pocket_internals.h"

PKAtom *pk_atom_nil(Pocket lisp) {
    return lisp->cache.nil;
}

PKAtom *pk_atom_nil_new(Pocket lisp) {
    PKAtom *atom = pk_atom_alloc(lisp);
    atom->tag = (PKAtomTag){ .ty = PKAtomTy_Nil, .marked = false };
    return atom;
}

PKAtom *pk_atom_t(Pocket lisp) {
    return (PKAtom *)lisp->cache.t;
}

PKAtomCFunc *pk_atom_cfunc(Pocket lisp, void *user_closure, PKFn fn, PKFuncArity arity) {
    PKAtomCFunc *atom = (PKAtomCFunc *)pk_atom_alloc(lisp);
    *atom = (PKAtomCFunc) {
        .tag.ty = PKAtomTy_CFunc,
        .tag.marked = false,
        .user_closure = user_closure,
        .fn = fn,
        .arity = arity,
    };
    return atom;
}

PKAtomCFunc *pk_atom_cast_cfunc(Pocket lisp, PKAtom *atom) {
    if (atom->tag.ty != PKAtomTy_CFunc) pk_error(lisp);
    return (PKAtomCFunc *)atom;
}

bool pk_atom_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    if (lhs == rhs) return true;
    if (lhs->tag.ty != rhs->tag.ty) return false;
    switch (lhs->tag.ty) {
        case PKAtomTy_Number: {
            return pk_number_eq(lisp, (PKAtomNumber *)lhs, (PKAtomNumber *)rhs);
        }
        case PKAtomTy_String: {
            return pk_atom_string_eq(lisp, (PKAtomString *)lhs, (PKAtomString *)rhs);
        }
        default: return false;
    }
}

bool pk_atom_is_nil(PKAtom *atom) {
    return atom->tag.ty == PKAtomTy_Nil;
}

bool pk_atom_is_true(PKAtom *atom) {
    return !pk_atom_is_nil(atom);
}

