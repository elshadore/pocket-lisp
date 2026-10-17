#include "pocket_internals.h"

void pk_stack_push(Pocket lisp, PKAtom *atom) {
    pk_array_push(lisp, &lisp->stack, atom, 128);
}

PKAtom *pk_stack_head(Pocket lisp) {
    if (pk_frame_length(lisp) == 0) {
        return lisp->cache.nil;
    } else {
        return lisp->stack.e[lisp->stack.count - 1];
    }
}

PKAtom *pk_stack_head_strict(Pocket lisp) {
    if (pk_frame_length(lisp) == 0) {
        pk_error(lisp);
        return lisp->cache.nil;
    } else {
        return lisp->stack.e[lisp->stack.count - 1];
    }
}

PKAtom *pk_stack_pop(Pocket lisp) {
    if (pk_frame_length(lisp) == 0) {
        pk_error(lisp);
        return lisp->cache.nil;
    }
    PKAtom *result = lisp->stack.e[lisp->stack.count - 1];
    lisp->stack.count -= 1;
    return result;
}

void pk_stack_pop_unchecked(Pocket lisp, size_t n) {
    lisp->stack.count -= n;
}

void pk_stack_set_unchecked(Pocket lisp, size_t count) {
    lisp->stack.count = count;
}

PKAtoms pk_stack_slice(Pocket lisp) {
    size_t offset = lisp->current_frame.stack_offset;
    return (PKAtoms) {
        .e = lisp->stack.e + offset,
        .length = lisp->stack.count - offset,
    };
}

PKAtoms pk_stack_slice_down(Pocket lisp, size_t depth) {
    PKAtoms slice = pk_stack_slice(lisp);
    
    if (depth > slice.length) {
        pk_error(lisp);
        return PK_ATOMS_EMPTY;
    }
    
    size_t offset = slice.length - depth;
    return (PKAtoms) {
        .e = slice.e + offset,
        .length = depth,
    };
}

void pk_stack_op_dig(Pocket lisp, size_t index) {
    PKAtoms slice = pk_stack_slice_down(lisp, index);
    return pk_stack_push(lisp, slice.e[0]);
}

void pk_stack_op_car(Pocket lisp) {
    PKAtom *atom = pk_stack_head_strict(lisp);
    PKAtom *result = pk_atom_car(lisp, atom);
    
    pk_stack_pop_unchecked(lisp, 1);
    
    pk_stack_push(lisp, result);
}

void pk_stack_op_cdr(Pocket lisp) {
    PKAtom *atom = pk_stack_head_strict(lisp);
    PKAtom *result = pk_atom_cdr(lisp, atom);
    
    pk_stack_pop_unchecked(lisp, 1);
    
    pk_stack_push(lisp, result);
}

void pk_stack_op_car_strict(Pocket lisp) {
    PKAtom *atom = pk_stack_head_strict(lisp);
    PKAtom *result = pk_atom_car_strict(lisp, atom);
    
    pk_stack_pop_unchecked(lisp, 1);
    
    pk_stack_push(lisp, result);
}

void pk_stack_op_cdr_strict(Pocket lisp) {
    PKAtom *atom = pk_stack_head_strict(lisp);
    PKAtom *result = pk_atom_cdr_strict(lisp, atom);
    
    pk_stack_pop_unchecked(lisp, 1);
    
    pk_stack_push(lisp, result);
}

void pk_stack_op_finish(Pocket lisp, PKAtom *result, size_t depth) {
    pk_stack_pop_unchecked(lisp, depth - 1);
    lisp->stack.e[lisp->stack.count - 1] = result;
}

void pk_stack_op_list(Pocket lisp, size_t depth) {
    if (depth == 0) return;
    
    PKAtoms slice = pk_stack_slice_down(lisp, depth);
    PKAtom *result = pk_atom_list(lisp, slice);
    
    pk_stack_op_finish(lisp, result, depth);
}

void pk_stack_op_list_packed(Pocket lisp, size_t depth) {
    if (depth == 0) return;
    
    PKAtoms slice = pk_stack_slice_down(lisp, depth);
    
    PKAtom *result = pk_atom_list_packed(lisp, slice);
    
    pk_stack_op_finish(lisp, result, depth);
}

void pk_stack_op_list_merge(Pocket lisp, size_t depth) {
    if (depth == 0) return;
    
    PKAtoms slice = pk_stack_slice_down(lisp, depth);
    
    PKAtom *result = pk_atom_list_flatten(lisp, slice);
    
    pk_stack_op_finish(lisp, result, depth);
}

void pk_stack_op_strcat(Pocket lisp, size_t depth) {
    if (depth == 0) return;
    
    PKAtoms slice = pk_stack_slice_down(lisp, depth);
    
    PKAtomString *result = pk_atom_string_cat(lisp, slice);
    
    pk_stack_op_finish(lisp, (PKAtom *)result, depth);
}

