#include "pocket_internals.h"

#define PK_INTERN_INIT_CAPACITY 64

static void pk_intern_grow(Pocket lisp) {
    size_t new_capacity = pk_grow_capacity(lisp->intern.capacity, PK_INTERN_INIT_CAPACITY);
    PKAtomSymbol **new_e = pk_malloc(lisp, new_capacity * sizeof(PKAtomSymbol *));
    for (size_t i = 0; i < new_capacity; i++) {
        new_e[i] = NULL;
    }

    for (size_t i = 0; i < lisp->intern.capacity; i++) {
        PKAtomSymbol *sym = lisp->intern.e[i];
        while (sym) {
            PKAtomSymbol *next = sym->chain;
            size_t bucket = sym->id->hash % new_capacity;
            sym->chain = new_e[bucket];
            new_e[bucket] = sym;
            sym = next;
        }
    }

    if (lisp->intern.e)
        pk_free(lisp, lisp->intern.e, lisp->intern.capacity * sizeof(PKAtomSymbol *));
    lisp->intern.e = new_e;
    lisp->intern.capacity = new_capacity;
}

PKAtomSymbol *pk_atom_symbol_interned(Pocket lisp, PKString id) {
    size_t hash = pk_hash_djb2(id.c, id.length);

    if (lisp->intern.capacity == 0) {
        pk_intern_grow(lisp);
    }

    size_t bucket = hash % lisp->intern.capacity;

    for (PKAtomSymbol *sym = lisp->intern.e[bucket]; sym; sym = sym->chain) {
        if (sym->id->hash == hash && pk_string_eq(sym->id->lit, id)) {
            return sym;
        }
    }

    PKAtomSymbol *sym = pk_atom_symbol_uninterned(lisp, id);
    
    sym->chain = lisp->intern.e[bucket];
    lisp->intern.e[bucket] = sym;
    lisp->intern.count++;
    return sym;
}

PKAtomSymbol *pk_atom_symbol_uninterned(Pocket lisp, PKString id) {
    PKAtomString *string = pk_atom_string(lisp, id);
    PKAtomSymbol *atom = (PKAtomSymbol *)pk_atom_alloc(lisp);
    *atom = (PKAtomSymbol) {
        .tag.ty = PKAtomTy_Symbol,
        .tag.marked = false,
        .id = string,
        .chain = NULL,
    };
    return atom;
}
