#include "pocket_internals.h"

#define PK_HASHMAP_INIT_CAPACITY (16)
#define PK_HASHMAP_LOAD_NUM (7)
#define PK_HASHMAP_LOAD_DEN (10)

PKHashmapSlot *pk_hashmap_alloc_slots(size_t capacity) {
    PKHashmapSlot *e = (PKHashmapSlot *)pk_malloc(capacity * sizeof(PKHashmapSlot));
    if (e == NULL) {
        return NULL;
    }
    memset(e, 0, capacity * sizeof(PKHashmapSlot));
    return e;
}

size_t pk_hashmap_index(PKHashmap *map, PKAtom key) {
    return pk_hash(key) & (map->capacity - 1);
}

PKAtom pk_hashmap_rehash(PKHashmap *map, size_t new_capacity) {
    PKHashmapSlot *new_e = pk_hashmap_alloc_slots(new_capacity);
    if (new_e == NULL) {
        return pk_error_oom();
    }
    
    PKHashmapSlot *old_e = map->e;
    size_t old_capacity = map->capacity;
    
    map->e = new_e;
    map->capacity = new_capacity;
    map->count = 0;
    
    PKAtom old_value = pk_undefined();
    
    for (size_t i = 0; i < old_capacity; ++i) {
        PKHashmapSlot slot = old_e[i];
        if (slot.state != PK_HASHMAP_SLOT_OCCUPIED) {
            continue;
        }
        size_t index = pk_hashmap_index(map, slot.key);
        while (map->e[index].state == PK_HASHMAP_SLOT_OCCUPIED) {
            if (pk_eq(map->e[index].key, slot.key)) {
                old_value = map->e[index].value;
                break;
            }
            index = (index + 1) & (new_capacity - 1);
        }
        map->e[index] = slot;
        map->count += 1;
    }
    
    pk_free(old_e);
    
    return old_value;
}

PKHashmapSlot *pk_hashmap_find(PKHashmap *map, PKAtom key, bool *found, size_t *vacancy) {
    size_t index = pk_hashmap_index(map, key);
    size_t first_vacancy = SIZE_MAX;
    
    for (size_t probe = 0; probe < map->capacity; ++probe) {
        PKHashmapSlot *slot = &map->e[index];
        switch (slot->state) {
            case PK_HASHMAP_SLOT_OCCUPIED: {
                if (pk_eq(slot->key, key)) {
                    *found = true;
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
                *found = false;
                *vacancy = (first_vacancy == SIZE_MAX) ? index : first_vacancy;
                return NULL;
            }
            default: break;
        }
        index = (index + 1) & (map->capacity - 1);
    }
    
    *found = false;
    *vacancy = (first_vacancy == SIZE_MAX) ? 0 : first_vacancy;
    return NULL;
}

PKAtom pk_hashmap_insert(PKHashmap *map, PKAtom key, PKAtom value) {
    if (map->capacity == 0) {
        PKHashmapSlot *e = pk_hashmap_alloc_slots(PK_HASHMAP_INIT_CAPACITY);
        if (e == NULL) {
            return pk_error_oom();
        }
        map->e = e;
        map->capacity = PK_HASHMAP_INIT_CAPACITY;
    }
    
    bool found = false;
    size_t vacancy = 0;
    PKHashmapSlot *slot = pk_hashmap_find(map, key, &found, &vacancy);
    if (found) {
        PKAtom old_value = slot->value;
        slot->value = value;
        return old_value;
    }
    
    if ((map->count + 1) * PK_HASHMAP_LOAD_DEN > (map->capacity * PK_HASHMAP_LOAD_NUM)) {
        pk_propagate(pk_hashmap_rehash(map, pk_grow_capacity(map->capacity, PK_HASHMAP_INIT_CAPACITY)));
        bool ignored = false;
        size_t ignored_vacancy = 0;
        slot = pk_hashmap_find(map, key, &ignored, &ignored_vacancy);
        if (slot != NULL) {
            PKAtom old_value = slot->value;
            slot->value = value;
            return old_value;
        }
        vacancy = ignored_vacancy;
    }
    
    map->e[vacancy] = (PKHashmapSlot) {
        .key = key,
        .value = value,
        .state = PK_HASHMAP_SLOT_OCCUPIED,
    };
    map->count += 1;
    
    return pk_undefined();
}

PKAtom pk_hashmap_get(PKHashmap *map, PKAtom key) {
    if (map->capacity == 0) {
        return pk_undefined();
    }
    
    bool found = false;
    size_t vacancy = 0;
    PKHashmapSlot *slot = pk_hashmap_find(map, key, &found, &vacancy);
    if (!found) {
        return pk_undefined();
    }
    return slot->value;
}

PKAtom pk_hashmap_remove(PKHashmap *map, PKAtom key) {
    if (map->capacity == 0) {
        return pk_undefined();
    }
    
    bool found = false;
    size_t vacancy = 0;
    PKHashmapSlot *slot = pk_hashmap_find(map, key, &found, &vacancy);
    if (!found) {
        return pk_undefined();
    }
    
    PKAtom old_value = slot->value;
    
    *slot = (PKHashmapSlot) {
        .key = pk_undefined(),
        .value = pk_undefined(),
        .state = PK_HASHMAP_SLOT_DELETED,
    };
    map->count -= 1;
    
    return old_value;
}

void pk_hashmap_deinit(PKHashmap *map) {
    pk_free(map->e);
    map->e = NULL;
    map->count = 0;
    map->capacity = 0;
}
