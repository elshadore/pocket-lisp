#include "pocket_internals.h"

PKAtom pk_funcall0(PKAtom fn) {
    if (!pk_is_cfun(fn)) {
        return pk_error();
    }
    
    pk_propagate(pk_frame_push(0));
    PKAtom result = (fn->cfun.as.fun0)(fn->cfun.user_closure);
    pk_frame_unwind();
    
    return result;
}

PKAtom pk_funcall1(PKAtom fn, PKAtom arg1) {
    if (!pk_is_cfun(fn)) {
        return pk_error();
    }
    
    PKAtom e[] = {
        arg1,
    };
    PKAtoms atoms = (PKAtoms){
        .e = e,
        .length = pk_alen(e)
    };
    
    pk_propagate(pk_frame_pushc(atoms));
    PKAtom result = (fn->cfun.as.fun1)(fn->cfun.user_closure, arg1);
    pk_frame_unwind();
    
    return result;
}

PKAtom pk_funcall2(PKAtom fn, PKAtom arg1, PKAtom arg2) {
    if (!pk_is_cfun(fn)) {
        return pk_error();
    }
    
    PKAtom e[] = {
        arg1,
        arg2,
    };
    PKAtoms atoms = (PKAtoms){
        .e = e,
        .length = pk_alen(e)
    };
    
    pk_propagate(pk_frame_pushc(atoms));
    PKAtom result = (fn->cfun.as.fun2)(fn->cfun.user_closure, arg1, arg2);
    pk_frame_unwind();
    
    return result;
}

PKAtom pk_funcall3(PKAtom fn, PKAtom arg1, PKAtom arg2, PKAtom arg3) {
    if (!pk_is_cfun(fn)) {
        return pk_error();
    }
    
    PKAtom e[] = {
        arg1,
        arg2,
        arg3,
    };
    PKAtoms atoms = (PKAtoms){
        .e = e,
        .length = pk_alen(e)
    };
    
    pk_propagate(pk_frame_pushc(atoms));
    PKAtom result = (fn->cfun.as.fun3)(fn->cfun.user_closure, arg1, arg2, arg3);
    pk_frame_unwind();
    
    return result;
}
