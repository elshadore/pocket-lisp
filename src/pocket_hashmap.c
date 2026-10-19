#include "pocket_internals.h"

#define PK_HASHMAP_INIT_CAPACITY (16)
#define PK_HASHMAP_LOAD_NUM (7)
#define PK_HASHMAP_LOAD_DEN (10)

PKHashmapSlot *pk_hashmap_alloc_slots(Pocket lisp, size_t capacity) {
    PKHashmapSlot *e = (PKHashmapSlot *)pk_malloc(lisp, capacity * sizeof(PKHashmapSlot));
    memset(e, 0, capacity * sizeof(PKHashmapSlot));
    return e;
}

size_t pk_hashmap_hash(PKHashmap *map, PKAtom *key) {
    return pk_atom_hash(key) & (map->capacity - 1);
}

void pk_hashmap_rehash(Pocket lisp, PKHashmap *map, size_t new_capacity) {
    PKHashmapSlot *new_e = pk_hashmap_alloc_slots(lisp, new_capacity);

    PKHashmapSlot *old_e = map->e;
    size_t old_capacity = map->capacity;

    map->e = new_e;
    map->capacity = new_capacity;
    map->count = 0;

    for (size_t i = 0; i < old_capacity; ++i) {
        PKHashmapSlot slot = old_e[i];
        if (slot.state != PK_HASHMAP_SLOT_OCCUPIED) {
            continue;
        }
        size_t index = pk_hashmap_hash(map, slot.key);
        while (map->e[index].state == PK_HASHMAP_SLOT_OCCUPIED) {
            index = (index + 1) & (new_capacity - 1);
        }
        map->e[index] = slot;
        map->count += 1;
    }

    pk_free(lisp, old_e);
}

PKHashmapSlot *pk_hashmap_find(Pocket lisp, PKHashmap *map, PKAtom *key, size_t *vacancy) {
    size_t index = pk_hashmap_hash(map, key);
    size_t first_vacancy = SIZE_MAX;

    for (size_t i = 0; i < map->capacity; ++i) {
        PKHashmapSlot *slot = &map->e[index];
        switch (slot->state) {
            case PK_HASHMAP_SLOT_OCCUPIED: {
                if (pk_atom_eq(lisp, slot->key, key)) {
                    *vacancy = index;
                    return slot;
                }
                break;
            }
            case PK_HASHMAP_SLOT_DELETED: {
                if (first_vacancy == SIZE_MAX) {
                    first_vacancy = index;
                }
                break;
            }
            case PK_HASHMAP_SLOT_UNOCCUPIED: {
                *vacancy = (first_vacancy == SIZE_MAX) ? index : first_vacancy;
                return NULL;
            }
            default: break;
        }
        index = (index + 1) & (map->capacity - 1);
    }

    *vacancy = (first_vacancy == SIZE_MAX) ? 0 : first_vacancy;
    return NULL;
}

PKAtom *pk_hashmap_insert(Pocket lisp, PKHashmap *map, PKAtom *key, PKAtom *value) {
    if (map->capacity == 0) {
        map->e = pk_hashmap_alloc_slots(lisp, PK_HASHMAP_INIT_CAPACITY);
        map->capacity = PK_HASHMAP_INIT_CAPACITY;
    }

    size_t vacancy = 0;
    PKHashmapSlot *slot = pk_hashmap_find(lisp, map, key, &vacancy);
    if (slot != NULL) {
        PKAtom *old_value = slot->value;
        slot->value = value;
        return old_value;
    }

    if ((map->count + 1) * PK_HASHMAP_LOAD_DEN > (map->capacity * PK_HASHMAP_LOAD_NUM)) {
        pk_hashmap_rehash(lisp, map, pk_grow_capacity(map->capacity, PK_HASHMAP_INIT_CAPACITY));
        size_t ignored = 0;
        slot = pk_hashmap_find(lisp, map, key, &ignored);
        if (slot != NULL) {
            PKAtom *old_value = slot->value;
            slot->value = value;
            return old_value;
        }
        vacancy = ignored;
    }

    map->e[vacancy] = (PKHashmapSlot) {
        .key = key,
        .value = value,
        .state = PK_HASHMAP_SLOT_OCCUPIED,
    };
    map->count += 1;

    return lisp->cache.undefined;
}

PKAtom *pk_hashmap_get_str(Pocket lisp, PKHashmap *map, PKStr key) {
    if (map->capacity == 0) {
        return lisp->cache.undefined;
    }

    size_t index = pk_hash_djb2(key) & (map->capacity - 1);

    for (size_t i = 0; i < map->capacity; ++i) {
        PKHashmapSlot *slot = &map->e[index];
        if (slot->state == PK_HASHMAP_SLOT_UNOCCUPIED) {
            return lisp->cache.undefined;
        }
        if ((slot->state == PK_HASHMAP_SLOT_OCCUPIED) && pk_atom_is_string(slot->key)) {
            if (pk_str_eq(slot->key->string.str, key)) {
                return slot->value;
            }
        }
        index = (index + 1) & (map->capacity - 1);
    }

    return lisp->cache.undefined;
}

PKAtom *pk_hashmap_get(Pocket lisp, PKHashmap *map, PKAtom *key) {
    if (map->capacity == 0) {
        return lisp->cache.undefined;
    }

    size_t vacancy = 0;
    PKHashmapSlot *slot = pk_hashmap_find(lisp, map, key, &vacancy);
    if (slot == NULL) {
        return lisp->cache.undefined;
    } else {
        return slot->value;
    }
}

PKAtom *pk_hashmap_remove(Pocket lisp, PKHashmap *map, PKAtom *key) {
    if (map->capacity == 0) {
        return lisp->cache.undefined;
    }

    size_t vacancy = 0;
    PKHashmapSlot *slot = pk_hashmap_find(lisp, map, key, &vacancy);
    if (slot == NULL) {
        return lisp->cache.undefined;
    }

    PKAtom *result = slot->value;

    *slot = (PKHashmapSlot) {
        .key = lisp->cache.undefined,
        .value = lisp->cache.undefined,
        .state = PK_HASHMAP_SLOT_DELETED,
    };
    map->count -= 1;

    return result;
}

void pk_hashmap_deinit(Pocket lisp, PKHashmap *map) {
    pk_free(lisp, map->e);
    *map = PK_HASHMAP_EMPTY;
}
