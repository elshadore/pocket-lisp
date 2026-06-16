#include "pocket_internals.h"

void pk_fastcall(void *user_closure, Pocket lisp, PKFn fn, int arity) {
    if (arity < 0) {
        pk_error(lisp);
    }
    size_t uarity = (size_t)arity;
    pk_frame_push(lisp, uarity);
    (fn)(user_closure, lisp);
    
    PKAtom *result = lisp->cache.nil;
    if (pk_get_top(lisp) > 0) {
        result = lisp->stack.e[lisp->stack.count - 1];
    }
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

void pk_interpret(Pocket lisp, PKAtom *function, int args) {
    
}
