#include "pocket_internals.h"

PKAtomData PK_ATOM_OOM = (PKAtomData) {
    .header = (PKAtomHeader) {
        .type = PK_ATOMTY_OOM,
        .flags = PK_ATOMFLAG_IMMORTAL,
        .refcount = 0,
    }
};

PKAtomData PK_ATOM_OOM_MESSAGE = (PKAtomData) {
    .string = (PKAtomString) {
        .header.type = PK_ATOMTY_STRING,
        .header.flags = PK_ATOMFLAG_IMMORTAL,
        .header.refcount = 0,
        .str = pk_str("out of memory")
    }
};

PKAtom pk_throw(PKAtom type, PKAtom contents) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    
    atom->header.type = PK_ATOMTY_THROW;
    atom->throw.type = type;
    atom->throw.contents = contents;

    return atom;
}

PKAtom pk_error(PKAtom message) {
    return pk_throw(PK_CORE.cache.error, message);
}

PKAtom pk_errors(PKStr message) {
    PKAtom atom = pk_string(message);
    pk_propagate(atom);
    return pk_error(atom);
}

PKAtom pk_error_type(PK_TYPE expected, PKAtom found) {
    (void)expected;
    (void)found;
    return pk_errors(pk_str("TODO: implement type error"));
}
    
PKAtom pk_error_oom(void) {
    return &PK_ATOM_OOM;
}

PKAtom pk_throw_type(PKAtom throw) {
    switch (throw->header.type) {
        case PK_ATOMTY_OOM: return PK_CORE.cache.error;
        case PK_ATOMTY_THROW: return throw->throw.type;
        default: return pk_error_type(PK_TYPE_THROW, throw);
    }
}

PKAtom pk_throw_contents(PKAtom throw) {
    switch (throw->header.type) {
        case PK_ATOMTY_OOM: return &PK_ATOM_OOM_MESSAGE;
        case PK_ATOMTY_THROW: return throw->throw.contents;
        default: return pk_error_type(PK_TYPE_THROW, throw);
    }
}

bool pk_is_throwing(PKAtom atom) {
    return (atom->header.type == PK_ATOMTY_THROW) || (atom->header.type == PK_ATOMTY_OOM);
}

bool pk_is_error(PKAtom atom) {
    return pk_is_throwing(atom);
}

PKAtom pk_typecheck_throw(PKAtom atom) {
    return pk_error_type(PK_TYPE_THROW, atom);
}
