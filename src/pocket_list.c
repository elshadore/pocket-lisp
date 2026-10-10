#include "pocket_internals.h"

PKAtomData PK_ATOM_NIL = (PKAtomData) {
    .header = (PKAtomHeader) {
        .type = PK_ATOMTY_NIL,
        .flags = PK_ATOMFLAG_IMMORTAL,
        .refcount = 0,
    }
};

PKAtom pk_nil() {
    return &PK_ATOM_NIL;
}

PKAtom pk_cons(PKAtom car, PKAtom cdr) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    
    atom->header.type = PK_ATOMTY_CONS;
    atom->cons.car = car;
    atom->cons.cdr = cdr;
    
    return atom;
}

PKAtom pk_car(PKAtom cons) {
    pk_propagate(pk_typecheck_cons(cons));
    return cons->cons.car;
}

PKAtom pk_cdr(PKAtom cons) {
    pk_propagate(pk_typecheck_cons(cons));
    return cons->cons.cdr;
}

PKAtom pk_set_car(PKAtom cons, PKAtom new_car) {
    pk_propagate(pk_typecheck_cons(cons));
    PKAtom result = cons->cons.car;
    cons->cons.car = new_car;
    return result;
}

PKAtom pk_set_cdr(PKAtom cons, PKAtom new_cdr) {
    pk_propagate(pk_typecheck_cons(cons));
    PKAtom result = cons->cons.cdr;
    cons->cons.cdr = new_cdr;
    return result;
}

PKAtom pk_list2(PKAtom first, PKAtom second) {
    PKAtom b = pk_cons(second, pk_nil());
    pk_propagate(b);
    PKAtom a = pk_cons(first, b);
    pk_propagate(a);
    return a;
}

PKAtom pk_list3(PKAtom first, PKAtom second, PKAtom third) {
    PKAtom c = pk_cons(third, pk_nil());
    pk_propagate(c);
    PKAtom b = pk_cons(second, c);
    pk_propagate(b);
    PKAtom a = pk_cons(first, b);
    pk_propagate(a);
    return a;
}

PKAtom pk_list(PKAtoms atoms) {
    if (atoms.length == 0) {
        return pk_nil();
    }
    
    PKAccum acc = pk_accum_new();
    for (size_t i = 0; i < atoms.length; ++i) {
        pk_propagate(pk_accum_push(&acc, atoms.e[i]));
    }
    return acc.head;
}

PKAtom pk_list_packed(PKAtoms atoms) {
    if (atoms.length == 0) {
        return pk_nil();
    }
    
    PKAccum acc = pk_accum_new();
    for (size_t i = 0; i < (atoms.length - 1); ++i) {
        pk_propagate(pk_accum_push(&acc, atoms.e[i]));
    }
    acc.tail->cons.cdr = atoms.e[atoms.length - 1];
    return acc.head;
}

PKAtom pk_list_flatten(PKAtoms lists) {
    if (lists.length == 0) {
        return pk_nil();
    }
    
    PKAccum acc = pk_accum_new();
    for (size_t i = 0; i < lists.length; ++i) {
        PKAtom list = lists.e[i];
        if (pk_is_nil(list)) {
            pk_propagate(pk_accum_push(&acc, pk_nil()));
            continue;
        }
        if (!pk_is_cons(list)) {
            return pk_error();
        }
        PKAtom iter = list;
        while (pk_is_cons(iter)) {
            pk_propagate(pk_accum_push(&acc, iter->cons.car));
            iter = iter->cons.car;
        }
    }
    return acc.head;
}

bool pk_is_nil(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_NIL;
}

bool pk_is_cons(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_CONS;
}

PKAtom pk_typecheck_nil(PKAtom atom) {
    if (pk_is_nil(atom)) {
        return pk_true();
    } else {
        return pk_error();
    }
}

PKAtom pk_typecheck_cons(PKAtom atom) {
    if (pk_is_cons(atom)) {
        return pk_true();
    } else {
        return pk_error();
    }
}
