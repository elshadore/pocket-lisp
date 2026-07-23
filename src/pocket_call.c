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

PKRes pk_callconv(Pocket lisp, PKAtom *atom, size_t arity, PKCallConv *output) {
    switch (atom->tag.ty) {
        case PKAtomTy_CFunc: {
            PKAtomCFunc *cfunc = (PKAtomCFunc *)atom;
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
            
            output->ty = PKFuncTy_CFunc;
            output->as.c.fn = cfunc->fn;
            output->as.c.user_closure = cfunc->user_closure;
            output->final_arity = farity;
            output->mode = PKFuncMode_Func;
            output->insert_result = PK_TRUE;
            output->extra_nils = nils;
            output->expression = atom;
            
            return PK_Ok;
        }
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            PKAtomSymbol *sym = NULL;
            PKAtomCons *a = NULL;
            PKFuncMode mode = PKFuncMode_Func;
            
            pk_try(pk_atom_cast_symbol(lisp, cons->car, &sym));

            if (sym == lisp->cache.lambda) {
                mode = PKFuncMode_Func;
            } else if (sym == lisp->cache.macro) {
                mode = PKFuncMode_Macro;
            } else {
                return pk_error(lisp);
            }

            pk_try(pk_atom_cast_cons(lisp, cons->cdr, &a));

            output->ty = PKFuncTy_Lambda;
            output->as.lisp.args = a->car;
            output->as.lisp.body = a->cdr;
            output->final_arity = arity;
            output->mode = mode;
            output->insert_result = PK_TRUE;
            output->expression = atom;
            
            return PK_Ok;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

PKRes pk_eval(Pocket lisp, int stack_pointer) {
    (void)stack_pointer;
    return pk_error(lisp);
}

PKRes pk_evlist(Pocket lisp, int stack_pointer) {
    (void)stack_pointer;
    return pk_error(lisp);
}

PKRes pk_fastcall(void *user_closure, Pocket lisp, PKFn fn, int arity) {
    (void)user_closure;
    (void)fn;
    (void)arity;
    return pk_error(lisp);
}

PKRes pk_funcall(Pocket lisp, int arity) {
    (void)arity;
    return pk_error(lisp);
}
