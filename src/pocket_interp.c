#include "pocket_internals.h"

void pk_atom_evlist(Pocket lisp, PKAtom *list) {
    pk_push_nil(lisp);
    pk_cdolist(lisp, el, list) {
        pk_atom_eval(lisp, el);
        pk_swap(lisp, -1, -2);
        pk_pop(lisp);
    }
}

size_t pk_atom_evrec(Pocket lisp, PKAtom *list) {
    switch (list->tag.ty) {
        case PKAtomTy_Nil: {
            return 0;
        }
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)list;
            size_t result = pk_atom_evrec(lisp, cons->cdr) + 1;
            pk_atom_eval(lisp, cons->car);
            return result;
        }
        default: {
            pk_error(lisp);
            return 0;
        }
    }
}

void pk_atom_eval(Pocket lisp, PKAtom *atom) {
    switch (atom->tag.ty) {
        case PKAtomTy_Nil:
        case PKAtomTy_Number:
        case PKAtomTy_String:
        case PKAtomTy_CFunc: {
            pk_push(lisp, atom);
            break;
        }
        case PKAtomTy_Symbol: {
            pk_push(lisp, pk_env_get(lisp, PKEnvTy_Var, (PKAtomSymbol *)atom));
            break;
        }
        case PKAtomTy_Cons: {
            PKAtomCons *form = (PKAtomCons *)atom;
            PKAtom *form_car = form->car;
            if (form_car->tag.ty == PKAtomTy_Symbol) {
                if (pk_atom_eval_special_form(lisp, (PKAtomSymbol *)form_car, form->cdr, (PKAtom *)form)) {
                    return;
                }
                pk_push(lisp, form_car);
            } else {
                pk_atom_eval(lisp, form_car);
            }
            
            int argc = 0;
            pk_cdolist(lisp, el, form->cdr) {
                pk_atom_eval(lisp, el);
                argc += 1;
            }
            pk_funcall(lisp, argc);
            break;
        }
        default: {
            pk_error(lisp);
            break;
        }
    }
}
