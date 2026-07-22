#include "pocket_internals.h"

PKRes pk_symtable_grow(Pocket lisp, PKSymTable *st) {
    PKSymTableSlot **new_e = NULL;
    size_t new_capacity = 0;
    size_t i = 0;
    
    new_capacity = pk_grow_capacity(st->capacity, PK_SYMTABLE_INIT_CAPACITY);
    
    pk_try(pk_malloc(lisp, new_capacity * sizeof(PKSymTableSlot *), (void **)&new_e));
    for (i = 0; i < new_capacity; i++) {
        new_e[i] = NULL;
    }

    for (i = 0; i < st->capacity; i++) {
        PKSymTableSlot *entry = st->e[i];
        while (entry) {
            PKSymTableSlot *next = entry->chain;
            size_t bucket = pk_hash_pointer(entry->key) % new_capacity;
            entry->chain = new_e[bucket];
            new_e[bucket] = entry;
            entry = next;
        }
    }

    if (st->e != NULL) {
        pk_free(lisp, st->e, st->capacity * sizeof(PKSymTableSlot *));
    }
    
    st->e = new_e;
    st->capacity = new_capacity;
    return PK_Ok;
}

PKRes pk_symtable_put(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom *value, PKAtom **output) {
    PKSymTableSlot *slot = NULL;
    PKSymTableSlot *entry = NULL;
    size_t bucket = 0;
    
    if (st-> count >= st->capacity) {
        pk_try(pk_symtable_grow(lisp, st));
    }

    bucket = pk_hash_pointer(key) % st->capacity;

    for (slot = st->e[bucket]; slot; slot = slot->chain) {
        if (slot->key == key) {
            *output = slot->value;
            slot->value = value;
            return PK_Ok;
        }
    }
    
    pk_try(pk_malloc(lisp, sizeof(PKSymTableSlot), (void **)&entry));
    entry->key = key;
    entry->value = value;
    entry->chain = st->e[bucket];
    
    st->e[bucket] = entry;
    st->count++;
    *output = NULL;
    return PK_Ok;
}

PKRes pk_symtable_get(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom **output) {
    (void)lisp;
    if (st->count == 0) {
        *output = NULL;
        return PK_Ok;
    }

    size_t bucket = pk_hash_pointer(key) % st->capacity;

    for (PKSymTableSlot *entry = st->e[bucket]; entry; entry = entry->chain) {
        if (entry->key == key) {
            *output = entry->value;
            return PK_Ok;
        }
    }

    *output = NULL;
    return PK_Ok;
}

PKRes pk_symtable_rem(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom **output) {
    if (st->capacity == 0) {
        *output = NULL;
        return PK_Ok;
    }

    size_t bucket = pk_hash_pointer(key) % st->capacity;

    PKSymTableSlot *prev = NULL;
    for (PKSymTableSlot *slot = st->e[bucket]; slot; prev = slot, slot = slot->chain) {
        if (slot->key == key) {
            *output = slot->value;
            if (prev) {
                prev->chain = slot->chain;
            } else {
                st->e[bucket] = slot->chain;
            }
            pk_free(lisp, slot, sizeof(PKSymTableSlot));
            st->count--;
            return PK_Ok;
        }
    }
    *output = NULL;
    return PK_Ok;
}

PKRes pk_symtable_alist(Pocket lisp, PKSymTable *st, PKAtom **output) {
    PKAtom *alist = pk_atom_nil(lisp);
    for (size_t i = 0; i < st->capacity; i++) {
        for (PKSymTableSlot *slot = st->e[i]; slot; slot = slot->chain) {
            PKAtomCons *pair;
            pk_try(pk_atom_cons(lisp, (PKAtom *)slot->key, slot->value, &pair));
            PKAtomCons *new_alist;
            pk_try(pk_atom_cons(lisp, (PKAtom *)pair, alist, &new_alist));
            alist = (PKAtom *)new_alist;
        }
    }
    *output = alist;
    return PK_Ok;
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
