#include "pocket_internals.h"
#include "pocket_std.h"

void pk_fun_stub(void *user_closure, Pocket lisp) {
    (void)user_closure;
    (void)lisp;
}

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
    (void)user_closure;
    int top = pk_top(lisp);
    if (top == 0) {
        pk_push_nil(lisp);
    } else {
        pk_list(lisp, 1, top);
    }
}

void pk_fun_car(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_car(lisp, 1);
}

void pk_fun_assoc(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_assoc(lisp, 1, 2);
}

void pk_fun_cdr(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_cdr(lisp, 1);
}

void pk_fun_car_strict(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_car_strict(lisp, 1);
}

void pk_fun_cdr_strict(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_cdr_strict(lisp, 1);
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

void pk_fun_clone(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_clone(lisp, 1);
}

void pk_fun_is_circular(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_is_circular(lisp, 1));
}

void pk_fun_set(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_set(lisp, PK_ENV_VAR, 1, 2);
}

void pk_fun_fset(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_set(lisp, PK_ENV_FUN, 1, 2);
}

void pk_fun_get(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get(lisp, PK_ENV_VAR, 1);
}

void pk_fun_fget(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get(lisp, PK_ENV_FUN, 1);
}

void pk_fun_get_raw(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get_raw(lisp, PK_ENV_VAR, 1);
}

void pk_fun_fget_raw(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get_raw(lisp, PK_ENV_FUN, 1);
}

void pk_fun_bound(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_is_bound(lisp, PK_ENV_VAR, 1));
}

void pk_fun_fbound(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_is_bound(lisp, PK_ENV_FUN, 1));
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

void pk_fun_list_vars(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_list_vars(lisp);
}

void pk_fun_list_funs(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_list_funs(lisp);
}

void pk_fun_list_types(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_list_types(lisp);
}

void pk_fun_list_methods(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_list_methods(lisp, 1);
}

void pk_fun_list_slots(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_list_slots(lisp, 1);
}

void pk_fun_list_implementors(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_list_implementors(lisp, 1);
}

void pk_fun_list_symbols(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_list_symbols(lisp);
}

void pk_fun_funcatch(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomSymbol *symbol = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, 1));
    pk_frame_mode_set_catch(lisp, symbol);
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

void pk_fun_parse(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_parse(lisp, 1, PK_MODE_EXPRESSION);
}

void pk_fun_parse_list(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_parse(lisp, 1, PK_MODE_EXPRLIST);
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
    PKAtom *atom = pk_stack_get(lisp, 1);
    pk_atom_compile(lisp, PK_CALL_FUN, atom, PK_MODE_EXPRESSION);
}

void pk_fun_print(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_print(lisp, 1);
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

void pk_fun_file_open(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomString *path = pk_atom_assert_string(lisp, pk_stack_get(lisp, 1));
    PK_OPEN_MODE mode = pk_open_mode_from_str(lisp, pk_atom_stream_mode_name(lisp, pk_stack_get(lisp, 2)));
    pk_stack_push(lisp, (PKAtom *)pk_atom_stream_file_open(lisp, path->str, mode));
}

void pk_fun_destroy(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtom *atom = pk_stack_get(lisp, 1);
    pk_stack_push(lisp, pk_atom_destroy(lisp, atom));
}

void pk_fun_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtoms slice = pk_stack_slice(lisp);
    pk_stack_push(lisp, (PKAtom *)pk_atom_array(lisp, slice));
}

void pk_fun_make_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    size_t count = 0;
    {
        PKAtom *acount = pk_stack_get(lisp, 1);
        if (!pk_atom_is_nil(acount)) {
            count = pk_atom_to_size_t(lisp, acount);
        }
    }
    PKAtom *fill = pk_stack_get(lisp, 2);
    pk_stack_push(lisp, (PKAtom *)pk_atom_array_make(lisp, count, fill));
}

void pk_fun_table(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtoms slice = pk_stack_slice(lisp);
    pk_stack_push(lisp, (PKAtom *)pk_atom_table_cons(lisp, slice));
}

void pk_fun_string_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtoms slice = pk_stack_slice(lisp);
    pk_stack_push(lisp, (PKAtom *)pk_atom_string_array(lisp, slice));
}

void pk_fun_byte_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtoms slice = pk_stack_slice(lisp);
    pk_stack_push(lisp, (PKAtom *)pk_atom_byte_array(lisp, slice));
}

