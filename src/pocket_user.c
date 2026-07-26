#include "pocket_internals.h"

PK_RES pk_push_nil(Pocket lisp) {
    pk_try(pk_push(lisp, lisp->cache.nil));
    return PK_OK;
}

PK_RES pk_push_t(Pocket lisp) {
    pk_try(pk_push(lisp, (PKAtom *)lisp->cache.t));
    return PK_OK;
}

PK_RES pk_push_cond(Pocket lisp, int cond) {
    if (cond) {
        pk_try(pk_push_t(lisp));
    } else {
        pk_try(pk_push_nil(lisp));
    }
    return PK_OK;
}

PK_RES pk_push_int(Pocket lisp, int integer) {
    PKAtomNumber *n = NULL;
    pk_try(pk_atom_int(lisp, integer, &n));
    pk_try(pk_push(lisp, (PKAtom *)n));
    return PK_OK;
}

PK_RES pk_push_float(Pocket lisp, float floater) {
    PKAtomNumber *n = NULL;
    pk_try(pk_atom_float(lisp, floater, &n));
    pk_try(pk_push(lisp, (PKAtom *)n));
    return PK_OK;
}

PK_RES pk_push_string(Pocket lisp, const char *cstr) {
    PKAtomString *a = NULL;
    pk_try(pk_atom_string(lisp, cstr, &a));
    pk_try(pk_push(lisp, (PKAtom *)a));
    return PK_OK;
}

PK_RES pk_push_stringn(Pocket lisp, const char *string, size_t length) {
    PKAtomString *a = NULL;
    
    pk_try(pk_atom_stringn(lisp, string, length, &a));
    pk_try(pk_push(lisp, (PKAtom *)a));
    return PK_OK;
}

PK_RES pk_push_symbol(Pocket lisp, const char *csym) {
    PKAtomSymbol *a = NULL;
    pk_try(pk_atom_symbol_interned(lisp, csym, &a));
    pk_try(pk_push(lisp, (PKAtom *)a));
    return PK_OK;
}

PK_RES pk_push_symboln(Pocket lisp, const char *symbol, size_t length) {
    PKAtomSymbol *a = NULL;
    pk_try(pk_atom_symboln_interned(lisp, symbol, length, &a));
    pk_try(pk_push(lisp, (PKAtom *)symbol));
    return PK_OK;
}

PK_RES pk_push_keyword(Pocket lisp, const char *csym) {
    PKAtomKeyword *a = NULL;
    pk_try(pk_atom_keyword(lisp, csym, &a));
    pk_try(pk_push(lisp, (PKAtom *)a));
    return PK_OK;
}

PK_RES pk_push_keywordn(Pocket lisp, const char *keyword, size_t length) {
    PKAtomKeyword *a = NULL;
    pk_try(pk_atom_keywordn(lisp, keyword, length, &a));
    pk_try(pk_push(lisp, (PKAtom *)a));
    return PK_OK;
}

PK_RES pk_push_cons(Pocket lisp, int car, int cdr) {
    PKAtom *car_atom = NULL;
    PKAtom *cdr_atom = NULL;
    PKAtomCons *cons = NULL;
    
    pk_try(pk_stack_get(lisp, car, &car_atom));
    pk_try(pk_stack_get(lisp, cdr, &cdr_atom));
    pk_try(pk_atom_cons(lisp, car_atom, cdr_atom, &cons));
    pk_try(pk_push(lisp, (PKAtom *)cons));
    return PK_OK;
}

PK_RES pk_push_cfunc(Pocket lisp, void *user_closure, PKFn fn, int args, PK_ARITY mode) {
    PKFuncArity arity;
    PKAtomCFunc *cfunc = NULL;
    
    arity.mode = mode;
    arity.args = args;

    pk_try(pk_atom_cfunc(lisp, user_closure, fn, arity, &cfunc));
    pk_try(pk_push(lisp, (PKAtom *)cfunc));
    return PK_OK;
}

