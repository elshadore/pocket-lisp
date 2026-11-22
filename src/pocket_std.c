#include "pocket_internals.h"
#include "pocket_std.h"

typedef struct PKFunLoad_ {
    PKStr symbol;
    PKFun fun;
    pk_u8 args;
    PK_ARITY arity;
    pk_u8 options;
} PKFunLoad;

void pk_fun_stub(void *user_closure, Pocket lisp) {
    (void)user_closure;
    (void)lisp;
}

void pk_fun_identity(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_dupe(lisp, 1);
}

void pk_fun_nil(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_nil(lisp);
}

void pk_fun_add(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    if (top == 0) {
        pk_push_int(lisp, 0);
        return;
    }
    pk_dupe(lisp, 1);
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
    pk_dupe(lisp, 1);
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
    pk_dupe(lisp, 1);
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
    pk_dupe(lisp, 1);
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
    PKAtoms slice = pk_stack_slice(lisp);
    pk_push_bool(lisp, pk_atom_eq_atoms(lisp, slice));
}

void pk_fun_struct_eq(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtoms slice = pk_stack_slice(lisp);
    pk_push_bool(lisp, pk_atom_struct_eq_atoms(lisp, slice));
}

void pk_fun_object_eq(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtoms slice = pk_stack_slice(lisp);
    pk_push_bool(lisp, pk_atom_object_eq_atoms(lisp, slice));
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

void pk_fun_fetch(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_fetch(lisp, PK_ENV_VAR, 1);
}

void pk_fun_ffetch(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_fetch(lisp, PK_ENV_FUN, 1);
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

void pk_fun_puts(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_atom_puts(lisp, pk_stack_get(lisp, 1));
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

void pk_fun_read_user_input(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_fs_read_user_input(lisp));
}

void pk_fun_file_read_string(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_fs_file_read_string(lisp, pk_atom_assert_string(lisp, pk_stack_get(lisp, 1))));
}

void pk_fun_file_read_bytes(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_fs_file_read_bytes(lisp, pk_atom_assert_string(lisp, pk_stack_get(lisp, 1))));
}

void pk_fun_file_write(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomString *file_path = pk_atom_assert_string(lisp, pk_stack_get(lisp, 1));
    PKAtom *contents = pk_stack_get(lisp, 2);
    pk_fs_file_write(lisp, file_path, contents);
}

void pk_fun_file_append(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomString *file_path = pk_atom_assert_string(lisp, pk_stack_get(lisp, 1));
    PKAtom *contents = pk_stack_get(lisp, 2);
    pk_fs_file_append(lisp, file_path, contents);
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

void pk_fun_append(void *user_closure, Pocket lisp) {
    (void)user_closure;
    int top = pk_top(lisp);
    for (int i = 2; i <= top; ++i) {
        pk_append(lisp, 1, i);
    }
}

void pk_fun_cat(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtoms atoms = pk_stack_slice(lisp);
    pk_stack_push(lisp, pk_generic_cat(lisp, atoms.e[0], atoms));
}
    
void pk_fun_gensym(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_gensym(lisp);
}

void pk_fun_typeof(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_ident_type(pk_typeof(lisp, 1))));
}

void pk_fun_is_nil(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_nil(pk_stack_get(lisp, 1)));
}

void pk_fun_is_cons(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_cons(pk_stack_get(lisp, 1)));
}

void pk_fun_is_number(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_number(pk_stack_get(lisp, 1)));
}

void pk_fun_is_integer(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_int(pk_stack_get(lisp, 1)));
}

void pk_fun_is_float(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_float(pk_stack_get(lisp, 1)));
}

void pk_fun_is_string(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_string(pk_stack_get(lisp, 1)));
}

void pk_fun_is_character(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_character(pk_stack_get(lisp, 1)));
}

void pk_fun_is_byte(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_byte(pk_stack_get(lisp, 1)));
}

void pk_fun_is_symbol(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_symbol(pk_stack_get(lisp, 1)));
}

void pk_fun_is_keyword(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_keyword(pk_stack_get(lisp, 1)));
}

void pk_fun_is_blob(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_blob(pk_stack_get(lisp, 1)));
}

void pk_fun_is_cfun(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_cfun(pk_stack_get(lisp, 1)));
}

void pk_fun_is_lfun(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_lfun(pk_stack_get(lisp, 1)));
}

void pk_fun_is_function(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_function(pk_stack_get(lisp, 1)));
}

