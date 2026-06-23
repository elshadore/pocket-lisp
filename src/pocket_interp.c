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
