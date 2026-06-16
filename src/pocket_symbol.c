#include "pocket_internals.h"

#define PK_INTERN_INIT_CAPACITY 64

void pk_intern_grow(Pocket lisp) {
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

PKAtomSymbol *pk_intern_lookup(Pocket lisp, PKString id) {
    if (lisp->intern.capacity == 0) return NULL;

    size_t hash = pk_hash_djb2(id.c, id.length);
    size_t bucket = hash % lisp->intern.capacity;

    for (PKAtomSymbol *sym = lisp->intern.e[bucket]; sym; sym = sym->chain) {
        if (sym->id->hash == hash && pk_string_eq(sym->id->lit, id)) {
            return sym;
        }
    }
    return NULL;
}

PKAtomSymbol *pk_atom_symbol_interned(Pocket lisp, PKString id) {
    PKAtomSymbol *existing = pk_intern_lookup(lisp, id);
    if (existing) return existing;

    if (lisp->intern.capacity == 0) {
        pk_intern_grow(lisp);
    }

    PKAtomSymbol *sym = pk_atom_symbol_uninterned(lisp, id);
    size_t hash = pk_hash_djb2(id.c, id.length);
    size_t bucket = hash % lisp->intern.capacity;

    sym->chain = lisp->intern.e[bucket];
    lisp->intern.e[bucket] = sym;
    lisp->intern.count++;
    return sym;
}

PKAtomSymbol *pk_atom_symbol_uninterned(Pocket lisp, PKString id) {
    PKAtomSymbol *existing = pk_intern_lookup(lisp, id);
    if (existing) return existing;

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

PKAtomSymbol *pk_atom_cast_symbol(Pocket lisp, PKAtom *atom) {
    if (atom->tag.ty != PKAtomTy_Symbol) pk_error(lisp);
    return (PKAtomSymbol *)atom;
}

bool pk_atom_symbol_eq(Pocket lisp, PKAtomSymbol *lhs, PKAtomSymbol *rhs) {
    if (lhs == rhs) return true;
    return pk_atom_string_eq(lisp, lhs->id, rhs->id);
}

