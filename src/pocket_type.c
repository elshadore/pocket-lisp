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

PKAtomSymbol *pk_atom_typeof(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_COBJECT: return atom->cobject.type;
        case PK_ATOMTY_LOBJECT: return atom->cobject.type;
        case PK_ATOMTY_UNDEFINED: return lisp->cache.type_undefined;
        case PK_ATOMTY_NIL: return lisp->cache.type_nil;
        case PK_ATOMTY_TRUE: return lisp->cache.type_true;
        case PK_ATOMTY_NUMBER: return lisp->cache.type_number;
        case PK_ATOMTY_STRING: return lisp->cache.type_string;
        case PK_ATOMTY_SYMBOL: return lisp->cache.type_symbol;
        case PK_ATOMTY_KEYWORD: return lisp->cache.type_keyword;
        case PK_ATOMTY_CONS: return lisp->cache.type_cons;
        case PK_ATOMTY_ARRAY: return lisp->cache.type_array;
        case PK_ATOMTY_TABLE: return lisp->cache.type_table;
        case PK_ATOMTY_CFUN: return lisp->cache.type_function;
        case PK_ATOMTY_LFUN: return lisp->cache.type_function;
        case PK_ATOMTY_CMACRO: return lisp->cache.type_macro;
        case PK_ATOMTY_LMACRO: return lisp->cache.type_macro;
        case PK_ATOMTY_CHARACTER: return lisp->cache.type_character;
        case PK_ATOMTY_BLOB: return lisp->cache.type_blob;
        case PK_ATOMTY_STRING_ARRAY: return lisp->cache.type_string_array;
        case PK_ATOMTY_BYTE_ARRAY: return lisp->cache.type_byte_array;
        case PK_ATOMTY_STREAM: return lisp->cache.type_stream;
        default: return lisp->cache.type_unknown;
    }
}
