#include "pocket_internals.h"

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
    
    PKStr dupe = PK_STR_EMPTY;
    pk_propagate(pk_str_dupe(string, &dupe));
    
    atom->header.type = PK_ATOMTY_SYMBOL;
    atom->symbol.id = dupe;
    
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
