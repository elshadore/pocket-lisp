#include "pocket_internals.h"

PKAtom pk_ref_inc(PKAtom atom) {
    if ((atom->header.flags & PK_ATOMFLAG_IMMORTAL) != 0) {
        return pk_true();
    }
    if (atom->header.refcount == UINT8_MAX) {
        // Building an error here would call pk_ref_inc again, so report the
        // shared out of memory throw instead.
        // TODO: we should probably make refcount overflow a IMMORTAL compiletime atom
        return pk_error_oom();
    }
    atom->header.refcount += 1;
    return pk_true();
}

PKAtom pk_ref_inc_many(PKAtoms atoms) {
    for (size_t i = 0; i < atoms.length; ++i) {
        PKAtom atom = pk_ref_inc(atoms.e[i]);
        if (pk_is_throwing(atom)) {
            for (size_t j = 0; j < i; ++j) {
                pk_ref_dec(atoms.e[j]);
            }
            return atom;
        }
    }
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
        pk_atom_free(atom);
    }
}

void pk_ref_dec_many(PKAtoms atoms) {
    for (size_t i = 0; i < atoms.length; ++i) {
        pk_ref_dec(atoms.e[i]);
    }
}

