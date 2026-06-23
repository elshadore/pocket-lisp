#include "pocket_internals.h"

PKRes pk_atom_int(Pocket lisp, int value, PKAtomNumber **output) {
    PKAtom *a;
    pk_try(pk_atom_alloc(lisp, &a));
    PKAtomNumber *atom = (PKAtomNumber *)a;
    *atom = (PKAtomNumber) {
        .tag.ty = PKAtomTy_Number,
        .tag.marked = false,
        .ty = PKNumberTy_Int,
        .as.i = value,
    };
    *output = atom;
    return PK_Ok;
}

PKRes pk_atom_float(Pocket lisp, float value, PKAtomNumber **output) {
    PKAtom *a;
    pk_try(pk_atom_alloc(lisp, &a));
    PKAtomNumber *atom = (PKAtomNumber *)a;
    *atom = (PKAtomNumber) {
        .tag.ty = PKAtomTy_Number,
        .tag.marked = false,
        .ty = PKNumberTy_Float,
        .as.f = value,
    };
    *output = atom;
    return PK_Ok;
}

PKRes pk_atom_cast_number(Pocket lisp, PKAtom *atom, PKAtomNumber **output) {
    if (atom->tag.ty != PKAtomTy_Number) return pk_error(lisp);
    *output = (PKAtomNumber *)atom;
    return PK_Ok;
}

#define PK_ATOM_NUMBER_OP_TEMPLATE(op_) \
    switch (lhs->ty) { \
        case PKNumberTy_Int: { \
            int a = lhs->as.i; \
            switch (rhs->ty) { \
                case PKNumberTy_Int: { \
                    int b = rhs->as.i; \
                    int result = a op_ b; \
                    PKAtomNumber *n; \
                    pk_try(pk_atom_int(lisp, (int)result, &n)); \
                    *output = n; \
                    return PK_Ok; \
                } \
                case PKNumberTy_Float: { \
                    float b = rhs->as.f; \
                    float result = (float)a op_ b; \
                    PKAtomNumber *n; \
                    pk_try(pk_atom_int(lisp, (int)result, &n)); \
                    *output = n; \
                    return PK_Ok; \
                } \
            } \
            break; \
        } \
        case PKNumberTy_Float: { \
            float a = lhs->as.f; \
            switch (rhs->ty) { \
                case PKNumberTy_Int: { \
                    int b = rhs->as.i; \
                    float result = a op_ (float)b; \
                    PKAtomNumber *n; \
                    pk_try(pk_atom_int(lisp, (int)result, &n)); \
                    *output = n; \
                    return PK_Ok; \
                } \
                case PKNumberTy_Float: { \
                    float b = rhs->as.f; \
                    float result = a op_ b; \
                    PKAtomNumber *n; \
                    pk_try(pk_atom_int(lisp, (int)result, &n)); \
                    *output = n; \
                    return PK_Ok; \
                } \
            } \
            break; \
        } \
    }

PKRes pk_number_add(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output) {
    PK_ATOM_NUMBER_OP_TEMPLATE(+)
    return pk_error(lisp);
}

PKRes pk_number_sub(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output) {
    PK_ATOM_NUMBER_OP_TEMPLATE(-)
    return pk_error(lisp);
}

PKRes pk_number_mul(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output) {
    PK_ATOM_NUMBER_OP_TEMPLATE(*)
    return pk_error(lisp);
}

PKRes pk_number_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output) {
    float result = pk_number_to_float(lhs) / pk_number_to_float(rhs);
    pk_try(pk_atom_float(lisp, result, output));
    return PK_Ok;
}

int pk_number_to_int(PKAtomNumber *num) {
    switch (num->ty) {
        case PKNumberTy_Int: return num->as.i;
        case PKNumberTy_Float: return (int)num->as.f;
        default: return 0;
    }
}

float pk_number_to_float(PKAtomNumber *num) {
    switch (num->ty) {
        case PKNumberTy_Int: return (float)num->as.i;
        case PKNumberTy_Float: return num->as.f;
        default: return 0.0f;
    }
}

PKRes pk_number_mod(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output) {
    int a = pk_number_to_int(lhs);
    int b = pk_number_to_int(rhs);
    int result = a % b;
    pk_try(pk_atom_int(lisp, result, output));
    return PK_Ok;
}

#define PK_ATOM_NUMBER_LOGIC_TEMPLATE(logic_) \
    switch (lhs->ty) { \
        case PKNumberTy_Int: { \
            int a = lhs->as.i; \
            switch (rhs->ty) { \
                case PKNumberTy_Int: { \
                    int b = rhs->as.i; \
                    *output = a logic_ b; \
                    return PK_Ok; \
                } \
                case PKNumberTy_Float: { \
                    float b = rhs->as.f; \
                    *output = (float)a logic_ b; \
                    return PK_Ok; \
                } \
            } \
            break; \
        } \
        case PKNumberTy_Float: { \
            float a = lhs->as.f; \
            switch (rhs->ty) { \
                case PKNumberTy_Int: { \
                    int b = rhs->as.i; \
                    *output = a logic_ (float)b; \
                    return PK_Ok; \
                } \
                case PKNumberTy_Float: { \
                    float b = rhs->as.f; \
                    *output = a logic_ b; \
                    return PK_Ok; \
                } \
            } \
            break; \
        } \
    }

PKRes pk_number_lt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, bool *output) {
    (void)lisp;
    PK_ATOM_NUMBER_LOGIC_TEMPLATE(<)
    return pk_error(lisp);
}

PKRes pk_number_lte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, bool *output) {
    (void)lisp;
    PK_ATOM_NUMBER_LOGIC_TEMPLATE(<=)
    return pk_error(lisp);
}

PKRes pk_number_gt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, bool *output) {
    (void)lisp;
    PK_ATOM_NUMBER_LOGIC_TEMPLATE(>)
    return pk_error(lisp);
}

PKRes pk_number_gte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, bool *output) {
    (void)lisp;
    PK_ATOM_NUMBER_LOGIC_TEMPLATE(>=)
    return pk_error(lisp);
}

PKRes pk_number_eq(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, bool *output) {
    (void)lisp;
    PK_ATOM_NUMBER_LOGIC_TEMPLATE(==)
    return pk_error(lisp);
}
