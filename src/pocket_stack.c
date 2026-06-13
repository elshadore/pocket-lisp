#include "pocket_internals.h"

void pk_push(Pocket lisp, PKAtom *atom) {
    PKStack *stack = &lisp->stack;
    if (stack->count >= stack->capacity) {
        size_t new_capacity = pk_grow_capacity(stack->capacity, 128);
        PKAtom **new_e = (PKAtom **)pk_realloc(lisp, stack->e, stack->capacity * sizeof(PKAtom *), new_capacity * sizeof(PKAtom *));
        stack->capacity = new_capacity;
        stack->e = new_e;
    }
    stack->e[stack->count++] = atom;
}

void pk_pop(Pocket lisp) {
    if (lisp->stack.count == 0) {
        pk_error(lisp);
    }
    lisp->stack.count -= 1;
}
