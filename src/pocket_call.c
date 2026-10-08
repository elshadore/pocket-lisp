#include "pocket_internals.h"

PKAtom pk_call(PKAtom function, pk_u8 arity) {
    PKAtoms stack = pk_stack_slice();
    if (stack.length != arity) {
        return pk_error();
    }
    switch (function->header.type) {
        case PK_ATOMTY_CFUN: {
            if (function->cfun.args != arity) {
                return pk_error();
            }
            switch (function->cfun.args) {
                case 0: return (function->cfun.as.fun0)(function->cfun.user_closure);
                case 1: return (function->cfun.as.fun1)(function->cfun.user_closure, stack.e[0]);
                case 2: return (function->cfun.as.fun2)(function->cfun.user_closure, stack.e[1], stack.e[0]);
                case 3: return (function->cfun.as.fun3)(function->cfun.user_closure, stack.e[2], stack.e[1], stack.e[0]);
                default: return pk_error();
            }
        }
        case PK_ATOMTY_LFUN: {
            if (function->lfun.args != arity) {
                return pk_error();
            }
            return pk_lfun_exec(function, 0);
        }
        default: return pk_error();
    }
}

PKAtom pk_stack_funcall(pk_u8 arity) {
    PKAtoms stack = pk_stack_slice();
    size_t uarity = (size_t)arity;
    if (stack.length < (uarity + 1)) {
        return pk_error();
    }
    PKAtom function = stack.e[stack.length - uarity - 1];
    pk_propagate(pk_frame_push(arity));
    PKAtom result = pk_call(function, arity);
    pk_frame_unwind();
    return result;
}

PKAtom pk_funcall0(PKAtom fn) {
    if (!pk_is_function(fn)) {
        return pk_error();
    }
    
    pk_propagate(pk_frame_push(0));
    PKAtom result = pk_call(fn, 0);
    pk_frame_unwind();
    
    return result;
}

PKAtom pk_funcall1(PKAtom fn, PKAtom arg1) {
    if (!pk_is_function(fn)) {
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
    PKAtom result = pk_call(fn, 1);
    pk_frame_unwind();
    
    return result;
}

PKAtom pk_funcall2(PKAtom fn, PKAtom arg1, PKAtom arg2) {
    if (!pk_is_function(fn)) {
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
    PKAtom result = pk_call(fn, 2);
    pk_frame_unwind();
    
    return result;
}

PKAtom pk_funcall3(PKAtom fn, PKAtom arg1, PKAtom arg2, PKAtom arg3) {
    if (!pk_is_function(fn)) {
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
    PKAtom result = pk_call(fn, 3);
    pk_frame_unwind();
    
    return result;
}
