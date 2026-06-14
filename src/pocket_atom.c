#include "pocket_internals.h"

PKAtomNumber *pk_make_atom_int(Pocket lisp, int value) {
    PKAtomNumber *atom = (PKAtomNumber *)pk_atom_alloc(lisp);
    *atom = (PKAtomNumber) {
        .tag.ty = PKAtomTy_Number,
        .tag.marked = false,
        .ty = PKNumber_Int,
        .as.i = value,
    };
    return atom;
}

PKAtomNumber *pk_make_atom_float(Pocket lisp, float value) {
    PKAtomNumber *atom = (PKAtomNumber *)pk_atom_alloc(lisp);
    *atom = (PKAtomNumber) {
        .tag.ty = PKAtomTy_Number,
        .tag.marked = false,
        .ty = PKNumber_Float,
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
