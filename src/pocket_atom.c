#include "pocket_internals.h"

PKAtomNumber *pk_make_atom_int(Pocket lisp, int value) {
    PKAtomNumber *atom = (PKAtomNumber *)pk_atom_alloc(lisp);
    *atom = (PKAtomNumber) {
        .tag.ty = PKAtomTy_Number,
        .tag.marked = false,
        .ty = PKNumberTy_Int,
        .as.i = value,
    };
    return atom;
}

PKAtomNumber *pk_make_atom_float(Pocket lisp, float value) {
    PKAtomNumber *atom = (PKAtomNumber *)pk_atom_alloc(lisp);
    *atom = (PKAtomNumber) {
        .tag.ty = PKAtomTy_Number,
        .tag.marked = false,
        .ty = PKNumberTy_Float,
        .as.f = value,
    };
    return atom;
}

PKAtomString *pk_make_atom_string(Pocket lisp, PKString string) {
    PKAtomString *atom = (PKAtomString *)pk_atom_alloc(lisp);
    *atom = (PKAtomString) {
        .tag.ty = PKAtomTy_String,
        .tag.marked = false,
        .lit = pk_string_dupe(lisp, string),
    };
    return atom;
}

PKAtomSymbol *pk_make_atom_symbol(Pocket lisp, PKString id) {
    PKAtomSymbol *atom = (PKAtomSymbol *)pk_atom_alloc(lisp);
    *atom = (PKAtomSymbol) {
        .tag.ty = PKAtomTy_Symbol,
        .tag.marked = false,
        .id = pk_string_dupe(lisp, id),
    };
    return atom;
}

PKAtomCons *pk_make_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr) {
    PKAtomCons *atom = (PKAtomCons *)pk_atom_alloc(lisp);
    *atom = (PKAtomCons) {
        .tag.ty = PKAtomTy_Cons,
        .tag.marked = false,
        .car = car,
        .cdr = cdr,
    };
    return atom;
}

PKAtom *pk_make_atom_nil(Pocket lisp) {
    PKAtom *atom = pk_atom_alloc(lisp);
    atom->tag = (PKAtomTag){ .ty = PKAtomTy_Nil, .marked = false };
    return atom;
}

PKAtomNumber *pk_atom_cast_number(Pocket lisp, PKAtom *atom) {
    if (atom->tag.ty != PKAtomTy_Number) pk_error(lisp);
    return (PKAtomNumber *)atom;
}

PKAtomString *pk_atom_cast_string(Pocket lisp, PKAtom *atom) {
    if (atom->tag.ty != PKAtomTy_String) pk_error(lisp);
    return (PKAtomString *)atom;
}

PKAtomSymbol *pk_atom_cast_symbol(Pocket lisp, PKAtom *atom) {
    if (atom->tag.ty != PKAtomTy_Symbol) pk_error(lisp);
    return (PKAtomSymbol *)atom;
}

PKAtomCons *pk_atom_cast_cons(Pocket lisp, PKAtom *atom) {
    if (atom->tag.ty != PKAtomTy_Cons) pk_error(lisp);
    return (PKAtomCons *)atom;
}
