#include "pocket_internals.h"

PKAtom *pk_method_set_(Pocket lisp, PKAtom *type, PKAtom *symbol, PKAtom *method) {
    PKAtomSymbol *asymbol = pk_atom_assert_symbol(lisp, type);
    pk_atom_assert_symbol(lisp, symbol);
    if (!pk_atom_is_function(method)) {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_cstr(lisp, &w, "expected function, got ");
        pk_writer_atom(lisp, &w, method);
        pk_errors(lisp, pk_writer_slice(&w));
    }

    if (asymbol->type_type == PK_TYPETY_NOTYPE) {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_cstr(lisp, &w, "not a type: ");
        pk_writer_atom(lisp, &w, type);
        pk_errors(lisp, pk_writer_slice(&w));
    }

    PKAtom *old_method = pk_hashmap_insert(lisp, &asymbol->methods, symbol, method);

    if (pk_atom_is_undefined(old_method)) {
        return lisp->cache.nil;
    } else {
        return old_method;
    }
}

PKAtom *pk_method_get_raw_(Pocket lisp, PKAtom *type, PKAtom *symbol) {
    PKAtomSymbol *asymbol = pk_atom_assert_symbol(lisp, type);
    pk_atom_assert_symbol(lisp, symbol);
    return pk_hashmap_get(lisp, &asymbol->methods, symbol);
}

PKAtom *pk_method_get_(Pocket lisp, PKAtom *type, PKAtom *symbol) {
    PKAtom *result = pk_method_get_raw_(lisp, type, symbol);
    if (pk_atom_is_undefined(result)) {
        return lisp->cache.nil;
    } else {
        return result;
    }
}