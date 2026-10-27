#include "pocket_internals.h"
#include "pocket_std.h"

void pk_fun_add(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    if (top == 0) {
        pk_push_int(lisp, 0);
        return;
    }
    pk_dig(lisp, 1);
    for (int i = 2; i <= top; ++i) {
        pk_add(lisp, -1, i);
        pk_swap(lisp, -1, -2);
        pk_pop(lisp);
    }
}

void pk_fun_sub(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    if (top == 0) {
        pk_push_int(lisp, 0);
        return;
    }
    pk_dig(lisp, 1);
    for (int i = 2; i <= top; ++i) {
        pk_sub(lisp, -1, i);
        pk_swap(lisp, -1, -2);
        pk_pop(lisp);
    }
}

void pk_fun_mul(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    if (top == 0) {
        pk_push_int(lisp, 0);
        return;
    }
    pk_dig(lisp, 1);
    for (int i = 2; i <= top; ++i) {
        pk_mul(lisp, -1, i);
        pk_swap(lisp, -1, -2);
        pk_pop(lisp);
    }
}

void pk_fun_div(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    if (top == 0) {
        pk_push_int(lisp, 0);
        return;
    }
    pk_dig(lisp, 1);
    for (int i = 2; i <= top; ++i) {
        pk_div(lisp, -1, i);
        pk_swap(lisp, -1, -2);
        pk_pop(lisp);
    }
}

void pk_fun_mod(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_mod(lisp, 1, 2);
}

void pk_fun_eq(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    for (int i = 1; i < top; ++i) {
        if (!pk_eq(lisp, i, i + 1)) {
            pk_push_bool(lisp, false);
            return;
        }
    }
    pk_push_bool(lisp, true);
}

void pk_fun_struct_eq(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    for (int i = 1; i < top; ++i) {
        if (!pk_struct_eq(lisp, i, i + 1)) {
            pk_push_bool(lisp, false);
            return;
        }
    }
    pk_push_bool(lisp, true);
}

void pk_fun_object_eq(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    for (int i = 1; i < top; ++i) {
        if (!pk_object_eq(lisp, i, i + 1)) {
            pk_push_bool(lisp, false);
            return;
        }
    }
    pk_push_bool(lisp, true);
}

void pk_fun_lt(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    for (int i = 1; i < top; ++i) {
        if (!pk_lt(lisp, i, i + 1)) {
            pk_push_bool(lisp, false);
            return;
        }
    }
    pk_push_bool(lisp, true);
}

void pk_fun_lte(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    for (int i = 1; i < top; ++i) {
        if (!pk_lte(lisp, i, i + 1)) {
            pk_push_bool(lisp, false);
            return;
        }
    }
    pk_push_bool(lisp, true);
}

void pk_fun_gt(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    for (int i = 1; i < top; ++i) {
        if (!pk_gt(lisp, i, i + 1)) {
            pk_push_bool(lisp, false);
            return;
        }
    }
    pk_push_bool(lisp, true);
}

void pk_fun_gte(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    for (int i = 1; i < top; ++i) {
        if (!pk_gte(lisp, i, i + 1)) {
            pk_push_bool(lisp, false);
            return;
        }
    }
    pk_push_bool(lisp, true);
}

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

void pk_fun_cat(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_cat(lisp, 1, 2);
}

void pk_fun_append(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_append(lisp, 1, 2);
}

void pk_fun_el(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int index = pk_atom_to_int(lisp, pk_stack_head(lisp), PK_NUMBER_CONV_FLOAT_CONV);
    pk_pop(lisp);
    pk_el(lisp, index);
}

void pk_fun_reverse(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_reverse(lisp, 1);
}

void pk_fun_put(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_put(lisp, 1, 2, 3);
}

void pk_fun_put_unique(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_put_unique(lisp, 1, 2, 3);
}

void pk_fun_rem(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_rem(lisp, 1, 2);
}

void pk_fun_rem_strict(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_rem_strict(lisp, 1, 2);
}

void pk_fun_find(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_find(lisp, 1, 2);
}

void pk_fun_find_raw(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_find_raw(lisp, 1, 2);
}

void pk_fun_find_strict(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_find_strict(lisp, 1, 2);
}

void pk_fun_is_member(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_is_member(lisp, 1, 2));
}

void pk_fun_set(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_set(lisp, PK_ENV_VAR, 1, 2);
}

void pk_fun_fset(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_set(lisp, PK_ENV_FUN, 1, 2);
}

void pk_fun_pset(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_set(lisp, PK_ENV_PROP, 1, 2);
}

void pk_fun_get(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get(lisp, PK_ENV_VAR, 1);
}

void pk_fun_fget(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get(lisp, PK_ENV_FUN, 1);
}

void pk_fun_pget(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get(lisp, PK_ENV_PROP, 1);
}

void pk_fun_get_raw(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get_raw(lisp, PK_ENV_VAR, 1);
}

void pk_fun_fget_raw(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get_raw(lisp, PK_ENV_FUN, 1);
}

void pk_fun_pget_raw(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get_raw(lisp, PK_ENV_PROP, 1);
}

void pk_fun_bound(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_is_bound(lisp, PK_ENV_VAR, 1));
}

void pk_fun_fbound(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_is_bound(lisp, PK_ENV_FUN, 1));
}

void pk_fun_pbound(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_is_bound(lisp, PK_ENV_PROP, 1));
}

