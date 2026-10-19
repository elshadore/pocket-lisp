#include "pocket_internals.h"

PK_RES pk_atom_apply(Pocket lisp, PK_CALL call, PKAtom *fun, PKAtom *args) {
    pk_stack_push(lisp, fun);

    pk_u8 count = 0;
    PKAtom *iter = args;
    while (pk_atom_is_cons(iter)) {
        if (count == UINT8_MAX) {
            pk_errorf(lisp, "pk_atom_apply: more than %u arguments", UINT8_MAX);
            return PK_YIELDED;
        }
        pk_stack_push(lisp, iter->cons.car);
        count += 1;
        iter = iter->cons.cdr;
    }

    return pk_funcall(lisp, call, count);
}

PK_RES pk_atom_eval_(Pocket lisp, PKAtom *expr) {
    pk_stack_push(lisp, expr);
    (void)pk_macroexpand_(lisp, -1);
    pk_stack_set(lisp, -1, pk_atom_compile(lisp, PK_CALL_FUN, pk_stack_get(lisp, -1), PK_MODE_EXPRESSION));
    return pk_funcall(lisp, PK_CALL_FUN, 0);
}

PK_RES pk_atom_evlist_(Pocket lisp, PKAtom *exprlist) {
    pk_frame_push(lisp, 0);
    pk_stack_push(lisp, exprlist);
    PKAtom *iter = exprlist;
    while (pk_atom_is_cons(iter)) {
        PK_RES res = pk_atom_eval_(lisp, iter->cons.car);
        if (res == PK_YIELDED) {
            return res;
        }
        iter = iter->cons.cdr;
    }
    pk_frame_unwind(lisp);
    pk_stack_push(lisp, lisp->cache.nil);

    return PK_OK;
}

PK_RES pk_atom_eval(Pocket lisp, PKAtom *expr, PK_MODE mode) {
    if (mode == PK_MODE_EXPRESSION) {
        return pk_atom_eval_(lisp, expr);
    } else {
        pk_pop(lisp);
        return pk_atom_evlist_(lisp, expr);
    }
}