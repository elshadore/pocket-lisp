#include "pocket_internals.h"

#define PK_SLOTMAP_INIT_CAPACITY (16)
#define PK_SLOTMAP_LOAD_NUM (7)
#define PK_SLOTMAP_LOAD_DEN (10)

size_t pk_slotmap_hash(PKSlotmap *map, PKAtom *symbol) {
    return pk_atom_hash(symbol) & (map->capacity - 1);
}

PKSlotmapSlot *pk_slotmap_get(Pocket lisp, PKSlotmap *map, PKAtom *symbol) {
    if (map->capacity == 0) {
        return NULL;
    }

    size_t index = pk_slotmap_hash(map, symbol);

    for (size_t i = 0; i < map->capacity; ++i) {
        PKSlotmapSlot *slot = &map->e[index];
        if (slot->state == PK_SLOTMAP_SLOT_UNOCCUPIED) {
            return NULL;
        }
        if ((slot->state == PK_SLOTMAP_SLOT_OCCUPIED) && pk_atom_eq(lisp, slot->symbol, symbol)) {
            return slot;
        }
        index = (index + 1) & (map->capacity - 1);
    }

    return NULL;
}

void pk_slotmap_rehash(Pocket lisp, PKSlotmap *map, size_t new_capacity) {
    PKSlotmapSlot *new_e = (PKSlotmapSlot *)pk_malloc(lisp, new_capacity * sizeof(PKSlotmapSlot));
    
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
        size_t index = pk_slotmap_hash(map, slot.symbol);
        while (map->e[index].state == PK_SLOTMAP_SLOT_OCCUPIED) {
            index = (index + 1) & (map->capacity - 1);
        }
        map->e[index] = slot;
    }

    pk_free(lisp, old_e);
}

void pk_slotmap_insert(Pocket lisp, PKSlotmap *map, PKAtom *symbol) {
    if (pk_slotmap_get(lisp, map, symbol) != NULL) {
        PKStr name = ((PKAtomSymbol *)symbol)->name->str;
        pk_errorf(lisp, "pk_slotmap_insert: slot key %.*s is already defined", (int)name.length, name.c);
        return;
    }

    if (map->capacity == 0) {
        PKSlotmapSlot *e = (PKSlotmapSlot *)pk_malloc(lisp, PK_SLOTMAP_INIT_CAPACITY * sizeof(PKSlotmapSlot));
        memset(e, 0, PK_SLOTMAP_INIT_CAPACITY * sizeof(PKSlotmapSlot));
        map->e = e;
        map->capacity = PK_SLOTMAP_INIT_CAPACITY;
    } else if ((map->count + 1) * PK_SLOTMAP_LOAD_DEN > (map->capacity * PK_SLOTMAP_LOAD_NUM)) {
        pk_slotmap_rehash(lisp, map, pk_grow_capacity(map->capacity, PK_SLOTMAP_INIT_CAPACITY));
    }

    size_t index = pk_slotmap_hash(map, symbol);
    while (map->e[index].state == PK_SLOTMAP_SLOT_OCCUPIED) {
        index = (index + 1) & (map->capacity - 1);
    }

    map->e[index] = (PKSlotmapSlot) {
        .symbol = symbol,
        .index = map->count,
        .state = PK_SLOTMAP_SLOT_OCCUPIED,
    };
    map->count += 1;
}

void pk_slotmap_deinit(Pocket lisp, PKSlotmap *map) {
    pk_free(lisp, map->e);
    *map = PK_SLOTMAP_EMPTY;
}