void pk_fun_gensym(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_gensym(lisp);
}

void pk_fun_typeof(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_typeof(lisp, 1);
}

void pk_fun_new(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomSymbol *type = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, 1));
    PKAtoms slots = pk_atoms_slice(pk_stack_slice(lisp), 1);
    pk_stack_push(lisp, (PKAtom *)pk_atom_object_new_cons(lisp, type, slots));
}

void pk_fun_slot_set(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_slot_set(lisp, 1, 2, 3);
    pk_push_nil(lisp);
}

void pk_fun_slot_get(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_slot_get(lisp, 1, 2);
}

void pk_fun_method_set(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_method_set(lisp, 1, 2, 3);
}

void pk_fun_method_get(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_method_get(lisp, 1, 2);
}

void pk_fun_method_get_raw(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_method_get_raw(lisp, 1, 2);
}

void pk_fun_map(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_map(lisp, 1, 2);
}

void pk_fun_filter(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_filter(lisp, 1, 2);
}

void pk_fun_fap(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_fap(lisp, 1, 2);
}

void pk_fun_reduce(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_reduce(lisp, 1, 2, 3);
}

void pk_fun_alist_to_plist(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, pk_atom_alist_to_plist(lisp, pk_stack_get(lisp, 1)));
}

void pk_fun_plist_to_alist(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, pk_atom_plist_to_alist(lisp, pk_stack_get(lisp, 1)));
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
        {.symbol = pk_str("object-eq?"), .fun = pk_fun_object_eq, .args = 2, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("struct-eq?"), .fun = pk_fun_struct_eq, .args = 2, .arity = PK_ARITY_VARIADIC},

        {.symbol = pk_str("cons"), .fun = pk_fun_cons, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("list"), .fun = pk_fun_list, .args = 0, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("car"), .fun = pk_fun_car, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("assoc"), .fun = pk_fun_assoc, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("cdr"), .fun = pk_fun_cdr, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("car-strict"), .fun = pk_fun_car_strict, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("cdr-strict"), .fun = pk_fun_cdr_strict, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("set-car"), .fun = pk_fun_set_car, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("set-cdr"), .fun = pk_fun_set_cdr, .args = 2, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("clone"), .fun = pk_fun_clone, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("circular?"), .fun = pk_fun_is_circular, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("set"), .fun = pk_fun_set, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fset"), .fun = pk_fun_fset, .args = 2, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("get"), .fun = pk_fun_get, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fget"), .fun = pk_fun_fget, .args = 1, .arity = PK_ARITY_NORMAL},
        
        {.symbol = pk_str("get-raw"), .fun = pk_fun_get_raw, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fget-raw"), .fun = pk_fun_fget_raw, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("bound?"), .fun = pk_fun_bound, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fbound?"), .fun = pk_fun_fbound, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("unbind"), .fun = pk_fun_unbind, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("funbind"), .fun = pk_fun_funbind, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("list-vars"), .fun = pk_fun_list_vars, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("list-funs"), .fun = pk_fun_list_funs, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("list-types"), .fun = pk_fun_list_types, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("list-methods"), .fun = pk_fun_list_methods, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("list-slots"), .fun = pk_fun_list_slots, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("list-implementors"), .fun = pk_fun_list_implementors, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("list-symbols"), .fun = pk_fun_list_symbols, .args = 0, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("funcatch"), .fun = pk_fun_funcatch, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("propagate"), .fun = pk_fun_propagate, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("throw"), .fun = pk_fun_throw, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("error"), .fun = pk_fun_error, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("hexdump"), .fun = pk_fun_hexdump, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("parse"), .fun = pk_fun_parse, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("parse-list"), .fun = pk_fun_parse_list, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("apply"), .fun = pk_fun_apply, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("macro-apply"), .fun = pk_fun_macro_apply, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("funcall"), .fun = pk_fun_funcall, .args = 1, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("print"), .fun = pk_fun_print, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("eval"), .fun = pk_fun_eval, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("evlist"), .fun = pk_fun_evlist, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("format"), .fun = pk_fun_format, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("compile"), .fun = pk_fun_compile, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("macroexpand1"), .fun = pk_fun_macroexpand1, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("macroexpand"), .fun = pk_fun_macroexpand, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("file-open"), .fun = pk_fun_file_open, .args = 2, .arity = PK_ARITY_OPTIONAL, .options = 1},

        {.symbol = pk_str("array"), .fun = pk_fun_array, .args = 0, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("make-array"), .fun = pk_fun_make_array, .args = 2, .arity = PK_ARITY_OPTIONAL, .options = 2},
        
        {.symbol = pk_str("string-array"), .fun = pk_fun_string_array, .args = 0, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("byte-array"), .fun = pk_fun_byte_array, .args = 0, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("table"), .fun = pk_fun_table, .args = 0, .arity = PK_ARITY_VARIADIC},

        {.symbol = pk_str("gensym"), .fun = pk_fun_gensym, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("typeof"), .fun = pk_fun_typeof, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("destroy"), .fun = pk_fun_destroy, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("new"), .fun = pk_fun_new, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("set-slot"), .fun = pk_fun_slot_set, .args = 3, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("get-slot"), .fun = pk_fun_slot_get, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("set-method"), .fun = pk_fun_method_set, .args = 3, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("get-method"), .fun = pk_fun_method_get, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("get-method-raw"), .fun = pk_fun_method_get_raw, .args = 2, .arity = PK_ARITY_NORMAL},
        
        {.symbol = pk_str("map"), .fun = pk_fun_map, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("filter"), .fun = pk_fun_filter, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fap"), .fun = pk_fun_fap, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("reduce"), .fun = pk_fun_reduce, .args = 3, .arity = PK_ARITY_OPTIONAL, .options = 1},
        
        {.symbol = pk_str("alist->plist"), .fun = pk_fun_alist_to_plist, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("plist->alist"), .fun = pk_fun_plist_to_alist, .args = 1, .arity = PK_ARITY_NORMAL},
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

void pk_method_append_list(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomCons *list = pk_atom_assert_cons(lisp, pk_stack_get(lisp, 1));
    PKAtom *atom = pk_stack_get(lisp, 2);
    pk_atom_list_append(lisp, list, atom);
    pk_stack_push(lisp, (PKAtom *)list);
}

void pk_method_append_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomArray *array = pk_atom_assert_array(lisp, pk_stack_get(lisp, 1));
    PKAtom *atom = pk_stack_get(lisp, 2);
    pk_atom_array_append(lisp, array, atom);
    pk_stack_push(lisp, (PKAtom *)array);
}

