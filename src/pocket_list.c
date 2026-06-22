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

PKAtomCons *pk_atom_cons_car(Pocket lisp, PKAtom *car) {
    PKAtomCons *atom = (PKAtomCons *)pk_atom_alloc(lisp);
    *atom = (PKAtomCons) {
        .tag.ty = PKAtomTy_Cons,
        .tag.marked = false,
        .car = car,
        .cdr = pk_atom_nil(lisp),
    };
    return atom;
}

PKAtomCons *pk_atom_cons_tail(Pocket lisp, PKAtomCons *cons) {
    PKAtomCons *curr = cons;
    while (curr->cdr != pk_atom_nil(lisp)) {
        curr = pk_atom_cast_cons(lisp, curr->cdr);
    }
    return curr;
}

PKAtomCons *pk_atom_cast_cons(Pocket lisp, PKAtom *atom) {
    if (atom->tag.ty != PKAtomTy_Cons) pk_error(lisp);
    return (PKAtomCons *)atom;
}

PKAtom *pk_slice_list(Pocket lisp, PKAtoms atoms) {
    if (atoms.length == 0) {
        return pk_atom_nil(lisp);
    }
    PKAtomCons *head = pk_atom_cons(lisp, atoms.e[0], pk_atom_nil(lisp));
    PKAtomCons *tail = head;
    for (size_t i = 1; i < atoms.length; ++i) {
        PKAtomCons *next = pk_atom_cons(lisp, atoms.e[i], pk_atom_nil(lisp));
        tail->cdr = (PKAtom *)next;
        tail = next;
    }
    return (PKAtom *)head;
}

PKAtom *pk_slice_list_rev(Pocket lisp, PKAtoms atoms) {
    if (atoms.length == 0) {
        return pk_atom_nil(lisp);
    }
    size_t last = atoms.length - 1;
    PKAtomCons *head = pk_atom_cons(lisp, atoms.e[last], pk_atom_nil(lisp));
    PKAtomCons *tail = head;
    for (size_t i = 0; i < last; ++i) {
        size_t index = pk_index_inv(i, last);
        PKAtomCons *next = pk_atom_cons(lisp, atoms.e[index], pk_atom_nil(lisp));
        tail->cdr = (PKAtom *)next;
        tail = next;
    }
    return (PKAtom *)head;
}

bool pk_atom_is_cons(PKAtom *atom) {
    return atom->tag.ty == PKAtomTy_Cons;
}
