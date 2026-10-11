#include "pocket_internals.h"

PKAtom pk_ref_inc(PKAtom atom) {
    if ((atom->header.flags & PK_ATOMFLAG_IMMORTAL) != 0) {
        return pk_true();
    }
    if (atom->header.refcount == UINT8_MAX) {
        return pk_error();
    }
    atom->header.refcount += 1;
    return pk_true();
}

void pk_ref_dec(PKAtom atom) {
    if ((atom->header.flags & PK_ATOMFLAG_IMMORTAL) != 0) {
        return;
    }
    if (atom->header.refcount == 0) {
        return;
    }
    atom->header.refcount -= 1;
    if (atom->header.refcount == 0) {
        // printf("TODO: free atoms on ref dec");
    }
}

PKAtom pk_ref_inc_many(PKAtoms atoms) {
    for (size_t i = 0; i < atoms.length; ++i) {
        PKAtom atom = pk_ref_inc(atoms.e[i]);
        if (pk_is_error(atom)) {
            for (size_t j = 0; j < i; ++j) {
                pk_ref_dec(atoms.e[j]);
            }
            return atom;
        }
    }
    return pk_true();
}