void pk_fun_is_array(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_array(pk_stack_get(lisp, 1)));
}

void pk_fun_is_object(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_object(pk_stack_get(lisp, 1)));
}

void pk_fun_is_macro(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_bool(lisp, pk_atom_is_macro(pk_stack_get(lisp, 1)));
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

void pk_fun_el(void *user_closure, Pocket lisp) {
    (void)user_closure;
    size_t index = pk_atom_to_size_t(lisp, pk_stack_get(lisp, 2));
    pk_el(lisp, 1, index);
}

void pk_fun_is_element(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_is_element(lisp, 1, 2);
}

void pk_fun_length(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_atom_size_t(lisp, pk_length(lisp, 1)));
}

void pk_fun_to_list(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_to_list(lisp, 1);
}

void pk_fun_to_string(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_generic_to_string(lisp, pk_stack_get(lisp, 1)));
}

void pk_fun_to_bytes(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_generic_to_bytes(lisp, pk_stack_get(lisp, 1)));
}

void pk_fun_to_bytes_raw(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_generic_to_bytes_raw(lisp, pk_stack_get(lisp, 1)));
}

void pk_fun_to_symbol(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_generic_to_symbol(lisp, pk_stack_get(lisp, 1)));
}

void pk_fun_to_keyword(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_generic_to_keyword(lisp, pk_stack_get(lisp, 1)));
}

void pk_fun_to_int(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_generic_to_int(lisp, pk_stack_get(lisp, 1)));
}

void pk_fun_to_float(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_stack_push(lisp, (PKAtom *)pk_generic_to_float(lisp, pk_stack_get(lisp, 1)));
}

void pk_fun_object(void *user_closure, Pocket lisp) {
    (void)user_closure;
    PKAtomSymbol *type = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, 1));
    PKAtoms slice = pk_atoms_slice(pk_stack_slice(lisp), 1);
    pk_stack_push(lisp, (PKAtom *)pk_atom_object_cons(lisp, type, NULL, NULL, slice));
}

void pk_fun_make_object(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_object(lisp, NULL, NULL, 1, 2);
}

void pk_fun_object_type(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_object_type(lisp, 1);
}

void pk_fun_put(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_put(lisp, 1, 2, 3);
}

void pk_fun_get(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get(lisp, 1, 2);
}

void pk_fun_remove(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_remove(lisp, 1, 2);
}

void pk_fun_is_property(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_get(lisp, 1, 2);
}

void pk_fun_properties(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_properties(lisp, 1);
}

