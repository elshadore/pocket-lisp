#include "pocket_internals.h"

PKRes pk_set_grow(Pocket lisp, PKSet *s) {
    size_t new_capacity = pk_grow_capacity(s->capacity, PK_SET_INIT_CAPACITY);
    
    PKSetSlot **new_e = NULL;
    pk_try(pk_malloc(lisp, new_capacity * sizeof(PKSetSlot *), (void **)&new_e));
    
    for (size_t i = 0; i < new_capacity; i++) {
        new_e[i] = NULL;
    }

    for (size_t i = 0; i < s->capacity; i++) {
        PKSetSlot *entry = s->e[i];
        while (entry) {
            PKSetSlot *next = entry->chain;
            size_t bucket = pk_hash_pointer(entry->key) % new_capacity;
            entry->chain = new_e[bucket];
            new_e[bucket] = entry;
            entry = next;
        }
    }

    if (s->e) {
        pk_free(lisp, s->e, s->capacity * sizeof(PKSetSlot *));
    }
    s->e = new_e;
    s->capacity = new_capacity;
    return PK_Ok;
}

PKRes pk_set_insert(Pocket lisp, PKSet *s, PKAtom *key, bool *already_present) {
    if (s->capacity == 0) {
        pk_try(pk_set_grow(lisp, s));
    }

    size_t bucket = pk_hash_pointer(key) % s->capacity;

    for (PKSetSlot *slot = s->e[bucket]; slot; slot = slot->chain) {
        if (slot->key == key) {
            *already_present = true;
            return PK_Ok;
        }
    }

    PKSetSlot *entry = NULL;
    pk_try(pk_malloc(lisp, sizeof(PKSetSlot), (void **)&entry));
    *entry = (PKSetSlot) {
        .key = key,
        .chain = s->e[bucket],
    };
    s->e[bucket] = entry;
    s->count++;
    *already_present = false;
    return PK_Ok;
}

void pk_set_deinit(Pocket lisp, PKSet *s) {
    for (size_t i = 0; i < s->capacity; i++) {
        PKSetSlot *entry = s->e[i];
        while (entry) {
            PKSetSlot *next = entry->chain;
            pk_free(lisp, entry, sizeof(PKSetSlot));
            entry = next;
        }
    }
    if (s->e) {
        pk_free(lisp, s->e, s->capacity * sizeof(PKSetSlot *));
    }
    *s = (PKSet){0};
}
