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
            PKStr ty = pk_ident_atomty(function->header.type);
            pk_errorf(lisp, "pk_callconv: expected a function, got %.*s", (int)ty.length, ty.c);
            return PK_CALLCONV_EMPTY;
        }
    }
}

PKAtom *pk_funcall_resolve_function(Pocket lisp, PKAtoms slice) {
    PKAtom *function = slice.e[0];

    if (!pk_atom_is_symbol(function)) {
        return function;
    }
    PKAtomSymbol *asym = (PKAtomSymbol *)function;
    PKAtom *lookup = pk_env_get(lisp, PK_ENV_FUN, asym);
    if (!pk_atom_is_undefined(lookup)) {
        return lookup;
    }
    
    PKStr name = asym->name->str;
    if (slice.length <= 1) {
        pk_errorf(lisp, "pk_funcall: undefined function or method %.*s", (int)name.length, name.c);
        return lisp->cache.nil;
    }
    
    PKAtom *object = slice.e[1];
    PKAtomSymbol *type = pk_atom_typeof(lisp, object);
    PKAtom *method = pk_atom_method_get_raw(lisp, type, asym);

    if (pk_atom_is_undefined(method)) {
        pk_errorf(lisp, "pk_funcall: undefined function or method %.*s", (int)name.length, name.c);
        return lisp->cache.nil;
    }
    
    return method;
}

void pk_funcall_(Pocket lisp, PK_CALL call, size_t args) {
    if (args > UINT8_MAX) {
        pk_errorf(lisp, "pk_fcall: %zu args exceeds the maximum of %u", args, UINT8_MAX);
        return;
    }

    PKAtoms slice = pk_stack_slice_down(lisp, args + 1);

    PKAtom *function = pk_funcall_resolve_function(lisp, slice);

    PKCallConv cc = pk_callconv(lisp, function, args);

    // TODO: this needs to be put under the pk_frame_push as we mutating the stack
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
                    PKStr ty = pk_ident_atomty(function->header.type);
                    pk_errorf(lisp, "pk_fcall: expected a cfun or lfun, got %.*s", (int)ty.length, ty.c);
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
                    PKStr ty = pk_ident_atomty(function->header.type);
                    pk_errorf(lisp, "pk_fcall: expected a cmacro or lmacro, got %.*s", (int)ty.length, ty.c);
                    break;
                }
            }
            break;
        }
        default: {
            pk_errorf(lisp, "pk_fcall: unknown call mode %d", call);
            break;
        }
    }
    
    pk_frame_return(lisp);
    pk_gc_collect(lisp);
}

PK_RES pk_funcall(Pocket lisp, PK_CALL call, size_t args) {
    PKJmp jmp;
    pk_jmp_enter(lisp, &jmp);
    
    if (setjmp(jmp.buf) != 0) {
        pk_gc_collect(lisp);
        pk_debug_stackdump(lisp, "ERROR");
        return PK_YIELDED;
    }
    
    pk_funcall_(lisp, call, args);
    pk_jmp_leave(lisp);

    return PK_OK;
}

