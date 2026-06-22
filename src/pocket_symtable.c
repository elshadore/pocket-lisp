#include "pocket_internals.h"

void pk_symtable_grow(Pocket lisp, PKSymTable *st) {
    size_t new_capacity = pk_grow_capacity(st->capacity, PK_SYMTABLE_INIT_CAPACITY);
    PKSymTableSlot **new_e = pk_malloc(lisp, new_capacity * sizeof(PKSymTableSlot *));
    for (size_t i = 0; i < new_capacity; i++) {
        new_e[i] = NULL;
    }

    for (size_t i = 0; i < st->capacity; i++) {
        PKSymTableSlot *entry = st->e[i];
        while (entry) {
            PKSymTableSlot *next = entry->chain;
            size_t bucket = pk_hash_pointer(entry->key) % new_capacity;
            entry->chain = new_e[bucket];
            new_e[bucket] = entry;
            entry = next;
        }
    }

    if (st->e) {
        pk_free(lisp, st->e, st->capacity * sizeof(PKSymTableSlot *));
    }
    st->e = new_e;
    st->capacity = new_capacity;
}

PKAtom *pk_symtable_put(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom *value) {
    if (st->capacity == 0) {
        pk_symtable_grow(lisp, st);
    }

    size_t bucket = pk_hash_pointer(key) % st->capacity;

    for (PKSymTableSlot *slot = st->e[bucket]; slot; slot = slot->chain) {
        if (slot->key == key) {
            PKAtom *result = slot->value;
            slot->value = value;
            return result;
        }
    }

    PKSymTableSlot *entry = pk_malloc(lisp, sizeof(PKSymTableSlot));
    *entry = (PKSymTableSlot) {
        .key = key,
        .value = value,
        .chain = st->e[bucket],
    };
    st->e[bucket] = entry;
    st->count++;
    return NULL;
}

PKAtom *pk_symtable_get(Pocket lisp, PKSymTable *st, PKAtomSymbol *key) {
    (void)lisp;
    if (st->capacity == 0) {
        return NULL;
    }

    size_t bucket = pk_hash_pointer(key) % st->capacity;

    for (PKSymTableSlot *entry = st->e[bucket]; entry; entry = entry->chain) {
        if (entry->key == key) {
            return entry->value;
        }
    }

    return NULL;
}

PKAtom *pk_symtable_rem(Pocket lisp, PKSymTable *st, PKAtomSymbol *key) {
    if (st->capacity == 0) {
        return NULL;
    }

    size_t bucket = pk_hash_pointer(key) % st->capacity;

    PKSymTableSlot *prev = NULL;
    for (PKSymTableSlot *slot = st->e[bucket]; slot; prev = slot, slot = slot->chain) {
        if (slot->key == key) {
            PKAtom *result = slot->value;
            if (prev) {
                prev->chain = slot->chain;
            } else {
                st->e[bucket] = slot->chain;
            }
            pk_free(lisp, slot, sizeof(PKSymTableSlot));
            st->count--;
            return result;
        }
    }
    return NULL;
}

PKAtom *pk_symtable_alist(Pocket lisp, PKSymTable *st) {
    PKAtom *alist = pk_atom_nil(lisp);
    for (size_t i = 0; i < st->capacity; i++) {
        for (PKSymTableSlot *slot = st->e[i]; slot; slot = slot->chain) {
            PKAtomCons *pair = pk_atom_cons(lisp, (PKAtom *)slot->key, slot->value);
            alist = (PKAtom *)pk_atom_cons(lisp, (PKAtom *)pair, alist);
        }
    }
    return alist;
}

void pk_symtable_deinit(Pocket lisp, PKSymTable *st) {
    for (size_t i = 0; i < st->capacity; i++) {
        PKSymTableSlot *entry = st->e[i];
        while (entry) {
            PKSymTableSlot *next = entry->chain;
            pk_free(lisp, entry, sizeof(PKSymTableSlot));
            entry = next;
        }
    }
    if (st->e) {
        pk_free(lisp, st->e, st->capacity * sizeof(PKSymTableSlot *));
    }
    *st = (PKSymTable){0};
}


