#include "pocket_internals.h"

void pk_atom_evlist(Pocket lisp, PKAtom *list) {
    pk_push_nil(lisp);
    pk_cdolist(lisp, el, list) {
        pk_atom_eval(lisp, el);
        pk_swap(lisp, -1, -2);
        pk_pop(lisp);
    }
}

bool pk_atom_eval_special_form(Pocket lisp, PKAtomSymbol *symbol, PKAtom *body, PKAtom *expression) {
    if (symbol == lisp->cache.lambda) {
        pk_push(lisp, expression);
    } else if (symbol == lisp->cache.quote) {
        if (body->tag.ty != PKAtomTy_Cons) {
            pk_error(lisp);
        }
        PKAtomCons *body_cons = (PKAtomCons *)body;
        if (body_cons->cdr != lisp->cache.nil) {
            pk_error(lisp);
        }
        pk_push(lisp, body_cons->car);
    } else if (symbol == lisp->cache.if_sym) {
        if (body->tag.ty != PKAtomTy_Cons) pk_error(lisp);
        PKAtomCons *body_cons = (PKAtomCons *)body;
        PKAtomCons *then_cons;
        PKAtomCons *else_cons;

        if (body_cons->cdr->tag.ty != PKAtomTy_Cons) pk_error(lisp);
        then_cons = (PKAtomCons *)body_cons->cdr;
        if (then_cons->cdr->tag.ty != PKAtomTy_Cons) pk_error(lisp);
        else_cons = (PKAtomCons *)then_cons->cdr;
        if (else_cons->cdr != lisp->cache.nil) pk_error(lisp);

        pk_atom_eval(lisp, body_cons->car);
        PKAtom *cond = pk_stack_get(lisp, -1);
        pk_pop(lisp);

        if (cond != lisp->cache.nil) {
            pk_atom_eval(lisp, then_cons->car);
        } else {
            pk_atom_eval(lisp, else_cons->car);
        }
    } else {
        return false;
    }
    return true;
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
