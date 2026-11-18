#include "pocket_internals.h"

typedef struct PKPushFmtData_ {
    const char *fmt;
    PKWriter w;
    va_list args;
} PKPushFmtData;

void pk_push_nil(Pocket lisp) {
    pk_stack_push(lisp, lisp->cache.nil);
}

void pk_push_true(Pocket lisp) {
    pk_stack_push(lisp, lisp->cache.atom_true);
}

void pk_push_bool(Pocket lisp, bool boolean) {
    pk_stack_push(lisp, pk_atom_bool(lisp, boolean));
}

void pk_push_undefined(Pocket lisp) {
    pk_stack_push(lisp, lisp->cache.undefined);
}

void pk_push_int(Pocket lisp, int integer) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_int(lisp, integer));
}

void pk_push_float(Pocket lisp, float floater) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_float(lisp, floater));
}

void pk_push_string(Pocket lisp, PKStr string) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_string(lisp, string));
}

void pk_push_stringc(Pocket lisp, char *cstring) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_stringc(lisp, cstring));
}

void pk_push_stringf_body(void *user_closure, Pocket lisp) {
    PKPushFmtData *data = (PKPushFmtData *)user_closure;
    pk_writer_vfmt(lisp, &data->w, data->fmt, data->args);
    pk_push_string(lisp, pk_writer_slice(&data->w));
}

void pk_push_stringf(Pocket lisp, const char *fmt, ...) {
    PKPushFmtData data = (PKPushFmtData) {
        .fmt = fmt,
        .w = PK_WRITER_EMPTY,
    };

    va_start(data.args, fmt);

    PK_RES result = pk_protect_noreturn(lisp, &data, pk_push_stringf_body, 0);

    pk_writer_deinit(lisp, &data.w);
    va_end(data.args);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }
}

void pk_push_symbol(Pocket lisp, PKStr symbol) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, symbol));
}

void pk_push_symbolc(Pocket lisp, char *csymbol) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbolc(lisp, csymbol));
}

void pk_push_symbolf_body(void *user_closure, Pocket lisp) {
    PKPushFmtData *data = (PKPushFmtData *)user_closure;
    pk_writer_vfmt(lisp, &data->w, data->fmt, data->args);
    pk_push_symbol(lisp, pk_writer_slice(&data->w));
}

void pk_push_symbolf(Pocket lisp, const char *fmt, ...) {
    PKPushFmtData data = (PKPushFmtData) {
        .fmt = fmt,
        .w = PK_WRITER_EMPTY,
    };

    va_start(data.args, fmt);

    PK_RES result = pk_protect_noreturn(lisp, &data, pk_push_symbolf_body, 0);

    pk_writer_deinit(lisp, &data.w);
    va_end(data.args);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }
}

void pk_push_gensym(Pocket lisp) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_gensym(lisp));
}

void pk_push_keyword(Pocket lisp, PKStr keyword) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_keyword(lisp, keyword));
}

void pk_push_keywordc(Pocket lisp, char *ckeyword) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_keywordc(lisp, ckeyword));
}

void pk_push_keywordf_body(void *user_closure, Pocket lisp) {
    PKPushFmtData *data = (PKPushFmtData *)user_closure;
    pk_writer_vfmt(lisp, &data->w, data->fmt, data->args);
    pk_push_keyword(lisp, pk_writer_slice(&data->w));
}

void pk_push_keywordf(Pocket lisp, const char *fmt, ...) {
    PKPushFmtData data = (PKPushFmtData) {
        .fmt = fmt,
        .w = PK_WRITER_EMPTY,
    };

    va_start(data.args, fmt);

    PK_RES result = pk_protect_noreturn(lisp, &data, pk_push_keywordf_body, 0);

    pk_writer_deinit(lisp, &data.w);
    va_end(data.args);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }
}

void pk_push_cons(Pocket lisp, int car, int cdr) {
    PKAtom *acar = pk_stack_get(lisp, car);
    PKAtom *acdr = pk_stack_get(lisp, cdr);
    pk_stack_push(lisp, (PKAtom *)pk_atom_cons(lisp, acar, acdr));
}

