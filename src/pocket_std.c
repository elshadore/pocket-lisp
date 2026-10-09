#include "pocket_internals.h"

#define pk_make_fun1(fun_name_, bind_name_) \
    PKAtom fun_name_(void *user_closure, PKAtom arg1) { \
        (void)user_closure; \
        return bind_name_(arg1); \
    }

#define pk_make_fun2(fun_name_, bind_name_) \
    PKAtom fun_name_(void *user_closure, PKAtom arg1, PKAtom arg2) { \
        (void)user_closure; \
        return bind_name_(arg1, arg2); \
    }

PKAtom pk_fun_add(void *user_closure, PKAtom arg1, PKAtom arg2, PKAtom argn) {
    (void)user_closure;
    PKAtom result = pk_add(arg1, arg2);
    PKAtom iter = argn;
    while (pk_is_cons(iter)) {
        PKAtom atom = pk_car(iter);
        result = pk_add(result, atom);
        iter = pk_cdr(iter);
    }
    return result;
}

PKAtom pk_fun_eq(void *user_closure, PKAtom arg1, PKAtom arg2) {
    (void)user_closure;
    if (pk_eq(arg1, arg2)) {
        return pk_true();
    } else {
        return pk_nil();
    }
}

pk_make_fun2(pk_fun_cons, pk_cons)
pk_make_fun1(pk_fun_car, pk_car)
pk_make_fun1(pk_fun_cdr, pk_cdr)
pk_make_fun2(pk_fun_set_car, pk_set_car)
pk_make_fun2(pk_fun_set_cdr, pk_set_cdr)
    
pk_make_fun1(pk_fun_print, pk_print)
pk_make_fun1(pk_fun_eval, pk_eval)
pk_make_fun1(pk_fun_format, pk_format)
pk_make_fun1(pk_fun_compile, pk_compile)

#define pk_load_fun(name_, fun_) do { \
    PKAtom name = pk_symbol(name_); \
    pk_propagate(name); \
    PKAtom fun = fun_; \
    pk_propagate(fun); \
    pk_propagate(pk_set(PK_ENV_FUN, name, fun)); \
} while(0)

PKAtom pk_std_load(void) {
    pk_load_fun(pk_str("+"), pk_cfun3(NULL, pk_fun_add, PK_ARITY_VARIADIC));
    
    pk_load_fun(pk_str("cons"), pk_cfun2(NULL, pk_fun_cons, PK_ARITY_NORMAL));
    pk_load_fun(pk_str("car"), pk_cfun1(NULL, pk_fun_car, PK_ARITY_NORMAL));
    pk_load_fun(pk_str("cdr"), pk_cfun1(NULL, pk_fun_cdr, PK_ARITY_NORMAL));
    pk_load_fun(pk_str("set_car"), pk_cfun2(NULL, pk_fun_set_car, PK_ARITY_NORMAL));
    pk_load_fun(pk_str("set_cdr"), pk_cfun2(NULL, pk_fun_set_cdr, PK_ARITY_NORMAL));
    
    pk_load_fun(pk_str("print"), pk_cfun1(NULL, pk_fun_print, PK_ARITY_NORMAL));
    pk_load_fun(pk_str("eval"), pk_cfun1(NULL, pk_fun_eval, PK_ARITY_NORMAL));
    pk_load_fun(pk_str("format"), pk_cfun1(NULL, pk_fun_format, PK_ARITY_NORMAL));
    return pk_true();
}
