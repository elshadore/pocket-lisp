#include "pocket_internals.h"

int pk_top(Pocket lisp) {
    return (int)pk_frame_length(lisp);
}

void pk_set_top(Pocket lisp, int stack_pointer) {
    int absolute = pk_sp_absolute(lisp, stack_pointer);

    if (absolute <= 0) {
        pk_errors(lisp, pk_str("stack underflow"));
        return;
    }

    size_t count = lisp->current_frame.stack_offset + (size_t)absolute;

    while (lisp->stack.count < count) {
        pk_array_push(lisp, &lisp->stack, lisp->cache.nil, 128);
    }

    lisp->stack.count = count;
}

int pk_sp_absolute(Pocket lisp, int stack_pointer) {
    int top = pk_top(lisp);
    if (stack_pointer >= 0) {
        return stack_pointer;
    } else {
        return top - (-stack_pointer - 1);
    }
}

int pk_sp_relative(Pocket lisp, int stack_pointer) {
    int top = pk_top(lisp);
    if (stack_pointer <= 0) {
        return stack_pointer;
    } else {
        return -(top - (stack_pointer - 1));
    }
}

size_t pk_sp_index(Pocket lisp, int stack_pointer) {
    int top = pk_top(lisp);
    if (top <= 0) {
        pk_errorf(lisp, "pk_sp_index: stack is empty, cannot resolve stack pointer %d", stack_pointer);
        return 0;
    }

    int absolute = pk_sp_absolute(lisp, stack_pointer);
    if (absolute <= 0) {
        pk_errorf(lisp, "pk_sp_index: stack pointer %d resolves outside the frame (top is %d)", stack_pointer, top);
        return 0;
    }

    size_t index = (size_t)absolute + lisp->current_frame.stack_offset - 1;
    if (index >= lisp->stack.count) {
        pk_errorf(lisp, "pk_sp_index: stack pointer %d resolves to index %zu, past stack count %zu",
            stack_pointer, index, lisp->stack.count);
        return 0;
    }

    return index;
}

void pk_pop(Pocket lisp){
    (void)pk_stack_pop(lisp);
}

void pk_pop2(Pocket lisp){
    (void)pk_stack_pop(lisp);
    (void)pk_stack_pop(lisp);
}

void pk_pop3(Pocket lisp){
    (void)pk_stack_pop(lisp);
    (void)pk_stack_pop(lisp);
    (void)pk_stack_pop(lisp);
}

void pk_popn(Pocket lisp, size_t n){
    for (size_t i = 0; i < n; ++i) {
        (void)pk_stack_pop(lisp);
    }
}

void pk_swap(Pocket lisp, int a, int b){
    size_t ia = pk_sp_index(lisp, a);
    size_t ib = pk_sp_index(lisp, b);
    PKAtom *temp = lisp->stack.e[ia];
    lisp->stack.e[ia] = lisp->stack.e[ib];
    lisp->stack.e[ib] = temp;
}

void pk_dig(Pocket lisp, int stack_pointer){
    pk_stack_push(lisp, pk_stack_get(lisp, stack_pointer));
}

void pk_dig_to(Pocket lisp, int from, int to){
    size_t ia = pk_sp_index(lisp, from);
    size_t ib = pk_sp_index(lisp, to);
    lisp->stack.e[ib] = lisp->stack.e[ia];
}

void pk_swap_pop2(Pocket lisp){
    PKAtoms slice = pk_stack_slice_down(lisp, 2);
    slice.e[1] = slice.e[0];
    pk_stack_pop_unchecked(lisp, 1);
}

void pk_swap_pop3(Pocket lisp){
    PKAtoms slice = pk_stack_slice_down(lisp, 3);
    slice.e[2] = slice.e[0];
    pk_stack_pop_unchecked(lisp, 2);
}

void pk_stack_push(Pocket lisp, PKAtom *atom) {
    pk_array_push(lisp, &lisp->stack, atom, 128);
}

PKAtom *pk_stack_get(Pocket lisp, int stack_pointer) {
    if (stack_pointer == 0) {
        return lisp->cache.nil;
    } else {
        return lisp->stack.e[pk_sp_index(lisp, stack_pointer)];
    }
    
}

PKAtom *pk_stack_set(Pocket lisp, int stack_pointer, PKAtom *value) {
    size_t index = pk_sp_index(lisp, stack_pointer);
    PKAtom *result = lisp->stack.e[index];
    lisp->stack.e[index] = value;
    return result;
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
        pk_errorf(lisp, "pk_stack_head_strict: stack underflow, frame holds %zu values", pk_frame_length(lisp));
        return lisp->cache.nil;
    } else {
        return lisp->stack.e[lisp->stack.count - 1];
    }
}

PKAtom *pk_stack_pop(Pocket lisp) {
    if (pk_frame_length(lisp) == 0) {
        pk_errorf(lisp, "pk_stack_pop: stack underflow, frame holds %zu values", pk_frame_length(lisp));
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
    if (offset > lisp->stack.count) {
        return PK_ATOMS_EMPTY;
    }
    return (PKAtoms) {
        .e = lisp->stack.e + offset,
        .length = lisp->stack.count - offset,
    };
}

PKAtoms pk_stack_slice_down(Pocket lisp, size_t depth) {
    PKAtoms slice = pk_stack_slice(lisp);
    
    if (depth > slice.length) {
        pk_errorf(lisp, "pk_stack_slice_down: wanted %zu values, frame only holds %zu", depth, slice.length);
        return PK_ATOMS_EMPTY;
    }
    
    size_t offset = slice.length - depth;
    return (PKAtoms) {
        .e = slice.e + offset,
        .length = depth,
    };
}

void pk_stack_op_dig(Pocket lisp, size_t index) {
    PKAtoms slice = pk_stack_slice_down(lisp, index + 1);
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

void pk_stack_op_list_rev(Pocket lisp, size_t depth) {
    if (depth == 0) return;
    
    PKAtoms slice = pk_stack_slice_down(lisp, depth);
    PKAtom *result = pk_atom_list_rev(lisp, slice);
    
    pk_stack_op_finish(lisp, result, depth);
}

void pk_stack_op_list_packed(Pocket lisp, size_t depth) {
    if (depth == 0) return;
    
    PKAtoms slice = pk_stack_slice_down(lisp, depth);
    
    PKAtom *result = pk_atom_list_packed(lisp, slice);
    
    pk_stack_op_finish(lisp, result, depth);
}

void pk_stack_op_list_rev_packed(Pocket lisp, size_t depth) {
    if (depth == 0) return;
    
    PKAtoms slice = pk_stack_slice_down(lisp, depth);
    
    PKAtom *result = pk_atom_list_rev_packed(lisp, slice);
    
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

