#include "pocket_internals.h"

PKRes pk_ret_top(Pocket lisp) {
    PKAtom *atom = pk_atom_nil(lisp);
    PKRes result = PK_Ok;
    if (!pk_stack_head(lisp, &atom)) {
        result = PK_Yield;
    }
    if (!pk_ret_this(lisp, atom)) {
        result = PK_Yield;
    }
    return result;
}

PKRes pk_ret_nil(Pocket lisp) {
    return pk_ret_this(lisp, pk_atom_nil(lisp));
}

PKRes pk_ret_this(Pocket lisp, PKAtom *atom) {
    PKRes result = PK_Ok;
    if (!pk_frame_pop_clear(lisp)) {
        result = PK_Yield;
    }
    if (!pk_push(lisp, atom)) {
        result = PK_Yield;
    }
    return result;
}

PKRes pk_ret_all(Pocket lisp) {
    return pk_frame_pop_return(lisp);
}

PKRes pk_ret_none(Pocket lisp) {
    return pk_frame_pop_clear(lisp);
}
