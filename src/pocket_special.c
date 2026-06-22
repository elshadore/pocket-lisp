#include "pocket_internals.h"

bool pk_atom_eval_quasiquote(Pocket lisp, PKAtom *atom) {
    switch (atom->tag.ty) {
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            if ((cons->car == (PKAtom *)lisp->cache.unquote) || (cons->car == (PKAtom *)lisp->cache.unquote_splice)) {
                PKAtomCons *a = pk_atom_cast_cons(lisp, cons->cdr);
                if (a->cdr != lisp->cache.nil) {
                    pk_error(lisp);
                }
                pk_atom_eval(lisp, a->car);
                return cons->car == (PKAtom *)lisp->cache.unquote_splice;
            }
            PKAtomCons *acc = NULL;
            pk_cdolist(lisp, el, atom) {
                bool res = pk_atom_eval_quasiquote(lisp, el);
                PKAtom *pop = pk_stack_pop(lisp);
                if (res) {
                    PKAtomCons *head = pk_atom_cast_cons(lisp, pop);
                    PKAtomCons *tail = pk_atom_cons_tail(lisp, head);
                    if (acc == NULL) {
                        acc = tail;
                        pk_push(lisp, (PKAtom *)acc);
                    } else {
                        acc->cdr = (PKAtom *)head;
                        acc = tail;
                    }
                } else {
                    PKAtomCons *next = pk_atom_cons(lisp, pop, lisp->cache.nil);
                    if (acc == NULL) {
                        acc = next;
                        pk_push(lisp, (PKAtom *)acc);
                    } else {
                        acc->cdr = (PKAtom *)next;
                        acc = next;
                    }
                }
            }
            if (acc == NULL) {
                pk_push_nil(lisp);
            }
            break;
        }
        default: {
            pk_push(lisp, atom);
            break;
        }
    }
    return false;
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
    } else if (symbol == lisp->cache.quasiquote) {
        if (body->tag.ty != PKAtomTy_Cons) {
            pk_error(lisp);
        }
        PKAtomCons *body_cons = (PKAtomCons *)body;
        if (body_cons->cdr != lisp->cache.nil) {
            pk_error(lisp);
        }
        (void)pk_atom_eval_quasiquote(lisp, body_cons->car);
    } else if (symbol == lisp->cache.unquote) {
        pk_error(lisp);
    } else if (symbol == lisp->cache.unquote_splice) {
        pk_error(lisp);
    } else if (symbol == lisp->cache.progn) {
        pk_atom_evlist(lisp, body);
    } else if (symbol == lisp->cache.while_sym) {
        if (body->tag.ty != PKAtomTy_Cons) {
            pk_error(lisp);
        }
        PKAtomCons *body_cons = (PKAtomCons *)body;
        PKAtom *cond = body_cons->car;
        PKAtom *loop = body_cons->cdr;
        
        pk_push_nil(lisp);
        for (;;) {
            pk_atom_eval(lisp, cond);
            PKAtom *a = pk_stack_get(lisp, -1);
            if (a == lisp->cache.nil) {
                break;
            }
            pk_pop(lisp);
            pk_atom_evlist(lisp, loop);
            pk_swap(lisp, -1, -2);
            pk_pop(lisp);
        }
    } else if (symbol == lisp->cache.if_sym) {
        if (body->tag.ty != PKAtomTy_Cons) {
            pk_error(lisp);
        }
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
