#include "pocket_internals.h"

PKAtom pk_ref_inc(PKAtom atom) {
    if ((atom->header.flags & PK_ATOMFLAG_IMMORTAL) != 0) {
        return pk_true();
    }
    if (atom->header.refcount == UINT8_MAX) {
        // Critical errors are immortal and never allocate, so reporting the
        // overflow cannot recurse back into pk_ref_inc.
        return pk_error_refcount_overflow();
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
        // Releasing a reference nobody owns means something else is still
        // pointing at this atom without a reference of its own.
        // TODO: This is kind of a critical error, but it really should not happen
        // We need to implement a debug runtime that can panic or log whenever this happens
        // and then on release just ignore.
        PKStr ident = pk_ident_atomty(atom->header.type);
        fprintf(
            stderr,
            "pk_ref_dec: refcount underflow at <%p> [%.*s]\n",
            atom,
            (int)ident.length,
            ident.c
        );
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

