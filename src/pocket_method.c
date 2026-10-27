#include "pocket_internals.h"

PKAtom *pk_atom_method_set(Pocket lisp, PKAtomSymbol *type, PKAtomSymbol *symbol, PKAtom *method) {
    if (!pk_atom_is_function(method)) {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_cstr(lisp, &w, "expected function, got ");
        pk_writer_atom(lisp, &w, method);
        pk_error_writer(lisp, &w);
    }

    if (type->type_type == PK_TYPETY_NOTYPE) {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_cstr(lisp, &w, "not a type: ");
        pk_writer_atom(lisp, &w, (PKAtom *)type);
        pk_error_writer(lisp, &w);
    }

    PKAtom *old_method = pk_hashmap_insert(lisp, &type->methods, (PKAtom *)symbol, method, PK_HASHMAP_INSERT_NORMAL);

    if (pk_atom_is_undefined(old_method)) {
        return lisp->cache.nil;
    } else {
        return old_method;
    }
}

PKAtom *pk_atom_method_get(Pocket lisp, PKAtomSymbol *type, PKAtomSymbol *symbol) {
    PKAtom *result = pk_atom_method_get_raw(lisp, type, symbol);
    if (pk_atom_is_undefined(result)) {
        return lisp->cache.nil;
    } else {
        return result;
    }
}

PKAtom *pk_atom_method_get_raw(Pocket lisp, PKAtomSymbol *type, PKAtomSymbol *symbol) {
    return pk_hashmap_get(lisp, &type->methods, (PKAtom *)symbol);
}
