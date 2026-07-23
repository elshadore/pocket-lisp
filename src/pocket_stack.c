#include "pocket_internals.h"

int pk_get_top(Pocket lisp) {
    return (int)pk_stack_length_frame(lisp);
}

PKRes pk_set_top(Pocket lisp, int new_top) {
    (void)new_top;
    return pk_error(lisp);
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
    size_t absolute = 0;
    if (top <= 0) {
        return pk_error(lisp);
    }

    absolute = (size_t)pk_sp_absolute(lisp, stack_pointer);
    if (absolute <= 0) {
        return pk_error(lisp);
    }

    *index = absolute + lisp->current_frame.stack_offset - 1;
    if (*index > lisp->stack.count) {
        return pk_error(lisp);
    }

    return PK_Ok;
}

PKRes pk_push(Pocket lisp, PKAtom *atom) {
    return pk_atoms_push(lisp, &lisp->stack, atom, PK_STACK_INIT_CAPACITY);
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
    if (pk_stack_length_frame(lisp) == 0) {
        return pk_error(lisp);
    }
    *output = lisp->stack.e[lisp->stack.count - 1];
    return PK_Ok;
}

PKAtom *pk_stack_result(Pocket lisp) {
    if (pk_stack_length_frame(lisp) == 0) {
        return pk_atom_nil(lisp);
    } else {
        return lisp->stack.e[lisp->stack.count - 1];
    }
}

PKRes pk_stack_pop(Pocket lisp, PKAtom **output) {
    size_t index = 0;
    
    if (pk_stack_length_frame(lisp) == 0) {
        return pk_error(lisp);
    }
    index = lisp->stack.count - 1;
    *output = lisp->stack.e[index];
    lisp->stack.count = index;
    return PK_Ok;
}

PKRes pk_stack_get(Pocket lisp, int stack_pointer, PKAtom **output) {
    size_t index = 0;
    
    if (stack_pointer == 0) {
        *output = pk_atom_nil(lisp);
        return PK_Ok;
    }
    pk_try(pk_sp_index(lisp, stack_pointer, &index));
    *output = lisp->stack.e[index];
    return PK_Ok;
}

PKAtomSlice pk_stack_slice(Pocket lisp) {
    size_t total = pk_stack_length_total(lisp);
    size_t offset = lisp->current_frame.stack_offset;
    size_t length = total - offset;
    PKAtom **e = lisp->stack.e + offset;
    PKAtomSlice result;
    
    result.e = e;
    result.length = length;
    
    return result;
}

PKRes pk_stack_slice_by(Pocket lisp, int a, int b, PKStackSlice *output) {
    size_t ia = 0;
    size_t ib = 0;
    size_t offset = 0;
    
    pk_try(pk_sp_index(lisp, a, &ia));
    pk_try(pk_sp_index(lisp, b, &ib));
    offset = lisp->current_frame.stack_offset;
    
    if (ia < ib) {
        output->order = PKOrder_Normal;
        output->slice.e = lisp->stack.e + offset + ia;
        output->slice.length = ib - ia + 1;
    } else {
        output->order = PKOrder_Reversed;
        output->slice.e = lisp->stack.e + offset + ib;
        output->slice.length = ia - ib + 1;
    }
    return PK_Ok;
}

PKRes pk_stack_set(Pocket lisp, int stack_pointer, PKAtom *atom) {
    size_t index = 0;
    pk_try(pk_sp_index(lisp, stack_pointer, &index));
    lisp->stack.e[index] = atom;
    return PK_Ok;
}

PKRes pk_frame_push(Pocket lisp, size_t arity) {
    PKFrames *frames = &lisp->frames;
    size_t stack_offset = pk_stack_length_total(lisp) - arity;
    size_t lets_offset = lisp->lets.count;
    
    if (frames->count >= frames->capacity) {
        size_t new_capacity = pk_grow_capacity(frames->capacity, PK_FRAMES_INIT_CAPACITY);
        pk_try(pk_realloc(
            lisp,
            frames->e,
            frames->capacity * sizeof(PKFrame),
            new_capacity * sizeof(PKFrame),
            (void **)&frames->e
        ));
        frames->capacity = new_capacity;
    }
    frames->e[frames->count++] = lisp->current_frame;

    lisp->current_frame.stack_offset = stack_offset;
    lisp->current_frame.lets_offset = lets_offset;
    lisp->current_frame.arity = arity;
    
    return PK_Ok;
}

PKRes pk_frame_pop_clear(Pocket lisp) {
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

PKRes pk_frame_pop_return(Pocket lisp) {
    PKFrames *frames = &lisp->frames;
    PKFrame popped = frames->e[frames->count - 1];

    size_t lets_pop = lisp->lets.count - lisp->current_frame.lets_offset;
    pk_try(pk_let_pop(lisp, lets_pop));

    frames->count -= 1;
    lisp->current_frame = popped;
    return PK_Ok;
}

PKAtomSlice pk_frame_slice(Pocket lisp, PKFrame *frame, size_t length) {
    size_t start = frame->stack_offset;
    PKAtomSlice result;
    
    result.e = lisp->stack.e + start;
    result.length = length;
    
    return result;
}

PKRes pk_let_push(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value) {
    PKLets *lets = &lisp->lets;
    PKAtom *restore = NULL;
    PKLet *let = NULL;
    
    if (lets->count >= lets->capacity) {
        size_t new_capacity = pk_grow_capacity(lets->capacity, PK_LET_INIT_CAPACITY);
        pk_try(pk_realloc(
            lisp,
            lets->e,
            lets->capacity * sizeof(PKLet),
            new_capacity * sizeof(PKLet),
            (void **)&lets->e
        ));
        lets->capacity = new_capacity;
    }
    pk_try(pk_env_set(lisp, ty, sym, value, &restore));

    let = &lets->e[lets->count++];
    
    let->ty = ty;
    let->symbol = sym;
    let->restore = restore;
    
    return PK_Ok;
}

PKRes pk_let_pop(Pocket lisp, size_t n) {
    PKLets *lets = &lisp->lets;
    size_t i = 0;
    for (i = 0; i < n; ++i) {
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
