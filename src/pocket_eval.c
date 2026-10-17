#include "pocket_internals.h"

void pk_atom_apply(Pocket lisp, PK_CALL call, PKAtom *fun, PKAtom *args) {
    pk_frame_push(lisp, 0);
    pk_stack_push(lisp, fun);
    pk_u8 count = 0;
    {
        PKAtom *iter = args;
        while (pk_atom_is_cons(iter)) {
            if (count == UINT8_MAX) {
                pk_error(lisp);
                return;
            }
            pk_stack_push(lisp, iter->cons.car);
            count += 1;
            iter = iter->cons.cdr;
        }
    }
    pk_fcall(lisp, call, count);
    pk_frame_return(lisp);
}

void pk_atom_eval_(Pocket lisp, PKAtom *expr) {
    (void)pk_atom_macroexpand(lisp, expr);
    PKAtom *function = pk_atom_compile(PK_CALL_FUN, pk_stack_head(lisp), PK_MODE_EXPRESSION);
    pk_pop(lisp);
    pk_stack_push(lisp, function);
    pk_fcall(lisp, PK_CALL_FUN, 0);
}
    
void pk_atom_evlist_(Pocket lisp, PKAtom *exprlist) {
    pk_frame_push(lisp, 0);
    PKAtom *iter = exprlist;
    while (pk_atom_is_cons(iter)) {
        pk_atom_eval_(lisp, iter->cons.car);
        iter = iter->cons.cdr;
    }
    pk_frame_return(lisp);
}

void pk_atom_eval(Pocket lisp, PKAtom *expr, PK_MODE mode) {
    if (mode == PK_MODE_EXPRESSION) {
        pk_atom_eval_(lisp, expr);
    } else {
        pk_atom_evlist_(lisp, expr);
    }
}
