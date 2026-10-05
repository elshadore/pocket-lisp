#include "user_internals.h"

ULAtom ul_int(int integer) {
    ULAtom atom = ul_atom_alloc();
    ul_propagate(atom);
    atom->header.type = UL_ATOMTY_NUMBER;
    atom->number.num_type = UL_NUMTY_INT;
    atom->number.as.i = integer;
    return atom;
}

ULAtom ul_float(float floater) {
    ULAtom atom = ul_atom_alloc();
    ul_propagate(atom);
    atom->header.type = UL_ATOMTY_NUMBER;
    atom->number.num_type = UL_NUMTY_FLOAT;
    atom->number.as.f = floater;
    return atom;
}

ULAtom ul_string(ULStr string) {
    ULAtom atom = ul_atom_alloc();
    ul_propagate(atom);
    
    ULStr dupe = UL_STR_EMPTY;
    ul_propagate(ul_str_dupe(string, &dupe));
    
    atom->header.type = UL_ATOMTY_STRING;
    atom->string.str = dupe;
    
    return atom;
}

ULAtom ul_symbol(ULStr string) {
    ULAtom atom = ul_atom_alloc();
    ul_propagate(atom);
    
    ULStr dupe = UL_STR_EMPTY;
    ul_propagate(ul_str_dupe(string, &dupe));
    
    atom->header.type = UL_ATOMTY_SYMBOL;
    atom->symbol.id = dupe;
    
    return atom;
}

ULAtom ul_cons(ULAtom car, ULAtom cdr) {
    ULAtom atom = ul_atom_alloc();
    ul_propagate(atom);
    
    atom->header.type = UL_ATOMTY_CONS;
    atom->cons.car = car;
    atom->cons.cdr = cdr;
    
    return atom;
}
