#include "pocket_internals.h"

PK_RES pk_bind_lambda_list(Pocket lisp, PKAtom *symbols, PKAtomSlice values) {
    size_t i = 0;
    PKAtom *acc = symbols;
    
    for (i = 0; i < values.length; ++i) {
        PKAtomCons *cons = NULL;
        PKAtomSymbol *symbol = NULL;
        
        pk_try(pk_atom_cast_cons(lisp, acc, &cons));
        pk_try(pk_atom_cast_symbol(lisp, cons->car, &symbol));

        pk_try(pk_let_push(lisp, PKEnvTy_Var, symbol, values.e[i]));
        
        acc = cons->cdr;
    }
    
    if (!pk_atom_is_nil(acc)) {
        return pk_error(lisp);
    }
    
    return PK_OK;
}

PK_RES pk_arity_convert(Pocket lisp, int arity, size_t *output) {
    if (arity < 0 || arity > UCHAR_MAX)  {
        return pk_error(lisp);
    }
    *output = (size_t)arity;
    return PK_OK;
}

PK_RES pk_callconv_handle_arity(Pocket lisp, size_t arity, pk_bool variadic_list, PKFuncArity func, PKCallConv *output) {
    size_t funargs = (size_t)func.args;

    switch (func.mode) {
        case PK_ARITY_NORMAL: {
            if (arity != funargs) {
                return pk_error(lisp);
            }
            output->final_arity = funargs;
            output->extra_nils = 0;
            output->variadic_list = 0;
            break;
        }
        case PK_ARITY_OPTIONAL: {
            if (arity == funargs) {
                output->extra_nils = 1;
            } else if (arity == (funargs + 1)) {
                output->extra_nils = 0;
            } else {
                return pk_error(lisp);
            }
            output->final_arity = arity;
            output->variadic_list = 0;
            break;
        }
        case PK_ARITY_VARIADIC: {
            if (arity < funargs) {
                return pk_error(lisp);
            }
            if (variadic_list) {
                output->variadic_list = arity - funargs;
                if (output->variadic_list == 0) {
                    output->extra_nils = 1;
                } else {
                    output->extra_nils = 0;
                }
            } else {
                output->variadic_list = 0;
                output->extra_nils = 0;
            }
            output->final_arity = arity;
            break;
        }
    }
    
    
    return PK_OK;
}

