#include "pocket_internals.h"

PKAtom pk_frame_push(size_t arity) {
    if (PK_CORE.frames.count >= PK_CORE.frames.capacity) {
        size_t new_capacity = pk_grow_capacity(PK_CORE.frames.capacity, 16);
        PKFrame *new_e = pk_realloc(PK_CORE.frames.e, new_capacity * sizeof(PKFrame));
        if (new_e == NULL) {
            return pk_error_oom();
        }
        PK_CORE.frames.e = new_e;
        PK_CORE.frames.capacity = new_capacity;
    }
    
    size_t stack_offset = PK_CORE.stack.count - arity;
    size_t lets_offset = PK_CORE.lets.count;
    
    PK_CORE.frames.e[PK_CORE.frames.count++] = PK_CORE.current_frame;

    PK_CORE.current_frame = (PKFrame) {
        .stack_offset = stack_offset,
        .let_offset = lets_offset,
    };

    return pk_true();
}

PKAtom pk_frame_pushc(PKAtoms atoms) {
    pk_propagate(pk_frame_push(0));
    for (size_t i = 0; i < atoms.length; ++i) {
        PKAtom atom = pk_stack_push(atoms.e[i]);
        if (pk_is_error(atom)) {
            pk_frame_unwind();
            return atom;
        }
    }
    return pk_true();
}
    
void pk_frame_unwind(void) {
    PKFrame unwind = PK_CORE.frames.e[PK_CORE.frames.count - 1];
    size_t stack_count = PK_CORE.current_frame.stack_offset;
    size_t diff = PK_CORE.stack.count - stack_count;
    size_t lets_unwind = PK_CORE.lets.count - PK_CORE.current_frame.let_offset;
    pk_let_unwind(lets_unwind);
    
    for (size_t i = 0; i < diff; ++i) {
        size_t index = PK_CORE.stack.count - i - 1;
        pk_ref_dec(PK_CORE.stack.e[index]);
    }
    
    PK_CORE.stack.count = stack_count;
    PK_CORE.frames.count -= 1;
    PK_CORE.current_frame = unwind;
}

void pk_frame_clear(void) {
    size_t diff = PK_CORE.stack.count - PK_CORE.current_frame.stack_offset;
    for (size_t i = 0; i < diff; ++i) {
        size_t index = PK_CORE.stack.count - i - 1;
        pk_ref_dec(PK_CORE.stack.e[index]);
    }
    PK_CORE.stack.count = PK_CORE.current_frame.stack_offset;
}

PKAtom pk_stack_push(PKAtom atom) {
    pk_propagate(pk_array_push(&PK_CORE.stack, atom, 128));
    
    PKAtom result = pk_ref_inc(atom);
    if (pk_is_error(result)) {
        PK_CORE.stack.count -= 1;
    }
    return result;
}

PKAtom pk_stack_head() {
    if (pk_frame_length() == 0) {
        return pk_nil();
    } else {
        return PK_CORE.stack.e[PK_CORE.stack.count - 1];
    }
}

PKAtom pk_stack_pop() {
    if (pk_frame_length() == 0) {
        return pk_error();
    }
    PKAtom result = PK_CORE.stack.e[PK_CORE.stack.count - 1];
    PK_CORE.stack.count -= 1;
    return result;
}

void pk_stack_pop_unchecked(size_t n) {
    PK_CORE.stack.count -= n;
}

PKAtoms pk_stack_slice() {
    size_t offset = PK_CORE.current_frame.stack_offset;
    return (PKAtoms) {
        .e = PK_CORE.stack.e + offset,
        .length = PK_CORE.stack.count - offset,
    };
}

PKAtom pk_stack_slice_down(size_t depth, PKAtoms *output) {
    PKAtoms slice = pk_stack_slice();
    
    if (depth > slice.length) {
        return pk_error();
    }
    
    size_t offset = slice.length - depth;
    *output = (PKAtoms) {
        .e = slice.e + offset,
        .length = depth,
    };
    
    return pk_true();
}

// Collapses the top `depth` stack slots into `result`, which becomes the sole
// slot left behind. The consumed slots each owned a reference.
static PKAtom pk_stack_op_finish(size_t depth, PKAtom result) {
    PKAtom inc = pk_ref_inc(result);
    pk_propagate(inc);

    pk_stack_pop_unchecked(depth - 1);
    for (size_t i = 0; i < depth; ++i) {
        size_t index = PK_CORE.stack.count - 1 + i;
        pk_ref_dec(PK_CORE.stack.e[index]);
    }
    PK_CORE.stack.e[PK_CORE.stack.count - 1] = result;

    return pk_true();
}

PKAtom pk_stack_op_list(size_t depth) {
    if (depth == 0) {
        return pk_true();
    }
    PKAtoms slice = PK_ATOMS_EMPTY;
    pk_propagate(pk_stack_slice_down(depth, &slice));
    
    PKAtom result = pk_list(slice);
    if (pk_is_error(result)) {
        return result;
    }
    
    return pk_stack_op_finish(depth, result);
}

PKAtom pk_stack_op_list_packed(size_t depth) {
    if (depth == 0) {
        return pk_true();
    }
    PKAtoms slice = PK_ATOMS_EMPTY;
    pk_propagate(pk_stack_slice_down(depth, &slice));
    
    PKAtom result = pk_list_packed(slice);
    if (pk_is_error(result)) {
        return result;
    }
    
    return pk_stack_op_finish(depth, result);
}

PKAtom pk_stack_op_list_merge(size_t depth) {
    if (depth == 0) {
        return pk_true();
    }
    PKAtoms slice = PK_ATOMS_EMPTY;
    pk_propagate(pk_stack_slice_down(depth, &slice));
    
    PKAtom result = pk_list_flatten(slice);
    if (pk_is_error(result)) {
        return result;
    }
    
    return pk_stack_op_finish(depth, result);
}

PKAtom pk_stack_op_strcat(size_t depth) {
    if (depth == 0) {
        return pk_true();
    }
    PKAtoms slice = PK_ATOMS_EMPTY;
    pk_propagate(pk_stack_slice_down(depth, &slice));
    PKAtom result = pk_string_cat(slice);
    pk_propagate(result);
    
    return pk_stack_op_finish(depth, result);
}
