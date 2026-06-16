#include "pocket_internals.h"

void pk_fn_add(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) >= 2) {
        pk_add(lisp, -1, -2);
        pk_swap(lisp, -1, -3);
        pk_popn(lisp, 2);
    }
}

void pk_fn_sub(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) >= 2) {
        pk_sub(lisp, -1, -2);
        pk_swap(lisp, -1, -3);
        pk_popn(lisp, 2);
    }
}

void pk_fn_mul(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) >= 2) {
        pk_mul(lisp, -1, -2);
        pk_swap(lisp, -1, -3);
        pk_popn(lisp, 2);
    }
}

void pk_fn_div(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) >= 2) {
        pk_div(lisp, -1, -2);
        pk_swap(lisp, -1, -3);
        pk_popn(lisp, 2);
    }
}

void pk_load_std(Pocket lisp) {
    PKFuncRecord lib[] = {
        {.sym = pkstr("+"), .fn = pk_fn_add, .args = 2, .mode = PKArity_Variadic},
        {.sym = pkstr("-"), .fn = pk_fn_sub, .args = 2, .mode = PKArity_Variadic},
        {.sym = pkstr("*"), .fn = pk_fn_mul, .args = 2, .mode = PKArity_Variadic},
        {.sym = pkstr("/"), .fn = pk_fn_div, .args = 2, .mode = PKArity_Variadic},
    };
    
    size_t count = pk_alen(lib);
    
    for (size_t i = 0; i < count; ++i) {
        PKFuncRecord *rec = &lib[i];
        PKAtomCFunc *cfunc = pk_atom_cfunc(lisp, rec->user_closure, rec->fn, (PKFuncArity){.args = rec->args, .mode = rec->mode});
        PKAtomSymbol *sym = pk_atom_symbol_interned(lisp, rec->sym);
        pk_env_set(lisp, PKEnvTy_Fun, sym, (PKAtom *)cfunc);
    }
}
