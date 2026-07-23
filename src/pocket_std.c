#include "pocket_internals.h"

PKRes pk_fn_add(void *user_closure, Pocket lisp) {
    int top = 0;
    int i = 0;
    
    (void)user_closure;
    top = pk_get_top(lisp);
    pk_try(pk_dupe(lisp, 1));
    for (i = 2; i <= top; ++i) {
        pk_try(pk_add(lisp, -1, i));
        pk_try(pk_swap(lisp, -1, -2));
        pk_try(pk_pop(lisp));
    }
    return PK_Ok;
}

PKRes pk_fn_sub(void *user_closure, Pocket lisp) {
    int top = 0;
    int i = 0;
    
    (void)user_closure;
    pk_get_top(lisp);
    pk_try(pk_dupe(lisp, 1));
    for (i = 2; i <= top; ++i) {
        pk_try(pk_sub(lisp, -1, i));
        pk_try(pk_swap(lisp, -1, -2));
        pk_try(pk_pop(lisp));
    }
    return PK_Ok;
}

PKRes pk_fn_mul(void *user_closure, Pocket lisp) {
    int top = 0;
    int i = 0;
    
    (void)user_closure;
    top = pk_get_top(lisp);
    pk_try(pk_dupe(lisp, 1));
    for (i = 2; i <= top; ++i) {
        pk_try(pk_add(lisp, -1, i));
        pk_try(pk_swap(lisp, -1, -2));
        pk_try(pk_pop(lisp));
    }
    return PK_Ok;
}

PKRes pk_fn_div(void *user_closure, Pocket lisp) {
    int top = 0;
    int i = 0;
    
    (void)user_closure;
    top = pk_get_top(lisp);
    pk_try(pk_dupe(lisp, 1));
    for (i = 2; i <= top; ++i) {
        pk_try(pk_add(lisp, -1, i));
        pk_try(pk_swap(lisp, -1, -2));
        pk_try(pk_pop(lisp));
    }
    return PK_Ok;
}

PKRes pk_fn_slurp(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_try(pk_slurp(lisp, 1));
    return PK_Ok;
}

PKRes pk_fn_read(void *user_closure, Pocket lisp) {
    PK_READ mode = PK_READ_EXPRESSION;
    int boolean = 0;
    
    (void)user_closure;
    
    pk_try(pk_is_nil(lisp, 2, &boolean));
    if (boolean) {
        mode = PK_READ_LISTED;
    }
    pk_try(pk_read(lisp, 1, mode));
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
    char *c = NULL;
    size_t length = 0;
    
    (void)user_closure;
    pk_try(pk_to_string(lisp, 1, &c, &length));
    pk_print(lisp, c, length);
    return PK_Ok;
}

PKRes pk_fn_puts(void *user_closure, Pocket lisp) {
    char *c = NULL;
    size_t length = 0;
    
    (void)user_closure;
    pk_try(pk_to_string(lisp, 1, &c, &length));
    pk_puts(lisp, c, length);
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
    PKAtom *c_atom = NULL;
    PKAtomCons *c = NULL;
    PKAtom *old = NULL;
    
    (void)user_closure;
    pk_try(pk_stack_get(lisp, 2, &c_atom));
    pk_try(pk_atom_cast_cons(lisp, c_atom, &c));
    old = c->car;
    pk_try(pk_set_car(lisp, 1, 2));
    pk_try(pk_push(lisp, old));
    return PK_Ok;
}

PKRes pk_fn_set_cdr(void *user_closure, Pocket lisp) {
    PKAtom *c_atom = NULL;
    PKAtomCons *c = NULL;
    PKAtom *old = NULL;
    (void)user_closure;
    pk_try(pk_stack_get(lisp, 2, &c_atom));
    pk_try(pk_atom_cast_cons(lisp, c_atom, &c));
    old = c->cdr;
    pk_try(pk_set_cdr(lisp, 1, 2));
    pk_try(pk_push(lisp, old));
    return PK_Ok;
}

PKRes pk_fn_gt(void *user_closure, Pocket lisp) {
    int top = 0;
    int i = 0;
    
    (void)user_closure;
    top = pk_get_top(lisp);
    for (i = 1; i < top; ++i) {
        int boolean = 0;
        pk_try(pk_gt(lisp, i, i + 1, &boolean));
        if (!boolean) {
            pk_try(pk_push_nil(lisp));
            return PK_Ok;
        }
    }
    pk_try(pk_push_t(lisp));
    return PK_Ok;
}

