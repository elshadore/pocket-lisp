#include "pocket_internals.h"

#define PK_TEMPLATE_FUN0(fun_name_, bind_name_) \
    PKAtom fun_name_(void *user_closure) { \
        (void)user_closure; \
        return bind_name_(); \
    }

#define PK_TEMPLATE_FUN1(fun_name_, bind_name_) \
    PKAtom fun_name_(void *user_closure, PKAtom arg1) { \
        (void)user_closure; \
        return bind_name_(arg1); \
    }

#define PK_TEMPLATE_FUN2(fun_name_, bind_name_) \
    PKAtom fun_name_(void *user_closure, PKAtom arg1, PKAtom arg2) { \
        (void)user_closure; \
        return bind_name_(arg1, arg2); \
    }

#define PK_TEMPLATE_FUN3V(fun_name_, bind_name_) \
    PKAtom fun_name_(void *user_closure, PKAtom arg1, PKAtom arg2, PKAtom argn) { \
        (void)user_closure; \
        PKAtom result = bind_name_(arg1, arg2); \
        pk_propagate(result); \
        PKAtom iter = argn; \
        while (pk_is_cons(iter)) { \
            PKAtom atom = pk_car(iter); \
            result = bind_name_(result, atom); \
            pk_propagate(result); \
            iter = pk_cdr(iter); \
        } \
        return result; \
    }

PK_TEMPLATE_FUN3V(pk_fun_add, pk_add)
PK_TEMPLATE_FUN3V(pk_fun_sub, pk_sub)
PK_TEMPLATE_FUN3V(pk_fun_mul, pk_mul)
PK_TEMPLATE_FUN3V(pk_fun_div, pk_div)
PK_TEMPLATE_FUN2(pk_fun_mod, pk_mod)

PKAtom pk_fun_eq(void *user_closure, PKAtom arg1, PKAtom arg2) {
    (void)user_closure;
    return pk_bool(pk_eq(arg1, arg2));
}

PK_TEMPLATE_FUN2(pk_fun_cons, pk_cons)
PK_TEMPLATE_FUN1(pk_fun_car, pk_car)
PK_TEMPLATE_FUN1(pk_fun_cdr, pk_cdr)
PK_TEMPLATE_FUN2(pk_fun_set_car, pk_set_car)
PK_TEMPLATE_FUN2(pk_fun_set_cdr, pk_set_cdr)

#define PK_TEMPLATE_ENV_SET(fun_name_, env_) \
    PKAtom fun_name_(void *user_closure, PKAtom arg1, PKAtom arg2) { \
        (void)user_closure; \
        pk_propagate(pk_typecheck_symbol(arg1)); \
        return pk_set(env_, arg1, arg2); \
    }

#define PK_TEMPLATE_ENV_GET(fun_name_, env_) \
    PKAtom fun_name_(void *user_closure, PKAtom arg1) { \
        (void)user_closure; \
        pk_propagate(pk_typecheck_symbol(arg1)); \
        return pk_get(env_, arg1); \
    }

#define PK_TEMPLATE_ENV_BOUND(fun_name_, env_) \
    PKAtom fun_name_(void *user_closure, PKAtom arg1) { \
        (void)user_closure; \
        pk_propagate(pk_typecheck_symbol(arg1)); \
        return pk_bool(pk_is_bound(env_, arg1)); \
    }

