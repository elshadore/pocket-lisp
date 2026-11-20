#include "pocket_internals.h"

void pk_arena_deinit(Pocket lisp) {
    if (lisp->arena.buffer != NULL) {
        pk_free(lisp, lisp->arena.buffer);
    }
}

void *pk_arena_ptr(Pocket lisp, size_t save) {
    return lisp->arena.buffer + save;
}

void *pk_arena_alloc(Pocket lisp, size_t save, size_t size, void *copy) {
    if (lisp->arena.buffer == NULL) {
        lisp->arena.buffer = pk_malloc(lisp, PK_ARENA_DEFAULT_SIZE);
    }
    size_t new_head = lisp->arena.head + size;
    if (new_head >= PK_ARENA_DEFAULT_SIZE) {
        pk_errors(lisp, pk_str("TODO: dynamically expanding arena"));
        return NULL;
    }
    if (copy != NULL) {
        memcpy(pk_arena_ptr(lisp, pk_arena_save(lisp)), copy, size);
    }
    lisp->arena.head = new_head;
    return pk_arena_ptr(lisp, save);
}

char *pk_arena_string(Pocket lisp, size_t save, PKStr string) {
    return (char *)pk_arena_alloc(lisp, save, string.length * sizeof(char), string.c);
}

pk_u8 *pk_arena_bytes(Pocket lisp, size_t save, PKBlob blob) {
    return (pk_u8 *)pk_arena_alloc(lisp, save, blob.length * sizeof(pk_u8), blob.e);
}

PKAtom *pk_arena_atoms(Pocket lisp, size_t save, PKAtoms atoms) {
    return (PKAtom *)pk_arena_alloc(lisp, save, atoms.length * sizeof(PKAtom), atoms.e);
}

void pk_arena_restore(Pocket lisp, size_t save) {
    lisp->arena.head = save;
}

size_t pk_arena_save(Pocket lisp) {
    return lisp->arena.head;
}

void *pk_arena_allign_word(Pocket lisp) {
    size_t more = lisp->arena.head % sizeof(size_t);
    return pk_arena_alloc(lisp, pk_arena_save(lisp), more, NULL);
}
