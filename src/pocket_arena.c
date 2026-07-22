#include "pocket_internals.h"

void pk_arena_deinit(Pocket lisp, PKArena *arena) {
    pk_free(lisp, arena->e, arena->size);
}

void pk_arena_deinit_all(Pocket lisp) {
    size_t i = 0;
    for (i = 0; i < lisp->arena_stack.count; ++i) {
        pk_arena_deinit(lisp, &lisp->arena_stack.e[i]);
    }
    pk_arena_deinit(lisp, &lisp->arena);
}

PKRes pk_arena_stack_push(Pocket lisp, PKArena arena) {
    PKArenaStack *s = &lisp->arena_stack;
    if (s->count >= s->capacity) {
        size_t new_capacity = pk_grow_capacity(s->capacity, PK_ARENA_STACK_INIT_CAPACITY);
        PKArena *new_e = NULL;
        pk_try(pk_trealloc(PKArena, lisp, s->e, s->capacity, new_capacity, (void **)&new_e));
        s->e = new_e;
        s->capacity = new_capacity;
    }
    s->e[s->count++] = arena;
    return PK_Ok;
}

PKRes pk_arena_expand(Pocket lisp, size_t size) {
    size_t final_size = 0;
    if (size < PK_ARENA_PAGE) {
        final_size = PK_ARENA_PAGE;
    } else if (lisp->arena.size > size) {
        final_size = lisp->arena.size * 2;
    } else {
        final_size = pk_next_pow2(size);
    }
    
    void *e = NULL;
    pk_try(pk_malloc(lisp, final_size, &e));
    PKRes result = PK_Yield;

    pk_defer(pk_arena_stack_push(lisp, lisp->arena));
    lisp->arena = (PKArena) {
        .e = e,
        .head = 0,
        .size = final_size,
    };
    
    result = PK_Ok;
    DEFER:
    return result;
}

PKRes pk_arena_alloc(Pocket lisp, size_t size, void **output) {
    size_t new_head = lisp->arena.head + size;
    if (new_head > lisp->arena.size) {
        pk_try(pk_arena_expand(lisp, size));
        *output = lisp->arena.e;
        lisp->arena.head = size;
    } else {
        *output = lisp->arena.e + lisp->arena.head;
        lisp->arena.head = new_head;
    }
    return PK_Ok;
}

bool pk_arena_inbetween(PKArena *arena, void *ptr) {
    if (ptr < arena->e) {
        return false;
    }
    if (ptr > (arena->e + arena->head)) {
        return false;
    }
    return true;
}

void *pk_arena_savepoint(Pocket lisp) {
    return lisp->arena.e + lisp->arena.head;
}

size_t pk_arena_savepoint_index(Pocket lisp, void *savepoint) {
    PKArenaStack *s = &lisp->arena_stack;
    size_t i = 0;
    for (i = 0; i < s->count; ++i) {
        size_t index = pk_index_inv(i, s->count);
        PKArena *arena = &s->e[index];
        if (pk_arena_inbetween(arena, savepoint)) {
            return i;
        }
    }
    return 0;
}

void pk_arena_restore(Pocket lisp, void *savepoint) {
    if (!pk_arena_inbetween(&lisp->arena, savepoint)) {
        PKArenaStack *s = &lisp->arena_stack;
        size_t index = pk_arena_savepoint_index(lisp, savepoint);
        size_t i = 0;
        for (i = (index + 1); i < s->count; ++i) {
            pk_arena_deinit(lisp, &s->e[i]);
        }
        lisp->arena = s->e[index];
        s->count = index;
    }
    lisp->arena.head = lisp->arena.e - savepoint;
}