void pk_std_load_cfun(Pocket lisp) {
    PKFunLoad funs[] = {
        {.symbol = pk_str("identity"), .fun = pk_fun_identity, .args = 1, .arity = PK_ARITY_NORMAL},
        
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

        {.symbol = pk_str("fetch"), .fun = pk_fun_fetch, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("ffetch"), .fun = pk_fun_ffetch, .args = 1, .arity = PK_ARITY_NORMAL},
        
        {.symbol = pk_str("bound?"), .fun = pk_fun_bound, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fbound?"), .fun = pk_fun_fbound, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("unbind"), .fun = pk_fun_unbind, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("funbind"), .fun = pk_fun_funbind, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("list-vars"), .fun = pk_fun_list_vars, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("list-funs"), .fun = pk_fun_list_funs, .args = 0, .arity = PK_ARITY_NORMAL},
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
        
        {.symbol = pk_str("eval"), .fun = pk_fun_eval, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("evlist"), .fun = pk_fun_evlist, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("format"), .fun = pk_fun_format, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("compile"), .fun = pk_fun_compile, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("macroexpand1"), .fun = pk_fun_macroexpand1, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("macroexpand"), .fun = pk_fun_macroexpand, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("print"), .fun = pk_fun_print, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("puts"), .fun = pk_fun_puts, .args = 1, .arity = PK_ARITY_NORMAL},
        
        {.symbol = pk_str("file-read-string"), .fun = pk_fun_file_read_string, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("file-read-bytes"), .fun = pk_fun_file_read_bytes, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("file-write"), .fun = pk_fun_file_write, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("file-append"), .fun = pk_fun_file_append, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("read-user-input"), .fun = pk_fun_read_user_input, .args = 0, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("array"), .fun = pk_fun_array, .args = 0, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("make-array"), .fun = pk_fun_make_array, .args = 2, .arity = PK_ARITY_OPTIONAL, .options = 2},
        {.symbol = pk_str("append"), .fun = pk_fun_append, .args = 2, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("cat"), .fun = pk_fun_cat, .args = 2, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("el"), .fun = pk_fun_el, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("element?"), .fun = pk_fun_is_element, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("length"), .fun = pk_fun_length, .args = 1, .arity = PK_ARITY_NORMAL},
        
        {.symbol = pk_str("gensym"), .fun = pk_fun_gensym, .args = 0, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("typeof"), .fun = pk_fun_typeof, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("nil?"), .fun = pk_fun_is_nil, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("cons?"), .fun = pk_fun_is_cons, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("number?"), .fun = pk_fun_is_number, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("integer?"), .fun = pk_fun_is_integer, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("float?"), .fun = pk_fun_is_float, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("string?"), .fun = pk_fun_is_string, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("char?"), .fun = pk_fun_is_character, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("byte?"), .fun = pk_fun_is_byte, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("symbol?"), .fun = pk_fun_is_symbol, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("keyword?"), .fun = pk_fun_is_keyword, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("blob?"), .fun = pk_fun_is_blob, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("c-function?"), .fun = pk_fun_is_cfun, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("lisp-function?"), .fun = pk_fun_is_lfun, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("function?"), .fun = pk_fun_is_function, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("array?"), .fun = pk_fun_is_array, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("object?"), .fun = pk_fun_is_object, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("macro?"), .fun = pk_fun_is_macro, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("destroy"), .fun = pk_fun_destroy, .args = 1, .arity = PK_ARITY_NORMAL},

        {.symbol = pk_str("map"), .fun = pk_fun_map, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("filter"), .fun = pk_fun_filter, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("fap"), .fun = pk_fun_fap, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("reduce"), .fun = pk_fun_reduce, .args = 3, .arity = PK_ARITY_OPTIONAL, .options = 1},
        
        {.symbol = pk_str("alist->plist"), .fun = pk_fun_alist_to_plist, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("plist->alist"), .fun = pk_fun_plist_to_alist, .args = 1, .arity = PK_ARITY_NORMAL},
        
        {.symbol = pk_str("->list"), .fun = pk_fun_to_list, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("->string"), .fun = pk_fun_to_string, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("->bytes"), .fun = pk_fun_to_bytes, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("->bytes-raw"), .fun = pk_fun_to_bytes_raw, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("->symbol"), .fun = pk_fun_to_symbol, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("->keyword"), .fun = pk_fun_to_keyword, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("->int"), .fun = pk_fun_to_int, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("->float"), .fun = pk_fun_to_float, .args = 1, .arity = PK_ARITY_NORMAL},
        
        {.symbol = pk_str("object"), .fun = pk_fun_object, .args = 1, .arity = PK_ARITY_VARIADIC},
        {.symbol = pk_str("make-object"), .fun = pk_fun_make_object, .args = 2, .arity = PK_ARITY_OPTIONAL, .options = 1},
        {.symbol = pk_str("object-type"), .fun = pk_fun_object_type, .args = 1, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("put"), .fun = pk_fun_put, .args = 3, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("get"), .fun = pk_fun_get, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("remove"), .fun = pk_fun_remove, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("property?"), .fun = pk_fun_is_property, .args = 2, .arity = PK_ARITY_NORMAL},
        {.symbol = pk_str("properties"), .fun = pk_fun_properties, .args = 1, .arity = PK_ARITY_NORMAL},
        
    };

    size_t length = pk_alen(funs);

    for (size_t i = 0; i < length; ++i) {
        PKFunLoad fun = funs[i];
        PKFunArgs args = (PKFunArgs) {
            .args = fun.args,
            .arity = fun.arity,
            .options = fun.options,
        };
        pk_push_symbol(lisp, fun.symbol);
        pk_push_function(lisp, NULL, fun.fun, &args);
        pk_set(lisp, PK_ENV_FUN, -2, -1);
        pk_pop3(lisp);
    }
}

void pk_std_load_lfun(Pocket lisp) {
    pk_push_string(lisp, pk_str(PK_STD_CODE));
    pk_parse(lisp, -1, PK_MODE_EXPRLIST);
    pk_eval(lisp, -1, PK_MODE_EXPRLIST);
    pk_pop2(lisp);
}

void pk_std_load(Pocket lisp) {
    pk_std_load_cfun(lisp);
    pk_std_load_lfun(lisp);
}