void pk_method_append_byte_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomByteArray *array = pk_atom_assert_byte_array(lisp, pk_stack_get(lisp, 1));
    PKAtom *atom = pk_stack_get(lisp, 2);
    pk_atom_byte_array_append(lisp, array, atom);
    pk_stack_push(lisp, (PKAtom *)array);
}

void pk_method_append_string_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomStringArray *array = pk_atom_assert_string_array(lisp, pk_stack_get(lisp, 1));
    PKAtom *atom = pk_stack_get(lisp, 2);
    pk_atom_string_array_append(lisp, array, atom);
    pk_stack_push(lisp, (PKAtom *)array);
}

void pk_method_el_list(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtom *list = pk_stack_get(lisp, 1);
    size_t index = pk_atom_to_size_t(lisp, pk_stack_get(lisp, 2));
    pk_stack_push(lisp, pk_atom_list_el(lisp, list, index));
}

void pk_method_el_string(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomString *string = pk_atom_assert_string(lisp, pk_stack_get(lisp, 1));
    size_t index = pk_atom_to_size_t(lisp, pk_stack_get(lisp, 2));
    pk_stack_push(lisp, (PKAtom *)pk_atom_char(lisp, pk_atom_string_el(lisp, string, index)));
}

void pk_method_el_blob(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomBlob *blob = pk_atom_assert_blob(lisp, pk_stack_get(lisp, 1));
    size_t index = pk_atom_to_size_t(lisp, pk_stack_get(lisp, 2));
    pk_stack_push(lisp, (PKAtom *)pk_atom_char(lisp, pk_atom_blob_el(lisp, blob, index)));
}

void pk_method_el_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomArray *array = pk_atom_assert_array(lisp, pk_stack_get(lisp, 1));
    size_t index = pk_atom_to_size_t(lisp, pk_stack_get(lisp, 2));
    pk_stack_push(lisp, pk_atom_array_el(lisp, array, index));
}

