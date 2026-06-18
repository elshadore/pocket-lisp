#include "pocket_internals.h"

void pk_call(Pocket lisp, PKFuncCall call) {
    for (size_t i = 0; i < call.extra_nils; ++i) {
        pk_push_nil(lisp);
    }
    
    pk_frame_push(lisp, call.final_arity);

    switch (call.ty) {
        case PKFuncTy_CFunc: {
            (call.as.c.fn)(call.as.c.user_closure, lisp);
            break;
        }
        case PKFuncTy_Lambda: {
            PKAtom *args = call.as.lisp.args;
          
            size_t i = 0;
            PKAtoms slice = pk_stack_slice(lisp);
            pk_cdolist(lisp, el, args) {
                if (i >= call.final_arity) {
                    pk_error(lisp);
                }
                size_t index = pk_index_inv(i, slice.length);
                PKAtom *atom = slice.e[index];
                PKAtomSymbol *sym = pk_atom_cast_symbol(lisp, el);
                pk_let_push(lisp, PKEnvTy_Var, sym, atom);
                i++;
            }
            pk_frame_clear(lisp);
            pk_atom_evlist(lisp, call.as.lisp.body);
            break;
        }
        case PKFuncTy_Expression: {
            pk_atom_eval(lisp, call.as.value);
            break;
        }
        case PKFuncTy_Evlist: {
            pk_atom_evlist(lisp, call.as.value);
            break;
        }
    }
    
    PKAtom *result = pk_get_result(lisp);
    pk_frame_pop(lisp);

    if (call.insert_result) {
        lisp->stack.e[lisp->stack.count - 1] = result;
    } else {
        pk_push(lisp, result);
    }
    pk_gc_collect(lisp);
}
    
PKFuncCall pk_get_callconv(Pocket lisp, PKAtom *atom, int arity) {
    if (arity < 0) {
        pk_error(lisp);
    }
    size_t uarity = (size_t)arity;
    switch (atom->tag.ty) {
        case PKAtomTy_CFunc: {
            PKAtomCFunc *cfunc = (PKAtomCFunc *)atom;
            size_t farity = uarity;
            size_t carity = (size_t)cfunc->arity.args;
            size_t nils = 0;
            switch (cfunc->arity.mode) {
                case PKArity_Normal: {
                    if (uarity != carity) {
                        pk_error(lisp);
                    }
                    break;
                }
                case PKArity_Optional: {
                    if (uarity < carity) {
                        pk_error(lisp);
                    }
                    nils = uarity - carity;
                    break;
                }
                case PKArity_Variadic: {
                    if (uarity < carity) {
                        pk_error(lisp);
                    }
                    break;
                }
            }
            return (PKFuncCall) {
                .ty = PKFuncTy_CFunc,
                .as.c.fn = cfunc->fn,
                .as.c.user_closure = cfunc->user_closure,
                .final_arity = farity,
                .mode = PKFuncMode_Func,
                .insert_result = true,
                .extra_nils = nils,
                .expression = atom,
            };
        }
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            PKFuncMode mode = PKFuncMode_Func;
            PKAtomSymbol *sym = pk_atom_cast_symbol(lisp, cons->car);
            if (sym == lisp->cache.lambda) {
                mode = PKFuncMode_Func;
            } else {
                pk_error(lisp);
            }
            
            PKAtomCons *a = pk_atom_cast_cons(lisp, cons->cdr);
            
            return (PKFuncCall) {
                .ty = PKFuncTy_Lambda,
                .as.lisp.args = a->car,
                .as.lisp.body = a->cdr,
                .final_arity = uarity,
                .mode = mode,
                .insert_result = true,
                .expression = atom,
            };
        }
        default: {
            pk_error(lisp);
            return (PKFuncCall){0};
        }
    }
}

void pk_eval(Pocket lisp, int stack_pointer) {
    PKAtom *expr = pk_stack_get(lisp, stack_pointer);
    PKFuncCall call = (PKFuncCall) {
        .ty = PKFuncTy_Expression,
        .as.value = expr,
        .mode = PKFuncMode_Func,
        .expression = lisp->cache.nil,
    };
    pk_call(lisp, call);
}

void pk_evlist(Pocket lisp, int stack_pointer) {
    PKAtom *expr = pk_stack_get(lisp, stack_pointer);
    PKFuncCall call = (PKFuncCall) {
        .ty = PKFuncTy_Evlist,
        .as.value = expr,
        .mode = PKFuncMode_Func,
        .expression = lisp->cache.nil,
    };
    pk_call(lisp, call);
}

void pk_fastcall(void *user_closure, Pocket lisp, PKFn fn, int arity) {
    if (arity < 0) {
        pk_error(lisp);
    }
    PKFuncCall call = (PKFuncCall) {
        .as.c.user_closure = user_closure,
        .as.c.fn = fn,
        .final_arity = (size_t)arity,
        .mode = PKFuncMode_Func,
        .expression = lisp->cache.nil,
    };
    pk_call(lisp, call);
}

void pk_funcall(Pocket lisp, int arity) {
    if (arity < 0) {
        pk_error(lisp);
    }

    PKAtom *atom = pk_stack_get(lisp, -(arity + 1));
 
    if (atom->tag.ty == PKAtomTy_Symbol) {
        PKAtomSymbol *sym = (PKAtomSymbol *)atom;
        atom = (PKAtom *)pk_env_get(lisp, PKEnvTy_Fun, sym);
    }

    PKFuncCall call = pk_get_callconv(lisp, atom, arity);
    
    pk_call(lisp, call);
}
