#include "pocket_internals.h"

void pk_generic_append(Pocket lisp, PKAtom *array, PKAtom *atom) {
    pk_stack_push(lisp, array);
    pk_stack_push(lisp, atom);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("append")));
    pk_funcall(lisp, PK_CALL_FUN, 2);
    pk_pop(lisp);
}

PKAtom *pk_generic_el(Pocket lisp, PKAtom *array, size_t index) {
    pk_stack_push(lisp, array);
    pk_stack_push(lisp, (PKAtom *)pk_atom_size_t(lisp, index));
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("el")));
    pk_funcall(lisp, PK_CALL_FUN, 2);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_length_atom(Pocket lisp, PKAtom *seq) {
    pk_stack_push(lisp, seq);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("length")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}
    
size_t pk_generic_length(Pocket lisp, PKAtom *seq) {
    return pk_atom_to_size_t(lisp, pk_generic_length_atom(lisp, seq));
}

PKAtom *pk_generic_put(Pocket lisp, PKAtom *obj, PKAtom *key, PKAtom *value) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, key);
    pk_stack_push(lisp, value);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("put")));
    pk_funcall(lisp, PK_CALL_FUN, 3);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_find(Pocket lisp, PKAtom *obj, PKAtom *key) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, key);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("find")));
    pk_funcall(lisp, PK_CALL_FUN, 2);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_remove(Pocket lisp, PKAtom *obj, PKAtom *key) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, key);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("remove")));
    pk_funcall(lisp, PK_CALL_FUN, 2);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_is_member_atom(Pocket lisp, PKAtom *obj, PKAtom *key) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, key);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("member?")));
    pk_funcall(lisp, PK_CALL_FUN, 2);
    return pk_stack_head_pop(lisp);
}

bool pk_generic_is_member(Pocket lisp, PKAtom *obj, PKAtom *key) {
    return pk_atom_is_true(pk_generic_is_member_atom(lisp, obj, key));
}