void pk_method_el_string_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomStringArray *array = pk_atom_assert_string_array(lisp, pk_stack_get(lisp, 1));
    size_t index = pk_atom_to_size_t(lisp, pk_stack_get(lisp, 2));
    pk_stack_push(lisp, (PKAtom *)pk_atom_char(lisp, pk_atom_string_array_el(lisp, array, index)));
}

void pk_method_el_byte_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomByteArray *array = pk_atom_assert_byte_array(lisp, pk_stack_get(lisp, 1));
    size_t index = pk_atom_to_size_t(lisp, pk_stack_get(lisp, 2));
    pk_stack_push(lisp, (PKAtom *)pk_atom_byte(lisp, pk_atom_byte_array_el(lisp, array, index)));
}

void pk_method_length_nil(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_int(lisp, 0);
}
    
void pk_method_length_list(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtom *list = pk_stack_get(lisp, 1);
    pk_stack_push(lisp, (PKAtom *)pk_atom_size_t(lisp, pk_atom_list_length(lisp, list)));
}
    
void pk_method_length_string(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomString *string = pk_atom_assert_string(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_size_t(lisp, pk_atom_string_length(lisp, string)));
}

void pk_method_length_character(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_int(lisp, 1);
}

void pk_method_length_blob(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomBlob *blob = pk_atom_assert_blob(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_size_t(lisp, pk_atom_blob_length(lisp, blob)));
}

void pk_method_length_table(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomTable *table = pk_atom_assert_table(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_size_t(lisp, pk_atom_table_length(lisp, table)));
}

void pk_method_length_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomArray *array = pk_atom_assert_array(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_size_t(lisp, pk_atom_array_length(lisp, array)));
}

void pk_method_length_string_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomStringArray *array = pk_atom_assert_string_array(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_size_t(lisp, pk_atom_string_array_length(lisp, array)));
}

void pk_method_length_byte_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomByteArray *array = pk_atom_assert_byte_array(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_size_t(lisp, pk_atom_byte_array_length(lisp, array)));
}

void pk_method_put_table(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomTable *table = pk_atom_assert_table(lisp, pk_stack_get(lisp, 1));
    PKAtom *key = pk_stack_get(lisp, 2);
    PKAtom *value = pk_stack_get(lisp, 3);
    pk_stack_push(lisp, pk_atom_table_put(lisp, table, key, value));
}

void pk_method_find_table(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomTable *table = pk_atom_assert_table(lisp, pk_stack_get(lisp, 1));
    PKAtom *key = pk_stack_get(lisp, 2);
    pk_stack_push(lisp, pk_atom_table_find(lisp, table, key));
}

void pk_method_remove_table(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomTable *table = pk_atom_assert_table(lisp, pk_stack_get(lisp, 1));
    PKAtom *key = pk_stack_get(lisp, 2);
    pk_stack_push(lisp, pk_atom_table_rem(lisp, table, key));
}

void pk_method_is_member_nil(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_nil(lisp);
}

void pk_method_is_member_list(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtom *list = pk_stack_get(lisp, 1);
    PKAtom *key = pk_stack_get(lisp, 2);
    pk_stack_push(lisp, pk_atom_bool(lisp, pk_atom_list_is_member(lisp, list, key)));
}

void pk_method_is_member_table(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomTable *table = pk_atom_assert_table(lisp, pk_stack_get(lisp, 1));
    PKAtom *key = pk_stack_get(lisp, 2);
    pk_stack_push(lisp, pk_atom_bool(lisp, pk_atom_table_is_member(lisp, table, key)));
}

void pk_method_read_stream(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomStream *stream = pk_atom_assert_stream(lisp, pk_stack_get(lisp, 1));
    size_t amount = pk_atom_to_size_t(lisp, pk_stack_get(lisp, 2));
    PKAtom *mode = pk_stack_get(lisp, 3);
    pk_stack_push(lisp, pk_atom_stream_read_atom(lisp, stream, amount, mode));
}

void pk_method_write_stream(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomStream *stream = pk_atom_assert_stream(lisp, pk_stack_get(lisp, 1));
    PKAtom *contents = pk_stack_get(lisp, 2);
    pk_atom_stream_write(lisp, stream, contents);
    pk_stack_push(lisp, contents);
}

