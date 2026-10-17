#include "pocket_internals.h"

PKAccum pk_accum_new(Pocket lisp) {
    return (PKAccum) {
        .lisp = lisp,
        .head = lisp->cache.nil,
        .tail = lisp->cache.nil,
    };
}

void pk_accum_push(PKAccum *acc, PKAtom *atom) {
    PKAtomCons *cons = pk_atom_list1(acc->lisp, atom);

    if (pk_atom_is_nil(acc->head)) {
        acc->head = (PKAtom *)cons;
        acc->tail = (PKAtom *)cons;
    } else {
        acc->tail = (PKAtom *)cons;
    }
}

void pk_array_push(Pocket lisp, PKArray *array, PKAtom *atom, size_t init_capacity) {
    if (array->count >= array->capacity) {
        size_t new_capacity = pk_grow_capacity(array->capacity, init_capacity);
        PKAtom **new_e = pk_realloc(lisp, array->e, new_capacity * sizeof(PKAtom *));
        array->e = new_e;
        array->capacity = new_capacity;
    }
    array->e[array->count++] = atom;
}

void pk_array_deinit(Pocket lisp, PKArray *array) {
    pk_free(lisp, array->e);
    *array = PK_ARRAY_EMPTY;
}

void pk_bytes_push(Pocket lisp, PKBytes *bytes, pk_u8 byte, size_t init_capacity) {
    if (bytes->count >= bytes->capacity) {
        size_t new_capacity = pk_grow_capacity(bytes->capacity, init_capacity);
        pk_u8 *new_e = pk_realloc(lisp, bytes->e, new_capacity * sizeof(pk_u8));
        bytes->e = new_e;
        bytes->capacity = new_capacity;
    }
    bytes->e[bytes->count++] = byte;
}

void pk_bytes_deinit(Pocket lisp, PKBytes *bytes) {
    pk_free(lisp, bytes->e);
    *bytes = PK_BYTES_EMPTY;
}

void pk_atoms_free(Pocket lisp, PKAtoms *atoms) {
    pk_free(lisp, atoms->e);
    *atoms = PK_ATOMS_EMPTY;
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
