#include "pocket_internals.h"

PKRes pk_hashtable_grow(Pocket lisp, PKHashTable *ht) {
    size_t new_capacity = pk_grow_capacity(ht->capacity, PK_SET_INIT_CAPACITY);
    size_t i = 0;
    PKHashTableSlot **new_e = NULL;
    
    pk_try(pk_malloc(lisp, new_capacity * sizeof(PKHashTableSlot *), (void **)&new_e));
    
    for (i = 0; i < new_capacity; i++) {
        new_e[i] = NULL;
    }

    for (i = 0; i < ht->capacity; i++) {
        PKHashTableSlot *entry = ht->e[i];
        while (entry != NULL) {
            PKHashTableSlot *next = entry->chain;
            size_t bucket = pk_hash_pointer(entry->key) % new_capacity;
            entry->chain = new_e[bucket];
            new_e[bucket] = entry;
            entry = next;
        }
    }

    if (ht->e != NULL) {
        pk_free(lisp, ht->e, ht->capacity * sizeof(PKHashTableSlot *));
    }
    ht->e = new_e;
    ht->capacity = new_capacity;
    return PK_Ok;
}

PKRes pk_hashtable_get(Pocket lisp, PKHashTable *ht, PKAtom *key, PKAtom **output) {
    PKHashTableSlot *entry = NULL;
    size_t bucket = 0;
    
    (void)lisp;
    if (ht->count == 0) {
        *output = NULL;
        return PK_Ok;
    }

    bucket = pk_hash_pointer(key) % ht->capacity;

    for (entry = ht->e[bucket]; entry; entry = entry->chain) {
        if (entry->key == key) {
            *output = entry->value;
            return PK_Ok;
        }
    }

    *output = NULL;
    return PK_Ok;
}
    
PKRes pk_hashtable_put(Pocket lisp, PKHashTable *ht, PKAtom *key, PKAtom *value) {
    PKHashTableSlot *slot = NULL;
    PKHashTableSlot *entry = NULL;
    size_t bucket = 0;
    
    if (ht->count >= ht->capacity) {
        pk_try(pk_hashtable_grow(lisp, ht));
    }

    bucket = pk_hash_pointer(key) % ht->capacity;

    for (slot = ht->e[bucket]; slot; slot = slot->chain) {
        if (slot->key == key) {
            slot->value = value;
            return PK_Ok;
        }
    }

    pk_try(pk_malloc(lisp, sizeof(PKHashTableSlot), (void **)&entry));
    
    entry->key = key;
    entry->value = value;
    entry->chain = ht->e[bucket];
    
    ht->e[bucket] = entry;
    ht->count++;
    return PK_Ok;
}
        
void pk_hashtable_deinit(Pocket lisp, PKHashTable *ht) {
    size_t i = 0;
    for (i = 0; i < ht->capacity; i++) {
        PKHashTableSlot *entry = ht->e[i];
        while (entry) {
            PKHashTableSlot *next = entry->chain;
            pk_free(lisp, entry, sizeof(PKHashTableSlot));
            entry = next;
        }
    }
    if (ht->e) {
        pk_free(lisp, ht->e, ht->capacity * sizeof(PKHashTableSlot *));
    }
    ht->e = NULL;
    ht->count = 0;
    ht->capacity = 0;
}