void pk_list(Pocket lisp, int head, int tail) {
    size_t a = pk_sp_index(lisp, head);
    size_t b = pk_sp_index(lisp, tail);

    if (a <= b) {
        PKAtoms atoms = (PKAtoms) { .e = &lisp->stack.e[a], .length = (b - a) + 1 };
        pk_stack_push(lisp, pk_atom_list(lisp, atoms));
    } else {
        PKAtoms atoms = (PKAtoms) { .e = &lisp->stack.e[b], .length = (a - b) + 1 };
        pk_stack_push(lisp, pk_atom_list_rev(lisp, atoms));
    }
}

void pk_push_function(Pocket lisp, void *user_closure, PKFun fun, PKFunArgs *args) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_cfun(lisp, user_closure, fun, PK_CALL_FUN, args));
}

void pk_push_macro(Pocket lisp, void *user_closure, PKFun fun, PKFunArgs *args) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_cfun(lisp, user_closure, fun, PK_CALL_MACRO, args));
}

void pk_push_array(Pocket lisp) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_array(lisp, PK_ATOMS_EMPTY));
}

void pk_destroy(Pocket lisp, int atom) {
    pk_stack_push(lisp, pk_atom_destroy(lisp, pk_stack_get(lisp, atom)));
}

void pk_car(Pocket lisp, int cons) {
    pk_stack_push(lisp, pk_atom_car(lisp, pk_stack_get(lisp, cons)));
}

void pk_cdr(Pocket lisp, int cons) {
    pk_stack_push(lisp, pk_atom_cdr(lisp, pk_stack_get(lisp, cons)));
}

void pk_car_strict(Pocket lisp, int cons) {
    pk_stack_push(lisp, pk_atom_car_strict(lisp, pk_stack_get(lisp, cons)));
}

void pk_cdr_strict(Pocket lisp, int cons) {
    pk_stack_push(lisp, pk_atom_cdr_strict(lisp, pk_stack_get(lisp, cons)));
}

void pk_set_car(Pocket lisp, int cons, int newcar) {
    PKAtom *acons = pk_stack_get(lisp, cons);
    PKAtom *anewcar = pk_stack_get(lisp, newcar);
    pk_atom_set_car(lisp, acons, anewcar);
}

void pk_set_cdr(Pocket lisp, int cons, int newcdr) {
    PKAtom *acons = pk_stack_get(lisp, cons);
    PKAtom *anewcdr = pk_stack_get(lisp, newcdr);
    pk_atom_set_cdr(lisp, acons, anewcdr);
}

void pk_add(Pocket lisp, int lhs, int rhs) {
    PKAtom *alhs = pk_stack_get(lisp, lhs);
    PKAtom *arhs = pk_stack_get(lisp, rhs);
    pk_stack_push(lisp, pk_atom_add(lisp, alhs, arhs));
}

void pk_sub(Pocket lisp, int lhs, int rhs) {
    PKAtom *alhs = pk_stack_get(lisp, lhs);
    PKAtom *arhs = pk_stack_get(lisp, rhs);
    pk_stack_push(lisp, pk_atom_sub(lisp, alhs, arhs));
}

void pk_mul(Pocket lisp, int lhs, int rhs) {
    PKAtom *alhs = pk_stack_get(lisp, lhs);
    PKAtom *arhs = pk_stack_get(lisp, rhs);
    pk_stack_push(lisp, pk_atom_mul(lisp, alhs, arhs));
}

void pk_div(Pocket lisp, int lhs, int rhs) {
    PKAtom *alhs = pk_stack_get(lisp, lhs);
    PKAtom *arhs = pk_stack_get(lisp, rhs);
    pk_stack_push(lisp, pk_atom_div(lisp, alhs, arhs));
}

void pk_mod(Pocket lisp, int lhs, int rhs) {
    PKAtom *alhs = pk_stack_get(lisp, lhs);
    PKAtom *arhs = pk_stack_get(lisp, rhs);
    pk_stack_push(lisp, pk_atom_mod(lisp, alhs, arhs));
}

void pk_let(Pocket lisp, PK_ENV env, int symbol, int value) {
    PKAtom *asymbol = pk_stack_get(lisp, symbol);
    PKAtom *avalue = pk_stack_get(lisp, value);
    (void)pk_env_let(lisp, env, pk_atom_assert_symbol(lisp, asymbol), avalue);
}

