#include "pocket_internals.h"

void pk_frame_push(Pocket lisp, size_t arity) {
    pk_frame_push_mode(lisp, arity, PK_FRAME_MODE_NORMAL);
}

void pk_frame_push_mode(Pocket lisp, size_t arity, pk_u8 mode) {
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
        .mode = mode,
        .jmp = NULL,
    };
}

void pk_frame_unwind(Pocket lisp) {
    PKFrame unwind = lisp->frames.e[lisp->frames.count - 1];
    
    size_t stack_count = lisp->current_frame.stack_offset;
    size_t lets_unwind = lisp->lets.count - lisp->current_frame.let_offset;
    
    pk_env_let_unwind(lisp, lets_unwind);
    
    lisp->stack.count = stack_count;
    lisp->frames.count -= 1;
    lisp->current_frame = unwind;
}

void pk_frame_unwindn(Pocket lisp, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        (void)i;
        pk_frame_unwind(lisp);
    }
}

void pk_frame_clear(Pocket lisp) {
    lisp->stack.count = lisp->current_frame.stack_offset;
}

void pk_frame_return(Pocket lisp) {
    PKAtom *result = pk_stack_head(lisp);
    pk_frame_unwind(lisp);
    lisp->stack.e[lisp->stack.count - 1] = result;
}

void pk_frame_return_push(Pocket lisp) {
    PKAtom *result = pk_stack_head(lisp);
    pk_frame_unwind(lisp);
    pk_stack_push(lisp, result);
}

size_t pk_frame_length(Pocket lisp) {
    return lisp->stack.count - lisp->current_frame.stack_offset;
}
