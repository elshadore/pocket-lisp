#include "pocket_internals.h"

void pk_fn_add(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) > 2) {
        pk_add(lisp, -1, -2);
        pk_swap(lisp, -1, -3);
        pk_popn(lisp, 2);
    }
}

void pk_fn_sub(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) > 2) {
        pk_sub(lisp, -1, -2);
        pk_swap(lisp, -1, -3);
        pk_popn(lisp, 2);
    }
}

void pk_fn_mul(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) > 2) {
        pk_mul(lisp, -1, -2);
        pk_swap(lisp, -1, -3);
        pk_popn(lisp, 2);
    }
}

void pk_fn_div(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) > 2) {
        pk_div(lisp, -1, -2);
        pk_swap(lisp, -1, -3);
        pk_popn(lisp, 2);
    }
}

void pk_fn_slurp(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKString path = pk_to_string(lisp, -1);
    PKString contents = pk_slurp(lisp, path.c);
    PKAtomString *string = pk_atom_string_nomemcpy(lisp, contents);
    pk_push(lisp, (PKAtom *)string);
}

void pk_fn_read(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_read(lisp, -1);
}

void pk_fn_eval(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_eval(lisp, -1);
}

void pk_fn_evlist(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_evlist(lisp, -1);
}

void pk_fn_format(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_format(lisp, -1);
}

void pk_fn_print(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKString string = pk_to_string(lisp, -1);
    pk_print(lisp, string.c, string.length);
}

void pk_fn_puts(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKString string = pk_to_string(lisp, -1);
    pk_puts(lisp, string.c, string.length);
}

void pk_fn_set(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_set(lisp, -1, -2);
}

void pk_fn_get(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get(lisp, -1);
}

void pk_fn_unbind(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_unbind(lisp, -1);
}

void pk_fn_fset(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_fset(lisp, -1, -2);
}

void pk_fn_fget(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_fget(lisp, -1);
}

void pk_fn_funbind(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_funbind(lisp, -1);
}

void pk_fn_car(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_car(lisp, -1);
}

void pk_fn_cdr(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_cdr(lisp, -1);
}

void pk_fn_set_car(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomCons *c = pk_atom_cast_cons(lisp, pk_stack_get(lisp, 2));
    PKAtom *old = c->car;
    pk_set_car(lisp, -1, -2);
    pk_push(lisp, old);
}

void pk_fn_set_cdr(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomCons *c = pk_atom_cast_cons(lisp, pk_stack_get(lisp, 2));
    PKAtom *old = c->cdr;
    pk_set_cdr(lisp, -1, -2);
    pk_push(lisp, old);
}

void pk_fn_gt(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) > 2) {
        if (!pk_gt(lisp, -1, -2)) {
            pk_push_nil(lisp);
            return;
        }
        pk_pop(lisp);
    }
    pk_push_t(lisp);
}

void pk_fn_gte(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) > 2) {
        if (!pk_gte(lisp, -1, -2)) {
            pk_push_nil(lisp);
            return;
        }
        pk_pop(lisp);
    }
    pk_push_t(lisp);
}

void pk_fn_lt(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) > 2) {
        if (!pk_lt(lisp, -1, -2)) {
            pk_push_nil(lisp);
            return;
        }
        pk_pop(lisp);
    }
    pk_push_t(lisp);
    
}

void pk_fn_lte(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) > 2) {
        if (!pk_lte(lisp, -1, -2)) {
            pk_push_nil(lisp);
            return;
        }
        pk_pop(lisp);
    }
    pk_push_t(lisp);
}

void pk_fn_eq(void *user_closure, Pocket lisp) {
    (void)user_closure;
    while (pk_get_top(lisp) > 2) {
        if (!pk_eq(lisp, -1, -2)) {
            pk_push_nil(lisp);
            return;
        }
        pk_pop(lisp);
    }
    pk_push_t(lisp);
}

void pk_fn_list_vars(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push(lisp, pk_symtable_alist(lisp, &lisp->vars));
}

void pk_fn_list_funs(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push(lisp, pk_symtable_alist(lisp, &lisp->funs));
}

void pk_load_std(Pocket lisp) {
    PKFuncRecord lib[] = {
        {.sym = pkstr("+"), .fn = pk_fn_add, .args = 2, .mode = PKArity_Variadic},
        {.sym = pkstr("-"), .fn = pk_fn_sub, .args = 2, .mode = PKArity_Variadic},
        {.sym = pkstr("*"), .fn = pk_fn_mul, .args = 2, .mode = PKArity_Variadic},
        {.sym = pkstr("/"), .fn = pk_fn_div, .args = 2, .mode = PKArity_Variadic},
        {.sym = pkstr("list-vars"), .fn = pk_fn_list_vars, .args = 0, .mode = PKArity_Normal},
        {.sym = pkstr("list-funs"), .fn = pk_fn_list_funs, .args = 0, .mode = PKArity_Normal},
        {.sym = pkstr("slurp"), .fn = pk_fn_slurp, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("read"), .fn = pk_fn_read, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("eval"), .fn = pk_fn_eval, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("evlist"), .fn = pk_fn_evlist, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("print"), .fn = pk_fn_print, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("puts"), .fn = pk_fn_puts, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("format"), .fn = pk_fn_format, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("set"), .fn = pk_fn_set, .args = 2, .mode = PKArity_Normal},
        {.sym = pkstr("get"), .fn = pk_fn_get, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("unbind"), .fn = pk_fn_unbind, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("fset"), .fn = pk_fn_fset, .args = 2, .mode = PKArity_Normal},
        {.sym = pkstr("fget"), .fn = pk_fn_fget, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("funbind"), .fn = pk_fn_funbind, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("car"), .fn = pk_fn_car, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("cdr"), .fn = pk_fn_cdr, .args = 1, .mode = PKArity_Normal},
        {.sym = pkstr("set-car"), .fn = pk_fn_set_car, .args = 2, .mode = PKArity_Normal},
        {.sym = pkstr("set-cdr"), .fn = pk_fn_set_cdr, .args = 2, .mode = PKArity_Normal},
        {.sym = pkstr(">"), .fn = pk_fn_gt, .args = 2, .mode = PKArity_Variadic},
        {.sym = pkstr(">="), .fn = pk_fn_gte, .args = 2, .mode = PKArity_Variadic},
        {.sym = pkstr("<"), .fn = pk_fn_lt, .args = 2, .mode = PKArity_Variadic},
        {.sym = pkstr("<="), .fn = pk_fn_lte, .args = 2, .mode = PKArity_Variadic},
        {.sym = pkstr("="), .fn = pk_fn_eq, .args = 2, .mode = PKArity_Variadic},
        {.sym = pkstr("eq?"), .fn = pk_fn_eq, .args = 2, .mode = PKArity_Variadic},
    };
    
    size_t count = pk_alen(lib);
    
    for (size_t i = 0; i < count; ++i) {
        PKFuncRecord *rec = &lib[i];
        PKAtomCFunc *cfunc = pk_atom_cfunc(lisp, rec->user_closure, rec->fn, (PKFuncArity){.args = rec->args, .mode = rec->mode});
        PKAtomSymbol *sym = pk_atom_symbol_interned(lisp, rec->sym);
        pk_env_set(lisp, PKEnvTy_Fun, sym, (PKAtom *)cfunc);
    }
}
