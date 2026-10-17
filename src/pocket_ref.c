#include "pocket_internals.h"

PKAtom pk_ref_inc(PKAtom atom) {
    // We are removing reference counting so for now this is a noop
    (void)atom;
    return pk_true();
}

PKAtom pk_ref_inc_many(PKAtoms atoms) {
    (void)atoms;
    return pk_true();
}

void pk_ref_dec(PKAtom atom) {
    // We are removing reference counting so for now this is a noop
    if ((atom->header.flags & PK_ATOMFLAG_IMMORTAL) != 0) {
        return;
    }
    (void)atom;
}

void pk_ref_dec_many(PKAtoms atoms) {
    (void)atoms;
}

