#include "pocket_internals.h"

PKAtom pk_stack_funcall_(PKAtom function, pk_u8 arity) {
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

PKAtom pk_stack_funcall(PK_CALL call, pk_u8 arity) {
    (void)call;
    PKAtoms stack = pk_stack_slice();
    size_t uarity = (size_t)arity;
    if (stack.length < (uarity + 1)) {
        return pk_error();
    }
    PKAtom function = stack.e[stack.length - uarity - 1];
    pk_propagate(pk_frame_push(arity));
    PKAtom result = pk_stack_funcall_(function, arity);
    pk_frame_return_head();
    return result;
}

PKAtom pk_funcall0(PK_CALL call, PKAtom fun) {
    if (!pk_is_function(fun)) {
        return pk_error();
    }
    
    PKAtom e[] = {
        fun,
    };
    PKAtoms atoms = (PKAtoms){
        .e = e,
        .length = pk_alen(e)
    };
    
    pk_propagate(pk_frame_pushc(atoms));
    PKAtom result = pk_stack_funcall(call, 0);
    pk_stack_pop_unchecked(1);
    
    return result;
}

PKAtom pk_funcall1(PK_CALL call, PKAtom fun, PKAtom arg1) {
    if (!pk_is_function(fun)) {
        return pk_error();
    }
    
    PKAtom e[] = {
        fun,
        arg1,
    };
    PKAtoms atoms = (PKAtoms){
        .e = e,
        .length = pk_alen(e)
    };
    
    pk_propagate(pk_frame_pushc(atoms));
    PKAtom result = pk_stack_funcall(call, 1);
    pk_stack_pop_unchecked(1);

    return result;
}

PKAtom pk_funcall2(PK_CALL call, PKAtom fun, PKAtom arg1, PKAtom arg2) {
    if (!pk_is_function(fun)) {
        return pk_error();
    }
    
    PKAtom e[] = {
        fun,
        arg1,
        arg2,
    };
    PKAtoms atoms = (PKAtoms){
        .e = e,
        .length = pk_alen(e)
    };
    
    pk_propagate(pk_frame_pushc(atoms));
    PKAtom result = pk_stack_funcall(call, 2);
    pk_stack_pop_unchecked(1);
    
    return result;
}

PKAtom pk_funcall3(PK_CALL call, PKAtom fun, PKAtom arg1, PKAtom arg2, PKAtom arg3) {
    if (!pk_is_function(fun)) {
        return pk_error();
    }
    
    PKAtom e[] = {
        fun,
        arg1,
        arg2,
        arg3,
    };
    PKAtoms atoms = (PKAtoms){
        .e = e,
        .length = pk_alen(e)
    };
    
    pk_propagate(pk_frame_pushc(atoms));
    PKAtom result = pk_stack_funcall(call, 3);
    pk_stack_pop_unchecked(1);
    
    return result;
}
