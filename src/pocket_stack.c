#include "pocket_internals.h"

int pk_get_top(Pocket lisp) {
    return (int)pk_frame_length(lisp);
}

PKRes pk_set_top(Pocket lisp, int new_top) {
    int absolute = pk_sp_absolute(lisp, new_top);
    if (absolute < 0) {
        return pk_error(lisp);
    }
    size_t new_count = lisp->current_frame.stack_offset + (size_t)absolute;
    pk_try(pk_stack_expand(lisp, new_count));
    lisp->stack.count = new_count;
    return PK_Ok;
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

PKRes pk_sp_index(Pocket lisp, int stack_pointer, size_t *index) {
    int top = pk_get_top(lisp);
    if (top <= 0) {
        return pk_error(lisp);
    }

    size_t absolute = (size_t)pk_sp_absolute(lisp, stack_pointer);
    if (absolute <= 0) {
        return pk_error(lisp);
    }

    *index = absolute + lisp->current_frame.stack_offset - 1;
    if (*index > lisp->stack.count) {
        return pk_error(lisp);
    }

    return PK_Ok;
}

PKRes pk_stack_expand(Pocket lisp, size_t total) {
    if (total <= lisp->stack.capacity) return PK_Ok;

    size_t new_capacity = pk_grow_capacity(lisp->stack.capacity, PK_STACK_INIT_CAPACITY);
    while (new_capacity < total) {
        new_capacity = pk_grow_capacity(new_capacity, PK_STACK_INIT_CAPACITY);
    }
    void *new_e;
    pk_try(pk_realloc(lisp, lisp->stack.e,
        lisp->stack.capacity * sizeof(PKAtom *),
        new_capacity * sizeof(PKAtom *), &new_e));
    lisp->stack.e = new_e;
    lisp->stack.capacity = new_capacity;

    for (size_t i = lisp->stack.count; i < new_capacity; i++) {
        lisp->stack.e[i] = pk_atom_nil(lisp);
    }
    return PK_Ok;
}

PKRes pk_push(Pocket lisp, PKAtom *atom) {
    pk_try(pk_stack_expand(lisp, lisp->stack.count + 1));
    lisp->stack.e[lisp->stack.count] = atom;
    lisp->stack.count++;
    return PK_Ok;
}

PKRes pk_pop(Pocket lisp) {
    PKAtom *popped;
    pk_try(pk_stack_pop(lisp, &popped));
    (void)popped;
    return PK_Ok;
}

PKRes pk_popn(Pocket lisp, int n) {
    if (n < 0) {
        return pk_error(lisp);
    }
    if (n > pk_get_top(lisp)) {
        return pk_error(lisp);
    }
    lisp->stack.count -= (size_t)n;
    return PK_Ok;
}

PKRes pk_stack_head(Pocket lisp, PKAtom **output) {
    if (pk_frame_length(lisp) == 0) {
        return pk_error(lisp);
    }
    *output = lisp->stack.e[lisp->stack.count - 1];
    return PK_Ok;
}

PKRes pk_stack_pop(Pocket lisp, PKAtom **output) {
    if (pk_frame_length(lisp) == 0) {
        return pk_error(lisp);
    }
    size_t index = lisp->stack.count - 1;
    *output = lisp->stack.e[index];
    lisp->stack.count = index;
    return PK_Ok;
}

PKRes pk_stack_get(Pocket lisp, int stack_pointer, PKAtom **output) {
    if (stack_pointer == 0) {
        *output = pk_atom_nil(lisp);
        return PK_Ok;
    }
    size_t index;
    pk_try(pk_sp_index(lisp, stack_pointer, &index));
    *output = lisp->stack.e[index];
    return PK_Ok;
}

PKAtoms pk_stack_slice(Pocket lisp) {
    size_t total = pk_stack_total(lisp);
    size_t offset = lisp->current_frame.stack_offset;
    size_t length = total - offset;
    PKAtom **e = lisp->stack.e + offset;
    return (PKAtoms){.e = e, .length = length};
}

PKRes pk_stack_slice_by(Pocket lisp, int a, int b, PKStackSlice *output) {
    size_t ia, ib;
    pk_try(pk_sp_index(lisp, a, &ia));
    pk_try(pk_sp_index(lisp, b, &ib));
    size_t offset = lisp->current_frame.stack_offset;
    if (ia < ib) {
        *output = (PKStackSlice) {
            .order = PKOrder_Normal,
            .slice = (PKAtoms) {
                .e = lisp->stack.e + offset + a,
                .length = b - a,
            },
        };
    } else {
        *output = (PKStackSlice) {
            .order = PKOrder_Reversed,
            .slice = (PKAtoms) {
                .e = lisp->stack.e + offset + b,
                .length = a - b,
            },
        };
    }
    return PK_Ok;
}

PKRes pk_stack_set(Pocket lisp, int stack_pointer, PKAtom *atom) {
    size_t index;
    pk_try(pk_sp_index(lisp, stack_pointer, &index));
    lisp->stack.e[index] = atom;
    return PK_Ok;
}

size_t pk_stack_total(Pocket lisp) {
    return lisp->stack.count;
}

PKRes pk_frame_push(Pocket lisp, size_t arity, PKFuncMode mode) {
    size_t stack_offset = pk_stack_total(lisp) - arity;
    size_t lets_offset = lisp->lets.count;

    PKFrames *frames = &lisp->frames;
    if (frames->count >= frames->capacity) {
        size_t new_capacity = pk_grow_capacity(frames->capacity, PK_FRAMES_INIT_CAPACITY);
        void *new_e;
        pk_try(pk_realloc(lisp, frames->e, frames->capacity * sizeof(PKFrame), new_capacity * sizeof(PKFrame), &new_e));
        frames->e = new_e;
        frames->capacity = new_capacity;
    }
    frames->e[frames->count++] = lisp->current_frame;

    PKFrame frame = (PKFrame) {
        .stack_offset = stack_offset,
        .arity = arity,
        .lets_offset = lets_offset,
        .mode = mode,
    };
    lisp->current_frame = frame;
    return PK_Ok;
}

PKRes pk_frame_pop(Pocket lisp) {
    PKFrames *frames = &lisp->frames;
    PKFrame popped = frames->e[frames->count - 1];
    size_t diff = lisp->current_frame.stack_offset - popped.stack_offset;
    size_t new_total = popped.stack_offset + diff;

    size_t lets_pop = lisp->lets.count - lisp->current_frame.lets_offset;
    pk_try(pk_let_pop(lisp, lets_pop));

    lisp->stack.count = new_total;
    frames->count -= 1;
    lisp->current_frame = popped;
    return PK_Ok;
}

PKRes pk_frame_clear(Pocket lisp) {
    lisp->stack.count = lisp->current_frame.stack_offset;
    return PK_Ok;
}

size_t pk_frame_length(Pocket lisp) {
    return pk_stack_total(lisp) - lisp->current_frame.stack_offset;
}

PKRes pk_let_push(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value) {
    PKLets *lets = &lisp->lets;
    if (lets->count >= lets->capacity) {
        size_t new_capacity = pk_grow_capacity(lets->capacity, PK_LET_INIT_CAPACITY);
        void *new_e;
        pk_try(pk_realloc(lisp, lets->e, lets->capacity * sizeof(PKLet), new_capacity * sizeof(PKLet), &new_e));
        lets->e = new_e;
        lets->capacity = new_capacity;
    }
    PKAtom *restore;
    pk_try(pk_env_set(lisp, ty, sym, value, &restore));
    lets->e[lets->count++] = (PKLet) {
        .ty = ty,
        .symbol = sym,
        .restore = restore,
    };
    return PK_Ok;
}

PKRes pk_let_pop(Pocket lisp, size_t n) {
    PKLets *lets = &lisp->lets;
    for (size_t i = 0; i < n; ++i) {
        size_t index = lets->count - i - 1;
        PKLet popped = lets->e[index];
        if (popped.restore != NULL) {
            PKAtom *_ignored;
            pk_try(pk_env_set(lisp, popped.ty, popped.symbol, popped.restore, &_ignored));
        } else {
            PKAtom *_ignored;
            pk_try(pk_env_unbind(lisp, popped.ty, popped.symbol, &_ignored));
        }
    }
    lets->count -= n;
    return PK_Ok;
}
