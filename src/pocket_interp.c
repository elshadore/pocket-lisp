#include "pocket_internals.h"

void pk_fastcall(void *user_closure, Pocket lisp, PKFn fn, int arity) {
    if (arity < 0) {
        pk_error(lisp);
    }
    size_t uarity = (size_t)arity;
    pk_frame_push(lisp, uarity);
    (fn)(user_closure, lisp);
    
    PKAtom *result = lisp->cache.nil;
    if (pk_get_top(lisp) > 0) {
        result = lisp->stack.e[lisp->stack.count - 1];
    }
    pk_frame_pop(lisp);
    pk_push(lisp, result);
}

// void pk_funcall(Pocket lisp, int args) {
    
// }

void pk_frame_push(Pocket lisp, size_t arity) {
    size_t stack_offset = pk_stack_total(lisp) - arity;
    
    PKFrames *frames = &lisp->frames;
    if (frames->count >= frames->capacity) {
        size_t new_capacity = pk_grow_capacity(frames->capacity, PK_FRAMES_INIT_CAPACITY);
        PKFrame *new_e = pk_realloc(lisp, frames->e, frames->capacity * sizeof(PKFrame), new_capacity * sizeof(PKFrame));
        frames->e = new_e;
        frames->capacity = new_capacity;
    }
    frames->e[frames->count++] = lisp->current_frame;
    
    PKFrame frame = (PKFrame) {
        .stack_offset = stack_offset,
        .arity = arity,
    };
    lisp->current_frame = frame;
}

void pk_frame_pop(Pocket lisp) {
    PKFrames *frames = &lisp->frames;
    // if (lisp->frames.count == 0) {
    //     pk_error(lisp);
    // }
    PKFrame popped = frames->e[frames->count - 1];
    size_t diff = lisp->current_frame.stack_offset - popped.stack_offset;
    size_t new_total = popped.stack_offset + diff;
    lisp->stack.count = new_total;
    frames->count -= 1;
    lisp->current_frame = popped;
}
