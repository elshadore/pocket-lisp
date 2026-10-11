#include "pocket_internals.h"

PKAtom pk_apply_(PK_CALL call, PKAtom fun, PKAtom args) {
    pk_propagate(pk_stack_push(fun));
    pk_u8 arity = 0;
    {
        PKAtom iter = args;
        while (pk_is_cons(args)) {
            if (arity == UINT8_MAX) {
                return pk_error();
            }
            pk_propagate(pk_stack_push(iter->cons.car));
            arity += 1;
            iter = args->cons.cdr;
        }
    }
    return pk_stack_funcall(call, arity);
}
    
PKAtom pk_apply(PK_CALL call, PKAtom fun, PKAtom args) {
    pk_propagate(pk_frame_push(0));
    PKAtom result = pk_apply_(call, fun, args);
    pk_frame_unwind();
    return result;
}

PKAtom pk_eval_(PKAtom expr) {
    PKAtom a = pk_macroexpand(expr);
    pk_propagate(a);
    PKAtom b = pk_compile(expr, PK_MODE_EXPRESSION);
    pk_propagate(b);
    return pk_funcall0(PK_CALL_FUN, b);
}
    
PKAtom pk_evlist_(PKAtom exprlist) {
    PKAtom iter = exprlist;
    PKAtom result = pk_nil();
    while (pk_is_cons(iter)) {
        result = pk_eval_(iter->cons.car);
        pk_propagate(result);
        iter = iter->cons.cdr;
    }
    return result;
}

PKAtom pk_eval(PKAtom expr, PK_MODE mode) {
    if (mode == PK_MODE_EXPRESSION) {
        return pk_eval_(expr);
    } else {
        return pk_evlist_(expr);
    }
}

