#include "pocket_internals.h"

PKNum pk_num_int(int integer) {
    return (PKNum) {
        .num_type = PK_NUMTY_INT,
        .as.i = integer,
    };
}

PKNum pk_num_float(float floater) {
    return (PKNum) {
        .num_type = PK_NUMTY_FLOAT,
        .as.f = floater,
    };
}

int pk_num_to_int(PKNum num) {
    if (num.num_type == PK_NUMTY_INT) {
        return num.as.i;
    } else {
        return (int)num.as.f;
    }
}

float pk_num_to_float(PKNum num) {
    if (num.num_type == PK_NUMTY_INT) {
        return (float)num.as.i;
    } else {
        return num.as.f;
    }
}

#define PK_TEMPLATE_MATH_OP(op_) \
    (void)lisp; \
    if (lhs.num_type == PK_NUMTY_INT) { \
        if (rhs.num_type == PK_NUMTY_INT) { \
            return pk_num_int(lhs.as.i op_ rhs.as.i); \
        } else { \
            return pk_num_float((float)lhs.as.i op_ rhs.as.f); \
        } \
    } else { \
        if (rhs.num_type == PK_NUMTY_INT) { \
            return pk_num_float(lhs.as.f op_ (float)rhs.as.i); \
        } else { \
            return pk_num_float(lhs.as.f op_ rhs.as.f); \
        } \
    }

PKNum pk_num_add(Pocket lisp, PKNum lhs, PKNum rhs) {
    PK_TEMPLATE_MATH_OP(+)
}

PKNum pk_num_sub(Pocket lisp, PKNum lhs, PKNum rhs) {
    PK_TEMPLATE_MATH_OP(-)
}

PKNum pk_num_mul(Pocket lisp, PKNum lhs, PKNum rhs) {
    PK_TEMPLATE_MATH_OP(*)
}

PKNum pk_num_div(Pocket lisp, PKNum lhs, PKNum rhs) {
    (void)lisp;
    float result = pk_num_to_float(lhs) / pk_num_to_float(rhs);
    return pk_num_float(result);
}

PKAtomNumber *pk_atom_number_add(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    return pk_atom_num(lisp, pk_num_add(lisp, lhs->num, rhs->num));
}

PKAtomNumber *pk_atom_number_sub(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    return pk_atom_num(lisp, pk_num_sub(lisp, lhs->num, rhs->num));
}

PKAtomNumber *pk_atom_number_mul(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    return pk_atom_num(lisp, pk_num_mul(lisp, lhs->num, rhs->num));
}

PKAtomNumber *pk_atom_number_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    return pk_atom_num(lisp, pk_num_div(lisp, lhs->num, rhs->num));
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
