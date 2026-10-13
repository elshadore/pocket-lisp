#include "pocket_internals.h"

PKAtom pk_callconv_(pk_u8 input_args, pk_u8 args, pk_u8 arity, PKCallConv *output) {
    switch (arity) {
        case PK_INT_ARITY_NORMAL: {
            if (input_args != args) {
                return pk_error_str(pk_str("TODO: error messages"));
            } else {
                *output = (PKCallConv) {
                    .args = args,
                    .variadic_count = 0,
                    .push_extra_nil = false,
                    .variadic = false,
                };
                return pk_true();
            }
        }
        case PK_INT_ARITY_OPTIONAL: {
            if (input_args == args) {
                *output = (PKCallConv) {
                    .args = args,
                    .variadic_count = 0,
                    .push_extra_nil = false,
                    .variadic = false,
                };
                return pk_true();
            } else if (input_args == (args - 1)) {
                *output = (PKCallConv) {
                    .args = args,
                    .variadic_count = 0,
                    .push_extra_nil = true,
                    .variadic = false,
                };
                return pk_true();
            } else {
                return pk_error_str(pk_str("TODO: error messages"));
            }
        }
        case PK_INT_ARITY_VARIADIC: {
            if (input_args == (args - 1)) {
                *output = (PKCallConv) {
                    .args = args,
                    .variadic_count = 0,
                    .push_extra_nil = true,
                    .variadic = false,
                };
                return pk_true();
            } else if (input_args >= args) {
                *output = (PKCallConv) {
                    .args = args,
                    .variadic_count = input_args - args,
                    .push_extra_nil = false,
                    .variadic = true,
                };
                return pk_true();
            } else {
                return pk_error_str(pk_str("TODO: error messages"));
            }
        }
        default: return pk_error_str(pk_str("TODO: error messages"));
    }
}
    
PKAtom pk_callconv(PKAtom fun, pk_u8 args, PKCallConv *output) {
    switch (fun->header.type) {
        case PK_ATOMTY_CFUN: return pk_callconv_(args, fun->cfun.args, fun->cfun.arity, output);
        case PK_ATOMTY_LFUN: return pk_callconv_(args, fun->lfun.args, fun->lfun.arity, output);
        case PK_ATOMTY_CMACRO: return pk_callconv_(args, fun->cmacro.args, fun->cmacro.arity, output);
        case PK_ATOMTY_LMACRO: return pk_callconv_(args, fun->lmacro.args, fun->lmacro.arity, output);
        default: return pk_error_str(pk_str("TODO: error messages"));
    }
}

PKAtom pk_cfun_exec(PKAtomCFun *cfun, PKAtoms stack) {
    switch (cfun->args) {
        case 0: return (cfun->as.fun0)(cfun->user_closure);
        case 1: return (cfun->as.fun1)(cfun->user_closure, stack.e[0]);
        case 2: return (cfun->as.fun2)(cfun->user_closure, stack.e[0], stack.e[1]);
        case 3: return (cfun->as.fun3)(cfun->user_closure, stack.e[0], stack.e[1], stack.e[2]);
        default: return pk_error_str(pk_str("TODO: error messages"));
    }
}

PKAtom pk_stack_funcall_(PK_CALL call, PKAtom function, pk_u8 args) {
    PKAtoms stack = pk_stack_slice();
    if (stack.length < 1) {
        return pk_error();
    }
    stack.e += 1;
    stack.length -= 1;
    if (stack.length != args) {
        return pk_error();
    }
    switch (call) {
        case PK_CALL_FUN: {
            switch (function->header.type) {
                case PK_ATOMTY_CFUN: return pk_cfun_exec((PKAtomCFun *)function, stack);
                case PK_ATOMTY_LFUN: return pk_lfun_exec(function, args);
                default: return pk_error_str(pk_str("TODO: error messages"));
            }
        }
        case PK_CALL_MACRO: {
            switch (function->header.type) {
                case PK_ATOMTY_CMACRO: return pk_cfun_exec((PKAtomCFun *)function, stack);
                case PK_ATOMTY_LMACRO: return pk_lfun_exec(function, args);
                default: return pk_error_str(pk_str("TODO: error messages"));
            }
        }
        default: return pk_error();
    }
}

PKAtom pk_stack_funcall(PK_CALL call, pk_u8 args) {
    PKAtoms stack = pk_stack_slice();
    size_t uargs = (size_t)args;
    if (stack.length < (uargs + 1)) {
        return pk_error();
    }
    
    PKAtom function = stack.e[stack.length - uargs - 1];
    
    if (pk_is_symbol(function)) {
        function = pk_get(PK_ENV_FUN, function);
        if (pk_is_nil(function)) {
            return pk_error();
        }
    }
    
    pk_propagate(pk_frame_push(args + 1));
    
    PKCallConv cc = PK_CALLCONV_EMPTY;
    {
        pk_propagate(pk_callconv(function, args, &cc));
        if (cc.push_extra_nil) {
            pk_propagate(pk_stack_push(pk_nil()));
        }
        if (cc.variadic) {
            pk_propagate(pk_stack_op_list((size_t)cc.variadic_count + 1));
        }
    }

    PKAtom result = pk_stack_funcall_(call, function, cc.args);
    pk_frame_unwind();
    
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
    pk_frame_unwind();
    
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
    pk_frame_unwind();

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
    pk_frame_unwind();
    
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
    pk_frame_unwind();
    
    return result;
}
