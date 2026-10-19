#include "pocket_internals.h"

PKAtomSymbol *pk_typecheck_new_type(Pocket lisp, PKAtom *atom) {
    PKAtomSymbol *symbol = pk_atom_assert_symbol(lisp, atom);
    if (symbol->type_type != PK_TYPETY_NOTYPE) {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_cstr(lisp, &w, "type already defined: ");
        pk_writer_atom(lisp, &w, atom);
        pk_error_writer(lisp, &w);
    }
    return symbol;
}

void pk_deftype_lisp_(Pocket lisp, PKAtomSymbol *symbol, PKAtoms slots) {
    (void)pk_typecheck_new_type(lisp, (PKAtom *)symbol);

    for (size_t i = 0; i < slots.length; ++i) {
        PKAtom *slot = (PKAtom *)pk_atom_assert_symbol(lisp, slots.e[i]);
        for (size_t j = 0; j < i; ++j) {
            if (pk_atom_eq(lisp, slot, slots.e[j])) {
                PKWriter w = PK_WRITER_EMPTY;
                pk_writer_cstr(lisp, &w, "slot already defined: ");
                pk_writer_atom(lisp, &w, slot);
                pk_error_writer(lisp, &w);
            }
        }
    }

    for (size_t i = 0; i < slots.length; ++i) {
        pk_slotmap_insert(lisp, &symbol->slots, slots.e[i]);
    }

    symbol->type_type = PK_TYPETY_LISP;
    symbol->owned_memory = 0;
}

void pk_deftype_c_owned_(Pocket lisp, PKAtomSymbol *symbol, size_t size) {
    (void)pk_typecheck_new_type(lisp, (PKAtom *)symbol);

    symbol->type_type = PK_TYPETY_C_OWNED;
    symbol->owned_memory = size;
}

void pk_deftype_c_user_(Pocket lisp, PKAtomSymbol *symbol) {
    (void)pk_typecheck_new_type(lisp, (PKAtom *)symbol);

    symbol->type_type = PK_TYPETY_C_USER;
    symbol->owned_memory = 0;
}
