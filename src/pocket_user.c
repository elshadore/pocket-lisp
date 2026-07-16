#include "pocket_internals.h"

PKRes pk_push_nil(Pocket lisp) {
    pk_try(pk_push(lisp, lisp->cache.nil));
    return PK_Ok;
}

PKRes pk_push_t(Pocket lisp) {
    pk_try(pk_push(lisp, (PKAtom *)lisp->cache.t));
    return PK_Ok;
}

PKRes pk_push_cond(Pocket lisp, bool cond) {
    if (cond) {
        pk_try(pk_push_t(lisp));
    } else {
        pk_try(pk_push_nil(lisp));
    }
    return PK_Ok;
}

PKRes pk_push_int(Pocket lisp, int integer) {
    PKAtomNumber *n;
    pk_try(pk_atom_int(lisp, integer, &n));
    pk_try(pk_push(lisp, (PKAtom *)n));
    return PK_Ok;
}

PKRes pk_push_float(Pocket lisp, float floater) {
    PKAtomNumber *n;
    pk_try(pk_atom_float(lisp, floater, &n));
    pk_try(pk_push(lisp, (PKAtom *)n));
    return PK_Ok;
}

PKRes pk_push_string(Pocket lisp, PKString string) {
    PKAtomString *s;
    pk_try(pk_atom_string(lisp, string, &s));
    pk_try(pk_push(lisp, (PKAtom *)s));
    return PK_Ok;
}

PKRes pk_push_cstr(Pocket lisp, char *cstr) {
    PKString s;
    pk_try(pk_string_from_cstr(cstr, &s));
    pk_try(pk_push_string(lisp, s));
    return PK_Ok;
}

PKRes pk_push_nstr(Pocket lisp, char *str, size_t length) {
    pk_try(pk_push_string(lisp, pk_string_new(str, length)));
    return PK_Ok;
}

PKRes pk_push_symbol(Pocket lisp, PKString symbol) {
    PKAtomSymbol *sym;
    pk_try(pk_atom_symbol_interned(lisp, symbol, &sym));
    pk_try(pk_push(lisp, (PKAtom *)sym));
    return PK_Ok;
}

PKRes pk_push_csym(Pocket lisp, char *cstr) {
    PKString s;
    pk_try(pk_string_from_cstr(cstr, &s));
    pk_try(pk_push_symbol(lisp, s));
    return PK_Ok;
}

PKRes pk_push_nsym(Pocket lisp, char *sym, size_t length) {
    pk_try(pk_push_symbol(lisp, pk_string_new(sym, length)));
    return PK_Ok;
}

PKRes pk_push_cons(Pocket lisp, int car, int cdr) {
    PKAtom *car_atom;
    pk_try(pk_stack_get(lisp, car, &car_atom));
    PKAtom *cdr_atom;
    pk_try(pk_stack_get(lisp, cdr, &cdr_atom));
    PKAtomCons *cons;
    pk_try(pk_atom_cons(lisp, car_atom, cdr_atom, &cons));
    pk_try(pk_push(lisp, (PKAtom *)cons));
    return PK_Ok;
}

PKRes pk_push_cfunc(Pocket lisp, void *user_closure, PKFn fn, int args, PKArity mode) {
    PKFuncArity arity = { .mode = mode, .args = args };
    PKAtomCFunc *cfunc;
    pk_try(pk_atom_cfunc(lisp, user_closure, fn, arity, &cfunc));
    pk_try(pk_push(lisp, (PKAtom *)cfunc));
    return PK_Ok;
}

PKRes pk_dupe(Pocket lisp, int stack_pointer) {
    PKAtom *atom;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    pk_try(pk_push(lisp, atom));
    return PK_Ok;
}

PKRes pk_insert(Pocket lisp, int from, int to) {
    size_t a, b;
    pk_try(pk_sp_index(lisp, from, &a));
    pk_try(pk_sp_index(lisp, to, &b));
    lisp->stack.e[b] = lisp->stack.e[a];
    return PK_Ok;
}

PKRes pk_swap(Pocket lisp, int a, int b) {
    size_t ia, ib;
    pk_try(pk_sp_index(lisp, a, &ia));
    pk_try(pk_sp_index(lisp, b, &ib));
    PKAtom *tmp = lisp->stack.e[ia];
    lisp->stack.e[ia] = lisp->stack.e[ib];
    lisp->stack.e[ib] = tmp;
    return PK_Ok;
}

PKRes pk_car(Pocket lisp, int cons) {
    PKAtom *c_atom;
    pk_try(pk_stack_get(lisp, cons, &c_atom));
    PKAtomCons *c;
    pk_try(pk_atom_cast_cons(lisp, c_atom, &c));
    pk_try(pk_push(lisp, c->car));
    return PK_Ok;
}

