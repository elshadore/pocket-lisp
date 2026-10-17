#include "pocket_internals.h"

PKAtomCons *pk_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr) {
    PKAtomCons *result = (PKAtomCons *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_CONS;
    result->car = car;
    result->cdr = cdr;
    
    return result;
}

PKAtom *pk_atom_car(Pocket lisp, PKAtom *atom) {
    PKAtomCons *cons = pk_atom_assert_cons(lisp, atom);
    return cons->car;
}

PKAtom *pk_atom_cdr(Pocket lisp, PKAtom *atom) {
    PKAtomCons *cons = pk_atom_assert_cons(lisp, atom);
    return cons->cdr;
}

PKAtom *pk_atom_set_car(Pocket lisp, PKAtom *atom, PKAtom *new_car) {
    PKAtomCons *cons = pk_atom_assert_cons(lisp, atom);
    PKAtom *result = cons->car;
    cons->car = new_car;
    return result;
}

PKAtom *pk_atom_set_cdr(Pocket lisp, PKAtom *atom, PKAtom *new_cdr) {
    PKAtomCons *cons = pk_atom_assert_cons(lisp, atom);
    PKAtom *result = cons->cdr;
    cons->cdr = new_cdr;
    return result;
}

PKAtomCons *pk_atom_list1(Pocket lisp, PKAtom *first) {
    return pk_atom_cons(lisp, first, lisp->cache.nil);
}
    
PKAtomCons *pk_atom_list2(Pocket lisp, PKAtom *first, PKAtom *second) {
    PKAtomCons *b = pk_atom_cons(lisp, second, lisp->cache.nil);
    PKAtomCons *a = pk_atom_cons(lisp, first, (PKAtom *)b);
    return a;
}

PKAtomCons *pk_atom_list3(Pocket lisp, PKAtom *first, PKAtom *second, PKAtom *third) {
    PKAtomCons *c = pk_atom_cons(lisp, third, lisp->cache.nil);
    PKAtomCons *b = pk_atom_cons(lisp, second, (PKAtom *)c);
    PKAtomCons *a = pk_atom_cons(lisp, first, (PKAtom *)b);
    return a;
}

PKAtom *pk_atom_list(Pocket lisp, PKAtoms atoms) {
    if (atoms.length == 0) {
        return lisp->cache.nil;
    }
    
    PKAccum acc = pk_accum_new(lisp);
    for (size_t i = 0; i < atoms.length; ++i) {
        pk_accum_push(&acc, atoms.e[i]);
    }
    
    return acc.head;
}

PKAtom *pk_atom_list_packed(Pocket lisp, PKAtoms atoms) {
    if (atoms.length == 0) {
        return lisp->cache.nil;
    }
    
    PKAccum acc = pk_accum_new(lisp);
    for (size_t i = 0; i < (atoms.length - 1); ++i) {
        pk_accum_push(&acc, atoms.e[i]);
    }
    if (atoms.length > 1) {
        acc.tail->cons.cdr = atoms.e[atoms.length - 1];
    }
    return acc.head;
}

PKAtom *pk_atom_list_flatten(Pocket lisp, PKAtoms lists) {
    if (lists.length == 0) {
        return lisp->cache.nil;
    }
    
    PKAccum acc = pk_accum_new(lisp);
    for (size_t i = 0; i < lists.length; ++i) {
        PKAtom *atom = lists.e[i];
        switch (atom->header.type) {
            case PK_ATOMTY_NIL: {
                continue;
            }
            case PK_ATOMTY_CONS: {
                PKAtom *iter = atom;
                while (pk_atom_is_cons(iter)) {
                    pk_accum_push(&acc, iter->cons.car);
                    iter = iter->cons.cdr;
                }
                break;
            }
            default: {
                pk_error(lisp);
                break;
            }
        }
    }
    return acc.head;
}

bool pk_atom_is_nil(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_NIL;
}

bool pk_atom_is_cons(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_CONS;
}

PKAtom *pk_atom_assert_nil(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_nil(atom)) {
        return atom;
    } else {
        pk_error(lisp);
        return atom;
    }
}

PKAtomCons *pk_atom_assert_cons(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_cons(atom)) {
        return (PKAtomCons *)atom;
    } else {
        pk_error(lisp);
        return NULL;
    }
}
