#include "pocket_internals.h"

PKRes pk_intern_grow(Pocket lisp) {
    size_t new_capacity = pk_grow_capacity(lisp->intern.capacity, PK_INTERN_INIT_CAPACITY);
    void *mem;
    pk_try(pk_malloc(lisp, new_capacity * sizeof(PKAtomSymbol *), &mem));
    PKAtomSymbol **new_e = mem;
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
    return PK_Ok;
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

PKRes pk_atom_symbol_interned(Pocket lisp, PKString id, PKAtomSymbol **output) {
    PKAtomSymbol *existing = pk_intern_lookup(lisp, id);
    if (existing) {
        *output = existing;
        return PK_Ok;
    }

    if (lisp->intern.capacity == 0) {
        pk_try(pk_intern_grow(lisp));
    }

    PKAtomSymbol *sym;
    pk_try(pk_atom_symbol_uninterned(lisp, id, &sym));
    size_t hash = pk_hash_djb2(id.c, id.length);
    size_t bucket = hash % lisp->intern.capacity;

    sym->chain = lisp->intern.e[bucket];
    lisp->intern.e[bucket] = sym;
    lisp->intern.count++;
    *output = sym;
    return PK_Ok;
}

PKRes pk_atom_symbol_uninterned(Pocket lisp, PKString id, PKAtomSymbol **output) {
    PKAtomSymbol *existing = pk_intern_lookup(lisp, id);
    if (existing) {
        *output = existing;
        return PK_Ok;
    }

    PKAtomString *string;
    pk_try(pk_atom_string(lisp, id, &string));
    PKAtom *a;
    pk_try(pk_atom_alloc(lisp, &a));
    PKAtomSymbol *atom = (PKAtomSymbol *)a;
    *atom = (PKAtomSymbol) {
        .tag.ty = PKAtomTy_Symbol,
        .tag.marked = false,
        .id = string,
        .chain = NULL,
    };
    *output = atom;
    return PK_Ok;
}

PKRes pk_atom_cast_symbol(Pocket lisp, PKAtom *atom, PKAtomSymbol **output) {
    if (atom->tag.ty != PKAtomTy_Symbol) return pk_error(lisp);
    *output = (PKAtomSymbol *)atom;
    return PK_Ok;
}

bool pk_atom_symbol_eq(Pocket lisp, PKAtomSymbol *lhs, PKAtomSymbol *rhs) {
    if (lhs == rhs) return true;
    return pk_atom_string_eq(lisp, lhs->id, rhs->id);
}

bool pk_atom_is_symbol(PKAtom *atom) {
    return atom->tag.ty == PKAtomTy_Symbol;
}