PKRes pk_fn_gte(void *user_closure, Pocket lisp) {
    int top = 0;
    int i = 0;
    
    (void)user_closure;
    top = pk_get_top(lisp);
    for (i = 1; i < top; ++i) {
        int boolean = 0;
        pk_try(pk_gte(lisp, i, i + 1, &boolean));
        if (!boolean) {
            pk_try(pk_push_nil(lisp));
            return PK_Ok;
        }
    }
    pk_try(pk_push_t(lisp));
    return PK_Ok;
}

PKRes pk_fn_lt(void *user_closure, Pocket lisp) {
    int top = 0;
    int i = 0;
    
    (void)user_closure;
    top = pk_get_top(lisp);
    for (i = 1; i < top; ++i) {
        int boolean = 0;
        pk_try(pk_lt(lisp, i, i + 1, &boolean));
        if (!boolean) {
            pk_try(pk_push_nil(lisp));
            return PK_Ok;
        }
    }
    pk_try(pk_push_t(lisp));
    return PK_Ok;
}

PKRes pk_fn_lte(void *user_closure, Pocket lisp) {
    int top = 0;
    int i = 0;
    
    (void)user_closure;
    top = pk_get_top(lisp);
    for (i = 1; i < top; ++i) {
        int boolean = 0;
        pk_try(pk_lte(lisp, i, i + 1, &boolean));
        if (!boolean) {
            pk_try(pk_push_nil(lisp));
            return PK_Ok;
        }
    }
    pk_try(pk_push_t(lisp));
    return PK_Ok;
}

PKRes pk_fn_eq(void *user_closure, Pocket lisp) {
    int top = 0;
    int i = 0;
    
    (void)user_closure;
    top = pk_get_top(lisp);
    for (i = 1; i < top; ++i) {
        int boolean = 0;
        pk_try(pk_eq(lisp, i, i + 1, &boolean));
        if (!boolean) {
            pk_try(pk_push_nil(lisp));
            return PK_Ok;
        }
    }
    pk_try(pk_push_t(lisp));
    return PK_Ok;
}

PKRes pk_fn_list(void *user_closure, Pocket lisp) {
    PKAtomSlice slice;
    PKAtom *result = NULL;
    
    (void)user_closure;
    slice = pk_stack_slice(lisp);
    pk_try(pk_slice_list(lisp, slice, &result));
    pk_try(pk_push(lisp, result));
    return PK_Ok;
}

PKRes pk_fn_list_reversed(void *user_closure, Pocket lisp) {
    PKAtomSlice slice;
    PKAtom *result = NULL;
    
    (void)user_closure;
    slice = pk_stack_slice(lisp);
    pk_try(pk_slice_list_rev(lisp, slice, &result));
    pk_try(pk_push(lisp, result));
    return PK_Ok;
}

PKRes pk_fn_list_vars(void *user_closure, Pocket lisp) {
    PKAtom *result = NULL;
    
    (void)user_closure;
    pk_try(pk_symtable_alist(lisp, &lisp->vars, &result));
    pk_try(pk_push(lisp, result));
    return PK_Ok;
}

PKRes pk_fn_list_funs(void *user_closure, Pocket lisp) {
    PKAtom *result = NULL;
    
    (void)user_closure;
    pk_try(pk_symtable_alist(lisp, &lisp->funs, &result));
    pk_try(pk_push(lisp, result));
    return PK_Ok;
}

PKRes pk_fn_cat(void *user_closure, Pocket lisp) {
    PKAtomString *result = NULL;
    PKAtomSlice slice;
    
    (void)user_closure;
    slice = pk_stack_slice(lisp);
    pk_try(pk_atom_string_concat(lisp, slice, &result));
    pk_try(pk_push(lisp, (PKAtom *)result));
    return PK_Ok;
}

PKRes pk_fn_clone(void *user_closure, Pocket lisp) {
    (void)user_closure;
    return pk_clone(lisp, 1);
}

PKRes pk_fn_nil_p(void *user_closure, Pocket lisp) {
    int boolean = 0;
    (void)user_closure;
    pk_try(pk_is_nil(lisp, 1, &boolean));
    return pk_push_cond(lisp, boolean);
}

PKRes pk_fn_num_p(void *user_closure, Pocket lisp) {
    int boolean = 0;
    (void)user_closure;
    pk_try(pk_is_number(lisp, 1, &boolean));
    return pk_push_cond(lisp, boolean);
}

PKRes pk_fn_symbol_p(void *user_closure, Pocket lisp) {
    int boolean = 0;
    (void)user_closure;
    pk_try(pk_is_symbol(lisp, 1, &boolean));
    return pk_push_cond(lisp, boolean);
}

PKRes pk_fn_string_p(void *user_closure, Pocket lisp) {
    int boolean = 0;
    (void)user_closure;
    pk_try(pk_is_string(lisp, 1, &boolean));
    return pk_push_cond(lisp, boolean);
}

