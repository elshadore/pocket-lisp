#include "pocket_internals.h"

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

void pk_push_stringf(Pocket lisp, const char *fmt, ...) {
    PKWriter w = PK_WRITER_EMPTY;
    va_list args;

    va_start(args, fmt);
    pk_writer_vfmt(lisp, &w, fmt, args);
    va_end(args);

    pk_push_string(lisp, pk_writer_slice(&w));
    pk_writer_deinit(lisp, &w);
}

void pk_push_symbol(Pocket lisp, PKStr symbol) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, symbol));
}

void pk_push_symbolc(Pocket lisp, char *csymbol) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbolc(lisp, csymbol));
}

void pk_push_symbolf(Pocket lisp, const char *fmt, ...) {
    PKWriter w = PK_WRITER_EMPTY;
    va_list args;

    va_start(args, fmt);
    pk_writer_vfmt(lisp, &w, fmt, args);
    va_end(args);

    pk_push_symbol(lisp, pk_writer_slice(&w));
    pk_writer_deinit(lisp, &w);
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

void pk_push_keywordf(Pocket lisp, const char *fmt, ...) {
    PKWriter w = PK_WRITER_EMPTY;
    va_list args;

    va_start(args, fmt);
    pk_writer_vfmt(lisp, &w, fmt, args);
    va_end(args);

    pk_push_keyword(lisp, pk_writer_slice(&w));
    pk_writer_deinit(lisp, &w);
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

void pk_push_cfun(Pocket lisp, void *user_closure, PKFun fun, pk_u8 args, PK_ARITY arity, pk_u8 options) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_cfun(lisp, user_closure, fun, PK_CALL_FUN, arity, args, options));
}

void pk_push_cmacro(Pocket lisp, void *user_closure, PKFun fun, pk_u8 args, PK_ARITY arity, pk_u8 options) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_cfun(lisp, user_closure, fun, PK_CALL_MACRO, arity, args, options));
}

void pk_push_array(Pocket lisp) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_array(lisp, PK_ATOMS_EMPTY));
}

void pk_push_string_array(Pocket lisp) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_string_array(lisp, PK_STR_EMPTY));
}

void pk_push_byte_array(Pocket lisp) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_byte_array(lisp, PK_BLOB_EMPTY));
}

void pk_push_table(Pocket lisp) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_table(lisp));
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

void pk_get(Pocket lisp, PK_ENV env, int symbol) {
    PKAtom *asymbol = pk_stack_get(lisp, symbol);
    pk_stack_push(lisp, pk_env_get_strict(lisp, env, pk_atom_assert_symbol(lisp, asymbol)));
}

bool pk_is_bound(Pocket lisp, PK_ENV env, int symbol) {
    PKAtom *asymbol = pk_stack_get(lisp, symbol);
    PKAtom *result = pk_env_get(lisp, env, pk_atom_assert_symbol(lisp, asymbol));
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

void pk_env_list(Pocket lisp, PK_ENV env) {
    pk_stack_push(lisp, pk_env_alist(lisp, env));
}

void pk_print(Pocket lisp, int atom) {
    pk_atom_print(lisp, pk_stack_get(lisp, atom));
}

void pk_write(Pocket lisp, int atom) {
    pk_atom_write(lisp, pk_stack_get(lisp, atom));
}

void pk_format(Pocket lisp, int atom) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_format(lisp, pk_stack_get(lisp, atom)));
}

void pk_hexdump(Pocket lisp, int atom) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_hexdump(lisp, pk_stack_get(lisp, atom)));
}

void pk_read(Pocket lisp, int string, PK_MODE mode) {
    PKAtom *astring = pk_stack_get(lisp, string);
    PKAtom *result = pk_read_atom_from_string(lisp, pk_atom_assert_string(lisp, astring), mode);
    pk_stack_push(lisp, result);
}

PK_RES pk_eval(Pocket lisp, int expr, PK_MODE mode) {
    return pk_atom_eval(lisp, pk_stack_get(lisp, expr), mode);
}

PK_RES pk_apply(Pocket lisp, PK_CALL call, int fun, int args) {
    PKAtom *afun = pk_stack_get(lisp, fun);
    PKAtom *aargs = pk_stack_get(lisp, args);
    return pk_atom_apply(lisp, call, afun, aargs);
}

