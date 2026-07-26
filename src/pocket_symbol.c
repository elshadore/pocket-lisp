#include "pocket_internals.h"

PK_RES pk_intern_grow(Pocket lisp) {
    PKAtomSymbol **new_e = NULL;
    size_t new_capacity = 0;
    size_t i = 0;
    
    new_capacity = pk_grow_capacity(lisp->intern.capacity, PK_INTERN_INIT_CAPACITY);
    
    pk_try(pk_malloc(lisp, new_capacity * sizeof(PKAtomSymbol *), (void **)&new_e));
    for (i = 0; i < new_capacity; i++) {
        new_e[i] = NULL;
    }

    for (i = 0; i < lisp->intern.capacity; i++) {
        PKAtomSymbol *sym = lisp->intern.e[i];
        while (sym) {
            PKAtomSymbol *next = sym->chain;
            size_t bucket = sym->id->hash % new_capacity;
            sym->chain = new_e[bucket];
            new_e[bucket] = sym;
            sym = next;
        }
    }

    if (lisp->intern.e != NULL) {
        pk_free(lisp, lisp->intern.e, lisp->intern.capacity * sizeof(PKAtomSymbol *));
    }
    lisp->intern.e = new_e;
    lisp->intern.capacity = new_capacity;
    return PK_OK;
}

PKAtomSymbol *pk_intern_lookup(Pocket lisp, const char *c, size_t length) {
    PKAtomSymbol *sym = NULL;
    size_t hash = 0;
    size_t bucket = 0;
    
    if (lisp->intern.capacity == 0) return NULL;

    hash = pk_hash_djb2(c, length);
    bucket = hash % lisp->intern.capacity;

    for (sym = lisp->intern.e[bucket]; sym; sym = sym->chain) {
        if (sym->id->hash == hash && pk_string_eq(sym->id->c, sym->id->length, c, length)) {
            return sym;
        }
    }
    
    return NULL;
}

PK_RES pk_atom_symbol_interned(Pocket lisp, const char *cstr, PKAtomSymbol **output) {
    size_t length = 0;
    length = strlen(cstr);
    return pk_atom_symboln_interned(lisp, cstr, length, output);
}

    
PK_RES pk_atom_symboln_interned(Pocket lisp, const char *string, size_t length, PKAtomSymbol **output) {
    PKAtomSymbol *existing = NULL;
    PKAtomSymbol *sym = NULL;
    size_t hash = 0;
    size_t bucket = 0;
    
    existing = pk_intern_lookup(lisp, string, length);
    
    if (existing != NULL) {
        *output = existing;
        return PK_OK;
    }

    if (lisp->intern.capacity == 0) {
        pk_try(pk_intern_grow(lisp));
    }

    pk_try(pk_atom_symboln_uninterned(lisp, string, length, &sym));
    hash = pk_hash_djb2(string, length);
    bucket = hash % lisp->intern.capacity;

    sym->chain = lisp->intern.e[bucket];
    lisp->intern.e[bucket] = sym;
    lisp->intern.count++;
    *output = sym;
    return PK_OK;
}

PK_RES pk_atom_symbol_uninterned(Pocket lisp, const char *csym, PKAtomSymbol **output) {
    size_t length = 0;
    length = strlen(csym);
    return pk_atom_symboln_uninterned(lisp, csym, length, output);
}
    
PK_RES pk_atom_symboln_uninterned(Pocket lisp, const char *string, size_t length, PKAtomSymbol **output) {
    PKAtomSymbol *existing = NULL;
    PKAtomString *a_string = NULL;
    PKAtom *a = NULL;
    
    existing = pk_intern_lookup(lisp, string, length);
    
    if (existing != NULL) {
        *output = existing;
        return PK_OK;
    }

    pk_try(pk_atom_stringn(lisp, string, length, &a_string));
    pk_try(pk_atom_alloc(lisp, &a));
    
    a->tag.ty = PKAtomTy_Symbol;
    a->symbol.id = a_string,
    a->symbol.chain = NULL,
        
    *output = (PKAtomSymbol *)a;
    return PK_OK;
}

PK_RES pk_atom_cast_symbol(Pocket lisp, PKAtom *atom, PKAtomSymbol **output) {
    if (atom->tag.ty != PKAtomTy_Symbol) return pk_error(lisp);
    *output = (PKAtomSymbol *)atom;
    return PK_OK;
}

pk_bool pk_atom_symbol_eq(Pocket lisp, PKAtomSymbol *lhs, PKAtomSymbol *rhs) {
    if (lhs == rhs) return PK_TRUE;
    return pk_atom_string_eq(lisp, lhs->id, rhs->id);
}

pk_bool pk_atom_is_symbol(PKAtom *atom) {
    return atom->tag.ty == PKAtomTy_Symbol;
}

