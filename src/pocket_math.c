#include "pocket_internals.h"

#define PK_TEMPLATE_MATH_OP(op_) \
    if (lhs->num.num_type == PK_NUMTY_INT) { \
        if (rhs->num.num_type == PK_NUMTY_INT) { \
            return pk_atom_int(lisp, lhs->num.as.i op_ rhs->num.as.i); \
        } else { \
            return pk_atom_float(lisp, (float)lhs->num.as.i op_ rhs->num.as.f); \
        } \
    } else { \
        if (rhs->num.num_type == PK_NUMTY_INT) { \
            return pk_atom_float(lisp, lhs->num.as.f op_ (float)rhs->num.as.i); \
        } else { \
            return pk_atom_float(lisp, lhs->num.as.f op_ rhs->num.as.f); \
        } \
    }

PKAtomNumber *pk_atom_number_add(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    PK_TEMPLATE_MATH_OP(+)
}

PKAtomNumber *pk_atom_number_sub(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    PK_TEMPLATE_MATH_OP(-)
}

PKAtomNumber *pk_atom_number_mul(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    PK_TEMPLATE_MATH_OP(*)
}

PKAtomNumber *pk_atom_number_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    float result = pk_atom_number_to_float(lisp, lhs, PK_NUMBER_CONV_FLOAT_CONV) / pk_atom_number_to_float(lisp, rhs, PK_NUMBER_CONV_FLOAT_CONV);
    return pk_atom_float(lisp, result);
}

PKAtomNumber *pk_atom_number_mod(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    int a = pk_atom_number_to_int(lisp, lhs, PK_NUMBER_CONV_FLOAT_CONV);
    int b = pk_atom_number_to_int(lisp, rhs, PK_NUMBER_CONV_FLOAT_CONV);
    int result = a % b;
    return pk_atom_int(lisp, result);
}

#define PK_TEMPLATE_MATH_LOGIC_OP(op_) \
    (void)lisp; \
    if (lhs->num.num_type == PK_NUMTY_INT) { \
        if (rhs->num.num_type == PK_NUMTY_INT) { \
            return lhs->num.as.i op_ rhs->num.as.i; \
        } else { \
            return (float)lhs->num.as.i op_ rhs->num.as.f; \
        } \
    } else { \
        if (rhs->num.num_type == PK_NUMTY_INT) { \
            return lhs->num.as.f op_ (float)rhs->num.as.i; \
        } else { \
            return lhs->num.as.f op_ rhs->num.as.f; \
        } \
    }

bool pk_atom_number_eq(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(==)
}

bool pk_atom_number_lt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(<)
}

bool pk_atom_number_lte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(<=)
}

bool pk_atom_number_gt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(>)
}

bool pk_atom_number_gte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(>=)
}
