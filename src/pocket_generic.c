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

PKAtom *pk_generic_read_char(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("read-char")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_read_byte(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("read-byte")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_read(Pocket lisp, PKAtom *obj, size_t amount, PKAtom *mode) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_size_t(lisp, amount));
    pk_stack_push(lisp, mode);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("read-byte")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

void pk_generic_write(Pocket lisp, PKAtom *obj, PKAtom *atom) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, atom);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("write")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    pk_pop(lisp);
}

PKAtom *pk_generic_cat(Pocket lisp, PKAtom *obj, PKAtoms atoms) {
    pk_stack_push(lisp, obj);
    pk_stack_push_many(lisp, atoms);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("cat")));
    pk_funcall(lisp, PK_CALL_FUN, 1 + atoms.length);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_reverse(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("reverse")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_to_list(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("->list")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_to_string(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("->string")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_to_symbol(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("->symbol")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_to_keyword(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("->keyword")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_to_char(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("->char")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_to_chars(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("->chars")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_to_bytes(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("->bytes")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_to_byte(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("->byte")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_to_int(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("->int")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}

PKAtom *pk_generic_to_float(Pocket lisp, PKAtom *obj) {
    pk_stack_push(lisp, obj);
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_str("->float")));
    pk_funcall(lisp, PK_CALL_FUN, 1);
    return pk_stack_head_pop(lisp);
}
