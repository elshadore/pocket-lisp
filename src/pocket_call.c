#include "pocket_internals.h"

PKCallConv pk_callconv_(Pocket lisp, PK_FUNTY type, pk_u8 args, PKArity arity) {
    switch (arity.type) {
        case PK_INT_ARITY_NORMAL: {
            if (args != arity.args) {
                pk_errorf(lisp, "pk_callconv_: %s function expects exactly %u args, got %u",
                    type == PK_FUNTY_LISP ? "lisp" : "c", arity.args, args);
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
                pk_errorf(lisp, "pk_callconv_: %s function expects at least %u args (of %u, %u optional), got %u",
                    type == PK_FUNTY_LISP ? "lisp" : "c", min, arity.args, arity.options, args);
                return PK_CALLCONV_EMPTY;
            } else if (args > arity.args) {
                pk_errorf(lisp, "pk_callconv_: %s function expects at most %u args (of %u, %u optional), got %u",
                    type == PK_FUNTY_LISP ? "lisp" : "c", arity.args, arity.args, arity.options, args);
                return PK_CALLCONV_EMPTY;
            } else {
                return (PKCallConv) {
                    .args = arity.args,
                    .extra_nils = arity.args - args,
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
                pk_errorf(lisp, "pk_callconv_: %s function expects at least %u args, got %u",
                    type == PK_FUNTY_LISP ? "lisp" : "c", arity.args, args);
                return PK_CALLCONV_EMPTY;
            }
        }
        default: {
            pk_errorf(lisp, "pk_callconv_: unknown arity type %u", arity.type);
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
            pk_error_type2(lisp, function, PK_TYPE_FUNCTION, PK_TYPE_MACRO);
            return PK_CALLCONV_EMPTY;
        }
    }
}

PKAtom *pk_funcall_resolve_function(Pocket lisp, PKAtom *symbol) {
    if (!pk_atom_is_symbol(symbol)) {
        return symbol;
    }
    
    PKAtomSymbol *asym = (PKAtomSymbol *)symbol;

    PKAtom *fetch = pk_env_fetch(lisp, PK_ENV_FUN, asym);
    if (pk_atom_is_undefined(fetch)) {
        pk_writer_reset(lisp);
        pk_writer_string(lisp, pk_str("undefined function: "));
        pk_writer_atom(lisp, symbol);
        pk_error_writer(lisp);
        return &PK_ATOM_NIL;
    } else {
        return fetch;
    }
}

void pk_funcall_(Pocket lisp, PK_CALL call, size_t args) {
    if (args > UINT8_MAX) {
        pk_errorf(lisp, "args of: %zu, exceeds the maximum of: %u", args, UINT8_MAX);
        return;
    }

    PKAtoms slice = pk_stack_slice_down(lisp, args + 1);

    PKAtom *symbol = slice.e[slice.length - 1];
    
    PKAtom *function = pk_funcall_resolve_function(lisp, symbol);

    PKCallConv cc = pk_callconv(lisp, function, args);

    pk_pop(lisp);

    for (size_t i = 0; i < cc.extra_nils; ++i) {
        pk_push_nil(lisp);
    }
    if (cc.variadic_count > 0) {
        pk_stack_op_list(lisp, cc.variadic_count);
    }

    pk_frame_push(lisp, symbol, cc.args);
    
    if (call == PK_CALL_FUN) {
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
                pk_error_type(lisp, function, PK_TYPE_FUNCTION);
                break;
            }
        }
    } else {
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
                pk_error_type(lisp, function, PK_TYPE_MACRO);
                break;
            }
        }
    }
    
    pk_frame_return_push(lisp);
    pk_gc_collect(lisp);
}

PK_RES pk_int_funcall(Pocket lisp, PK_CALL call, size_t args) {
    PKJmp jmp;
    pk_jmp_enter(lisp, &jmp);
    
    if (setjmp(jmp.buf) != 0) {
        pk_gc_collect(lisp);
        #ifdef PK_CFLAG_DEBUG
        pk_debug_stackdump(lisp, "ERROR");
        #endif
        return PK_YIELDED;
    }
    
    pk_funcall_(lisp, call, args);
    pk_jmp_leave(lisp);

    return PK_OK;
}

PK_RES pk_int_fastcall(Pocket lisp, void *user_closure, PKFun fun, size_t args, PKAtom *frame) {
    if (args > UINT8_MAX) {
        pk_errorf(lisp, "args of: %zu, exceeds the maximum of: %u", args, UINT8_MAX);
        return PK_YIELDED;
    }

    PKJmp jmp;
    pk_jmp_enter(lisp, &jmp);

    if (setjmp(jmp.buf) != 0) {
        pk_gc_collect(lisp);
        #ifdef PK_CFLAG_DEBUG
        pk_debug_stackdump(lisp, "ERROR");
        #endif
        return PK_YIELDED;
    }

    pk_frame_push(lisp, frame, args);

    fun(user_closure, lisp);

    pk_frame_return_push(lisp);
    pk_gc_collect(lisp);

    pk_jmp_leave(lisp);

    return PK_OK;
}

PK_RES pk_protect(Pocket lisp, void *user_closure, PKFun fun, size_t args) {
    pk_frame_push(lisp, (PKAtom *)lisp->cache.protect, args);
    pk_frame_mode_set_protect(lisp);
    PK_RES result = pk_int_fastcall(lisp, user_closure, fun, args, &PK_ATOM_NIL);
    pk_frame_return_push(lisp);
    return result;
}

PK_RES pk_protect_noreturn(Pocket lisp, void *user_closure, PKFun fun, size_t args) {
    pk_frame_push(lisp, (PKAtom *)lisp->cache.protect, args);
    pk_frame_mode_set_protect(lisp);
    PK_RES result = pk_int_fastcall(lisp, user_closure, fun, args, &PK_ATOM_NIL);
    if (result == PK_YIELDED) {
        pk_frame_return_push(lisp);
    } else {
        pk_frame_unwind(lisp);
    }
    return result;
}
    
PK_RES pk_atom_catch(Pocket lisp, PKAtomSymbol *symbol, void *user_closure, PKFun fun, size_t args) {
    pk_frame_push(lisp, (PKAtom *)symbol, args);
    pk_frame_mode_set_catch(lisp, symbol);
    PK_RES result = pk_int_fastcall(lisp, user_closure, fun, args, (PKAtom *)symbol);
    pk_frame_return_push(lisp);
    return result;
}

