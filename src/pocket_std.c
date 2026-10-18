#include "pocket_internals.h"
#include "pocket_std.h"

#define pk_std_args(lisp_) (pk_frame_length(lisp_))

void pk_fun_add(void *user_closure, Pocket lisp) {
    (void)user_closure;
    size_t n = pk_std_args(lisp);
    if (n == 0) {
        pk_push_int(lisp, 0);
        return;
    }
    PKAtoms args = pk_stack_slice_down(lisp, n);
    PKAtom *acc = args.e[0];
    for (size_t i = 1; i < n; ++i) {
        acc = pk_atom_add(lisp, acc, args.e[i]);
    }
    pk_stack_push(lisp, acc);
}

void pk_fun_sub(void *user_closure, Pocket lisp) {
    (void)user_closure;
    size_t n = pk_std_args(lisp);
    if (n == 0) {
        pk_push_int(lisp, 0);
        return;
    }
    PKAtoms args = pk_stack_slice_down(lisp, n);
    PKAtom *acc = args.e[0];
    for (size_t i = 1; i < n; ++i) {
        acc = pk_atom_sub(lisp, acc, args.e[i]);
    }
    pk_stack_push(lisp, acc);
}

void pk_fun_mul(void *user_closure, Pocket lisp) {
    (void)user_closure;
    size_t n = pk_std_args(lisp);
    if (n == 0) {
        pk_push_int(lisp, 1);
        return;
    }
    PKAtoms args = pk_stack_slice_down(lisp, n);
    PKAtom *acc = args.e[0];
    for (size_t i = 1; i < n; ++i) {
        acc = pk_atom_mul(lisp, acc, args.e[i]);
    }
    pk_stack_push(lisp, acc);
}

void pk_fun_div(void *user_closure, Pocket lisp) {
    (void)user_closure;
    size_t n = pk_std_args(lisp);
    if (n == 0) {
        pk_push_int(lisp, 1);
        return;
    }
    PKAtoms args = pk_stack_slice_down(lisp, n);
    PKAtom *acc = args.e[0];
    for (size_t i = 1; i < n; ++i) {
        acc = pk_atom_div(lisp, acc, args.e[i]);
    }
    pk_stack_push(lisp, acc);
}

void pk_fun_mod(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, pk_atom_mod(lisp, pk_stack_get(lisp, -2), pk_stack_get(lisp, -1)));
}

#define PK_TEMPLATE_CMP(fun_name_, op_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        (void)user_closure; \
        pk_stack_push(lisp, op_(lisp, pk_stack_get(lisp, -2), pk_stack_get(lisp, -1))); \
    }

PK_TEMPLATE_CMP(pk_fun_lt, pk_atom_lt)
PK_TEMPLATE_CMP(pk_fun_lte, pk_atom_lte)
PK_TEMPLATE_CMP(pk_fun_gt, pk_atom_gt)
PK_TEMPLATE_CMP(pk_fun_gte, pk_atom_gte)

void pk_fun_eq(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, pk_atom_bool(lisp, pk_atom_eq(pk_stack_get(lisp, -2), pk_stack_get(lisp, -1))));
}

void pk_fun_cons(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_atom_cons(lisp, pk_stack_get(lisp, -2), pk_stack_get(lisp, -1)));
}

void pk_fun_list(void *user_closure, Pocket lisp) {
    (void)user_closure;
    size_t n = pk_std_args(lisp);
    if (n == 0) {
        pk_push_nil(lisp);
        return;
    }
    // A variadic c function keeps its arguments as separate slots, so the
    // frame slice is the argument list already.
    pk_stack_push(lisp, pk_atom_list(lisp, pk_stack_slice_down(lisp, n)));
}

void pk_fun_car(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_op_car(lisp);
}

void pk_fun_cdr(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_op_cdr(lisp);
}

void pk_fun_set_car(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, pk_atom_set_car(lisp, pk_stack_get(lisp, -2), pk_stack_get(lisp, -1)));
}

void pk_fun_set_cdr(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, pk_atom_set_cdr(lisp, pk_stack_get(lisp, -2), pk_stack_get(lisp, -1)));
}