void pk_compile(Pocket lisp, PK_CALL type, int value, PK_MODE mode) {
    pk_stack_push(lisp, pk_atom_compile(lisp, type, pk_stack_get(lisp, value), mode));
}

bool pk_macroexpand1(Pocket lisp, int atom) {
    return pk_macroexpand1_(lisp, atom);
}

bool pk_macroexpand(Pocket lisp, int atom) {
    return pk_macroexpand_(lisp, atom);
}

void pk_get_raw(Pocket lisp, PK_ENV env, int symbol) {
    pk_stack_push(lisp, pk_env_get(lisp, env, pk_atom_assert_symbol(lisp, pk_stack_get(lisp, symbol))));
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
    pk_stack_push(lisp, pk_atom_reverse(lisp, pk_stack_get(lisp, seq)));
}

void pk_el(Pocket lisp, size_t index) {
    pk_stack_push(lisp, pk_atom_el(lisp, pk_stack_head(lisp), index));
}

int pk_to_int(Pocket lisp, int number) {
    return pk_atom_to_int(lisp, pk_stack_get(lisp, number), PK_NUMBER_CONV_FLOAT_CONV);
}

float pk_to_float(Pocket lisp, int number) {
    return pk_atom_to_float(lisp, pk_stack_get(lisp, number), PK_NUMBER_CONV_FLOAT_CONV);
}

bool pk_is_type(Pocket lisp, int atom, PK_TYPE check) {
    return pk_atom_is_type(pk_stack_get(lisp, atom), check);
}

void pk_assert_type(Pocket lisp, int atom, PK_TYPE check) {
    if (!pk_is_type(lisp, atom, check)) {
        pk_error_type(lisp, check, pk_stack_get(lisp, atom));
    }
}

void pk_define_type_c_owned(Pocket lisp, int symbol, size_t size) {
    pk_atom_define_type_c_owned(lisp, pk_atom_assert_symbol(lisp, pk_stack_get(lisp, symbol)),  size);
}

void pk_define_type_c_user(Pocket lisp, int symbol) {
    pk_atom_define_type_c_user(lisp, pk_atom_assert_symbol(lisp, pk_stack_get(lisp, symbol)));
}

void pk_define_type_lisp(Pocket lisp, int symbol, int slots) {
    PKAtomSymbol *asymbol = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, symbol));
    PKAtom *aslots = pk_stack_get(lisp, slots);
    pk_atom_define_type_lisp_atom(lisp, asymbol, aslots);
}

void pk_slot_set(Pocket lisp, int object, int slot, int value) {
    PKAtomLObject *aobject = pk_atom_assert_lobject(lisp, pk_stack_get(lisp, object));
    PKAtomSymbol *aslot = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, slot));
    PKAtom *avalue = pk_stack_get(lisp, value);
    pk_atom_slot_set(lisp, aobject, aslot, avalue);
}

void pk_slot_get(Pocket lisp, int object, int slot) {
    PKAtomLObject *aobject = pk_atom_assert_lobject(lisp, pk_stack_get(lisp, object));
    PKAtomSymbol *aslot = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, slot));
    pk_stack_push(lisp, pk_atom_slot_get(lisp, aobject, aslot));
}

void pk_method_set(Pocket lisp, int type, int symbol, int method) {
    PKAtomSymbol *atype = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, type));
    PKAtomSymbol *asymbol = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, symbol));
    PKAtom *amethod = pk_stack_get(lisp, method);
    pk_stack_push(lisp, pk_atom_method_set(lisp, atype, asymbol, amethod));
}

void pk_method_get(Pocket lisp, int type, int symbol) {
    PKAtomSymbol *atype = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, type));
    PKAtomSymbol *asymbol = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, symbol));
    pk_stack_push(lisp, pk_atom_method_get(lisp, atype, asymbol));
}

void pk_method_get_raw(Pocket lisp, int type, int symbol) {
    PKAtomSymbol *atype = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, type));
    PKAtomSymbol *asymbol = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, symbol));
    pk_stack_push(lisp, pk_atom_method_get_raw(lisp, atype, asymbol));
}