PKRes pk_cdr(Pocket lisp, int cons) {
    PKAtom *c_atom;
    pk_try(pk_stack_get(lisp, cons, &c_atom));
    PKAtomCons *c;
    pk_try(pk_atom_cast_cons(lisp, c_atom, &c));
    pk_try(pk_push(lisp, c->cdr));
    return PK_Ok;
}

PKRes pk_set_car(Pocket lisp, int cons, int new_car) {
    PKAtom *c_atom;
    pk_try(pk_stack_get(lisp, cons, &c_atom));
    PKAtomCons *c;
    pk_try(pk_atom_cast_cons(lisp, c_atom, &c));
    PKAtom *val;
    pk_try(pk_stack_get(lisp, new_car, &val));
    c->car = val;
    return PK_Ok;
}

PKRes pk_set_cdr(Pocket lisp, int cons, int new_cdr) {
    PKAtom *c_atom;
    pk_try(pk_stack_get(lisp, cons, &c_atom));
    PKAtomCons *c;
    pk_try(pk_atom_cast_cons(lisp, c_atom, &c));
    PKAtom *val;
    pk_try(pk_stack_get(lisp, new_cdr, &val));
    c->cdr = val;
    return PK_Ok;
}

PKRes pk_add(Pocket lisp, int lhs, int rhs) {
    PKAtom *a, *b;
    pk_try(pk_stack_get(lisp, lhs, &a));
    pk_try(pk_stack_get(lisp, rhs, &b));
    PKAtomNumber *na, *nb, *n;
    pk_try(pk_atom_cast_number(lisp, a, &na));
    pk_try(pk_atom_cast_number(lisp, b, &nb));
    pk_try(pk_number_add(lisp, na, nb, &n));
    pk_try(pk_push(lisp, (PKAtom *)n));
    return PK_Ok;
}

PKRes pk_sub(Pocket lisp, int lhs, int rhs) {
    PKAtom *a, *b;
    pk_try(pk_stack_get(lisp, lhs, &a));
    pk_try(pk_stack_get(lisp, rhs, &b));
    PKAtomNumber *na, *nb, *n;
    pk_try(pk_atom_cast_number(lisp, a, &na));
    pk_try(pk_atom_cast_number(lisp, b, &nb));
    pk_try(pk_number_sub(lisp, na, nb, &n));
    pk_try(pk_push(lisp, (PKAtom *)n));
    return PK_Ok;
}

PKRes pk_mul(Pocket lisp, int lhs, int rhs) {
    PKAtom *a, *b;
    pk_try(pk_stack_get(lisp, lhs, &a));
    pk_try(pk_stack_get(lisp, rhs, &b));
    PKAtomNumber *na, *nb, *n;
    pk_try(pk_atom_cast_number(lisp, a, &na));
    pk_try(pk_atom_cast_number(lisp, b, &nb));
    pk_try(pk_number_mul(lisp, na, nb, &n));
    pk_try(pk_push(lisp, (PKAtom *)n));
    return PK_Ok;
}

PKRes pk_div(Pocket lisp, int lhs, int rhs) {
    PKAtom *a, *b;
    pk_try(pk_stack_get(lisp, lhs, &a));
    pk_try(pk_stack_get(lisp, rhs, &b));
    PKAtomNumber *na, *nb, *n;
    pk_try(pk_atom_cast_number(lisp, a, &na));
    pk_try(pk_atom_cast_number(lisp, b, &nb));
    pk_try(pk_number_div(lisp, na, nb, &n));
    pk_try(pk_push(lisp, (PKAtom *)n));
    return PK_Ok;
}

PKRes pk_gt(Pocket lisp, int lhs, int rhs, bool *output) {
    PKAtom *a, *b;
    pk_try(pk_stack_get(lisp, lhs, &a));
    pk_try(pk_stack_get(lisp, rhs, &b));
    PKAtomNumber *na, *nb;
    pk_try(pk_atom_cast_number(lisp, a, &na));
    pk_try(pk_atom_cast_number(lisp, b, &nb));
    pk_try(pk_number_gt(lisp, na, nb, output));
    return PK_Ok;
}

PKRes pk_gte(Pocket lisp, int lhs, int rhs, bool *output) {
    PKAtom *a, *b;
    pk_try(pk_stack_get(lisp, lhs, &a));
    pk_try(pk_stack_get(lisp, rhs, &b));
    PKAtomNumber *na, *nb;
    pk_try(pk_atom_cast_number(lisp, a, &na));
    pk_try(pk_atom_cast_number(lisp, b, &nb));
    pk_try(pk_number_gte(lisp, na, nb, output));
    return PK_Ok;
}