void pk_fun_unbind(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_unbind(lisp, PK_ENV_VAR, 1);
    pk_push_nil(lisp);
}

void pk_fun_funbind(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_unbind(lisp, PK_ENV_FUN, 1);
    pk_push_nil(lisp);
}

void pk_fun_punbind(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_unbind(lisp, PK_ENV_PROP, 1);
    pk_push_nil(lisp);
}

void pk_fun_list_vars(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_env_list(lisp, PK_ENV_VAR);
}

void pk_fun_list_funs(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_env_list(lisp, PK_ENV_FUN);
}

void pk_fun_list_props(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_env_list(lisp, PK_ENV_PROP);
}

void pk_fun_funcatch(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_assert_type(lisp, 1, PK_TYPE_SYMBOL);
    pk_catch(lisp, 1);
    (void)pk_funcall(lisp, PK_CALL_FUN, 0);
}

void pk_fun_propagate(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_propagate(lisp);
}

void pk_fun_throw(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_throw(lisp, 1, 2);
}

void pk_fun_error(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_error(lisp, 1);
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
    if (top == 1) {
        pk_push_nil(lisp);
        pk_apply(lisp, PK_CALL_FUN, 1, 2);
    } else {
        pk_list(lisp, 2, top);
        pk_apply(lisp, PK_CALL_FUN, 1, -1);
    }
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

void pk_fun_write(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_write(lisp, 1);
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

void pk_fun_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_array(lisp);
}

void pk_fun_table(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_table(lisp);
}

void pk_fun_string_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_string_array(lisp);
}

void pk_fun_byte_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_byte_array(lisp);
}

void pk_fun_gensym(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_gensym(lisp);
}

void pk_fun_typeof(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_typeof(lisp, 1);
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
        {.symbol = pk_str("object-eq?"), .fun = pk_fun_object_eq, .args = 2, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("struct-eq?"), .fun = pk_fun_struct_eq, .args = 2, .arity = PK_ARITY_VARIADIC},

        {.symbol = pk_str("cons"), .fun = pk_fun_cons, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("list"), .fun = pk_fun_list, .args = 0, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("car"), .fun = pk_fun_car, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("cdr"), .fun = pk_fun_cdr, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("set-car"), .fun = pk_fun_set_car, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("set-cdr"), .fun = pk_fun_set_cdr, .args = 2, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("cat"), .fun = pk_fun_cat, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("append"), .fun = pk_fun_append, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("el"), .fun = pk_fun_el, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("reverse"), .fun = pk_fun_reverse, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("set"), .fun = pk_fun_set, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fset"), .fun = pk_fun_fset, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("pset"), .fun = pk_fun_pset, .args = 2, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("get"), .fun = pk_fun_get, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fget"), .fun = pk_fun_fget, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("pget"), .fun = pk_fun_pget, .args = 1, .arity = PK_ARITY_NORMAL},
        
        {.symbol = pk_str("get-raw"), .fun = pk_fun_get_raw, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fget-raw"), .fun = pk_fun_fget_raw, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("pget-raw"), .fun = pk_fun_pget_raw, .args = 1, .arity = PK_ARITY_NORMAL},

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
        {.symbol = pk_str("propagate"), .fun = pk_fun_propagate, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("throw"), .fun = pk_fun_throw, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("error"), .fun = pk_fun_error, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("hexdump"), .fun = pk_fun_hexdump, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("read"), .fun = pk_fun_read, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("read-list"), .fun = pk_fun_read_list, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("apply"), .fun = pk_fun_apply, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("macro-apply"), .fun = pk_fun_macro_apply, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("funcall"), .fun = pk_fun_funcall, .args = 1, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("print"), .fun = pk_fun_print, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("write"), .fun = pk_fun_write, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("eval"), .fun = pk_fun_eval, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("evlist"), .fun = pk_fun_evlist, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("format"), .fun = pk_fun_format, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("compile"), .fun = pk_fun_compile, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("macroexpand1"), .fun = pk_fun_macroexpand1, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("macroexpand"), .fun = pk_fun_macroexpand, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("file-slurp"), .fun = pk_fun_fs_slurp, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("read-user-input"), .fun = pk_fun_fs_read_user_input, .args = 0, .arity = PK_ARITY_NORMAL},
        
        {.symbol = pk_str("array"), .fun = pk_fun_array, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("string-array"), .fun = pk_fun_string_array, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("byte-array"), .fun = pk_fun_byte_array, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("table"), .fun = pk_fun_table, .args = 0, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("put"), .fun = pk_fun_put, .args = 3, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("put-unique"), .fun = pk_fun_put_unique, .args = 3, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("rem"), .fun = pk_fun_rem, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("rem-strict"), .fun = pk_fun_rem_strict, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("find"), .fun = pk_fun_find, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("find-raw"), .fun = pk_fun_find_raw, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("find-strict"), .fun = pk_fun_find_strict, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("member?"), .fun = pk_fun_is_member, .args = 2, .arity = PK_ARITY_NORMAL},
        
        {.symbol = pk_str("gensym"), .fun = pk_fun_gensym, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("typeof"), .fun = pk_fun_typeof, .args = 1, .arity = PK_ARITY_NORMAL},
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
    pk_pop2(lisp);
}

void pk_std_load(Pocket lisp) {
    pk_std_load_cfun(lisp);
    pk_std_load_lfun(lisp);
}