void pk_method_flush_stream(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomStream *stream = pk_atom_assert_stream(lisp, pk_stack_get(lisp, 1));
    pk_atom_stream_flush(lisp, stream);
}

void pk_method_to_string_string(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, pk_stack_get(lisp, 1));
}

void pk_method_to_char_string(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomString *string = pk_atom_assert_string(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_string_to_char(lisp, string));
}

void pk_method_to_chars_string(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomString *string = pk_atom_assert_string(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, pk_atom_string_to_chars(lisp, string));
}

void pk_method_to_bytes_string(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomString *string = pk_atom_assert_string(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_string_to_bytes(lisp, string));
}

void pk_method_to_string_character(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomChar *character = pk_atom_assert_character(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_string_char(lisp, character->c));
}

void pk_method_to_int_character(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomChar *character = pk_atom_assert_character(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_int(lisp, (int)character->c));
}

void pk_method_to_char_character(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, pk_stack_get(lisp, 1));
}

void pk_method_to_string_number(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_atom_format(lisp, pk_stack_get(lisp, 1)));
}

void pk_method_to_int_number(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomNumber *number = pk_atom_assert_number(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_int(lisp, pk_atom_to_int(lisp, (PKAtom *)number, PK_NUMBER_CONV_FLOAT_CONV)));
}

void pk_method_to_float_number(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomNumber *number = pk_atom_assert_number(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_float(lisp, pk_atom_to_float(lisp, (PKAtom *)number, PK_NUMBER_CONV_FLOAT_CONV)));
}

void pk_method_to_list_list(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, pk_stack_get(lisp, 1));
}

void pk_method_to_bytes_blob(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomBlob *blob = pk_atom_assert_blob(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_byte_array_cons(lisp, blob->bytes));
}

void pk_method_to_list_blob(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomBlob *blob = pk_atom_assert_blob(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, pk_atom_to_list_bytes(lisp, blob->bytes));
}

void pk_method_to_byte_blob(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomBlob *blob = pk_atom_assert_blob(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_byte(lisp, pk_blob_to_byte(lisp, blob->bytes)));
}

void pk_method_to_byte_number(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomNumber *number = pk_atom_assert_number(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_byte(lisp, pk_atom_number_to_byte(lisp, number)));
}

void pk_method_to_byte_character(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomChar *character = pk_atom_assert_character(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_byte(lisp, pk_atom_character_to_byte(lisp, character)));
}

void pk_method_to_list_table(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomTable *table = pk_atom_assert_table(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, pk_atom_table_to_list(lisp, table));
}

void pk_method_to_list_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomArray *array = pk_atom_assert_array(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, pk_atom_array_to_list(lisp, array));
}

void pk_method_to_string_string_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomStringArray *array = pk_atom_assert_string_array(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_string_array_to_string(lisp, array));
}

void pk_method_to_chars_string_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomStringArray *array = pk_atom_assert_string_array(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, pk_atom_string_array_to_chars(lisp, array));
}

void pk_method_to_list_string_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomStringArray *array = pk_atom_assert_string_array(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, pk_atom_string_array_to_chars(lisp, array));
}

void pk_method_to_char_string_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomStringArray *array = pk_atom_assert_string_array(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_string_array_to_char(lisp, array));
}

void pk_method_to_byte_byte_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomByteArray *array = pk_atom_assert_byte_array(lisp, pk_stack_get(lisp, 1));
    size_t index = pk_atom_to_size_t(lisp, pk_stack_get(lisp, 2));
    pk_stack_push(lisp, (PKAtom *)pk_atom_byte(lisp, pk_atom_byte_array_to_byte(lisp, array, index)));
}

void pk_method_to_bytes_byte_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomByteArray *array = pk_atom_assert_byte_array(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, (PKAtom *)pk_atom_byte_array_cons(lisp, pk_atom_byte_array_to_bytes(lisp, array)));
}

void pk_method_to_list_byte_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomByteArray *array = pk_atom_assert_byte_array(lisp, pk_stack_get(lisp, 1));
    pk_stack_push(lisp, pk_atom_byte_array_to_list(lisp, array));
}

void pk_method_to_list_nil(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_nil(lisp);
}

void pk_method_to_string_nil(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_string(lisp, PK_STR_EMPTY);
}

void pk_method_cat_list(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtoms slice = pk_stack_slice(lisp);
    pk_stack_push(lisp, pk_atom_list_cat(lisp, slice));
}