void pk_set(Pocket lisp, PK_ENV env, int symbol, int value) {
    PKAtom *asymbol = pk_stack_get(lisp, symbol);
    PKAtom *avalue = pk_stack_get(lisp, value);
    (void)pk_env_set(lisp, env, pk_atom_assert_symbol(lisp, asymbol), avalue);
    pk_stack_push(lisp, avalue);
}

void pk_fetch(Pocket lisp, PK_ENV env, int symbol) {
    PKAtom *asymbol = pk_stack_get(lisp, symbol);
    pk_stack_push(lisp, pk_env_fetch_strict(lisp, env, pk_atom_assert_symbol(lisp, asymbol)));
}

bool pk_is_bound(Pocket lisp, PK_ENV env, int symbol) {
    PKAtom *asymbol = pk_stack_get(lisp, symbol);
    PKAtom *result = pk_env_fetch(lisp, env, pk_atom_assert_symbol(lisp, asymbol));
    if (pk_atom_is_undefined(result)) {
        return false;
    } else {
        return true;
    }
}

void pk_unbind(Pocket lisp, PK_ENV env, int symbol) {
    PKAtom *asymbol = pk_stack_get(lisp, symbol);
    (void)pk_env_set(lisp, env, pk_atom_assert_symbol(lisp, asymbol), lisp->cache.undefined);
}

void pk_list_funs(Pocket lisp) {
    pk_stack_push(lisp, pk_env_alist(lisp, PK_ENV_FUN));
}

void pk_list_vars(Pocket lisp) {
    pk_stack_push(lisp, pk_env_alist(lisp, PK_ENV_VAR));
}

void pk_list_symbols(Pocket lisp) {
    pk_stack_push(lisp, pk_atom_list_symbols(lisp));
}

void pk_print(Pocket lisp, int atom) {
    pk_atom_print(lisp, pk_stack_get(lisp, atom));
}

void pk_format(Pocket lisp, int atom) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_format(lisp, pk_stack_get(lisp, atom)));
}

void pk_hexdump(Pocket lisp, int atom) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_hexdump(lisp, pk_stack_get(lisp, atom)));
}

void pk_parse(Pocket lisp, int string, PK_MODE mode) {
    PKAtom *astring = pk_stack_get(lisp, string);
    PKAtom *result = pk_parse_atom_from_string(lisp, pk_atom_assert_string(lisp, astring), mode);
    pk_stack_push(lisp, result);
}

void pk_eval(Pocket lisp, int expr, PK_MODE mode) {
    pk_atom_eval(lisp, pk_stack_get(lisp, expr), mode);
}

void pk_apply(Pocket lisp, PK_CALL call, int fun, int args) {
    PKAtom *afun = pk_stack_get(lisp, fun);
    PKAtom *aargs = pk_stack_get(lisp, args);
    pk_atom_apply(lisp, call, afun, aargs);
}

void pk_funcall(Pocket lisp, PK_CALL call, size_t args) {
    (void)pk_int_funcall(lisp, call, args);
}

void pk_fastcall(Pocket lisp, void *user_closure, PKFun fun, size_t args) {
    (void)pk_int_fastcall(lisp, user_closure, fun, args, lisp->cache.nil);
}
    
PK_RES pk_catch(Pocket lisp, int symbol, void *user_closure, PKFun fun, size_t args) {
    PKAtomSymbol *asymbol = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, symbol));
    return pk_atom_catch(lisp, asymbol, user_closure, fun, args);
}

bool pk_macroexpand1(Pocket lisp, int atom) {
    return pk_macroexpand1_(lisp, atom);
}

bool pk_macroexpand(Pocket lisp, int atom) {
    return pk_macroexpand_(lisp, atom);
}

bool pk_eq(Pocket lisp, int lhs, int rhs) {
    return pk_atom_eq(lisp, pk_stack_get(lisp, lhs), pk_stack_get(lisp, rhs));
}

bool pk_struct_eq(Pocket lisp, int lhs, int rhs) {
    return pk_atom_object_eq(lisp, pk_stack_get(lisp, lhs), pk_stack_get(lisp, rhs));
}

