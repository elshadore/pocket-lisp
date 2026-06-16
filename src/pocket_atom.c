#include "pocket_internals.h"

PKAtomCons *pk_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr) {
    PKAtomCons *atom = (PKAtomCons *)pk_atom_alloc(lisp);
    *atom = (PKAtomCons) {
        .tag.ty = PKAtomTy_Cons,
        .tag.marked = false,
        .car = car,
        .cdr = cdr,
    };
    return atom;
}

PKAtom *pk_atom_nil(Pocket lisp) {
    PKAtom *atom = pk_atom_alloc(lisp);
    atom->tag = (PKAtomTag){ .ty = PKAtomTy_Nil, .marked = false };
    return atom;
}

PKAtomCons *pk_atom_cast_cons(Pocket lisp, PKAtom *atom) {
    if (atom->tag.ty != PKAtomTy_Cons) pk_error(lisp);
    return (PKAtomCons *)atom;
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
