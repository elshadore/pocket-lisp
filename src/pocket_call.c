#include "pocket_internals.h"

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

    PKAtom *call = pk_stack_get(lisp, -(arity + 1));

    if (call->tag.ty == PKAtomTy_Symbol) {
        PKAtomSymbol *sym = (PKAtomSymbol *)call;
        call = (PKAtom *)pk_env_get(lisp, PKEnvTy_Fun, sym);
    }

    size_t uarity = (size_t)arity;
    pk_frame_push(lisp, uarity);

    switch (call->tag.ty) {
        case PKAtomTy_CFunc: {
            PKAtomCFunc *cfunc = (PKAtomCFunc *)call;
            (cfunc->fn)(cfunc->user_closure, lisp);
            break;
        }
        case PKAtomTy_Cons: {
            PKAtomCons *a = (PKAtomCons *)call;
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
            
            pk_atom_evlist(lisp, b->cdr);
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
    
    lisp->stack.e[lisp->stack.count - 1] = result;

    pk_gc_collect(lisp);
}
