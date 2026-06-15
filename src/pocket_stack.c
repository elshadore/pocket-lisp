#include "pocket_internals.h"

int pk_get_top(Pocket lisp) {
    return (int)(pk_stack_total(lisp) - lisp->current_frame.stack_offset);
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

void pk_push(Pocket lisp, PKAtom *atom) {
    PKStack *stack = &lisp->stack;
    if (stack->count >= stack->capacity) {
        size_t new_capacity = pk_grow_capacity(stack->capacity, PK_STACK_INIT_CAPACITY);
        PKAtom **new_e = (PKAtom **)pk_realloc(lisp, stack->e, stack->capacity * sizeof(PKAtom *), new_capacity * sizeof(PKAtom *));
        stack->capacity = new_capacity;
        stack->e = new_e;
    }
    
    size_t index = stack->count;
    stack->e[index] = atom;
    size_t grow = stack->count + 1;
    
    if (grow > INT_MAX) {
        pk_error(lisp);
    }
    stack->count = grow;
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
    return lisp->stack.e[pk_sp_index(lisp, stack_pointer)];
}

void pk_stack_set(Pocket lisp, int stack_pointer, PKAtom *atom) {
    size_t index = pk_sp_index(lisp, stack_pointer);
    lisp->stack.e[index] = atom;
}

size_t pk_stack_total(Pocket lisp) {
    return lisp->stack.count;
}

