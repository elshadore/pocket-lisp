#include "pocket_internals.h"

int pk_get_top(Pocket lisp) {
    return (int)(pk_stack_total(lisp) - lisp->current_frame.stack_offset);
}

void pk_set_top(Pocket lisp, int new_top) {
    int absolute = pk_sp_absolute(lisp, new_top);
    if (absolute < 0) {
        pk_error(lisp);
    }
    size_t new_count = lisp->current_frame.stack_offset + (size_t)absolute;
    pk_stack_expand(lisp, new_count);
    lisp->stack.count = new_count;
}

int pk_sp_absolute(Pocket lisp, int stack_pointer) {
    int top = pk_get_top(lisp);
    if (stack_pointer >= 0) {
        return stack_pointer;
    } else {
        return top - (-stack_pointer - 1);
    }
}

int pk_sp_relative(Pocket lisp, int stack_pointer) {
    int top = pk_get_top(lisp);
    if (stack_pointer <= 0) {
        return stack_pointer;
    } else {
        return -(top - (stack_pointer - 1));
    }
}

size_t pk_sp_index(Pocket lisp, int stack_pointer) {
    int top = pk_get_top(lisp);
    if (top <= 0) {
        pk_error(lisp);
    }

    size_t absolute = (size_t)pk_sp_absolute(lisp, stack_pointer);
    if (absolute <= 0) {
        pk_error(lisp);
    }

    size_t index = absolute + lisp->current_frame.stack_offset - 1;
    if (index > lisp->stack.count) {
        pk_error(lisp);
    }

    return index;
}

void pk_stack_expand(Pocket lisp, size_t total) {
    if (total <= lisp->stack.capacity) return;

    size_t new_capacity = pk_grow_capacity(lisp->stack.capacity, PK_STACK_INIT_CAPACITY);
    while (new_capacity < total) {
        new_capacity = pk_grow_capacity(new_capacity, PK_STACK_INIT_CAPACITY);
    }
    PKAtom **new_e = pk_realloc(lisp, lisp->stack.e,
        lisp->stack.capacity * sizeof(PKAtom *),
        new_capacity * sizeof(PKAtom *));
    lisp->stack.e = new_e;
    lisp->stack.capacity = new_capacity;

    for (size_t i = lisp->stack.count; i < new_capacity; i++) {
        lisp->stack.e[i] = lisp->cache.nil;
    }
}

void pk_push(Pocket lisp, PKAtom *atom) {
    pk_stack_expand(lisp, lisp->stack.count + 1);
    lisp->stack.e[lisp->stack.count] = atom;
    lisp->stack.count++;
}

void pk_pop(Pocket lisp) {
    if (lisp->stack.count == 0) {
        pk_error(lisp);
    }
    lisp->stack.count -= 1;
}

void pk_popn(Pocket lisp, int n) {
    if (n < 0) {
        pk_error(lisp);
    }
    if (n > pk_get_top(lisp)) {
        pk_error(lisp);
    }
    lisp->stack.count -= (size_t)n;
}

PKAtom *pk_stack_get(Pocket lisp, int stack_pointer) {
    if (stack_pointer == 0) return lisp->cache.nil;
    return lisp->stack.e[pk_sp_index(lisp, stack_pointer)];
}

void pk_stack_set(Pocket lisp, int stack_pointer, PKAtom *atom) {
    size_t index = pk_sp_index(lisp, stack_pointer);
    lisp->stack.e[index] = atom;
}

size_t pk_stack_total(Pocket lisp) {
    return lisp->stack.count;
}

