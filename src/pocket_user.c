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

void pk_push_symbol(Pocket lisp, PKStr symbol) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, symbol));
}

void pk_push_symbolc(Pocket lisp, char *csymbol) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_symbolc(lisp, csymbol));
}

void pk_push_gensym(Pocket lisp) {
    pk_error(lisp);
}

void pk_push_keyword(Pocket lisp, PKStr keyword) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_keyword(lisp, keyword));
}

void pk_push_keywordc(Pocket lisp, char *ckeyword) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_keywordc(lisp, ckeyword));
}

void pk_push_cons(Pocket lisp, int car, int cdr) {
    PKAtom *acar = pk_stack_get(lisp, car);
    PKAtom *acdr = pk_stack_get(lisp, cdr);
    pk_stack_push(lisp, (PKAtom *)pk_atom_cons(lisp, acar, acdr));
}

void pk_push_cfun(Pocket lisp, void *user_closure, PKFun fun, pk_u8 args, PK_ARITY arity, pk_u8 options) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_cfun(lisp, user_closure, fun, PK_CALL_FUN, arity, args, options));
}

void pk_push_cmacro(Pocket lisp, void *user_closure, PKFun fun, pk_u8 args, PK_ARITY arity, pk_u8 options) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_cfun(lisp, user_closure, fun, PK_CALL_MACRO, arity, args, options));
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
    pk_stack_push(pk_env_set(lisp, env, pk_atom_assert_symbol(lisp, asymbol), avalue));
}

void pk_get(Pocket lisp, PK_ENV env, int symbol) {
    PKAtom *asymbol = pk_stack_get(lisp, symbol);
    pk_stack_push(pk_env_get(lisp, env, pk_atom_assert_symbol(lisp, asymbol)));
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
    pk_env_set(lisp, env, pk_atom_assert_symbol(lisp, asymbol), lisp->cache.undefined);
}

// void pk_env_list(Pocket lisp, PK_ENV env);

void pk_print(Pocket lisp, int atom) {
    pk_atom_print(lisp, pk_stack_get(lisp, atom));
}

void pk_write(Pocket lisp, int atom) {
    pk_atom_write(lisp, pk_stack_get(lisp, atom));
}

void pk_format(Pocket lisp, int atom) {
    pk_stack_push(lisp, (PKAtom *)pk_atom_format(lisp, pk_stack_get(lisp, atom)));
}

void pk_read(Pocket lisp, int string, PK_MODE mode) {
    PKAtom *astring = pk_stack_get(lisp, string);
    PKAtom *result = pk_read_atom_from_string(lisp, pk_atom_assert_string(lisp, astring), mode);
    pk_stack_push(lisp, result);
}

void pk_eval(Pocket lisp, int expr, PK_MODE mode);
void pk_apply(Pocket lisp, PK_CALL call, int fun, int args);
void pk_fcall(Pocket lisp, PK_CALL call, size_t args);
void pk_pcall(Pocket lisp, PK_CALL call, size_t args);
void pk_compile(Pocket lisp, PK_CALL type, int value, PK_MODE mode);
void pk_macroexpand1(Pocket lisp, int atom);
void pk_macroexpand(Pocket lisp, int atom);
