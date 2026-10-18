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

typedef struct PKFunLoad_ {
    PKStr symbol;
    PKFun fun;
    pk_u8 args;
    PK_ARITY arity;
    pk_u8 options;
} PKFunLoad;

void pk_std_load_cfun(Pocket lisp) {
    PKFunLoad funs[] = {
        {.symbol = pk_str("+"), .fun = pk_fun_add, .args = 2, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("-"), .fun = pk_fun_sub, .args = 2, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("*"), .fun = pk_fun_mul, .args = 2, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("/"), .fun = pk_fun_div, .args = 2, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("%"), .fun = pk_fun_mod, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("mod"), .fun = pk_fun_mod, .args = 2, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("<"), .fun = pk_fun_lt, .args = 2, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("<="), .fun = pk_fun_lte, .args = 2, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str(">"), .fun = pk_fun_gt, .args = 2, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str(">="), .fun = pk_fun_gte, .args = 2, .arity = PK_ARITY_VARIADIC},

        {.symbol = pk_str("="), .fun = pk_fun_eq, .args = 2, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("eq?"), .fun = pk_fun_eq, .args = 2, .arity = PK_ARITY_VARIADIC},

        {.symbol = pk_str("cons"), .fun = pk_fun_cons, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("list"), .fun = pk_fun_list, .args = 0, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("car"), .fun = pk_fun_car, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("cdr"), .fun = pk_fun_cdr, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("set-car"), .fun = pk_fun_set_car, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("set-cdr"), .fun = pk_fun_set_cdr, .args = 2, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("set"), .fun = pk_fun_set, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fset"), .fun = pk_fun_fset, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("pset"), .fun = pk_fun_pset, .args = 2, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("get"), .fun = pk_fun_get, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fget"), .fun = pk_fun_fget, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("pget"), .fun = pk_fun_pget, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("bound?"), .fun = pk_fun_bound, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fbound?"), .fun = pk_fun_fbound, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("pbound?"), .fun = pk_fun_pbound, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("unbind"), .fun = pk_fun_unbind, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("funbind"), .fun = pk_fun_funbind, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("punbind"), .fun = pk_fun_punbind, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("list-vars"), .fun = pk_fun_list_vars, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("list-funs"), .fun = pk_fun_list_funs, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("list-props"), .fun = pk_fun_list_props, .args = 0, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("funcatch"), .fun = pk_fun_funcatch, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("throw"), .fun = pk_fun_throw, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("error"), .fun = pk_fun_error, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("hexdump"), .fun = pk_fun_hexdump, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("read"), .fun = pk_fun_read, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("read-list"), .fun = pk_fun_read_list, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("apply"), .fun = pk_fun_apply, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("macro-apply"), .fun = pk_fun_macro_apply, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("funcall"), .fun = pk_fun_funcall, .args = 0, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("print"), .fun = pk_fun_print, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("puts"), .fun = pk_fun_puts, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("eval"), .fun = pk_fun_eval, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("evlist"), .fun = pk_fun_evlist, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("format"), .fun = pk_fun_format, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("compile"), .fun = pk_fun_compile, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("macroexpand1"), .fun = pk_fun_macroexpand1, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("macroexpand"), .fun = pk_fun_macroexpand, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("fs/slurp"), .fun = pk_fun_fs_slurp, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fs/read-user-input"), .fun = pk_fun_fs_read_user_input, .args = 0, .arity = PK_ARITY_NORMAL},
    };

    size_t length = pk_alen(funs);

    for (size_t i = 0; i < length; ++i) {
        PKFunLoad fun = funs[i];
        pk_push_symbol(lisp, fun.symbol);
        pk_push_cfun(lisp, NULL, fun.fun, fun.args, fun.arity, fun.options);
        pk_set(lisp, PK_ENV_FUN, -2, -1);
        pk_pop3(lisp);
    }
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
