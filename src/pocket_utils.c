#include "pocket_internals.h"

PKAccum pk_accum_new(void) {
    return (PKAccum) {
        .head = pk_nil(),
        .tail = pk_nil(),
    };
}

PKAtom pk_accum_push(PKAccum *acc, PKAtom atom) {
    PKAtom cons = pk_cons(atom, pk_nil());

    pk_propagate(cons);
    
    if (pk_is_nil(acc->head)) {
        acc->head = cons;
        acc->tail = cons;
    } else {
        pk_cons_link(acc->tail, cons);
        acc->tail = cons;
    }

    return cons;
}

PKAtom pk_array_push(PKArray *array, PKAtom atom, size_t init_capacity) {
    if (array->count >= array->capacity) {
        size_t new_capacity = pk_grow_capacity(array->capacity, init_capacity);
        PKAtom *new_e = pk_realloc(array->e, new_capacity * sizeof(PKAtom));
        if (new_e == NULL) {
            return pk_error_oom();
        }
        array->e = new_e;
        array->capacity = new_capacity;
    }
    array->e[array->count++] = atom;
    return pk_true();
}

void pk_array_deinit(PKArray *array) {
    for (size_t i = 0; i < array->count; ++i) {
        pk_ref_dec(array->e[i]);
    }
    pk_free(array->e);
    *array = PK_ARRAY_EMPTY;
}

PKAtom pk_bytes_push(PKBytes *bytes, pk_u8 byte, size_t init_capacity) {
    if (bytes->count >= bytes->capacity) {
        size_t new_capacity = pk_grow_capacity(bytes->capacity, init_capacity);
        pk_u8 *new_e = pk_realloc(bytes->e, new_capacity * sizeof(pk_u8));
        if (new_e == NULL) {
            return pk_error_oom();
        }
        bytes->e = new_e;
        bytes->capacity = new_capacity;
    }
    bytes->e[bytes->count++] = byte;
    return pk_true();
}

void pk_bytes_deinit(PKBytes *bytes) {
    pk_free(bytes->e);
    *bytes = PK_BYTES_EMPTY;
}

void pk_atoms_free(PKAtoms *atoms) {
    pk_ref_dec_many(*atoms);
    *atoms = PK_ATOMS_EMPTY;
}

PKAtom pk_str_alloc(size_t length, PKStr *output) {
    if (length == 0) {
        *output = PK_STR_EMPTY;
        return pk_true();
    }
    char *buffer = pk_malloc(length * sizeof(char));
    if (buffer == NULL) {
        return pk_error_oom();
    }
    *output = (PKStr) {
        .c = buffer,
        .length = length,
    };

    return pk_true();
}

size_t pk_hash_djb2(PKStr string) {
    size_t hash = 5381;
    size_t i = 0;
    for (i = 0; i < string.length; ++i) {
        hash = ((hash << 5) + hash) + (size_t)string.c[i];
    }
    return hash;
}

size_t pk_hash_pointer(void *ptr) {
    size_t p = (size_t)ptr;
    return (size_t)(p >> 3);
}