PK_RES pk_dupe(Pocket lisp, int stack_pointer) {
    PKAtom *atom = NULL;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    pk_try(pk_push(lisp, atom));
    return PK_OK;
}

PK_RES pk_insert(Pocket lisp, int from, int to) {
    size_t a = 0;
    size_t b = 0;
    
    pk_try(pk_sp_index(lisp, from, &a));
    pk_try(pk_sp_index(lisp, to, &b));
    lisp->stack.e[b] = lisp->stack.e[a];
    return PK_OK;
}

PK_RES pk_swap(Pocket lisp, int a, int b) {
    size_t ia = 0;
    size_t ib = 0;
    PKAtom *tmp = NULL;
    
    pk_try(pk_sp_index(lisp, a, &ia));
    pk_try(pk_sp_index(lisp, b, &ib));
    
    tmp = lisp->stack.e[ia];
    lisp->stack.e[ia] = lisp->stack.e[ib];
    lisp->stack.e[ib] = tmp;
    return PK_OK;
}

PK_RES pk_car(Pocket lisp, int cons) {
    PKAtom *c_atom = NULL;
    PKAtomCons *c = NULL;
    
    pk_try(pk_stack_get(lisp, cons, &c_atom));
    pk_try(pk_atom_cast_cons(lisp, c_atom, &c));
    pk_try(pk_push(lisp, c->car));
    return PK_OK;
}

PK_RES pk_cdr(Pocket lisp, int cons) {
    PKAtom *c_atom = NULL;
    PKAtomCons *c = NULL;
    
    pk_try(pk_stack_get(lisp, cons, &c_atom));
    pk_try(pk_atom_cast_cons(lisp, c_atom, &c));
    pk_try(pk_push(lisp, c->cdr));
    return PK_OK;
}

PK_RES pk_set_car(Pocket lisp, int cons, int new_car) {
    PKAtom *c_atom = NULL;
    PKAtomCons *c = NULL;
    PKAtom *val = NULL;
    
    pk_try(pk_stack_get(lisp, cons, &c_atom));
    pk_try(pk_atom_cast_cons(lisp, c_atom, &c));
    pk_try(pk_stack_get(lisp, new_car, &val));
    c->car = val;
    return PK_OK;
}

PK_RES pk_set_cdr(Pocket lisp, int cons, int new_cdr) {
    PKAtom *c_atom = NULL;
    PKAtomCons *c = NULL;
    PKAtom *val = NULL;
    
    pk_try(pk_stack_get(lisp, cons, &c_atom));
    pk_try(pk_atom_cast_cons(lisp, c_atom, &c));
    pk_try(pk_stack_get(lisp, new_cdr, &val));
    c->cdr = val;
    return PK_OK;
}

#define PK_MATH_OP_TEMPLATE(op_) \
    PKAtom *a = NULL; \
    PKAtom *b = NULL; \
    PKAtomNumber *na = NULL; \
    PKAtomNumber *nb = NULL; \
    PKAtomNumber *n = NULL; \
    pk_try(pk_stack_get(lisp, lhs, &a)); \
    pk_try(pk_stack_get(lisp, rhs, &b)); \
    pk_try(pk_atom_cast_number(lisp, a, &na)); \
    pk_try(pk_atom_cast_number(lisp, b, &nb)); \
    pk_try(op_(lisp, na, nb, &n)); \
    pk_try(pk_push(lisp, (PKAtom *)n)); \
    return PK_OK;

PK_RES pk_add(Pocket lisp, int lhs, int rhs) {
    PK_MATH_OP_TEMPLATE(pk_number_add)
}

PK_RES pk_sub(Pocket lisp, int lhs, int rhs) {
    PK_MATH_OP_TEMPLATE(pk_number_sub)
}

PK_RES pk_mul(Pocket lisp, int lhs, int rhs) {
    PK_MATH_OP_TEMPLATE(pk_number_mul)
}