#define PK_TEMPLATE_ENV_UNBIND(fun_name_, env_) \
    PKAtom fun_name_(void *user_closure, PKAtom arg1) { \
        (void)user_closure; \
        pk_propagate(pk_typecheck_symbol(arg1)); \
        return pk_unbind(env_, arg1); \
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
    
PKAtom pk_fun_read(void *user_closure, PKAtom arg1) {
    (void)user_closure;
    return pk_read(arg1, PK_MODE_EXPRESSION);
}

PKAtom pk_fun_read_list(void *user_closure, PKAtom arg1) {
    (void)user_closure;
    return pk_read(arg1, PK_MODE_EXPRLIST);
}

PKAtom pk_fun_apply(void *user_closure, PKAtom arg1, PKAtom arg2) {
    (void)user_closure;
    return pk_apply(PK_CALL_FUN, arg1, arg2);
}

PKAtom pk_fun_macro_apply(void *user_closure, PKAtom arg1, PKAtom arg2) {
    (void)user_closure;
    return pk_apply(PK_CALL_MACRO, arg1, arg2);
}

PKAtom pk_fun_funcall(void *user_closure, PKAtom arg1, PKAtom argn) {
    (void)user_closure;
    return pk_apply(PK_CALL_FUN, arg1, argn);
}

PKAtom pk_fun_eval(void *user_closure, PKAtom arg1) {
    (void)user_closure;
    return pk_eval(arg1, PK_MODE_EXPRESSION);
}

PKAtom pk_fun_evlist(void *user_closure, PKAtom arg1) {
    (void)user_closure;
    return pk_eval(arg1, PK_MODE_EXPRLIST);
}

PKAtom pk_fun_compile(void *user_closure, PKAtom arg1) {
    (void)user_closure;
    return pk_compile(arg1, PK_MODE_EXPRESSION);
}

PK_TEMPLATE_FUN1(pk_fun_print, pk_print)
PK_TEMPLATE_FUN1(pk_fun_puts, pk_puts)
PK_TEMPLATE_FUN1(pk_fun_format, pk_format)
PK_TEMPLATE_FUN1(pk_fun_macroexpand1, pk_macroexpand1)
PK_TEMPLATE_FUN1(pk_fun_macroexpand, pk_macroexpand)
    
PK_TEMPLATE_FUN1(pk_fun_fs_slurp, pk_fs_slurp)
PK_TEMPLATE_FUN0(pk_fun_fs_read_user_input, pk_fs_read_user_input)

#define PK_FLOAD(name_, fun_) do { \
    PKAtom name = pk_symbol(name_); \
    pk_propagate(name); \
    PKAtom fun = fun_; \
    pk_propagate(fun); \
    pk_propagate(pk_set(PK_ENV_FUN, name, fun)); \
} while(0)

PKAtom pk_std_load(void) {
    PK_FLOAD(pk_str("+"), pk_cfun3(NULL, PK_CALL_FUN, pk_fun_add, PK_ARITY_VARIADIC));
    PK_FLOAD(pk_str("-"), pk_cfun3(NULL, PK_CALL_FUN, pk_fun_sub, PK_ARITY_VARIADIC));
    PK_FLOAD(pk_str("*"), pk_cfun3(NULL, PK_CALL_FUN, pk_fun_mul, PK_ARITY_VARIADIC));
    PK_FLOAD(pk_str("/"), pk_cfun3(NULL, PK_CALL_FUN, pk_fun_div, PK_ARITY_VARIADIC));
    PK_FLOAD(pk_str("%"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_mod, PK_ARITY_NORMAL));
    
    PK_FLOAD(pk_str("cons"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_cons, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("car"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_car, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("cdr"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_cdr, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("set_car"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_set_car, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("set_cdr"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_set_cdr, PK_ARITY_NORMAL));

    PK_FLOAD(pk_str("set"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_set, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("fset"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_fset, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("pset"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_pset, PK_ARITY_NORMAL));

    PK_FLOAD(pk_str("get"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_get, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("fget"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_fget, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("pget"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_pget, PK_ARITY_NORMAL));

    PK_FLOAD(pk_str("bound"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_bound, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("fbound"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_fbound, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("pbound"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_pbound, PK_ARITY_NORMAL));

    PK_FLOAD(pk_str("unbind"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_unbind, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("funbind"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_funbind, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("punbind"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_punbind, PK_ARITY_NORMAL));
    
    PK_FLOAD(pk_str("read"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_read, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("read-list"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_read_list, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("apply"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_apply, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("macro-apply"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_macro_apply, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("funcall"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_funcall, PK_ARITY_VARIADIC));
    PK_FLOAD(pk_str("print"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_print, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("puts"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_puts, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("eval"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_eval, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("evlist"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_evlist, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("format"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_format, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("compile"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_compile, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("macroexpand1"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_macroexpand1, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("macroexpand"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_macroexpand, PK_ARITY_NORMAL));
    
    PK_FLOAD(pk_str("fs/slurp"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_fs_slurp, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("fs/read-user-input"), pk_cfun0(NULL, PK_CALL_FUN, pk_fun_fs_read_user_input));
    return pk_true();
}