bool pk_object_eq(Pocket lisp, int lhs, int rhs) {
    return pk_atom_struct_eq(lisp, pk_stack_get(lisp, lhs), pk_stack_get(lisp, rhs));
}

bool pk_lt(Pocket lisp, int lhs, int rhs) {
    return pk_atom_lt(lisp, pk_stack_get(lisp, lhs), pk_stack_get(lisp, rhs));
}

bool pk_lte(Pocket lisp, int lhs, int rhs) {
    return pk_atom_lte(lisp, pk_stack_get(lisp, lhs), pk_stack_get(lisp, rhs));
}

bool pk_gt(Pocket lisp, int lhs, int rhs) {
    return pk_atom_gt(lisp, pk_stack_get(lisp, lhs), pk_stack_get(lisp, rhs));
}

bool pk_gte(Pocket lisp, int lhs, int rhs) {
    return pk_atom_gte(lisp, pk_stack_get(lisp, lhs), pk_stack_get(lisp, rhs));
}

void pk_reverse(Pocket lisp, int seq) {
    pk_stack_push(lisp, pk_generic_reverse(lisp, pk_stack_get(lisp, seq)));
}

void pk_clone(Pocket lisp, int stack_pointer) {
    pk_stack_push(lisp, pk_atom_clone(lisp, pk_stack_get(lisp, stack_pointer)));
}

void pk_to_list(Pocket lisp, int seq) {
    PKAtom *aseq = pk_stack_get(lisp, seq);
    pk_stack_push(lisp, pk_generic_to_list(lisp, aseq));
}

bool pk_is_circular(Pocket lisp, int stack_pointer) {
    return pk_atom_is_circular(lisp, pk_stack_get(lisp, stack_pointer));
}

int pk_to_int(Pocket lisp, int number) {
    return pk_atom_to_int(lisp, pk_stack_get(lisp, number), PK_NUMBER_CONV_FLOAT_CONV);
}

float pk_to_float(Pocket lisp, int number) {
    return pk_atom_to_float(lisp, pk_stack_get(lisp, number), PK_NUMBER_CONV_FLOAT_CONV);
}

void pk_error(Pocket lisp, int message) {
    pk_error_atom(lisp, pk_stack_get(lisp, message));
}

void pk_push_throw_type(Pocket lisp) {
    if (lisp->throw_symbol == NULL) {
        pk_stack_push(lisp, lisp->cache.nil);
    } else {
        pk_stack_push(lisp, lisp->throw_symbol);
    }
}

void pk_push_throw_value(Pocket lisp) {
    if (lisp->throw_value == NULL) {
        pk_stack_push(lisp, lisp->cache.nil);
    } else {
        pk_stack_push(lisp, lisp->throw_value);
    }
}

void pk_assoc(Pocket lisp, int alist, int key) {
    PKAtom *aalist = pk_stack_get(lisp, alist);
    PKAtom *akey = pk_stack_get(lisp, key);
    pk_stack_push(lisp, pk_atom_assoc(lisp, aalist, akey));
}

void pk_append(Pocket lisp, int array, int new) {
    PKAtomArray *aarray = pk_atom_assert_array(lisp, pk_stack_get(lisp, array));
    PKAtom *anew = pk_stack_get(lisp, new);
    pk_atom_array_append(lisp, aarray, anew);
}

void pk_cat(Pocket lisp, int lhs, int rhs) {
    PKAtom *alhs = pk_stack_get(lisp, lhs);
    PKAtom *arhs = pk_stack_get(lisp, rhs);
    PKAtom *e[] = {alhs, arhs};
    PKAtoms hack = (PKAtoms) {
        .e = e,
        .length = 2,
    };
    pk_stack_push(lisp, (PKAtom *)pk_generic_cat(lisp, alhs, hack));
}

void pk_el(Pocket lisp, int seq, size_t index) {
    PKAtom *aseq = pk_stack_get(lisp, seq);
    pk_stack_push(lisp, pk_generic_el(lisp, aseq, index));
}

bool pk_is_element(Pocket lisp, int seq, int element) {
    PKAtom *aseq = pk_stack_get(lisp, seq);
    PKAtom *aelement = pk_stack_get(lisp, element);
    return pk_generic_is_element(lisp, aseq, aelement);
}

