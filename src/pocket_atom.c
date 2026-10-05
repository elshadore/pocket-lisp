#include "pocket_internals.h"

PKAtomData PK_ATOM_NIL = (PKAtomData) {
    .header = (PKAtomHeader) {
        .type = PK_ATOMTY_NIL,
        .flags = PK_ATOMFLAG_IMMORTAL,
        .refcount = 0,
    }
};

PKAtomData PK_ATOM_TRUE = (PKAtomData) {
    .header = (PKAtomHeader) {
        .type = PK_ATOMTY_TRUE,
        .flags = PK_ATOMFLAG_IMMORTAL,
        .refcount = 0,
    }
};

PKAtomData PK_ATOM_ERROR = (PKAtomData) {
    .header = (PKAtomHeader) {
        .type = PK_ATOMTY_ERROR,
        .flags = PK_ATOMFLAG_IMMORTAL,
        .refcount = 0,
    }
};

PKAtom pk_nil() {
    return &PK_ATOM_NIL;
}

PKAtom pk_true() {
    return &PK_ATOM_TRUE;
}

PKAtom pk_error(void) {
    return &PK_ATOM_ERROR;
}
    
PKAtom pk_error_oom(void) {
    return &PK_ATOM_ERROR;
}

PKAtom pk_int(int integer) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    atom->header.type = PK_ATOMTY_NUMBER;
    atom->number.num_type = PK_NUMTY_INT;
    atom->number.as.i = integer;
    return atom;
}

PKAtom pk_float(float floater) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    atom->header.type = PK_ATOMTY_NUMBER;
    atom->number.num_type = PK_NUMTY_FLOAT;
    atom->number.as.f = floater;
    return atom;
}

PKAtom pk_string(PKStr string) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    
    PKStr dupe = PK_STR_EMPTY;
    pk_propagate(pk_str_dupe(string, &dupe));
    
    atom->header.type = PK_ATOMTY_STRING;
    atom->string.str = dupe;
    
    return atom;
}

PKAtom pk_symbol(PKStr string) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);

    PKAtom name = pk_string(string);
    pk_propagate(name);

    atom->header.type = PK_ATOMTY_SYMBOL;
    atom->symbol.var = pk_nil();
    atom->symbol.fun = pk_nil();
    atom->symbol.name = name;
    
    return atom;
}

PKAtom pk_cons(PKAtom car, PKAtom cdr) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    
    atom->header.type = PK_ATOMTY_CONS;
    atom->cons.car = car;
    atom->cons.cdr = cdr;
    
    return atom;
}

bool pk_is_nil(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_NIL;
}

bool pk_is_number(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_NUMBER;
}

bool pk_is_error(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_ERROR;
}
