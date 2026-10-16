#include "pocket_internals.h"

#define PK_SLOTMAP_INIT_CAPACITY (16)
#define PK_SLOTMAP_LOAD_NUM (7)
#define PK_SLOTMAP_LOAD_DEN (10)

size_t pk_slotmap_index(PKSlotmap *map, PKAtom symbol) {
    return pk_hash(symbol) & (map->capacity - 1);
}

PKSlotmapSlot *pk_slotmap_find(PKSlotmap *map, PKAtom symbol) {
    if (map->capacity == 0) {
        return NULL;
    }

    size_t index = pk_slotmap_index(map, symbol);

    for (size_t i = 0; i < map->capacity; ++i) {
        PKSlotmapSlot *slot = &map->e[index];
        if (slot->state == PK_SLOTMAP_SLOT_UNOCCUPIED) {
            return NULL;
        }
        if ((slot->state == PK_SLOTMAP_SLOT_OCCUPIED) && pk_eq(slot->symbol, symbol)) {
            return slot;
        }
        index = (index + 1) & (map->capacity - 1);
    }

    return NULL;
}

bool pk_slotmap_index_of(PKSlotmap *map, PKAtom symbol, size_t *index) {
    PKSlotmapSlot *slot = pk_slotmap_find(map, symbol);
    if (slot == NULL) {
        return false;
    }
    *index = slot->index;
    return true;
}

static PKAtom pk_slotmap_rehash(PKSlotmap *map, size_t new_capacity) {
    PKSlotmapSlot *new_e = (PKSlotmapSlot *)pk_malloc(new_capacity * sizeof(PKSlotmapSlot));
    if (new_e == NULL) {
        return pk_error_oom();
    }
    memset(new_e, 0, new_capacity * sizeof(PKSlotmapSlot));

    PKSlotmapSlot *old_e = map->e;
    size_t old_capacity = map->capacity;

    map->e = new_e;
    map->capacity = new_capacity;

    for (size_t i = 0; i < old_capacity; ++i) {
        PKSlotmapSlot slot = old_e[i];
        if (slot.state != PK_SLOTMAP_SLOT_OCCUPIED) {
            continue;
        }
        size_t index = pk_slotmap_index(map, slot.symbol);
        while (map->e[index].state == PK_SLOTMAP_SLOT_OCCUPIED) {
            index = (index + 1) & (map->capacity - 1);
        }
        map->e[index] = slot;
    }

    pk_free(old_e);

    return pk_undefined();
}

PKAtom pk_slotmap_insert(PKSlotmap *map, PKAtom symbol, size_t *index) {
    if (pk_slotmap_find(map, symbol) != NULL) {
        PKWriter w = PK_WRITER_EMPTY;
        pk_propagate(pk_writer_string(&w, pk_str("slot already defined: ")));
        pk_propagate(pk_writer_atom(&w, symbol));
        PKAtom message = pk_string(pk_writer_slice(&w));
        pk_writer_deinit(&w);
        pk_propagate(message);
        PKAtom error = pk_error(message);
        pk_ref_dec(message);
        return error;
    }

    if (map->capacity == 0) {
        PKSlotmapSlot *e = (PKSlotmapSlot *)pk_malloc(PK_SLOTMAP_INIT_CAPACITY * sizeof(PKSlotmapSlot));
        if (e == NULL) {
            return pk_error_oom();
        }
        memset(e, 0, PK_SLOTMAP_INIT_CAPACITY * sizeof(PKSlotmapSlot));
        map->e = e;
        map->capacity = PK_SLOTMAP_INIT_CAPACITY;
    }

    if ((map->count + 1) * PK_SLOTMAP_LOAD_DEN > (map->capacity * PK_SLOTMAP_LOAD_NUM)) {
        pk_propagate(pk_slotmap_rehash(map, pk_grow_capacity(map->capacity, PK_SLOTMAP_INIT_CAPACITY)));
    }

    size_t vacancy = pk_slotmap_index(map, symbol);
    while (map->e[vacancy].state == PK_SLOTMAP_SLOT_OCCUPIED) {
        vacancy = (vacancy + 1) & (map->capacity - 1);
    }

    pk_propagate(pk_ref_inc(symbol));
    *index = map->count;
    map->e[vacancy] = (PKSlotmapSlot) {
        .symbol = symbol,
        .index = map->count,
        .state = PK_SLOTMAP_SLOT_OCCUPIED,
    };
    map->count += 1;

    return pk_undefined();
}

PKAtom pk_slotmap_reserve(PKSlotmap *map, size_t count) {
    size_t capacity = (map->capacity == 0) ? PK_SLOTMAP_INIT_CAPACITY : map->capacity;

    while ((count + 1) * PK_SLOTMAP_LOAD_DEN > (capacity * PK_SLOTMAP_LOAD_NUM)) {
        capacity = pk_grow_capacity(capacity, PK_SLOTMAP_INIT_CAPACITY);
    }

    if (capacity == map->capacity) {
        return pk_undefined();
    }

    if (map->capacity == 0) {
        PKSlotmapSlot *e = (PKSlotmapSlot *)pk_malloc(capacity * sizeof(PKSlotmapSlot));
        if (e == NULL) {
            return pk_error_oom();
        }
        memset(e, 0, capacity * sizeof(PKSlotmapSlot));
        map->e = e;
        map->capacity = capacity;
        return pk_undefined();
    }

    return pk_slotmap_rehash(map, capacity);
}

void pk_slotmap_deinit(PKSlotmap *map) {
    for (size_t i = 0; i < map->capacity; ++i) {
        PKSlotmapSlot *slot = &map->e[i];
        if (slot->state != PK_SLOTMAP_SLOT_OCCUPIED) {
            continue;
        }
        pk_ref_dec(slot->symbol);
    }

    pk_free(map->e);
    map->e = NULL;
    map->count = 0;
    map->capacity = 0;
}