PKRes pk_lt(Pocket lisp, int lhs, int rhs, bool *output) {
    PKAtom *a, *b;
    pk_try(pk_stack_get(lisp, lhs, &a));
    pk_try(pk_stack_get(lisp, rhs, &b));
    PKAtomNumber *na, *nb;
    pk_try(pk_atom_cast_number(lisp, a, &na));
    pk_try(pk_atom_cast_number(lisp, b, &nb));
    pk_try(pk_number_lt(lisp, na, nb, output));
    return PK_Ok;
}

PKRes pk_lte(Pocket lisp, int lhs, int rhs, bool *output) {
    PKAtom *a, *b;
    pk_try(pk_stack_get(lisp, lhs, &a));
    pk_try(pk_stack_get(lisp, rhs, &b));
    PKAtomNumber *na, *nb;
    pk_try(pk_atom_cast_number(lisp, a, &na));
    pk_try(pk_atom_cast_number(lisp, b, &nb));
    pk_try(pk_number_lte(lisp, na, nb, output));
    return PK_Ok;
}

PKRes pk_eq(Pocket lisp, int lhs, int rhs, bool *output) {
    PKAtom *a, *b;
    pk_try(pk_stack_get(lisp, lhs, &a));
    pk_try(pk_stack_get(lisp, rhs, &b));
    PKAtomNumber *na, *nb;
    pk_try(pk_atom_cast_number(lisp, a, &na));
    pk_try(pk_atom_cast_number(lisp, b, &nb));
    pk_try(pk_number_eq(lisp, na, nb, output));
    return PK_Ok;
}

PKRes pk_to_int(Pocket lisp, int stack_pointer, int *output) {
    PKAtom *atom;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    PKAtomNumber *num;
    pk_try(pk_atom_cast_number(lisp, atom, &num));
    *output = pk_number_to_int(num);
    return PK_Ok;
}

PKRes pk_to_float(Pocket lisp, int stack_pointer, float *output) {
    PKAtom *atom;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    PKAtomNumber *num;
    pk_try(pk_atom_cast_number(lisp, atom, &num));
    *output = pk_number_to_float(num);
    return PK_Ok;
}

PKRes pk_to_string(Pocket lisp, int stack_pointer, PKString *output) {
    PKAtom *atom;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    PKAtomString *s;
    pk_try(pk_atom_cast_string(lisp, atom, &s));
    *output = s->lit;
    return PK_Ok;
}

PKRes pk_read(Pocket lisp, int stack_pointer) {
    PKAtom *atom;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    PKAtomString *s;
    pk_try(pk_atom_cast_string(lisp, atom, &s));
    pk_try(pk_read_string(lisp, s->lit));
    return PK_Ok;
}

PKRes pk_read_cstr(Pocket lisp, char *cstr) {
    PKString string;
    pk_try(pk_string_from_cstr(cstr, &string));
    pk_try(pk_read_string(lisp, string));
    return PK_Ok;
}

PKRes pk_read_nstr(Pocket lisp, char *string, size_t length) {
    pk_try(pk_read_string(lisp, pk_string_new(string, length)));
    return PK_Ok;
}

PKRes pk_format(Pocket lisp, int stack_pointer) {
    PKAtom *atom;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    PKWriter w = pk_writer_init(lisp);
    PKRes res = pk_writer_atom(&w, atom);
    if (res == PK_Ok) {
        PKAtomString *string;
        res = pk_atom_string(lisp, pk_string_new(w.c, w.count), &string);
        if (res == PK_Ok) {
            res = pk_push(lisp, (PKAtom *)string);
        }
    }
    pk_writer_deinit(&w);
    return res;
}

PKRes pk_list(Pocket lisp, int head, int tail) {
    PKStackSlice slice;
    pk_try(pk_stack_slice_by(lisp, head, tail, &slice));
    PKAtom *result = NULL;
    switch (slice.order) {
        case PKOrder_Normal: {
            pk_try(pk_slice_list(lisp, slice.slice, &result));
            break;
        }
        case PKOrder_Reversed: {
            pk_try(pk_slice_list_rev(lisp, slice.slice, &result));
            break;
        }
    }
    pk_try(pk_push(lisp, result));
    return PK_Ok;
}

PKRes pk_clone(Pocket lisp, int stack_pointer) {
    return pk_quickcaller(lisp, stack_pointer, PKEvalMode_Clone);
}
