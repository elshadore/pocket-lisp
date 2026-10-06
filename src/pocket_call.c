#include "pocket_internals.h"

PKAtom pk_funcall0(PKAtom fn) {
    if (!pk_is_cfun(fn)) {
        return pk_error();
    }
    return (fn->cfun.as.fun0)(fn->cfun.user_closure);
}

PKAtom pk_funcall1(PKAtom fn, PKAtom arg1) {
    if (!pk_is_cfun(fn)) {
        return pk_error();
    }
    return (fn->cfun.as.fun1)(fn->cfun.user_closure, arg1);
}

PKAtom pk_funcall2(PKAtom fn, PKAtom arg1, PKAtom arg2) {
    if (!pk_is_cfun(fn)) {
        return pk_error();
    }
    return (fn->cfun.as.fun2)(fn->cfun.user_closure, arg1, arg2);
}

PKAtom pk_funcall3(PKAtom fn, PKAtom arg1, PKAtom arg2, PKAtom arg3) {
    if (!pk_is_cfun(fn)) {
        return pk_error();
    }
    return (fn->cfun.as.fun3)(fn->cfun.user_closure, arg1, arg2, arg3);
}