void pk_method_cat_string(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtoms slice = pk_stack_slice(lisp);
    pk_stack_push(lisp, (PKAtom *)pk_atom_string_cat(lisp, slice));
}

void pk_method_cat_blob(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtoms slice = pk_stack_slice(lisp);
    pk_stack_push(lisp, (PKAtom *)pk_atom_blob_cat(lisp, slice));
}

void pk_method_loader(Pocket lisp, PKStr type, PKFunLoad *methods, size_t length) {
    pk_push_symbol(lisp, type);
    for (size_t i = 0; i < length; ++i) {
        PKFunLoad method = methods[i];
        pk_push_symbol(lisp, method.symbol);
        pk_push_cfun(lisp, NULL, method.fun, method.args, method.arity, method.options);
        pk_method_set(lisp, -3, -2, -1);
        pk_pop3(lisp);
    }
    pk_pop(lisp);
}

void pk_std_load_cmethods(Pocket lisp) {
    {
        // Nil
        PKFunLoad methods[] = {
            {.symbol = pk_str("append"), .fun = pk_fun_stub, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("el"), .fun = pk_fun_stub, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("cat"), .fun = pk_method_cat_list, .args = 1, .arity = PK_ARITY_VARIADIC},
            {.symbol = pk_str("length"), .fun = pk_method_length_nil, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("member?"), .fun = pk_method_is_member_nil, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->list"), .fun = pk_method_to_list_nil, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->string"), .fun = pk_method_to_string_nil, .args = 1, .arity = PK_ARITY_NORMAL},
        };
        pk_method_loader(lisp, pk_str("Nil"), methods, pk_alen(methods));
    }
    {
        // Cons
        PKFunLoad methods[] = {
            {.symbol = pk_str("append"), .fun = pk_method_append_list, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("el"), .fun = pk_method_el_list, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("cat"), .fun = pk_method_cat_list, .args = 1, .arity = PK_ARITY_VARIADIC},
            {.symbol = pk_str("length"), .fun = pk_method_length_list, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("member?"), .fun = pk_method_is_member_list, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->list"), .fun = pk_method_to_list_list, .args = 1, .arity = PK_ARITY_NORMAL},
        };
        pk_method_loader(lisp, pk_str("Cons"), methods, pk_alen(methods));
    }
    {
        // Number
        PKFunLoad methods[] = {
            {.symbol = pk_str("->string"), .fun = pk_method_to_string_number, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->int"), .fun = pk_method_to_int_number, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->float"), .fun = pk_method_to_float_number, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->byte"), .fun = pk_method_to_byte_number, .args = 1, .arity = PK_ARITY_NORMAL},
        };
        pk_method_loader(lisp, pk_str("Number"), methods, pk_alen(methods));
    }
    {
        // String
        PKFunLoad methods[] = {
            {.symbol = pk_str("el"), .fun = pk_method_el_string, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("cat"), .fun = pk_method_cat_string, .args = 1, .arity = PK_ARITY_VARIADIC},
            {.symbol = pk_str("length"), .fun = pk_method_length_string, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->string"), .fun = pk_method_to_string_string, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->char"), .fun = pk_method_to_char_string, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->chars"), .fun = pk_method_to_chars_string, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->bytes"), .fun = pk_method_to_bytes_string, .args = 1, .arity = PK_ARITY_NORMAL},
        };
        pk_method_loader(lisp, pk_str("String"), methods, pk_alen(methods));
    }
    {
        // Character
        PKFunLoad methods[] = {
            {.symbol = pk_str("cat"), .fun = pk_method_cat_string, .args = 1, .arity = PK_ARITY_VARIADIC},
            {.symbol = pk_str("length"), .fun = pk_method_length_character, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->string"), .fun = pk_method_to_string_character, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->int"), .fun = pk_method_to_int_character, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->char"), .fun = pk_method_to_char_character, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->byte"), .fun = pk_method_to_byte_character, .args = 1, .arity = PK_ARITY_NORMAL},
        };
        pk_method_loader(lisp, pk_str("Character"), methods, pk_alen(methods));
    }
    {
        // Blob
        PKFunLoad methods[] = {
            {.symbol = pk_str("el"), .fun = pk_method_el_blob, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("cat"), .fun = pk_method_cat_blob, .args = 1, .arity = PK_ARITY_VARIADIC},
            {.symbol = pk_str("length"), .fun = pk_method_length_blob, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->bytes"), .fun = pk_method_to_bytes_blob, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->list"), .fun = pk_method_to_list_blob, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->byte"), .fun = pk_method_to_byte_blob, .args = 1, .arity = PK_ARITY_NORMAL},
        };
        pk_method_loader(lisp, pk_str("Blob"), methods, pk_alen(methods));
    }
    {
        // Table
        PKFunLoad methods[] = {
            {.symbol = pk_str("length"), .fun = pk_method_length_table, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("put"), .fun = pk_method_put_table, .args = 3, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("find"), .fun = pk_method_find_table, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("remove"), .fun = pk_method_remove_table, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("member?"), .fun = pk_method_is_member_table, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->list"), .fun = pk_method_to_list_table, .args = 1, .arity = PK_ARITY_NORMAL},
        };
        pk_method_loader(lisp, pk_str("Table"), methods, pk_alen(methods));
    }
    {
        // Array
        PKFunLoad methods[] = {
            {.symbol = pk_str("append"), .fun = pk_method_append_array, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("el"), .fun = pk_method_el_array, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("length"), .fun = pk_method_length_array, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->list"), .fun = pk_method_to_list_array, .args = 1, .arity = PK_ARITY_NORMAL},
        };
        pk_method_loader(lisp, pk_str("Array"), methods, pk_alen(methods));
    }
    {
        // String-Array
        PKFunLoad methods[] = {
            {.symbol = pk_str("append"), .fun = pk_method_append_string_array, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("el"), .fun = pk_method_el_string_array, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("length"), .fun = pk_method_length_string_array, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->string"), .fun = pk_method_to_string_string_array, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->chars"), .fun = pk_method_to_chars_string_array, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->list"), .fun = pk_method_to_list_string_array, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->char"), .fun = pk_method_to_char_string_array, .args = 1, .arity = PK_ARITY_NORMAL},
        };
        pk_method_loader(lisp, pk_str("String-Array"), methods, pk_alen(methods));
    }
    {
        // Byte-Array
        PKFunLoad methods[] = {
            {.symbol = pk_str("append"), .fun = pk_method_append_byte_array, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("el"), .fun = pk_method_el_byte_array, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("length"), .fun = pk_method_length_byte_array, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->byte"), .fun = pk_method_to_byte_byte_array, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->bytes"), .fun = pk_method_to_bytes_byte_array, .args = 1, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("->list"), .fun = pk_method_to_list_byte_array, .args = 1, .arity = PK_ARITY_NORMAL},
        };
        pk_method_loader(lisp, pk_str("Byte-Array"), methods, pk_alen(methods));
    }
    {
        // Stream
        PKFunLoad methods[] = {
            {.symbol = pk_str("read"), .fun = pk_method_read_stream, .args = 3, .arity = PK_ARITY_OPTIONAL, .options = 1},
            {.symbol = pk_str("write"), .fun = pk_method_write_stream, .args = 2, .arity = PK_ARITY_NORMAL},
            {.symbol = pk_str("flush"), .fun = pk_method_flush_stream, .args = 1, .arity = PK_ARITY_NORMAL},
        };
        pk_method_loader(lisp, pk_str("Stream"), methods, pk_alen(methods));
    }
}

void pk_std_load_lfun(Pocket lisp) {
    pk_push_string(lisp, pk_str(PK_STD_CODE));
    pk_parse(lisp, -1, PK_MODE_EXPRLIST);
    pk_eval(lisp, -1, PK_MODE_EXPRLIST);
    pk_pop2(lisp);
}

void pk_std_load_stream(Pocket lisp, PKStr symbol, FILE *stream) {
    pk_push_symbol(lisp, symbol);
    pk_push_stream(lisp, stream, true);
    pk_set(lisp, PK_ENV_VAR, -2, -1);
    pk_pop3(lisp);
}

void pk_std_load(Pocket lisp) {
    pk_std_load_cfun(lisp);
    pk_std_load_cmethods(lisp);
    pk_std_load_stream(lisp, pk_str("stdin"), stdin);
    pk_std_load_stream(lisp, pk_str("stdout"), stdout);
    pk_std_load_stream(lisp, pk_str("stderr"), stderr);
    pk_std_load_lfun(lisp);
}
