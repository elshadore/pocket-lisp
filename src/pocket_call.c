#include "pocket_internals.h"

PKCallConv pk_callconv_(Pocket lisp, PK_FUNTY type, pk_u8 args, PKArity arity) {
    switch (arity.type) {
        case PK_INT_ARITY_NORMAL: {
            if (args != arity.args) {
                pk_error(lisp);
                return PK_CALLCONV_EMPTY;
            } else {
                return (PKCallConv) {
                    .args = args,
                    .variadic_count = 0,
                    .extra_nils = 0,
                };
            }
        }
        case PK_INT_ARITY_OPTIONAL: {
            pk_u8 min = arity.args - arity.options;
            if (args < min) {
                pk_error(lisp);
                return PK_CALLCONV_EMPTY;
            } else if (args > arity.args) {
                pk_error(lisp);
                return PK_CALLCONV_EMPTY;
            } else {
                return (PKCallConv) {
                    .args = args,
                    .extra_nils = args - min,
                    .variadic_count = 0,
                };
            }
        }
        case PK_INT_ARITY_VARIADIC: {
            if (args == arity.args) {
                if (type == PK_FUNTY_LISP) {
                    return (PKCallConv) {
                        .args = args + 1,
                        .extra_nils = 1,
                        .variadic_count = 0,
                    };
                } else {
                    return (PKCallConv) {
                        .args = args,
                        .extra_nils = 0,
                        .variadic_count = 0,
                    };
                }
            } else if (args > arity.args) {
                if (type == PK_FUNTY_LISP) {
                    return (PKCallConv) {
                        .args = arity.args + 1,
                        .extra_nils = 0,
                        .variadic_count = args - arity.args,
                    };
                } else {
                    return (PKCallConv) {
                        .args = args,
                        .extra_nils = 0,
                        .variadic_count = 0,
                    };
                }
            } else {
                pk_error(lisp);
                return PK_CALLCONV_EMPTY;
            }
        }
        default: {
            pk_error(lisp);
            return PK_CALLCONV_EMPTY;
        }
    }
}

PKCallConv pk_callconv(Pocket lisp, PKAtom *function, pk_u8 args) {
    switch (function->header.type) {
        case PK_ATOMTY_CFUN: return pk_callconv_(lisp, PK_FUNTY_C, args, function->cfun.arity);
        case PK_ATOMTY_LFUN: return pk_callconv_(lisp, PK_FUNTY_LISP, args, function->lfun.arity);
        case PK_ATOMTY_CMACRO: return pk_callconv_(lisp, PK_FUNTY_C, args, function->cmacro.arity);
        case PK_ATOMTY_LMACRO: return pk_callconv_(lisp, PK_FUNTY_LISP, args, function->lmacro.arity);
        default: {
            pk_error(lisp);
            return PK_CALLCONV_EMPTY;
        }
    }
}

void pk_fcall(Pocket lisp, PK_CALL call, size_t args) {
    PKAtoms slice = pk_stack_slice_down(lisp, args + 1);
    
    PKAtom *function = slice.e[0];
    
    if (pk_atom_is_symbol(function)) {
        PKAtomSymbol *asym = (PKAtomSymbol *)function;
        function = pk_env_get(lisp, PK_ENV_FUN, asym);
        if (pk_atom_is_undefined(function)) {
            pk_error(lisp);
            return;
        }
    }
    if (args > UINT8_MAX) {
        pk_error(lisp);
        return;
    }
    PKCallConv cc = pk_callconv(lisp, function, args);
    
    for (size_t i = 0; i < cc.extra_nils; ++i) {
        pk_push_nil(lisp);
    }
    if (cc.variadic_count > 0) {
        pk_stack_op_list(lisp, cc.variadic_count);
    }
    
    pk_frame_push(lisp, cc.args);

    switch (call) {
        case PK_CALL_FUN: {
            switch (function->header.type) {
                case PK_ATOMTY_CFUN: {
                    pk_atom_cfun_exec(lisp, (PKAtomCFun *)function);
                    break;
                }
                case PK_ATOMTY_LFUN: {
                    pk_atom_lfun_exec(lisp, (PKAtomLFun *)function);
                    break;
                }
                default: {
                    pk_error(lisp);
                    break;
                }
            }
            break;
        }
        case PK_CALL_MACRO: {
            switch (function->header.type) {
                case PK_ATOMTY_CMACRO: {
                    pk_atom_cfun_exec(lisp, (PKAtomCFun *)function);
                    break;
                }
                case PK_ATOMTY_LMACRO: {
                    pk_atom_lfun_exec(lisp, (PKAtomLFun *)function);
                    break;
                }
                default: {
                    pk_error(lisp);
                    break;
                }
            }
            break;
        }
        default: {
            pk_error(lisp);
            break;
        }
    }
    
    pk_frame_return(lisp);
}

void pk_pcall(Pocket lisp, PK_CALL call, size_t args) {
    (void)call;
    (void)args;
    pk_errors(lisp, pk_str("pk_pcall is not the way this function should work"));
}