PKRes pk_fn_cons_p(void *user_closure, Pocket lisp) {
    int boolean = 0;
    (void)user_closure;
    pk_try(pk_is_cons(lisp, 1, &boolean));
    return pk_push_cond(lisp, boolean);
}

PKRes pk_fn_error(void *user_closure, Pocket lisp) {
    (void)user_closure;
    return pk_error(lisp);
}

PKRes pk_load_std(Pocket lisp) {
    size_t i = 0;
    
    #define PK_STD_LIB_COUNT (39)
    PKFuncRecord lib[PK_STD_LIB_COUNT] = {
        {"+", pk_fn_add, 2, PKArity_Variadic, NULL},
        {"-", pk_fn_sub, 2, PKArity_Variadic, NULL},
        {"*", pk_fn_mul, 2, PKArity_Variadic, NULL},
        {"/", pk_fn_div, 2, PKArity_Variadic, NULL},
        {"list-vars", pk_fn_list_vars, 0, PKArity_Normal, NULL},
        {"list-funs", pk_fn_list_funs, 0, PKArity_Normal, NULL},
        {"slurp", pk_fn_slurp, 1, PKArity_Normal, NULL},
        {"read", pk_fn_read, 1, PKArity_Optional, NULL},
        {"eval", pk_fn_eval, 1, PKArity_Normal, NULL},
        {"evlist", pk_fn_evlist, 1, PKArity_Normal, NULL},
        {"print", pk_fn_print, 1, PKArity_Normal, NULL},
        {"puts", pk_fn_puts, 1, PKArity_Normal, NULL},
        {"format", pk_fn_format, 1, PKArity_Normal, NULL},
        {"set", pk_fn_set, 2, PKArity_Normal, NULL},
        {"get", pk_fn_get, 1, PKArity_Normal, NULL},
        {"unbind", pk_fn_unbind, 1, PKArity_Normal, NULL},
        {"fset", pk_fn_fset, 2, PKArity_Normal, NULL},
        {"fget", pk_fn_fget, 1, PKArity_Normal, NULL},
        {"funbind", pk_fn_funbind, 1, PKArity_Normal, NULL},
        {"car", pk_fn_car, 1, PKArity_Normal, NULL},
        {"cdr", pk_fn_cdr, 1, PKArity_Normal, NULL},
        {"set-car", pk_fn_set_car, 2, PKArity_Normal, NULL},
        {"set-cdr", pk_fn_set_cdr, 2, PKArity_Normal, NULL},
        {">", pk_fn_gt, 2, PKArity_Variadic, NULL},
        {">=", pk_fn_gte, 2, PKArity_Variadic, NULL},
        {"<", pk_fn_lt, 2, PKArity_Variadic, NULL},
        {"<=", pk_fn_lte, 2, PKArity_Variadic, NULL},
        {"=", pk_fn_eq, 2, PKArity_Variadic, NULL},
        {"eq?", pk_fn_eq, 2, PKArity_Variadic, NULL},
        {"list", pk_fn_list, 0, PKArity_Variadic, NULL},
        {"list-rev", pk_fn_list_reversed, 0, PKArity_Variadic, NULL},
        {"cat", pk_fn_cat, 0, PKArity_Variadic, NULL},
        {"clone", pk_fn_clone, 1, PKArity_Normal, NULL},
        
        {"nil?", pk_fn_nil_p, 1, PKArity_Normal, NULL},
        {"number?", pk_fn_num_p, 1, PKArity_Normal, NULL},
        {"string?", pk_fn_string_p, 1, PKArity_Normal, NULL},
        {"symbol?", pk_fn_symbol_p, 1, PKArity_Normal, NULL},
        {"cons?", pk_fn_cons_p, 1, PKArity_Normal, NULL},
        
        {"error", pk_fn_error, 0, PKArity_Normal, NULL},
    };

    for (i = 0; i < PK_STD_LIB_COUNT; ++i) {
        PKAtomCFunc *cfunc = NULL;
        PKAtomSymbol *sym = NULL;
        PKAtom *_ignored = NULL;
        PKFuncArity arity;
        PKFuncRecord *rec = &lib[i];

        arity.args = rec->args;
        arity.mode = rec->mode;
        
        pk_try(pk_atom_cfunc(lisp, rec->user_closure, rec->fn, arity, &cfunc));
        pk_try(pk_atom_symbol_interned(lisp, rec->sym, &sym));
        pk_try(pk_env_set(lisp, PKEnvTy_Fun, sym, (PKAtom *)cfunc, &_ignored));
    }
    return PK_Ok;
}
