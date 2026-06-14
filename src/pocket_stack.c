#include "pocket_internals.h"

int pk_get_top(Pocket lisp) {
    return (int)lisp->stack.count;
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

    size_t index = absolute - 1;
    if (index > lisp->stack.count) {
        pk_error(lisp);
    }

    return index;
}

void pk_push(Pocket lisp, PKAtom *atom) {
    PKStack *stack = &lisp->stack;
    if (stack->count >= stack->capacity) {
        size_t new_capacity = pk_grow_capacity(stack->capacity, 128);
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

PKAtom *pk_stack_get(Pocket lisp, int stack_pointer) {
    return lisp->stack.e[pk_sp_index(lisp, stack_pointer)];
}
