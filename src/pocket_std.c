#include "pocket_internals.h"

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
    
PKAtom pk_fun_read(void *user_closure, PKAtom arg1) {
    (void)user_closure;
    return pk_read(arg1, PK_READ_EXPRESSION);
}

PKAtom pk_fun_read_exprlist(void *user_closure, PKAtom arg1) {
    (void)user_closure;
    return pk_read(arg1, PK_READ_EXPRLIST);
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

PK_TEMPLATE_FUN1(pk_fun_print, pk_print)
PK_TEMPLATE_FUN1(pk_fun_eval, pk_eval)
PK_TEMPLATE_FUN1(pk_fun_format, pk_format)
PK_TEMPLATE_FUN1(pk_fun_compile, pk_compile)
PK_TEMPLATE_FUN1(pk_fun_macroexpand1, pk_macroexpand1)
PK_TEMPLATE_FUN1(pk_fun_macroexpand, pk_macroexpand)
    
PK_TEMPLATE_FUN1(pk_fun_fs_slurp, pk_fs_slurp)

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
    
    PK_FLOAD(pk_str("read"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_read, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("read-exprlist"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_read_exprlist, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("apply"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_apply, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("macro-apply"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_macro_apply, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("funcall"), pk_cfun2(NULL, PK_CALL_FUN, pk_fun_funcall, PK_ARITY_VARIADIC));
    PK_FLOAD(pk_str("print"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_print, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("eval"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_eval, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("format"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_format, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("compile"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_compile, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("macroexpand1"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_macroexpand1, PK_ARITY_NORMAL));
    PK_FLOAD(pk_str("macroexpand"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_macroexpand, PK_ARITY_NORMAL));
    
    PK_FLOAD(pk_str("fs/slurp"), pk_cfun1(NULL, PK_CALL_FUN, pk_fun_fs_slurp, PK_ARITY_NORMAL));
    return pk_true();
}