#define PK_TEMPLATE_ENV_SET(fun_name_, env_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        (void)user_closure; \
        pk_atom_assert_symbol(lisp, pk_stack_get(lisp, -2)); \
        pk_env_set(lisp, env_, (PKAtomSymbol *)pk_stack_get(lisp, -2), pk_stack_get(lisp, -1)); \
        pk_stack_push(lisp, pk_stack_get(lisp, -1)); \
    }

#define PK_TEMPLATE_ENV_GET(fun_name_, env_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        (void)user_closure; \
        pk_atom_assert_symbol(lisp, pk_stack_get(lisp, -1)); \
        pk_stack_push(lisp, pk_env_get(lisp, env_, (PKAtomSymbol *)pk_stack_get(lisp, -1))); \
    }

#define PK_TEMPLATE_ENV_BOUND(fun_name_, env_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        (void)user_closure; \
        pk_atom_assert_symbol(lisp, pk_stack_get(lisp, -1)); \
        pk_stack_push(lisp, pk_atom_bool(lisp, !pk_atom_is_undefined(pk_env_get(lisp, env_, (PKAtomSymbol *)pk_stack_get(lisp, -1))))); \
    }

#define PK_TEMPLATE_ENV_UNBIND(fun_name_, env_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        (void)user_closure; \
        pk_atom_assert_symbol(lisp, pk_stack_get(lisp, -1)); \
        pk_stack_push(lisp, pk_env_set(lisp, env_, (PKAtomSymbol *)pk_stack_get(lisp, -1), lisp->cache.undefined)); \
    }

#define PK_TEMPLATE_ENV_LIST(fun_name_, env_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        (void)user_closure; \
        pk_stack_push(lisp, pk_env_alist(lisp, env_)); \
    }

PK_TEMPLATE_ENV_SET(pk_fun_set, PK_ENV_VAR)
PK_TEMPLATE_ENV_SET(pk_fun_fset, PK_ENV_FUN)
PK_TEMPLATE_ENV_SET(pk_fun_pset, PK_ENV_PROP)

PK_TEMPLATE_ENV_GET(pk_fun_get, PK_ENV_VAR)
PK_TEMPLATE_ENV_GET(pk_fun_fget, PK_ENV_FUN)
PK_TEMPLATE_ENV_GET(pk_fun_pget, PK_ENV_PROP)

PK_TEMPLATE_ENV_BOUND(pk_fun_bound, PK_ENV_VAR)
PK_TEMPLATE_ENV_BOUND(pk_fun_fbound, PK_ENV_FUN)
PK_TEMPLATE_ENV_BOUND(pk_fun_pbound, PK_ENV_PROP)

PK_TEMPLATE_ENV_UNBIND(pk_fun_unbind, PK_ENV_VAR)
PK_TEMPLATE_ENV_UNBIND(pk_fun_funbind, PK_ENV_FUN)
PK_TEMPLATE_ENV_UNBIND(pk_fun_punbind, PK_ENV_PROP)

PK_TEMPLATE_ENV_LIST(pk_fun_list_vars, PK_ENV_VAR)
PK_TEMPLATE_ENV_LIST(pk_fun_list_funs, PK_ENV_FUN)
PK_TEMPLATE_ENV_LIST(pk_fun_list_props, PK_ENV_PROP)

void pk_fun_read(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomString *string = pk_atom_assert_string(lisp, pk_stack_get(lisp, -1));
    pk_stack_push(lisp, pk_read_atom_from_string(lisp, string, PK_MODE_EXPRESSION));
}

void pk_fun_read_list(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomString *string = pk_atom_assert_string(lisp, pk_stack_get(lisp, -1));
    pk_stack_push(lisp, pk_read_atom_from_string(lisp, string, PK_MODE_EXPRLIST));
}

void pk_fun_apply(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_atom_apply(lisp, PK_CALL_FUN, pk_stack_get(lisp, -2), pk_stack_get(lisp, -1));
}

