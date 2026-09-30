#include "pocket_internals.h"

PKAtom pk_int(int integer) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    atom->number = (PKAtomNumber) {
        .header = (PKAtomHeader) {
            .type = PK_ATOMTY_NUMBER,
            .refcount = 0,
        },
        .num_type = PK_NUMTY_INT,
        .as.i = integer,
    };
    return atom;
}

PKAtom pk_float(float floater) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    atom->number = (PKAtomNumber) {
        .header = (PKAtomHeader) {
            .type = PK_ATOMTY_NUMBER,
            .refcount = 0,
        },
        .num_type = PK_NUMTY_FLOAT,
        .as.f = floater,
    };
    return atom;
}