void *pk_cobject_data(Pocket lisp, int cobject) {
    return pk_atom_cobject_data(lisp, pk_atom_assert_cobject(lisp, pk_stack_get(lisp, cobject)));
}

size_t pk_cobject_size(Pocket lisp, int cobject) {
    return pk_atom_cobject_size(lisp, pk_atom_assert_cobject(lisp, pk_stack_get(lisp, cobject)));
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

PK_API void pk_cat(Pocket lisp, int lhs, int rhs) {
    // TODO: support list cat and other catting
    
    PKAtom *alhs = pk_stack_get(lisp, lhs);
    PKAtom *arhs = pk_stack_get(lisp, rhs);
    PKAtom *e[] = {alhs, arhs};
    PKAtoms hack = (PKAtoms) {
        .e = e,
        .length = 2,
    };
    pk_stack_push(lisp, (PKAtom *)pk_atom_string_cat(lisp, hack));
}

PK_API void pk_append(Pocket lisp, int seq, int element) {
    PKAtom *aseq = pk_stack_get(lisp, seq);
    PKAtom *aelement = pk_stack_get(lisp, element);
    pk_atom_append(lisp, aseq, aelement);
}


PK_API void pk_put(Pocket lisp, int table, int key, int value) {
    PKAtomTable *atable = pk_atom_assert_table(lisp, pk_stack_get(lisp, table));
    PKAtom *akey = pk_stack_get(lisp, key);
    PKAtom *avalue = pk_stack_get(lisp, value);
    pk_stack_push(lisp, pk_atom_table_put(lisp, atable, akey, avalue));
}

PK_API void pk_put_unique(Pocket lisp, int table, int key, int value) {
    PKAtomTable *atable = pk_atom_assert_table(lisp, pk_stack_get(lisp, table));
    PKAtom *akey = pk_stack_get(lisp, key);
    PKAtom *avalue = pk_stack_get(lisp, value);
    pk_stack_push(lisp, pk_atom_table_put_unique(lisp, atable, akey, avalue));
}

PK_API void pk_rem(Pocket lisp, int table, int key) {
    PKAtomTable *atable = pk_atom_assert_table(lisp, pk_stack_get(lisp, table));
    PKAtom *akey = pk_stack_get(lisp, key);
    pk_stack_push(lisp, pk_atom_table_rem(lisp, atable, akey));
}

PK_API void pk_rem_strict(Pocket lisp, int table, int key) {
    PKAtomTable *atable = pk_atom_assert_table(lisp, pk_stack_get(lisp, table));
    PKAtom *akey = pk_stack_get(lisp, key);
    pk_stack_push(lisp, pk_atom_table_rem_strict(lisp, atable, akey));
}

PK_API void pk_find(Pocket lisp, int table, int key) {
    PKAtomTable *atable = pk_atom_assert_table(lisp, pk_stack_get(lisp, table));
    PKAtom *akey = pk_stack_get(lisp, key);
    pk_stack_push(lisp, pk_atom_table_find(lisp, atable, akey));
}

PK_API void pk_find_raw(Pocket lisp, int table, int key) {
    PKAtomTable *atable = pk_atom_assert_table(lisp, pk_stack_get(lisp, table));
    PKAtom *akey = pk_stack_get(lisp, key);
    pk_stack_push(lisp, pk_atom_table_find_raw(lisp, atable, akey));
}

PK_API void pk_find_strict(Pocket lisp, int table, int key) {
    PKAtomTable *atable = pk_atom_assert_table(lisp, pk_stack_get(lisp, table));
    PKAtom *akey = pk_stack_get(lisp, key);
    pk_stack_push(lisp, pk_atom_table_find_strict(lisp, atable, akey));
}

PK_API bool pk_is_member(Pocket lisp, int table, int key) {
    PKAtomTable *atable = pk_atom_assert_table(lisp, pk_stack_get(lisp, table));
    PKAtom *akey = pk_stack_get(lisp, key);
    return pk_atom_table_is_member(lisp, atable, akey);
}

PK_API void pk_typeof(Pocket lisp, int stack_pointer) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_typeof(lisp, pk_stack_get(lisp, stack_pointer)));
}
