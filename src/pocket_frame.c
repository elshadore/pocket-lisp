#include "pocket_internals.h"

void pk_frame_push(Pocket lisp, size_t arity) {
    if (lisp->frames.count >= lisp->frames.capacity) {
        size_t new_capacity = pk_grow_capacity(lisp->frames.capacity, 16);
        PKFrame *new_e = pk_realloc(lisp, lisp->frames.e, new_capacity * sizeof(PKFrame));
        lisp->frames.e = new_e;
        lisp->frames.capacity = new_capacity;
    }
    
    size_t stack_offset = lisp->stack.count - arity;
    size_t lets_offset = lisp->lets.count;
    
    lisp->frames.e[lisp->frames.count++] = lisp->current_frame;

    lisp->current_frame = (PKFrame) {
        .stack_offset = stack_offset,
        .let_offset = lets_offset,
    };
}

void pk_frame_unwind(Pocket lisp) {
    PKFrame unwind = lisp->frames.e[lisp->frames.count - 1];
    size_t stack_count = lisp->current_frame.stack_offset;
    size_t diff = lisp->stack.count - stack_count;
    size_t lets_unwind = lisp->lets.count - lisp->current_frame.let_offset;
    pk_let_unwind(lets_unwind);
    
    for (size_t i = 0; i < diff; ++i) {
        size_t index = lisp->stack.count - i - 1;
        pk_ref_dec(lisp->stack.e[index]);
    }
    
    lisp->stack.count = stack_count;
    lisp->frames.count -= 1;
    lisp->current_frame = unwind;
}

void pk_frame_clear(Pocket lisp) {
    size_t diff = lisp->stack.count - lisp->current_frame.stack_offset;
    for (size_t i = 0; i < diff; ++i) {
        size_t index = lisp->stack.count - i - 1;
        pk_ref_dec(lisp->stack.e[index]);
    }
    lisp->stack.count = lisp->current_frame.stack_offset;
}

size_t pk_frame_length(Pocket lisp) {
    return lisp->stack.count - lisp->current_frame.stack_offset;
}
