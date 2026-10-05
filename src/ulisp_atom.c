#include "ulisp_internals.h"

ULAtomData UL_ATOM_NIL = (ULAtomData) {
    .header = (ULAtomHeader) {
        .type = UL_ATOMTY_NIL,
        .flags = UL_ATOMFLAG_IMMORTAL,
        .refcount = 0,
    }
};

ULAtomData UL_ATOM_TRUE = (ULAtomData) {
    .header = (ULAtomHeader) {
        .type = UL_ATOMTY_TRUE,
        .flags = UL_ATOMFLAG_IMMORTAL,
        .refcount = 0,
    }
};

ULAtomData UL_ATOM_ERROR = (ULAtomData) {
    .header = (ULAtomHeader) {
        .type = UL_ATOMTY_ERROR,
        .flags = UL_ATOMFLAG_IMMORTAL,
        .refcount = 0,
    }
};

ULAtom ul_nil() {
    return &UL_ATOM_NIL;
}

ULAtom ul_true() {
    return &UL_ATOM_TRUE;
}

ULAtom ul_error(void) {
    return &UL_ATOM_ERROR;
}
    
ULAtom ul_error_oom(void) {
    return &UL_ATOM_ERROR;
}

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

    ULAtom name = ul_string(string);
    ul_propagate(name);

    atom->header.type = UL_ATOMTY_SYMBOL;
    atom->symbol.var = ul_nil();
    atom->symbol.fun = ul_nil();
    atom->symbol.name = name;
    
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

bool ul_is_nil(ULAtom atom) {
    return atom->header.type == UL_ATOMTY_NIL;
}

bool ul_is_number(ULAtom atom) {
    return atom->header.type == UL_ATOMTY_NUMBER;
}

bool ul_is_error(ULAtom atom) {
    return atom->header.type == UL_ATOMTY_ERROR;
}
