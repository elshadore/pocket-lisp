#include "pocket_internals.h"

PKRes pk_atom_evlist(Pocket lisp, PKAtom *list) {
    pk_try(pk_push_nil(lisp));
    pk_cdolist(lisp, el, list) {
        pk_try(pk_atom_eval(lisp, el));
        pk_try(pk_swap(lisp, -1, -2));
        pk_try(pk_pop(lisp));
    }
    return PK_Ok;
}

PKRes pk_atom_eval(Pocket lisp, PKAtom *atom) {
    switch (atom->tag.ty) {
        case PKAtomTy_Nil:
        case PKAtomTy_Number:
        case PKAtomTy_String:
        case PKAtomTy_CFunc: {
            pk_try(pk_push(lisp, atom));
            return PK_Ok;
        }
        case PKAtomTy_Symbol: {
            PKAtom *value;
            pk_try(pk_env_get(lisp, PKEnvTy_Var, (PKAtomSymbol *)atom, &value));
            pk_try(pk_push(lisp, value));
            return PK_Ok;
        }
        case PKAtomTy_Cons: {
            PKAtomCons *form = (PKAtomCons *)atom;
            PKAtom *form_car = form->car;
            if (form_car->tag.ty == PKAtomTy_Symbol) {
                bool handled;
                pk_try(pk_atom_eval_special_form(lisp, (PKAtomSymbol *)form_car, form->cdr, (PKAtom *)form, &handled));
                if (handled) return PK_Ok;
                pk_try(pk_push(lisp, form_car));
            } else {
                pk_try(pk_atom_eval(lisp, form_car));
            }

            int argc = 0;
            pk_cdolist(lisp, el, form->cdr) {
                pk_try(pk_atom_eval(lisp, el));
                argc += 1;
            }
            pk_try(pk_funcall(lisp, argc));
            return PK_Ok;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

PKRes pk_ret(Pocket lisp) {
    PKAtom *atom = pk_atom_nil(lisp);
    PKRes result = PK_Ok;
    if (!pk_stack_head(lisp, &atom)) {
        result = PK_Yield;
    }
    if (!pk_frame_pop(lisp)) {
        result = PK_Yield;
    }
    return result;
}

PKRes pk_interp_eval(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.atom;
    switch (atom->tag.ty) {
        case PKAtomTy_Nil:
        case PKAtomTy_Number:
        case PKAtomTy_String:
        case PKAtomTy_CFunc: {
            pk_try(pk_push(lisp, atom));
            pk_try(pk_ret(lisp));
            return PK_Ok;
        }
        case PKAtomTy_Symbol: {
            PKAtom *value;
            pk_try(pk_env_get(lisp, PKEnvTy_Var, (PKAtomSymbol *)atom, &value));
            pk_try(pk_push(lisp, value));
            pk_try(pk_ret(lisp));
            return PK_Ok;
        }
        case PKAtomTy_Cons: {
            pk_try(pk_ret(lisp));
            PKAtomCons *form = (PKAtomCons *)atom;
            pk_try(pk_frame_push(lisp, 0, PKEvalMode_Apply, form->car));
            pk_cdolist(lisp, el, form->cdr) {
                pk_try(pk_frame_push(lisp, 0, PKEvalMode_Eval, el));
            }
            return PK_Ok;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

PKRes pk_interp_apply(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.atom;
    PKAtomCFunc *func = NULL;
    pk_try(pk_atom_cast_cfunc(lisp, atom, &func));
    pk_try((func->fn)(func->user_closure, lisp));
    pk_try(pk_ret(lisp));
    return PK_Ok;
}

PKRes pk_interp(Pocket lisp, size_t stop) {
    while (lisp->stack.count > stop) {
        switch (lisp->current_frame.mode) {
            case PKEvalMode_Eval: {
                pk_try(pk_interp_eval(lisp));
                break;
            }
            case PKEvalMode_Apply: {
                pk_try(pk_interp_apply(lisp));
                break;
            }
        }
    }
    return PK_Ok;
}