PK_RES pk_callconv(Pocket lisp, PKAtom *atom, size_t arity, PK_CALLFLAG flags, PKCallConv *output) {
    PKAtom *function = NULL;
    pk_bool insert_result = PK_FALSE;
    pk_bool macro_call = PK_FALSE;
    
    if (atom->tag.ty == PKAtomTy_Symbol) {
        pk_try(pk_env_get(lisp, PKEnvTy_Fun, (PKAtomSymbol *)atom, &function));
    } else {
        function = atom;
    }

    insert_result = (flags & PK_CALLFLAG_INSERT_RESULT);
    macro_call = (flags & PK_CALLFLAG_MACRO_CALL);

    switch (function->tag.ty) {
        case PKAtomTy_CFunc: {
            PKAtomCFunc *cfunc = (PKAtomCFunc *)function;
            
            if (macro_call) {
                return pk_error(lisp);
            }
            
            pk_try(pk_callconv_handle_arity(lisp, arity, PK_FALSE, cfunc->arity, output));
            
            output->ty = PK_CALLTY_CFUNC;
            output->as.c = cfunc;
            output->insert_result = insert_result;

            return PK_OK;
        }
        case PKAtomTy_LFunc: {
            PKAtomLFunc *lfunc = (PKAtomLFunc *)function;
            
            if (macro_call) {
                return pk_error(lisp);
            }
            
            pk_try(pk_callconv_handle_arity(lisp, arity, PK_TRUE, lfunc->arity, output));
            
            output->ty = PK_CALLTY_LFUNC;
            output->as.lisp = lfunc;
            output->insert_result = insert_result;
            
            return PK_OK;
        }
        case PKAtomTy_LMacro: {
            PKAtomLFunc *lfunc = (PKAtomLFunc *)function;
            
            if (!macro_call) {
                return pk_error(lisp);
            }
            
            pk_try(pk_callconv_handle_arity(lisp, arity, PK_TRUE, lfunc->arity, output));
            
            output->ty = PK_CALLTY_LFUNC;
            output->as.lisp = lfunc;
            output->insert_result = insert_result;
            
            return PK_OK;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

void pk_callconv_quick(void *user_closure, PKFn fn, size_t arity, PKCallConv *output) {
    output->ty = PK_CALLTY_QUICK;
    output->as.quick.user_closure = user_closure;
    output->as.quick.fn = fn;
    output->final_arity = arity;
    output->extra_nils = 0;
    output->insert_result = PK_FALSE;
    output->variadic_list = 0;
}

PK_RES pk_call(Pocket lisp, PKCallConv *conv) {
    size_t i = 0;
    size_t start = 0;
    PK_RES result = PK_YIELD;

    pk_try(pk_frame_push(lisp, conv->final_arity));
    start = lisp->frames.count;
    
    if (conv->variadic_list > 0) {
        pk_try(pk_stack_op_list(lisp, conv->variadic_list));
    }
    
    for (i = 0; i < conv->extra_nils; ++i) {
        pk_defer(pk_push_nil(lisp));
    }
    
    switch (conv->ty) {
        case PK_CALLTY_QUICK: {
            pk_defer((conv->as.quick.fn)(conv->as.quick.user_closure, lisp));
            break;
        }
        case PK_CALLTY_CFUNC: {
            pk_defer((conv->as.c->fn)(conv->as.c->user_closure, lisp));
            break;
        }
        case PK_CALLTY_LFUNC: {
            pk_defer(pk_lfunc_exec(lisp, conv->as.lisp, start));
            break;
        }
        default: {
            pk_defer(pk_error(lisp));
            break;
        }
    }
    
    result = PK_OK;
    
    DEFER:

    if (lisp->frames.count != start) {
        pk_abort(lisp, "pk_call(): frame not correctly reset");
    }

    if (conv->insert_result) {
        pk_try(pk_return_insert(lisp));
    } else {
        pk_try(pk_return_push(lisp));
    }

    if (lisp->current_frame.catch_symbol != NULL) {
        if (lisp->current_frame.catch_symbol == lisp->throwing) {
            pk_catch_all(lisp);
            return PK_OK;
        }
    }

    return result;
}

PK_RES pk_atom_apply(Pocket lisp, PKAtom *function, PKAtom *args) {
    size_t acc = 0;
    PKAtom *iter = args;
    PK_RES result = PK_YIELD;
    PKCallConv call;
    
    pk_try(pk_frame_push(lisp, 0));
    
    pk_defer(pk_push(lisp, function));
    
    while (!pk_atom_is_nil(iter)) {
        PKAtomCons *cons = NULL;
        pk_defer(pk_atom_cast_cons(lisp, iter, &cons));
        pk_defer(pk_push(lisp, cons->car));
        acc += 1;
        iter = cons->cdr;
    }
    
    pk_defer(pk_callconv(lisp, function, acc, PK_CALLFLAG_INSERT_RESULT, &call));

    pk_defer(pk_call(lisp, &call));

    result = PK_OK;
    
    DEFER:
    
    pk_try(pk_return_push(lisp));
        
    return result;
}

PK_RES pk_atom_eval(Pocket lisp, PKAtom *atom) {
    PKCallConv call;
    PKAtom *expanded = NULL;
    PKAtomLFunc *function = NULL;

    /* pk_try(pk_atom_macroexpand(lisp, atom, &expanded)); */
    expanded = atom;
       
    pk_try(pk_compile_atom(lisp, expanded, PK_FUN_FUNCTION, &function));
    
    pk_try(pk_callconv(lisp, (PKAtom *)function, 0, PK_CALLFLAG_NONE, &call));

    pk_try(pk_call(lisp, &call));
    
    return PK_OK;
}

PK_RES pk_atom_evlist(Pocket lisp, PKAtom *atom) {
    PKAtom *iter = atom;

    while (!pk_atom_is_nil(iter)) {
        PKAtomCons *cons = NULL;
        
        pk_try(pk_atom_cast_cons(lisp, iter, &cons));
        pk_try(pk_atom_eval(lisp, cons->car));
        if (!pk_atom_is_nil(cons->cdr)) {
            pk_try(pk_pop(lisp));
        }
        iter = cons->cdr;
    }

    return PK_OK;
}
