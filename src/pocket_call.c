#include "pocket_internals.h"

PKRes pk_bind_lambda_list(Pocket lisp, PKAtom *symbols, PKAtoms values) {
    size_t i = 0;
    pk_cdolist(lisp, el, symbols) {
        if (i >= values.length) {
            return pk_error(lisp);
        }
        size_t index = pk_index_inv(i, values.length);
        PKAtom *atom = values.e[index];
        PKAtomSymbol *sym = NULL;
        pk_try(pk_atom_cast_symbol(lisp, el, &sym));
        pk_try(pk_let_push(lisp, PKEnvTy_Var, sym, atom));
        i++;
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
            *output = (PKCallConv) {
                .ty = PKFuncTy_CFunc,
                .as.c.fn = cfunc->fn,
                .as.c.user_closure = cfunc->user_closure,
                .final_arity = farity,
                .mode = PKFuncMode_Func,
                .insert_result = true,
                .extra_nils = nils,
                .expression = atom,
            };
            return PK_Ok;
        }
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            PKAtomSymbol *sym = NULL;
            pk_try(pk_atom_cast_symbol(lisp, cons->car, &sym));

            PKFuncMode mode = PKFuncMode_Func;
            if (sym == lisp->cache.lambda) {
                mode = PKFuncMode_Func;
            } else if (sym == lisp->cache.macro) {
                mode = PKFuncMode_Macro;
            } else {
                return pk_error(lisp);
            }

            PKAtomCons *a = NULL;
            pk_try(pk_atom_cast_cons(lisp, cons->cdr, &a));

            *output = (PKCallConv) {
                .ty = PKFuncTy_Lambda,
                .as.lisp.args = a->car,
                .as.lisp.body = a->cdr,
                .final_arity = arity,
                .mode = mode,
                .insert_result = true,
                .expression = atom,
            };
            return PK_Ok;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

PKRes pk_call(Pocket lisp, PKAtom *atom, size_t arity) {
    size_t save = pk_frame_savepoint(lisp);
    pk_try(pk_frame_push(lisp, (size_t)arity, PKEvalMode_Apply, (PKFrameData){.atom = (PKAtom *)atom}));
    pk_try(pk_interp(lisp, save));
    return PK_Ok;
}

PKRes pk_eval(Pocket lisp, int stack_pointer) {
    PKAtom *atom = NULL;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    size_t save = pk_frame_savepoint(lisp);
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Eval, (PKFrameData){.atom = (PKAtom *)atom}));
    pk_try(pk_interp(lisp, save));
    return PK_Ok;
}

PKRes pk_evlist(Pocket lisp, int stack_pointer) {
    PKAtom *atom = NULL;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    size_t save = pk_frame_savepoint(lisp);
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Evlist, (PKFrameData){.atom = (PKAtom *)atom}));
    pk_try(pk_interp(lisp, save));
    return PK_Ok;
}

PKRes pk_fastcall(void *user_closure, Pocket lisp, PKFn fn, int arity) {
    PKAtomCFunc *quick = NULL;
    pk_try(pk_atom_cfunc(lisp, user_closure, fn, (PKFuncArity){.args = arity, .mode = PKArity_Normal}, &quick));
    pk_try(pk_call(lisp, (PKAtom *)quick, (size_t)arity));
    return PK_Ok;
}

PKRes pk_funcall(Pocket lisp, int arity) {
    if (arity < 0) {
        return pk_error(lisp);
    }
    PKAtom *atom = NULL;
    pk_try(pk_stack_get(lisp, -(arity + 1), &atom));
    pk_try(pk_call(lisp, (PKAtom *)atom, (size_t)arity));
    return PK_Ok;
}
