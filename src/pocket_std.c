#include "pocket_internals.h"

PKRes pk_fn_add(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_get_top(lisp);
    pk_try(pk_dupe(lisp, 1));
    for (int i = 2; i <= top; ++i) {
        pk_try(pk_add(lisp, -1, i));
        pk_try(pk_swap(lisp, -1, -2));
        pk_try(pk_pop(lisp));
    }
    return PK_Ok;
}

PKRes pk_fn_sub(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_get_top(lisp);
    pk_try(pk_dupe(lisp, 1));
    for (int i = 2; i <= top; ++i) {
        pk_try(pk_sub(lisp, -1, i));
        pk_try(pk_swap(lisp, -1, -2));
        pk_try(pk_pop(lisp));
    }
    return PK_Ok;
}

PKRes pk_fn_mul(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_get_top(lisp);
    pk_try(pk_dupe(lisp, 1));
    for (int i = 2; i <= top; ++i) {
        pk_try(pk_add(lisp, -1, i));
        pk_try(pk_swap(lisp, -1, -2));
        pk_try(pk_pop(lisp));
    }
    return PK_Ok;
}

PKRes pk_fn_div(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_get_top(lisp);
    pk_try(pk_dupe(lisp, 1));
    for (int i = 2; i <= top; ++i) {
        pk_try(pk_add(lisp, -1, i));
        pk_try(pk_swap(lisp, -1, -2));
        pk_try(pk_pop(lisp));
    }
    return PK_Ok;
}

PKRes pk_fn_slurp(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKString path;
    pk_try(pk_to_string(lisp, 1, &path));
    PKString contents;
    pk_try(pk_slurp(lisp, path.c, &contents));
    PKAtomString *string;
    pk_try(pk_atom_string_nomemcpy(lisp, contents, &string));
    pk_try(pk_push(lisp, (PKAtom *)string));
    return PK_Ok;
}

PKRes pk_fn_read(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_read(lisp, 1));
    return PK_Ok;
}

PKRes pk_fn_eval(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_eval(lisp, 1));
    return PK_Ok;
}

PKRes pk_fn_evlist(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_evlist(lisp, 1));
    return PK_Ok;
}

PKRes pk_fn_format(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_format(lisp, 1));
    return PK_Ok;
}

PKRes pk_fn_print(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKString string;
    pk_try(pk_to_string(lisp, 1, &string));
    pk_try(pk_print(lisp, string.c, string.length));
    return PK_Ok;
}

PKRes pk_fn_puts(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKString string;
    pk_try(pk_to_string(lisp, 1, &string));
    pk_try(pk_puts(lisp, string.c, string.length));
    return PK_Ok;
}

PKRes pk_fn_set(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_set(lisp, 1, 2));
    return PK_Ok;
}

PKRes pk_fn_get(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_get(lisp, 1));
    return PK_Ok;
}

PKRes pk_fn_unbind(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_unbind(lisp, 1));
    return PK_Ok;
}

PKRes pk_fn_fset(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_fset(lisp, 1, 2));
    return PK_Ok;
}

PKRes pk_fn_fget(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_fget(lisp, 1));
    return PK_Ok;
}

PKRes pk_fn_funbind(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_funbind(lisp, 1));
    return PK_Ok;
}

PKRes pk_fn_car(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_car(lisp, 1));
    return PK_Ok;
}

PKRes pk_fn_cdr(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_cdr(lisp, 1));
    return PK_Ok;
}

PKRes pk_fn_set_car(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtom *c_atom;
    pk_try(pk_stack_get(lisp, 2, &c_atom));
    PKAtomCons *c;
    pk_try(pk_atom_cast_cons(lisp, c_atom, &c));
    PKAtom *old = c->car;
    pk_try(pk_set_car(lisp, 1, 2));
    pk_try(pk_push(lisp, old));
    return PK_Ok;
}