void pk_fun_macro_apply(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_atom_apply(lisp, PK_CALL_MACRO, pk_stack_get(lisp, -2), pk_stack_get(lisp, -1));
}

void pk_fun_funcall(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_atom_apply(lisp, PK_CALL_FUN, pk_stack_get(lisp, -2), pk_stack_get(lisp, -1));
}

void pk_fun_eval(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_atom_eval(lisp, pk_stack_get(lisp, -1), PK_MODE_EXPRESSION);
}

void pk_fun_evlist(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_atom_eval(lisp, pk_stack_get(lisp, -1), PK_MODE_EXPRLIST);
}

void pk_fun_compile(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, pk_atom_compile(lisp, PK_CALL_FUN, pk_stack_get(lisp, -1), PK_MODE_EXPRESSION));
}

void pk_fun_print(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_atom_print(lisp, pk_stack_get(lisp, -1));
    pk_push_nil(lisp);
}

void pk_fun_puts(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_atom_print(lisp, pk_stack_get(lisp, -1));
    fputc('\n', stdout);
    fflush(stdout);
    pk_push_nil(lisp);
}

void pk_fun_format(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_atom_format(lisp, pk_stack_get(lisp, -1)));
}

void pk_fun_macroexpand1(void *user_closure, Pocket lisp) {
    (void)user_closure;
    (void)pk_macroexpand1_(lisp, -1);
}

void pk_fun_macroexpand(void *user_closure, Pocket lisp) {
    (void)user_closure;
    (void)pk_macroexpand_(lisp, -1);
}

void pk_fun_fs_slurp(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomString *string = pk_atom_assert_string(lisp, pk_stack_get(lisp, -1));
    pk_stack_push(lisp, (PKAtom *)pk_fs_slurp(lisp, string));
}

void pk_fun_fs_read_user_input(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, pk_fs_read_user_input(lisp));
}

void pk_fun_funcatch(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_atom_assert_symbol(lisp, pk_stack_get(lisp, -2));
    pk_fcall(lisp, PK_CALL_FUN, 0);
}

void pk_fun_throw(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_throw(lisp, -2, -1);
}

void pk_fun_error(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKWriter w = PK_WRITER_EMPTY;
    pk_writer_cstr(lisp, &w, "error: ");
    pk_writer_atom(lisp, &w, pk_stack_get(lisp, -1));
    pk_errors(lisp, pk_writer_slice(&w));
}

void pk_fun_hexdump(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_atom_hexdump(lisp, pk_stack_get(lisp, -1)));
}

void pk_fload(Pocket lisp, PKStr name, pk_u8 nargs, PK_ARITY arity, PKFun fun) {
    pk_push_symbol(lisp, name);
    pk_push_cfun(lisp, NULL, fun, nargs, arity, 0);
    pk_set(lisp, PK_ENV_FUN, -2, -1);
    pk_pop3(lisp);
}

#define PK_FLOAD_N(name_, nargs_, fun_) pk_fload(lisp, name_, nargs_, PK_ARITY_NORMAL, fun_)
#define PK_FLOAD_V(name_, fun_) pk_fload(lisp, name_, 0, PK_ARITY_VARIADIC, fun_)

