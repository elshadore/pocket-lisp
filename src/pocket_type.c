#include "pocket_internals.h"

void pk_atom_assert_untyped(Pocket lisp, PKAtomSymbol *type) {
    if (type->type_type != PK_TYPETY_NOTYPE) {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_cstr(lisp, &w, "type already defined: ");
        pk_writer_atom(lisp, &w, (PKAtom *)type);
        pk_error_writer(lisp, &w);
    }
}

void pk_atom_define_type_lisp(Pocket lisp, PKAtomSymbol *type, PKAtoms slots) {
    pk_atom_assert_untyped(lisp, type);

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
        pk_slotmap_insert(lisp, &type->slots, slots.e[i]);
    }

    type->type_type = PK_TYPETY_LISP;
    type->owned_memory = 0;
}

void pk_atom_define_type_lisp_atom(Pocket lisp, PKAtomSymbol *type, PKAtom *slots) {
    PKArray array = PK_ARRAY_EMPTY;
    pk_atom_list_to_array(lisp, slots, &array);
    pk_atom_define_type_lisp(lisp, type, pk_array_slice(&array));
    pk_array_deinit(lisp, &array);
}

void pk_atom_define_type_c_owned(Pocket lisp, PKAtomSymbol *type, size_t size) {
    pk_atom_assert_untyped(lisp, type);

    type->type_type = PK_TYPETY_C_OWNED;
    type->owned_memory = size;
}

void pk_atom_define_type_c_user(Pocket lisp, PKAtomSymbol *type) {
    pk_atom_assert_untyped(lisp, type);

    type->type_type = PK_TYPETY_C_USER;
    type->owned_memory = 0;
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