PKRes pk_fn_set_cdr(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtom *c_atom;
    pk_try(pk_stack_get(lisp, 2, &c_atom));
    PKAtomCons *c;
    pk_try(pk_atom_cast_cons(lisp, c_atom, &c));
    PKAtom *old = c->cdr;
    pk_try(pk_set_cdr(lisp, 1, 2));
    pk_try(pk_push(lisp, old));
    return PK_Ok;
}

PKRes pk_fn_gt(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_get_top(lisp);
    for (int i = 1; i < top; ++i) {
        bool result;
        pk_try(pk_gt(lisp, i, i + 1, &result));
        if (!result) {
            pk_try(pk_push_nil(lisp));
            return PK_Ok;
        }
    }
    pk_try(pk_push_t(lisp));
    return PK_Ok;
}

PKRes pk_fn_gte(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_get_top(lisp);
    for (int i = 1; i < top; ++i) {
        bool result;
        pk_try(pk_gte(lisp, i, i + 1, &result));
        if (!result) {
            pk_try(pk_push_nil(lisp));
            return PK_Ok;
        }
    }
    pk_try(pk_push_t(lisp));
    return PK_Ok;
}

PKRes pk_fn_lt(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_get_top(lisp);
    for (int i = 1; i < top; ++i) {
        bool result;
        pk_try(pk_lt(lisp, i, i + 1, &result));
        if (!result) {
            pk_try(pk_push_nil(lisp));
            return PK_Ok;
        }
    }
    pk_try(pk_push_t(lisp));
    return PK_Ok;
}

PKRes pk_fn_lte(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_get_top(lisp);
    for (int i = 1; i < top; ++i) {
        bool result;
        pk_try(pk_lte(lisp, i, i + 1, &result));
        if (!result) {
            pk_try(pk_push_nil(lisp));
            return PK_Ok;
        }
    }
    pk_try(pk_push_t(lisp));
    return PK_Ok;
}

PKRes pk_fn_eq(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_get_top(lisp);
    for (int i = 1; i < top; ++i) {
        bool result;
        pk_try(pk_eq(lisp, i, i + 1, &result));
        if (!result) {
            pk_try(pk_push_nil(lisp));
            return PK_Ok;
        }
    }
    pk_try(pk_push_t(lisp));
    return PK_Ok;
}

PKRes pk_fn_list(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtoms slice = pk_stack_slice(lisp);
    PKAtom *result;
    pk_try(pk_slice_list(lisp, slice, &result));
    pk_try(pk_push(lisp, result));
    return PK_Ok;
}

PKRes pk_fn_list_reversed(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtoms slice = pk_stack_slice(lisp);
    PKAtom *result;
    pk_try(pk_slice_list_rev(lisp, slice, &result));
    pk_try(pk_push(lisp, result));
    return PK_Ok;
}

PKRes pk_fn_list_vars(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtom *result;
    pk_try(pk_symtable_alist(lisp, &lisp->vars, &result));
    pk_try(pk_push(lisp, result));
    return PK_Ok;
}

PKRes pk_fn_list_funs(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtom *result;
    pk_try(pk_symtable_alist(lisp, &lisp->funs, &result));
    pk_try(pk_push(lisp, result));
    return PK_Ok;
}

PKRes pk_load_std(Pocket lisp) {
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
        {.sym = pkstr("list"), .fn = pk_fn_list, .args = 0, .mode = PKArity_Variadic},
        {.sym = pkstr("list-rev"), .fn = pk_fn_list_reversed, .args = 0, .mode = PKArity_Variadic},
    };

    size_t count = pk_alen(lib);

    for (size_t i = 0; i < count; ++i) {
        PKFuncRecord *rec = &lib[i];
        PKAtomCFunc *cfunc;
        pk_try(pk_atom_cfunc(lisp, rec->user_closure, rec->fn, (PKFuncArity){.args = rec->args, .mode = rec->mode}, &cfunc));
        PKAtomSymbol *sym;
        pk_try(pk_atom_symbol_interned(lisp, rec->sym, &sym));
        PKAtom *_ignored;
        pk_try(pk_env_set(lisp, PKEnvTy_Fun, sym, (PKAtom *)cfunc, &_ignored));
    }
    return PK_Ok;
}
