#include "pocket_internals.h"

PKRes pk_bind_lambda_list(Pocket lisp, PKAtom *symbols, PKAtomSlice values) {
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
    
    return PK_Ok;
}

PKRes pk_arity_convert(Pocket lisp, int arity, size_t *output) {
    if (arity < 0 || arity > UCHAR_MAX)  {
        return pk_error(lisp);
    }
    *output = (size_t)arity;
    return PK_Ok;
}

PKRes pk_callconv(Pocket lisp, PKAtom *atom, size_t arity, PKCallConv *output) {
    PKAtom *function = NULL;
    
    if (atom->tag.ty == PKAtomTy_Symbol) {
        pk_try(pk_env_get(lisp, PKEnvTy_Fun, (PKAtomSymbol *)atom, &function));
    } else {
        function = atom;
    }
    
    switch (function->tag.ty) {
        case PKAtomTy_CFunc: {
            PKAtomCFunc *cfunc = (PKAtomCFunc *)function;
            size_t farity = arity;
            size_t carity = (size_t)cfunc->arity.args;
            size_t nils = 0;
            
            switch (cfunc->arity.mode) {
                case PKArity_Normal: {
                    if (arity != carity) {
                        return pk_error(lisp);
                    }
                    break;
                }
                case PKArity_Optional: {
                    if (arity < carity) {
                        return pk_error(lisp);
                    }
                    nils = arity - carity;
                    break;
                }
                case PKArity_Variadic: {
                    if (arity < carity) {
                        return pk_error(lisp);
                    }
                    break;
                }
            }
            
            output->ty = PK_CALLTY_CFUNC;
            output->as.c = cfunc;
            output->final_arity = farity;
            output->insert_result = PK_TRUE;
            output->extra_nils = nils;

            return PK_Ok;
        }
        case PKAtomTy_LFunc: {
            PKAtomLFunc *lfunc = (PKAtomLFunc *)function;
            output->ty = PK_CALLTY_LFUNC;
            output->as.lisp = lfunc;
            output->final_arity = arity;
            output->insert_result = PK_TRUE;
            output->extra_nils = 0;
            return PK_Ok;
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
}

PKRes pk_call(Pocket lisp, PKCallConv *conv) {
    size_t i = 0;
    PKRes result = PK_Yield;
    
    pk_try(pk_frame_push(lisp, conv->final_arity));
    
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
            pk_defer(pk_lfunc_exec(lisp, conv->as.lisp));
            break;
        }
        default: {
            pk_defer(pk_error(lisp));
            break;
        }
    }
    
    result = PK_Ok;
    
    DEFER:
        
    if (conv->insert_result) {
        pk_try(pk_return_insert(lisp));
    } else {
        pk_try(pk_return_push(lisp));
    }

    return result;
}

PKRes pk_atom_eval(Pocket lisp, PKAtom *atom) {
    PKCallConv call;
    PKAtomLFunc *function = NULL;
    
    pk_try(pk_compile_atom(lisp, atom, &function));
    pk_try(pk_callconv(lisp, (PKAtom *)function, 0, &call));

    pk_try(pk_call(lisp, &call));
    
    return PK_Ok;
}

PKRes pk_atom_evlist(Pocket lisp, PKAtom *atom) {
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

    return PK_Ok;
}
