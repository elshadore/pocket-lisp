#include "pocket_internals.h"

void pk_stack_push(Pocket lisp, PKAtom atom) {
    pk_array_push(&lisp->stack, atom, 128);
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
        return pk_errors(pk_str("stack underflow"));
    }
    PKAtom result = PK_CORE.stack.e[PK_CORE.stack.count - 1];
    PK_CORE.stack.count -= 1;
    return result;
}

void pk_stack_pop_unchecked(size_t n) {
    PK_CORE.stack.count -= n;
}

void pk_stack_reset_to(size_t count) {
    for (size_t i = PK_CORE.stack.count; i > count; --i) {
        pk_ref_dec(PK_CORE.stack.e[i - 1]);
    }
    PK_CORE.stack.count = count;
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
        return pk_errors(pk_str("stack slice too deep"));
    }
    
    size_t offset = slice.length - depth;
    *output = (PKAtoms) {
        .e = slice.e + offset,
        .length = depth,
    };
    
    return pk_true();
}

PKAtom pk_stack_op_finish(size_t depth, PKAtom result) {
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
    
    return pk_stack_op_finish(depth, result);
}

PKAtom pk_stack_op_list_packed(size_t depth) {
    if (depth == 0) {
        return pk_true();
    }
    PKAtoms slice = PK_ATOMS_EMPTY;
    pk_propagate(pk_stack_slice_down(depth, &slice));
    
    PKAtom result = pk_list_packed(slice);
    
    return pk_stack_op_finish(depth, result);
}

PKAtom pk_stack_op_list_merge(size_t depth) {
    if (depth == 0) {
        return pk_true();
    }
    PKAtoms slice = PK_ATOMS_EMPTY;
    pk_propagate(pk_stack_slice_down(depth, &slice));
    
    PKAtom result = pk_list_flatten(slice);
    
    return pk_stack_op_finish(depth, result);
}

PKAtom pk_stack_op_strcat(size_t depth) {
    if (depth == 0) {
        return pk_true();
    }
    PKAtoms slice = PK_ATOMS_EMPTY;
    pk_propagate(pk_stack_slice_down(depth, &slice));
    PKAtom result = pk_string_cat(slice);
    
    return pk_stack_op_finish(depth, result);
}

PKAtom pk_stack_op_dig(size_t depth) {
    PKAtoms slice = PK_ATOMS_EMPTY;
    pk_propagate(pk_stack_slice_down(depth + 1, &slice));
    
    return pk_stack_push(slice.e[0]);
}

PKAtom pk_stack_op_car(bool strict) {
    PKAtom top = pk_stack_head();
    PKAtom car = pk_nil();
    if (!pk_is_nil(top)) {
        car = pk_car(top);
        pk_propagate(car);
    } else if (strict) {
        return pk_errors(pk_str("not enough elements"));
    }
    
    pk_ref_dec(top);
    pk_stack_pop_unchecked(1);
    
    PKAtom result = pk_stack_push(car);
    pk_ref_dec(car);
    return result;
}

PKAtom pk_stack_op_cdr(bool strict) {
    PKAtom top = pk_stack_head();
    PKAtom cdr = pk_nil();
    if (!pk_is_nil(top)) {
        cdr = pk_cdr(top);
        pk_propagate(cdr);
    } else if (strict) {
        return pk_errors(pk_str("not enough elements"));
    }
    
    pk_ref_dec(top);
    pk_stack_pop_unchecked(1);
    
    PKAtom result = pk_stack_push(cdr);
    pk_ref_dec(cdr);
    return result;
}
