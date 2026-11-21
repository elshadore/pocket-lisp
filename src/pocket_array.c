#include "pocket_internals.h"

PKAtomArray *pk_atom_array(Pocket lisp, PKAtoms init) {
    PKAtomArray *result = (PKAtomArray *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_ARRAY;
    result->atoms = PK_ARRAY_EMPTY;
    
    for (size_t i = 0; i < init.length; ++i) {
        pk_atom_array_append(lisp, result, init.e[i]);
    }
    
    return result;
}

PKAtomArray *pk_atom_array_make(Pocket lisp, size_t count, PKAtom *fill) {
    PKAtomArray *result = (PKAtomArray *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_ARRAY;
    result->atoms = PK_ARRAY_EMPTY;
    
    for (size_t i = 0; i < count; ++i) {
        pk_atom_array_append(lisp, result, fill);
    }
    
    return result;
}

void pk_atom_array_append(Pocket lisp, PKAtomArray *array, PKAtom *atom) {
    pk_array_push(lisp, &array->atoms, atom, 16);
}

void pk_atom_array_append_many(Pocket lisp, PKAtomArray *array, PKAtoms atoms) {
    for (size_t i = 0; i < atoms.length; ++i) {
        pk_atom_array_append(lisp, array, atoms.e[i]);
    }
}

PKAtom *pk_atom_array_el(Pocket lisp, PKAtomArray *array, size_t index) {
    PKArray *atoms = &array->atoms;
    if (index >= atoms->count) {
        pk_errorf(lisp, "array index: %zu, is out of bounds, array length is %zu", index, atoms->count);
        return &PK_ATOM_NIL;
    }
    return atoms->e[index];
}

bool pk_atom_array_is_element(Pocket lisp, PKAtomArray *array, PKAtom *element) {
    PKAtoms atoms = pk_array_slice(&array->atoms);
    for (size_t i = 0; i < atoms.length; ++i) {
        if (pk_atom_eq(lisp, element, atoms.e[i])) {
            return true;
        }
    }
    return false;
}

size_t pk_atom_array_length(Pocket lisp, PKAtomArray *array) {
    (void)lisp;
    return array->atoms.count;
}

bool pk_atom_is_array(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_ARRAY;
}

PKAtomArray *pk_atom_assert_array(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_array(atom)) {
        pk_error_type(lisp, atom, PK_TYPE_ARRAY);
    }
    return (PKAtomArray *)atom;
}