void pk_std_load_cfun(Pocket lisp) {
    PK_FLOAD_V(pk_str("+"), pk_fun_add);
    PK_FLOAD_V(pk_str("-"), pk_fun_sub);
    PK_FLOAD_V(pk_str("*"), pk_fun_mul);
    PK_FLOAD_V(pk_str("/"), pk_fun_div);
    PK_FLOAD_N(pk_str("%"), 2, pk_fun_mod);
    PK_FLOAD_N(pk_str("mod"), 2, pk_fun_mod);

    PK_FLOAD_N(pk_str("<"), 2, pk_fun_lt);
    PK_FLOAD_N(pk_str("<="), 2, pk_fun_lte);
    PK_FLOAD_N(pk_str(">"), 2, pk_fun_gt);
    PK_FLOAD_N(pk_str(">="), 2, pk_fun_gte);

    PK_FLOAD_N(pk_str("="), 2, pk_fun_eq);
    PK_FLOAD_N(pk_str("eq?"), 2, pk_fun_eq);

    PK_FLOAD_N(pk_str("cons"), 2, pk_fun_cons);
    PK_FLOAD_V(pk_str("list"), pk_fun_list);
    PK_FLOAD_N(pk_str("car"), 1, pk_fun_car);
    PK_FLOAD_N(pk_str("cdr"), 1, pk_fun_cdr);
    PK_FLOAD_N(pk_str("set-car"), 2, pk_fun_set_car);
    PK_FLOAD_N(pk_str("set-cdr"), 2, pk_fun_set_cdr);

    PK_FLOAD_N(pk_str("set"), 2, pk_fun_set);
    PK_FLOAD_N(pk_str("fset"), 2, pk_fun_fset);
    PK_FLOAD_N(pk_str("pset"), 2, pk_fun_pset);

    PK_FLOAD_N(pk_str("get"), 1, pk_fun_get);
    PK_FLOAD_N(pk_str("fget"), 1, pk_fun_fget);
    PK_FLOAD_N(pk_str("pget"), 1, pk_fun_pget);

    PK_FLOAD_N(pk_str("bound?"), 1, pk_fun_bound);
    PK_FLOAD_N(pk_str("fbound?"), 1, pk_fun_fbound);
    PK_FLOAD_N(pk_str("pbound?"), 1, pk_fun_pbound);

    PK_FLOAD_N(pk_str("unbind"), 1, pk_fun_unbind);
    PK_FLOAD_N(pk_str("funbind"), 1, pk_fun_funbind);
    PK_FLOAD_N(pk_str("punbind"), 1, pk_fun_punbind);

    PK_FLOAD_N(pk_str("list-vars"), 0, pk_fun_list_vars);
    PK_FLOAD_N(pk_str("list-funs"), 0, pk_fun_list_funs);
    PK_FLOAD_N(pk_str("list-props"), 0, pk_fun_list_props);

    PK_FLOAD_N(pk_str("funcatch"), 2, pk_fun_funcatch);
    PK_FLOAD_N(pk_str("throw"), 2, pk_fun_throw);
    PK_FLOAD_N(pk_str("error"), 1, pk_fun_error);

    PK_FLOAD_N(pk_str("hexdump"), 1, pk_fun_hexdump);

    PK_FLOAD_N(pk_str("read"), 1, pk_fun_read);
    PK_FLOAD_N(pk_str("read-list"), 1, pk_fun_read_list);
    PK_FLOAD_N(pk_str("apply"), 2, pk_fun_apply);
    PK_FLOAD_N(pk_str("macro-apply"), 2, pk_fun_macro_apply);
    PK_FLOAD_V(pk_str("funcall"), pk_fun_funcall);
    PK_FLOAD_N(pk_str("print"), 1, pk_fun_print);
    PK_FLOAD_N(pk_str("puts"), 1, pk_fun_puts);
    PK_FLOAD_N(pk_str("eval"), 1, pk_fun_eval);
    PK_FLOAD_N(pk_str("evlist"), 1, pk_fun_evlist);
    PK_FLOAD_N(pk_str("format"), 1, pk_fun_format);
    PK_FLOAD_N(pk_str("compile"), 1, pk_fun_compile);
    PK_FLOAD_N(pk_str("macroexpand1"), 1, pk_fun_macroexpand1);
    PK_FLOAD_N(pk_str("macroexpand"), 1, pk_fun_macroexpand);

    PK_FLOAD_N(pk_str("fs/slurp"), 1, pk_fun_fs_slurp);
    PK_FLOAD_N(pk_str("fs/read-user-input"), 0, pk_fun_fs_read_user_input);
}

void pk_std_load_lfun(Pocket lisp) {
    pk_push_string(lisp, pk_str(PK_STD_CODE));
    pk_read(lisp, -1, PK_MODE_EXPRLIST);
    pk_eval(lisp, -1, PK_MODE_EXPRLIST);
    pk_pop(lisp);
}

void pk_std_load(Pocket lisp) {
    pk_std_load_cfun(lisp);
    pk_std_load_lfun(lisp);
}