PK_RES pk_div(Pocket lisp, int lhs, int rhs) {
    PK_MATH_OP_TEMPLATE(pk_number_div)
}

#define PK_MATH_LOGIC_OP_TEMPLATE(op_) \
    PKAtom *a = NULL; \
    PKAtom *b = NULL; \
    PKAtomNumber *na = NULL; \
    PKAtomNumber *nb = NULL; \
    pk_try(pk_stack_get(lisp, lhs, &a)); \
    pk_try(pk_stack_get(lisp, rhs, &b)); \
    pk_try(pk_atom_cast_number(lisp, a, &na)); \
    pk_try(pk_atom_cast_number(lisp, b, &nb)); \
    pk_try(op_(lisp, na, nb, (pk_bool *)output)); \
    return PK_OK;

PK_RES pk_gt(Pocket lisp, int lhs, int rhs, int *output) {
    PK_MATH_LOGIC_OP_TEMPLATE(pk_number_gt)
}

PK_RES pk_gte(Pocket lisp, int lhs, int rhs, int *output) {
    PK_MATH_LOGIC_OP_TEMPLATE(pk_number_gte)
}

PK_RES pk_lt(Pocket lisp, int lhs, int rhs, int *output) {
    PK_MATH_LOGIC_OP_TEMPLATE(pk_number_lt)
}

PK_RES pk_lte(Pocket lisp, int lhs, int rhs, int *output) {
    PK_MATH_LOGIC_OP_TEMPLATE(pk_number_lte)
}

PK_RES pk_eq(Pocket lisp, int lhs, int rhs, int *output) {
    PK_MATH_LOGIC_OP_TEMPLATE(pk_number_eq)
}

PK_RES pk_to_int(Pocket lisp, int stack_pointer, int *output) {
    PKAtom *atom = NULL;
    PKAtomNumber *num = NULL;
    
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    pk_try(pk_atom_cast_number(lisp, atom, &num));
    *output = pk_number_to_int(num);
    return PK_OK;
}

PK_RES pk_to_float(Pocket lisp, int stack_pointer, float *output) {
    PKAtom *atom = NULL;
    PKAtomNumber *num = NULL;
    
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    pk_try(pk_atom_cast_number(lisp, atom, &num));
    *output = pk_number_to_float(num);
    return PK_OK;
}

PK_RES pk_to_string(Pocket lisp, int stack_pointer, char **out_str, size_t *out_length) {
    PKAtom *atom;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    switch (atom->tag.ty) {
        case PKAtomTy_String: {
            *out_str = atom->string.c;
            *out_length = atom->string.length;
            break;
        }
        case PKAtomTy_Symbol: {
            *out_str = atom->symbol.id->c;
            *out_length = atom->symbol.id->length;
            break;
        }
        default: {
            return pk_error(lisp);
        }
    }
    return PK_OK;
}

PK_RES pk_read(Pocket lisp, int stack_pointer, PK_READ mode) {
    PKAtom *atom = NULL;
    PKAtom *result = NULL;
    PKAtomString *s = NULL;
    
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    pk_try(pk_atom_cast_string(lisp, atom, &s));
    pk_try(pk_read_string(lisp, s->c, s->length, mode, &result));
    pk_try(pk_push(lisp, result));
    return PK_OK;
}

PK_RES pk_slurp(Pocket lisp, int file_path) {
    PKAtom *atom = NULL;
    PKAtomString *result = NULL;
    PKAtomString *spath = NULL;
    
    pk_try(pk_stack_get(lisp, file_path, &atom));
    pk_try(pk_atom_cast_string(lisp, atom, &spath));
    pk_try(pk_atom_string_slurp(lisp, spath, &result));
    pk_try(pk_push(lisp, (PKAtom *)result));

    return PK_OK;
}

