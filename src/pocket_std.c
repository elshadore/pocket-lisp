#include "pocket_internals.h"
#include "pocket_std.h"

#define PK_TEMPLATE_ARITH(fun_name_, op_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        int top = pk_top(lisp); \
        (void)user_closure; \
        if (top == 0) { \
            pk_push_int(lisp, 0); \
            return; \
        } \
        pk_dig(lisp, 1); \
        for (int i = 2; i <= top; ++i) { \
            op_(lisp, -1, i); \
            pk_swap(lisp, -1, -2); \
            pk_pop(lisp); \
        } \
    }

PK_TEMPLATE_ARITH(pk_fun_add, pk_add)
PK_TEMPLATE_ARITH(pk_fun_sub, pk_sub)
PK_TEMPLATE_ARITH(pk_fun_mul, pk_mul)
PK_TEMPLATE_ARITH(pk_fun_div, pk_div)

void pk_fun_mod(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_mod(lisp, 1, 2);
}

#define PK_TEMPLATE_CMP(fun_name_, op_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        int top = pk_top(lisp); \
        (void)user_closure; \
        for (int i = 1; i < top; ++i) { \
            if (!op_(lisp, i, i + 1)) { \
                pk_push_bool(lisp, false); \
                return; \
            } \
        } \
        pk_push_bool(lisp, true); \
    }

PK_TEMPLATE_CMP(pk_fun_lt, pk_lt)
PK_TEMPLATE_CMP(pk_fun_lte, pk_lte)
PK_TEMPLATE_CMP(pk_fun_gt, pk_gt)
PK_TEMPLATE_CMP(pk_fun_gte, pk_gte)
PK_TEMPLATE_CMP(pk_fun_eq, pk_eq)

void pk_fun_cons(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_cons(lisp, 1, 2);
}

void pk_fun_list(void *user_closure, Pocket lisp) {
    int top = pk_top(lisp);
    (void)user_closure;
    if (top == 0) {
        pk_push_nil(lisp);
        return;
    }
    pk_list(lisp, 1, top);
}

void pk_fun_car(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_car(lisp, 1);
}

void pk_fun_cdr(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_cdr(lisp, 1);
}

void pk_fun_set_car(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_car_strict(lisp, 1);
    pk_set_car(lisp, 1, 2);
}

void pk_fun_set_cdr(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_cdr_strict(lisp, 1);
    pk_set_cdr(lisp, 1, 2);
}

#define PK_TEMPLATE_ENV_SET(fun_name_, env_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        (void)user_closure; \
        pk_set(lisp, env_, 1, 2); \
    }

#define PK_TEMPLATE_ENV_GET(fun_name_, env_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        (void)user_closure; \
        pk_get(lisp, env_, 1); \
    }

#define PK_TEMPLATE_ENV_BOUND(fun_name_, env_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        (void)user_closure; \
        pk_push_bool(lisp, pk_is_bound(lisp, env_, 1)); \
    }

#define PK_TEMPLATE_ENV_UNBIND(fun_name_, env_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        (void)user_closure; \
        pk_unbind(lisp, env_, 1); \
        pk_push_nil(lisp); \
    }

#define PK_TEMPLATE_ENV_LIST(fun_name_, env_) \
    void fun_name_(void *user_closure, Pocket lisp) { \
        (void)user_closure; \
        pk_env_list(lisp, env_); \
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

void pk_fun_funcatch(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_assert_type(lisp, 1, PK_TYPE_SYMBOL);
    pk_fcall(lisp, PK_CALL_FUN, 0);
}

void pk_fun_throw(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_throw(lisp, 1, 2);
}

void pk_fun_error(void *user_closure, Pocket lisp) {
    PKWriter w = PK_WRITER_EMPTY;
    (void)user_closure;
    pk_writer_cstr(lisp, &w, "error: ");
    pk_writer_atom(lisp, &w, pk_stack_get(lisp, 1));
    pk_errors(lisp, pk_writer_slice(&w));
}

void pk_fun_hexdump(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_hexdump(lisp, 1);
}

void pk_fun_read(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_read(lisp, 1, PK_MODE_EXPRESSION);
}

void pk_fun_read_list(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_read(lisp, 1, PK_MODE_EXPRLIST);
}

void pk_fun_apply(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_apply(lisp, PK_CALL_FUN, 1, 2);
}

void pk_fun_macro_apply(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_apply(lisp, PK_CALL_MACRO, 1, 2);
}

void pk_fun_funcall(void *user_closure, Pocket lisp) {
    int top = pk_top(lisp);
    (void)user_closure;
    if (top < 2) {
        pk_push_nil(lisp);
        return;
    }
    pk_list(lisp, 2, top);
    pk_apply(lisp, PK_CALL_FUN, 1, -1);
}

void pk_fun_eval(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_eval(lisp, 1, PK_MODE_EXPRESSION);
}

void pk_fun_evlist(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_eval(lisp, 1, PK_MODE_EXPRLIST);
}

void pk_fun_compile(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_compile(lisp, PK_CALL_FUN, 1, PK_MODE_EXPRESSION);
}

void pk_fun_print(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_print(lisp, 1);
    pk_push_nil(lisp);
}

void pk_fun_puts(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_print(lisp, 1);
    fputc('\n', stdout);
    fflush(stdout);
    pk_push_nil(lisp);
}

void pk_fun_format(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_format(lisp, 1);
}

void pk_fun_macroexpand1(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_macroexpand1(lisp, 1);
}

void pk_fun_macroexpand(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_macroexpand(lisp, 1);
}

void pk_fun_fs_slurp(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_slurp(lisp, 1);
}

void pk_fun_fs_read_user_input(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_read_user_input(lisp);
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

    PK_FLOAD_V(pk_str("<"), pk_fun_lt);
    PK_FLOAD_V(pk_str("<="), pk_fun_lte);
    PK_FLOAD_V(pk_str(">"), pk_fun_gt);
    PK_FLOAD_V(pk_str(">="), pk_fun_gte);

    PK_FLOAD_V(pk_str("="), pk_fun_eq);
    PK_FLOAD_V(pk_str("eq?"), pk_fun_eq);

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
