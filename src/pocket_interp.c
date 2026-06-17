#include "pocket_internals.h"

void pk_atom_evlist(Pocket lisp, PKAtom *list) {
    pk_push_nil(lisp);
    pk_cdolist(lisp, el, list) {
        pk_atom_eval(lisp, el);
        pk_swap(lisp, -1, -2);
        pk_pop(lisp);
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
            if (form->car->tag.ty == PKAtomTy_Symbol && (PKAtomSymbol *)form->car == lisp->cache.lambda) {
                pk_push(lisp, atom);
                break;
            }
            int argc = 0;
            PKAtom *fn_form = form->car;
            if (fn_form->tag.ty == PKAtomTy_Symbol) {
                pk_push(lisp, fn_form);
            } else {
                pk_atom_eval(lisp, fn_form);
            }
            pk_cdolist(lisp, arg, form->cdr) {
                pk_atom_eval(lisp, arg);
                argc++;
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
