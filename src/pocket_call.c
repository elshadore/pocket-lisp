#include "pocket_internals.h"

PKRes pk_call(Pocket lisp, PKFuncCall call) {
    for (size_t i = 0; i < call.extra_nils; ++i) {
        pk_try(pk_push_nil(lisp));
    }

    pk_try(pk_frame_push(lisp, call.final_arity, lisp->current_frame.mode));

    PKRes call_res = PK_Ok;
    switch (call.ty) {
        case PKFuncTy_CFunc: {
            call_res = (call.as.c.fn)(call.as.c.user_closure, lisp);
            break;
        }
        case PKFuncTy_Lambda: {
            PKAtom *args = call.as.lisp.args;
            size_t i = 0;
            PKAtoms slice = pk_stack_slice(lisp);
            pk_cdolist(lisp, el, args) {
                if (i >= call.final_arity) {
                    call_res = pk_error(lisp);
                    break;
                }
                size_t index = pk_index_inv(i, slice.length);
                PKAtom *atom = slice.e[index];
                PKAtomSymbol *sym;
                call_res = pk_atom_cast_symbol(lisp, el, &sym);
                if (call_res == PK_Yield) break;
                call_res = pk_let_push(lisp, PKEnvTy_Var, sym, atom);
                if (call_res == PK_Yield) break;
                i++;
            }
            if (call_res == PK_Ok) {
                pk_try(pk_frame_clear(lisp));
                call_res = pk_atom_evlist(lisp, call.as.lisp.body);
            }
            break;
        }
        case PKFuncTy_Expression: {
            call_res = pk_atom_eval(lisp, call.as.value);
            break;
        }
        case PKFuncTy_Evlist: {
            call_res = pk_atom_evlist(lisp, call.as.value);
            break;
        }
    }

    if (call_res == PK_Yield) {
        pk_frame_pop(lisp);
        return PK_Yield;
    }

    PKAtom *result;
    pk_try(pk_stack_head(lisp, &result));
    pk_try(pk_frame_pop(lisp));

    if (call.insert_result) {
        lisp->stack.e[lisp->stack.count - 1] = result;
    } else {
        pk_try(pk_push(lisp, result));
    }
    pk_try(pk_gc_collect(lisp));
    return PK_Ok;
}

PKFuncCall pk_get_callconv(Pocket lisp, PKAtom *atom, int arity) {
    if (arity < 0) {
        pk_error(lisp);
        return (PKFuncCall){0};
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
                        return (PKFuncCall){0};
                    }
                    break;
                }
                case PKArity_Optional: {
                    if (uarity < carity) {
                        pk_error(lisp);
                        return (PKFuncCall){0};
                    }
                    nils = uarity - carity;
                    break;
                }
                case PKArity_Variadic: {
                    if (uarity < carity) {
                        pk_error(lisp);
                        return (PKFuncCall){0};
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
            PKAtomSymbol *sym;
            PKRes r = pk_atom_cast_symbol(lisp, cons->car, &sym);
            if (r == PK_Yield) return (PKFuncCall){0};

            PKFuncMode mode = PKFuncMode_Func;
            if (sym == lisp->cache.lambda) {
                mode = PKFuncMode_Func;
            } else if (sym == lisp->cache.macro) {
                mode = PKFuncMode_Macro;
            } else {
                pk_error(lisp);
                return (PKFuncCall){0};
            }

            PKAtomCons *a;
            r = pk_atom_cast_cons(lisp, cons->cdr, &a);
            if (r == PK_Yield) return (PKFuncCall){0};

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

PKRes pk_eval(Pocket lisp, int stack_pointer) {
    PKAtom *expr;
    pk_try(pk_stack_get(lisp, stack_pointer, &expr));
    PKFuncCall call = (PKFuncCall) {
        .ty = PKFuncTy_Expression,
        .as.value = expr,
        .mode = PKFuncMode_Func,
        .expression = pk_atom_nil(lisp),
    };
    pk_try(pk_call(lisp, call));
    return PK_Ok;
}

PKRes pk_evlist(Pocket lisp, int stack_pointer) {
    PKAtom *expr;
    pk_try(pk_stack_get(lisp, stack_pointer, &expr));
    PKFuncCall call = (PKFuncCall) {
        .ty = PKFuncTy_Evlist,
        .as.value = expr,
        .mode = PKFuncMode_Func,
        .expression = pk_atom_nil(lisp),
    };
    pk_try(pk_call(lisp, call));
    return PK_Ok;
}

PKRes pk_fastcall(void *user_closure, Pocket lisp, PKFn fn, int arity) {
    if (arity < 0) {
        return pk_error(lisp);
    }
    PKFuncCall call = (PKFuncCall) {
        .as.c.user_closure = user_closure,
        .as.c.fn = fn,
        .final_arity = (size_t)arity,
        .mode = PKFuncMode_Func,
        .expression = pk_atom_nil(lisp),
    };
    pk_try(pk_call(lisp, call));
    return PK_Ok;
}

PKRes pk_funcall(Pocket lisp, int arity) {
    if (arity < 0) {
        return pk_error(lisp);
    }

    PKAtom *atom;
    pk_try(pk_stack_get(lisp, -(arity + 1), &atom));

    if (atom->tag.ty == PKAtomTy_Symbol) {
        PKAtomSymbol *sym = (PKAtomSymbol *)atom;
        pk_try(pk_env_get(lisp, PKEnvTy_Fun, sym, &atom));
    }

    PKFuncCall call = pk_get_callconv(lisp, atom, arity);

    pk_try(pk_call(lisp, call));
    return PK_Ok;
}
