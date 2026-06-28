#include "pocket_internals.h"

PKRes pk_ret_top(Pocket lisp) {
    PKAtom *atom = pk_atom_nil(lisp);
    PKRes result = PK_Ok;
    if (!pk_stack_head(lisp, &atom)) {
        result = PK_Yield;
    }
    if (!pk_ret_this(lisp, atom)) {
        result = PK_Yield;
    }
    return result;
}

PKRes pk_ret_this(Pocket lisp, PKAtom *atom) {
    PKRes result = PK_Ok;
    if (!pk_frame_pop_clear(lisp)) {
        result = PK_Yield;
    }
    if (!pk_push(lisp, atom)) {
        result = PK_Yield;
    }
    return result;
}

PKRes pk_ret_all(Pocket lisp) {
    return pk_frame_pop_return(lisp);
}

PKRes pk_ret_none(Pocket lisp) {
    return pk_frame_pop_clear(lisp);
}

PKRes pk_interp_eval(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.as.atom;
    switch (atom->tag.ty) {
        case PKAtomTy_Nil:
        case PKAtomTy_Number:
        case PKAtomTy_String:
        case PKAtomTy_CFunc: {
            pk_try(pk_push(lisp, atom));
            pk_try(pk_ret_top(lisp));
            return PK_Ok;
        }
        case PKAtomTy_Symbol: {
            PKAtom *value;
            pk_try(pk_env_get(lisp, PKEnvTy_Var, (PKAtomSymbol *)atom, &value));
            pk_try(pk_push(lisp, value));
            pk_try(pk_ret_top(lisp));
            return PK_Ok;
        }
        case PKAtomTy_Cons: {
            pk_try(pk_ret_all(lisp));
            PKAtomCons *form = (PKAtomCons *)atom;
            if (pk_atom_is_symbol(form->car)) {
                bool is_special = false;
                pk_try(pk_interp_special_form(lisp, (PKAtomSymbol *)form->car, form->cdr, (PKAtom *)form, &is_special));
                if (is_special) {
                    return PK_Ok;
                }
            }
            pk_try(pk_frame_push(lisp, 0, PKEvalMode_Apply, (PKFrameData){.atom = form->car}));
            pk_try(pk_frame_push(lisp, 0, PKEvalMode_Evargs, (PKFrameData){.atom = form->cdr}));
            return PK_Ok;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

PKRes pk_interp_apply(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.as.atom;
    if (atom->tag.ty == PKAtomTy_Symbol) {
        PKAtomSymbol *sym = (PKAtomSymbol *)atom;
        pk_try(pk_env_get(lisp, PKEnvTy_Fun, sym, &atom));
    }
    PKAtomCFunc *func = NULL;
    pk_try(pk_atom_cast_cfunc(lisp, atom, &func));
    pk_try((func->fn)(func->user_closure, lisp));
    pk_try(pk_ret_top(lisp));
    return PK_Ok;
}

PKRes pk_interp_evlist(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.as.atom;
    switch (atom->tag.ty) {
        case PKAtomTy_Nil: {
            pk_try(pk_push_nil(lisp));
            pk_try(pk_ret_top(lisp));
            return PK_Ok;
        }
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            lisp->current_frame.mode = PKEvalMode_Evlist_2;
            lisp->current_frame.as.cons = cons;
            pk_try(pk_push_nil(lisp));
            return PK_Ok;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

PKRes pk_interp_evlist_2(Pocket lisp) {
    PKAtomCons *cons = lisp->current_frame.as.cons;
    switch (cons->cdr->tag.ty) {
        case PKAtomTy_Cons: {
            pk_try(pk_pop(lisp));
            pk_try(pk_frame_push(lisp, 0, PKEvalMode_Eval, (PKFrameData){.atom = cons->car}));
            return PK_Ok;
        }
        case PKAtomTy_Nil: {
            pk_try(pk_ret_none(lisp));
            pk_try(pk_frame_push(lisp, 0, PKEvalMode_Eval, (PKFrameData){.atom = cons->car}));
            return PK_Ok;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

PKRes pk_interp_evargs(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.as.atom;
    switch (atom->tag.ty) {
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            lisp->current_frame.as.atom = cons->cdr;
            pk_try(pk_frame_push(lisp, 0, PKEvalMode_Eval, (PKFrameData){.atom = cons->car}));
            return PK_Ok;
        }
        case PKAtomTy_Nil: {
            pk_try(pk_ret_all(lisp));
            return PK_Ok;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

PKRes pk_interp(Pocket lisp, size_t stop) {
    while (lisp->frames.count > stop) {
        (void)pk_trace_dump(lisp, "interp");
        switch (lisp->current_frame.mode) {
            case PKEvalMode_Root: {
                return pk_error(lisp);
            }
            case PKEvalMode_Eval: {
                pk_try(pk_interp_eval(lisp));
                break;
            }
            case PKEvalMode_Evlist: {
                pk_try(pk_interp_evlist(lisp));
                break;
            }
            case PKEvalMode_Evlist_2: {
                pk_try(pk_interp_evlist_2(lisp));
                break;
            }
            case PKEvalMode_Evargs: {
                pk_try(pk_interp_evargs(lisp));
                break;
            }
            case PKEvalMode_Apply: {
                pk_try(pk_interp_apply(lisp));
                break;
            }
            case PKEvalMode_Quote: {
                pk_try(pk_interp_quote(lisp));
                break;
            }
            case PKEvalMode_If: {
                pk_try(pk_interp_if(lisp));
                break;
            }
        }
    }
    (void)pk_trace_dump(lisp, "result");
    return PK_Ok;
}
