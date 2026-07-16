#include "pocket_internals.h"

PKRes pk_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr, PKAtomCons **output) {
    PKAtom *a;
    pk_try(pk_atom_alloc(lisp, &a));
    PKAtomCons *atom = (PKAtomCons *)a;
    *atom = (PKAtomCons) {
        .tag.ty = PKAtomTy_Cons,
        .tag.marked = false,
        .car = car,
        .cdr = cdr,
    };
    *output = atom;
    return PK_Ok;
}

PKRes pk_atom_cons_car(Pocket lisp, PKAtom *car, PKAtomCons **output) {
    PKAtom *a;
    pk_try(pk_atom_alloc(lisp, &a));
    PKAtomCons *atom = (PKAtomCons *)a;
    *atom = (PKAtomCons) {
        .tag.ty = PKAtomTy_Cons,
        .tag.marked = false,
        .car = car,
        .cdr = pk_atom_nil(lisp),
    };
    *output = atom;
    return PK_Ok;
}

PKRes pk_atom_cons_tail(Pocket lisp, PKAtomCons *cons, PKAtomCons **output) {
    PKAtomCons *curr = cons;
    while (curr->cdr != pk_atom_nil(lisp)) {
        PKAtomCons *next;
        pk_try(pk_atom_cast_cons(lisp, curr->cdr, &next));
        curr = next;
    }
    *output = curr;
    return PK_Ok;
}

PKRes pk_atom_list2(Pocket lisp, PKAtom *first, PKAtom *second, PKAtomCons **output) {
    PKAtomCons *a = NULL;
    PKAtomCons *b = NULL;
    pk_try(pk_atom_cons(lisp, second, pk_atom_nil(lisp), &b));
    pk_try(pk_atom_cons(lisp, first, (PKAtom *)b, &a));
    *output = a;
    return PK_Ok;
}

PKRes pk_atom_cast_cons(Pocket lisp, PKAtom *atom, PKAtomCons **output) {
    if (atom->tag.ty != PKAtomTy_Cons) return pk_error(lisp);
    *output = (PKAtomCons *)atom;
    return PK_Ok;
}

PKRes pk_slice_list(Pocket lisp, PKAtoms atoms, PKAtom **output) {
    if (atoms.length == 0) {
        *output = pk_atom_nil(lisp);
        return PK_Ok;
    }
    PKAtomCons *head = NULL;
    pk_try(pk_atom_cons(lisp, atoms.e[0], pk_atom_nil(lisp), &head));
    PKAtomCons *tail = head;
    for (size_t i = 1; i < atoms.length; ++i) {
        PKAtomCons *next;
        pk_try(pk_atom_cons(lisp, atoms.e[i], pk_atom_nil(lisp), &next));
        tail->cdr = (PKAtom *)next;
        tail = next;
    }
    *output = (PKAtom *)head;
    return PK_Ok;
}

PKRes pk_slice_list_rev(Pocket lisp, PKAtoms atoms, PKAtom **output) {
    if (atoms.length == 0) {
        *output = pk_atom_nil(lisp);
        return PK_Ok;
    }
    size_t last = atoms.length - 1;
    PKAtomCons *head = NULL;
    pk_try(pk_atom_cons(lisp, atoms.e[last], pk_atom_nil(lisp), &head));
    PKAtomCons *tail = head;
    for (size_t i = 0; i < last; ++i) {
        size_t index = pk_index_inv(i, last);
        PKAtomCons *next;
        pk_try(pk_atom_cons(lisp, atoms.e[index], pk_atom_nil(lisp), &next));
        tail->cdr = (PKAtom *)next;
        tail = next;
    }
    *output = (PKAtom *)head;
    return PK_Ok;
}

PKRes pk_slice_list_tailed(Pocket lisp, PKAtoms atoms, PKAtom **output) {
    if (atoms.length == 0) {
        *output = pk_atom_nil(lisp);
        return PK_Ok;
    }
    PKAtomCons *head = NULL;
    pk_try(pk_atom_cons(lisp, atoms.e[0], pk_atom_nil(lisp), &head));
    PKAtomCons *tail = head;
    for (size_t i = 1; i < (atoms.length - 1); ++i) {
        PKAtomCons *next;
        pk_try(pk_atom_cons(lisp, atoms.e[i], pk_atom_nil(lisp), &next));
        tail->cdr = (PKAtom *)next;
        tail = next;
    }
    tail->cdr = atoms.e[atoms.length - 1];
    *output = (PKAtom *)head;
    return PK_Ok;
}
