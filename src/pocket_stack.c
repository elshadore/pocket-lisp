#include "pocket_internals.h"

PKAtom *pk_get_result(Pocket lisp) {
    if (pk_get_top(lisp) > 0) {
        return lisp->stack.e[lisp->stack.count - 1];
    } else {
        return lisp->cache.nil;;
    }
}

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
    (void)pk_stack_pop(lisp);
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

PKAtom *pk_stack_pop(Pocket lisp) {
    if (lisp->stack.count == 0) {
        pk_error(lisp);
    }
    size_t index = lisp->stack.count - 1;
    PKAtom *popped = lisp->stack.e[index];
    lisp->stack.count = index;
    return popped;
}

PKAtom *pk_stack_get(Pocket lisp, int stack_pointer) {
    if (stack_pointer == 0) return lisp->cache.nil;
    return lisp->stack.e[pk_sp_index(lisp, stack_pointer)];
}

PKAtoms pk_stack_slice(Pocket lisp) {
    size_t total = pk_stack_total(lisp);
    size_t offset = lisp->current_frame.stack_offset;
    size_t length = total - offset;
    PKAtom **e = lisp->stack.e + offset;
    return (PKAtoms){.e = e, .length = length};
}

void pk_stack_set(Pocket lisp, int stack_pointer, PKAtom *atom) {
    size_t index = pk_sp_index(lisp, stack_pointer);
    lisp->stack.e[index] = atom;
}

size_t pk_stack_total(Pocket lisp) {
    return lisp->stack.count;
}

void pk_frame_push(Pocket lisp, size_t arity) {
    size_t stack_offset = pk_stack_total(lisp) - arity;
    size_t lets_offset = lisp->lets.count;
    
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
        .lets_offset = lets_offset,
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
    
    size_t lets_pop = lisp->lets.count - lisp->current_frame.lets_offset;
    pk_let_pop(lisp, lets_pop);
    
    lisp->stack.count = new_total;
    frames->count -= 1;
    lisp->current_frame = popped;
}

void pk_frame_clear(Pocket lisp) {
    lisp->stack.count = lisp->current_frame.stack_offset;
}

void pk_let_push(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value) {
    PKLets *lets = &lisp->lets;
    if (lets->count >= lets->capacity) {
        size_t new_capacity = pk_grow_capacity(lets->capacity, PK_LET_INIT_CAPACITY);
        PKLet *new_e = pk_realloc(lisp, lets->e, lets->capacity * sizeof(PKLet), new_capacity * sizeof(PKLet));
        lets->e = new_e;
        lets->capacity = new_capacity;
    }
    PKAtom *restore = pk_env_set(lisp, ty, sym, value);
    lets->e[lets->count++] = (PKLet) {
        .ty = ty,
        .symbol = sym,
        .restore = restore,
    };
}

void pk_let_pop(Pocket lisp, size_t n) {
    PKLets *lets = &lisp->lets;
    for (size_t i = 0; i < n; ++i) {
        size_t index = lets->count - i - 1;
        PKLet popped = lets->e[index];
        if (popped.restore != NULL) {
            pk_env_set(lisp, popped.ty, popped.symbol, popped.restore);
        } else {
            pk_env_unbind(lisp, popped.ty, popped.symbol);
        }
    }
    lets->count -= n;
}