size_t pk_length(Pocket lisp, int obj) {
    PKAtom *aobj = pk_stack_get(lisp, obj);
    return pk_generic_length(lisp, aobj);
}

void pk_object(Pocket lisp, void *object_data, PKGCFun gc, int type, int properties) {
    (void)properties;
    PKAtomSymbol *atype = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, type));
    pk_stack_push(lisp, (PKAtom *)pk_atom_object(lisp, atype, object_data, gc));
}

void *pk_object_data(Pocket lisp, int object, int type) {
    PKAtomObject *aobject = pk_atom_assert_object(lisp, pk_stack_get(lisp, object));
    PKAtom *typecheck = pk_stack_get(lisp, type);
    return pk_atom_object_data(lisp, aobject, typecheck);
}

void pk_object_type(Pocket lisp, int object) {
    PKAtomObject *aobject = pk_atom_assert_object(lisp, pk_stack_get(lisp, object));
    pk_stack_push(lisp, (PKAtom *)aobject->type);
}

void pk_put(Pocket lisp, int object, int key, int value) {
    PKAtomObject *aobject = pk_atom_assert_object(lisp, pk_stack_get(lisp, object));
    PKAtom *akey = pk_stack_get(lisp, key);
    PKAtom *avalue = pk_stack_get(lisp, value);
    pk_stack_push(lisp, pk_atom_object_put(lisp, aobject, akey, avalue));
}

void pk_get(Pocket lisp, int object, int key) {
    PKAtomObject *aobject = pk_atom_assert_object(lisp, pk_stack_get(lisp, object));
    PKAtom *akey = pk_stack_get(lisp, key);
    pk_stack_push(lisp, pk_atom_object_get(lisp, aobject, akey));
}

void pk_remove(Pocket lisp, int object, int key) {
    PKAtomObject *aobject = pk_atom_assert_object(lisp, pk_stack_get(lisp, object));
    PKAtom *akey = pk_stack_get(lisp, key);
    pk_stack_push(lisp, pk_atom_object_remove(lisp, aobject, akey));
}

bool pk_is_property(Pocket lisp, int object, int key) {
    PKAtomObject *aobject = pk_atom_assert_object(lisp, pk_stack_get(lisp, object));
    PKAtom *akey = pk_stack_get(lisp, key);
    return pk_atom_object_is_property(lisp, aobject, akey);
}

void pk_properties(Pocket lisp, int stack_pointer) {
    PKAtomObject *aobject = pk_atom_assert_object(lisp, pk_stack_get(lisp, stack_pointer));
    pk_stack_push(lisp, pk_atom_object_to_plist(lisp, aobject));
}

PK_TYPE pk_typeof(Pocket lisp, int stack_pointer) {
    return pk_atom_typeof_user(pk_stack_get(lisp, stack_pointer));
}

void pk_map(Pocket lisp, int seq, int fun) {
    PKAtom *aseq = pk_stack_get(lisp, seq);
    PKAtom *afun = pk_stack_get(lisp, fun);
    pk_stack_push(lisp, pk_atom_list_map(lisp, aseq, afun));
}

void pk_filter(Pocket lisp, int seq, int fun) {
    PKAtom *aseq = pk_stack_get(lisp, seq);
    PKAtom *afun = pk_stack_get(lisp, fun);
    pk_stack_push(lisp, pk_atom_list_filter(lisp, aseq, afun));
}

void pk_fap(Pocket lisp, int seq, int fun) {
    PKAtom *aseq = pk_stack_get(lisp, seq);
    PKAtom *afun = pk_stack_get(lisp, fun);
    pk_stack_push(lisp, pk_atom_list_fap(lisp, aseq, afun));
}

void pk_reduce(Pocket lisp, int seq, int fun, int init) {
    PKAtom *aseq = pk_stack_get(lisp, seq);
    PKAtom *afun = pk_stack_get(lisp, fun);
    PKAtom *ainit = pk_stack_get(lisp, init);
    pk_stack_push(lisp, pk_atom_list_reduce(lisp, aseq, afun, ainit));
}