PK_RES pk_format(Pocket lisp, int stack_pointer) {
    PKAtom *atom = NULL;
    PKWriter w = pk_writer_init(lisp);
    PK_RES res = PK_YIELD;
    
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    res = pk_writer_atom(&w, atom);
    if (res == PK_OK) {
        PKAtomString *string = NULL;
        res = pk_atom_stringn(lisp, w.c, w.count, &string);
        if (res == PK_OK) {
            res = pk_push(lisp, (PKAtom *)string);
        }
    }
    pk_writer_deinit(&w);
    return res;
}

PK_RES pk_list(Pocket lisp, int head, int tail) {
    PKStackSlice slice;
    PKAtom *result = NULL;
    
    pk_try(pk_stack_slice_by(lisp, head, tail, &slice));
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
    return PK_OK;
}

PK_RES pk_compile(Pocket lisp, int stack_pointer) {
    PKAtom *atom = NULL;
    PKAtomLFunc *result = NULL;
    
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    pk_try(pk_compile_atom(lisp, atom, &result));
    pk_try(pk_push(lisp, (PKAtom *)result));
    return PK_OK;
}

PK_RES pk_eval(Pocket lisp, int stack_pointer) {
    PKAtom *atom = NULL;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    pk_try(pk_atom_eval(lisp, atom));
    return PK_OK;
}

PK_RES pk_evlist(Pocket lisp, int stack_pointer) {
    PKAtom *atom = NULL;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    pk_try(pk_atom_evlist(lisp, atom));
    return PK_OK;
}

PK_RES pk_fastcall(void *user_closure, Pocket lisp, PKFn fn, int arity) {
    PKCallConv call;
    size_t carity = 0;
    
    pk_try(pk_arity_convert(lisp, arity, &carity));
    pk_callconv_quick(user_closure, fn, carity, &call);

    pk_try(pk_call(lisp, &call));

    return PK_OK;
}

PK_RES pk_funcall(Pocket lisp, int arity) {
    PKCallConv call;
    PKAtom *atom = NULL;
    size_t carity;
    
    pk_try(pk_arity_convert(lisp, arity, &carity));
    pk_try(pk_stack_get(lisp, (-arity) - 1, &atom));
    pk_try(pk_callconv(lisp, atom, carity, PK_TRUE, &call));
    pk_try(pk_call(lisp, &call));
    
    return PK_OK;;
}

PK_RES pk_clone(Pocket lisp, int stack_pointer) {
    (void)stack_pointer;
    return pk_error(lisp);
}

PK_RES pk_typeof(Pocket lisp, int stack_pointer, PK_TYPE *output) {
    PKAtom *atom = NULL;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    *output = pk_atom_typeof(atom->tag.ty);
    return PK_OK;
}

#define PK_TYPEOF_TEMPLATE(ty_) \
    PKAtom *atom = NULL; \
    pk_try(pk_stack_get(lisp, stack_pointer, &atom)); \
    if (atom->tag.ty == (ty_)) { \
        *output = 1; \
    } else { \
        *output = 0; \
    } \
    return PK_OK;

PK_RES pk_is_nil(Pocket lisp, int stack_pointer, int *output) {
    PK_TYPEOF_TEMPLATE(PKAtomTy_Nil)
}

PK_RES pk_is_t(Pocket lisp, int stack_pointer, int *output) {
    PKAtom *atom = NULL;
    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    if (pk_atom_is_nil(atom)) {
        *output = 0;
    } else {
        *output = 1;
    }
    return PK_OK;
}

PK_RES pk_is_number(Pocket lisp, int stack_pointer, int *output) {
    PK_TYPEOF_TEMPLATE(PKAtomTy_Number)
}

PK_RES pk_is_symbol(Pocket lisp, int stack_pointer, int *output) {
    PK_TYPEOF_TEMPLATE(PKAtomTy_Symbol)
}

PK_RES pk_is_string(Pocket lisp, int stack_pointer, int *output) {
    PK_TYPEOF_TEMPLATE(PKAtomTy_String)
}

PK_RES pk_is_cons(Pocket lisp, int stack_pointer, int *output) {
    PK_TYPEOF_TEMPLATE(PKAtomTy_Cons)
}
