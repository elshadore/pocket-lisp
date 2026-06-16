#include "pocket_internals.h"

PKAtom *pk_get_result(Pocket lisp) {
    if (pk_get_top(lisp) > 0) {
        return lisp->stack.e[lisp->stack.count - 1];
    } else {
        return lisp->cache.nil;;
    }
}

void pk_fastcall(void *user_closure, Pocket lisp, PKFn fn, int arity) {
    if (arity < 0) {
        pk_error(lisp);
    }
    size_t uarity = (size_t)arity;
    pk_frame_push(lisp, uarity);
    (fn)(user_closure, lisp);
    PKAtom *result = pk_get_result(lisp);
    pk_frame_pop(lisp);
    pk_push(lisp, result);
    
    pk_gc_collect(lisp);
}

void pk_funcall(Pocket lisp, int arity) {
    if (arity < 0) {
        pk_error(lisp);
    }

    PKAtom *fn = pk_stack_get(lisp, -1);
    pk_pop(lisp);

    if (fn->tag.ty == PKAtomTy_Symbol) {
        PKAtomSymbol *sym = (PKAtomSymbol *)fn;
        fn = (PKAtom *)pk_env_get(lisp, PKEnvTy_Fun, sym);
    }

    size_t uarity = (size_t)arity;
    pk_frame_push(lisp, uarity);

    switch (fn->tag.ty) {
        case PKAtomTy_CFunc: {
            PKAtomCFunc *cfunc = (PKAtomCFunc *)fn;
            (cfunc->fn)(cfunc->user_closure, lisp);
            break;
        }
        case PKAtomTy_Cons: {
            PKAtomCons *a = (PKAtomCons *)fn;
            if (a->car != (PKAtom *)lisp->cache.lambda) {
                pk_error(lisp);
            }
            
            PKAtomCons *b = pk_atom_cast_cons(lisp, a->cdr);
            PKAtom *args = b->car;
          
            int i = 0;
            pk_cdolist(lisp, el, args) {
                if ((size_t)i > uarity) {
                    pk_error(lisp);
                }
                PKAtomSymbol *sym = pk_atom_cast_symbol(lisp, el);
                pk_let_push(lisp, PKEnvTy_Var, sym, pk_stack_get(lisp, i + 1));
                i++;
            }
            
            PKAtom *body = b->cdr;
            if (body->tag.ty != PKAtomTy_Cons) {
                pk_error(lisp);
            }
            if (body->cons.cdr != lisp->cache.nil) {
                pk_error(lisp);
            }

            pk_atom_evlist(lisp, body);
            break;
        }
        default: {
            pk_error(lisp);
            break;
        }
    }

    PKAtom *result = lisp->cache.nil;
    if (pk_get_top(lisp) > 0) {
        result = lisp->stack.e[lisp->stack.count - 1];
    }
    pk_frame_pop(lisp);
    pk_push(lisp, result);

    pk_gc_collect(lisp);
}

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
            if (form->car->tag.ty == PKAtomTy_Symbol &&
                (PKAtomSymbol *)form->car == lisp->cache.lambda) {
                pk_push(lisp, atom);
                break;
            }
            int argc = 0;
            pk_cdolist(lisp, arg, form->cdr) {
                pk_atom_eval(lisp, arg);
                argc++;
            }
            PKAtom *fn_form = form->car;
            if (fn_form->tag.ty == PKAtomTy_Symbol) {
                pk_push(lisp, fn_form);
            } else {
                pk_atom_eval(lisp, fn_form);
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

void pk_eval(Pocket lisp, int stack_pointer) {
    PKAtom *expr = pk_stack_get(lisp, stack_pointer);
    pk_frame_push(lisp, 0);
    pk_atom_eval(lisp, expr);
    PKAtom *result = pk_get_result(lisp);
    pk_frame_pop(lisp);
    pk_push(lisp, result);
}

void pk_evlist(Pocket lisp, int stack_pointer) {
    PKAtom *expr = pk_stack_get(lisp, stack_pointer);
    pk_frame_push(lisp, 0);
    pk_atom_evlist(lisp, expr);
    PKAtom *result = pk_get_result(lisp);
    pk_frame_pop(lisp);
    pk_push(lisp, result);
}
