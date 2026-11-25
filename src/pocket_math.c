#include "pocket_internals.h"
#include <math.h>

PKNum pk_num_int(int integer) {
    return (PKNum) {
        .num_type = PK_NUMTY_INT,
        .as.i = integer,
    };
}

PKNum pk_num_float(float floater) {
    int integer = 0;
    if (pk_float_is_whole(floater, &integer)) {
        return pk_num_int(integer);
    }

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
    int a = pk_num_to_int(lhs->num);
    int b = pk_num_to_int(rhs->num);
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

PKAtomNumber *pk_atom_number_add_vec(Pocket lisp, PKAtoms vec) {
    if (vec.length == 0) {
        return &PK_ATOM_NUMBER_CACHE[0];
    } else if (vec.length == 1) {
        return pk_atom_assert_number(lisp, vec.e[0]);
    } else {
        PKAtomNumber *lhs = pk_atom_assert_number(lisp, vec.e[0]);
        PKAtomNumber *rhs = pk_atom_assert_number(lisp, vec.e[1]);
        PKNum acc = pk_num_add(lisp, lhs->num, rhs->num);
        for (size_t i = 2; i < vec.length; ++i) {
            acc = pk_num_add(lisp, acc, pk_atom_assert_number(lisp, vec.e[i])->num);
        }
        return pk_atom_num(lisp, acc);
    }
}

PKAtomNumber *pk_atom_number_sub_vec(Pocket lisp, PKAtoms vec) {
    if (vec.length == 0) {
        return &PK_ATOM_NUMBER_CACHE[0];
    } else if (vec.length == 1) {
        return pk_atom_assert_number(lisp, vec.e[0]);
    } else {
        PKAtomNumber *lhs = pk_atom_assert_number(lisp, vec.e[0]);
        PKAtomNumber *rhs = pk_atom_assert_number(lisp, vec.e[1]);
        PKNum acc = pk_num_sub(lisp, lhs->num, rhs->num);
        for (size_t i = 2; i < vec.length; ++i) {
            acc = pk_num_sub(lisp, acc, pk_atom_assert_number(lisp, vec.e[i])->num);
        }
        return pk_atom_num(lisp, acc);
    }
}

PKAtomNumber *pk_atom_number_mul_vec(Pocket lisp, PKAtoms vec) {
    if (vec.length == 0) {
        return &PK_ATOM_NUMBER_CACHE[0];
    } else if (vec.length == 1) {
        return pk_atom_assert_number(lisp, vec.e[0]);
    } else {
        PKAtomNumber *lhs = pk_atom_assert_number(lisp, vec.e[0]);
        PKAtomNumber *rhs = pk_atom_assert_number(lisp, vec.e[1]);
        PKNum acc = pk_num_mul(lisp, lhs->num, rhs->num);
        for (size_t i = 2; i < vec.length; ++i) {
            acc = pk_num_mul(lisp, acc, pk_atom_assert_number(lisp, vec.e[i])->num);
        }
        return pk_atom_num(lisp, acc);
    }
}

PKAtomNumber *pk_atom_number_div_vec(Pocket lisp, PKAtoms vec) {
    if (vec.length == 0) {
        return &PK_ATOM_NUMBER_CACHE[0];
    } else if (vec.length == 1) {
        return pk_atom_assert_number(lisp, vec.e[0]);
    } else {
        PKAtomNumber *lhs = pk_atom_assert_number(lisp, vec.e[0]);
        PKAtomNumber *rhs = pk_atom_assert_number(lisp, vec.e[1]);
        PKNum acc = pk_num_div(lisp, lhs->num, rhs->num);
        for (size_t i = 2; i < vec.length; ++i) {
            acc = pk_num_div(lisp, acc, pk_atom_assert_number(lisp, vec.e[i])->num);
        }
        return pk_atom_num(lisp, acc);
    }
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

#define PK_TEMPLATE_MATH_LOGIC_VEC_OP(op_) \
    if (vec.length < 2) { \
        return true; \
    } \
    PKAtomNumber *lhs = pk_atom_assert_number(lisp, vec.e[0]); \
    for (size_t i = 1; i < vec.length; ++i) { \
        PKAtomNumber *rhs = pk_atom_assert_number(lisp, vec.e[i]); \
        if (!pk_atom_number_##op_(lisp, lhs, rhs)) { \
            return false; \
        } \
        lhs = rhs; \
    } \
    return true;

bool pk_atom_number_eq_vec(Pocket lisp, PKAtoms vec) {
    PK_TEMPLATE_MATH_LOGIC_VEC_OP(eq)
}

bool pk_atom_number_lt_vec(Pocket lisp, PKAtoms vec) {
    PK_TEMPLATE_MATH_LOGIC_VEC_OP(lt)
}

bool pk_atom_number_lte_vec(Pocket lisp, PKAtoms vec) {
    PK_TEMPLATE_MATH_LOGIC_VEC_OP(lte)
}

bool pk_atom_number_gt_vec(Pocket lisp, PKAtoms vec) {
    PK_TEMPLATE_MATH_LOGIC_VEC_OP(gt)
}

bool pk_atom_number_gte_vec(Pocket lisp, PKAtoms vec) {
    PK_TEMPLATE_MATH_LOGIC_VEC_OP(gte)
}

PKAtomNumber *pk_atom_number_sin(Pocket lisp, PKAtomNumber *number) {
    float result = sinf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

PKAtomNumber *pk_atom_number_cos(Pocket lisp, PKAtomNumber *number) {
    float result = cosf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

PKAtomNumber *pk_atom_number_tan(Pocket lisp, PKAtomNumber *number) {
    float result = tanf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

PKAtomNumber *pk_atom_number_exp(Pocket lisp, PKAtomNumber *number) {
    float result = expf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

PKAtomNumber *pk_atom_number_log(Pocket lisp, PKAtomNumber *number) {
    float result = logf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

PKAtomNumber *pk_atom_number_asin(Pocket lisp, PKAtomNumber *number) {
    float result = asinf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

PKAtomNumber *pk_atom_number_acos(Pocket lisp, PKAtomNumber *number) {
    float result = acosf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

PKAtomNumber *pk_atom_number_atan(Pocket lisp, PKAtomNumber *number) {
    float result = atanf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

PKAtomNumber *pk_atom_number_sqrt(Pocket lisp, PKAtomNumber *number) {
    float result = sqrtf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

PKAtomNumber *pk_atom_number_floor(Pocket lisp, PKAtomNumber *number) {
    float result = floorf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

PKAtomNumber *pk_atom_number_ceil(Pocket lisp, PKAtomNumber *number) {
    float result = ceilf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

PKAtomNumber *pk_atom_number_truncate(Pocket lisp, PKAtomNumber *number) {
    float result = truncf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

PKAtomNumber *pk_atom_number_round(Pocket lisp, PKAtomNumber *number) {
    float result = roundf(pk_num_to_float(number->num));
    return pk_atom_num(lisp, pk_num_float(result));
}

bool pk_atom_number_is_infinite(Pocket lisp, PKAtomNumber *number) {
    (void)lisp;
    return isinf(pk_num_to_float(number->num)) != 0;
}

bool pk_atom_number_is_nan(Pocket lisp, PKAtomNumber *number) {
    (void)lisp;
    return isnan(pk_num_to_float(number->num)) != 0;
}

bool pk_atom_number_is_zero(Pocket lisp, PKAtomNumber *number) {
    (void)lisp;
    return pk_num_to_float(number->num) == 0.0f;
}

bool pk_atom_number_is_positive(Pocket lisp, PKAtomNumber *number) {
    (void)lisp;
    return pk_num_to_float(number->num) > 0.0f;
}

bool pk_atom_number_is_negative(Pocket lisp, PKAtomNumber *number) {
    (void)lisp;
    return pk_num_to_float(number->num) < 0.0f;
}

bool pk_atom_number_is_odd(Pocket lisp, PKAtomNumber *number) {
    int integer = pk_atom_number_to_int(lisp, number, PK_NUMBER_CONV_STRICT);
    return (integer % 2) != 0;
}

bool pk_atom_number_is_even(Pocket lisp, PKAtomNumber *number) {
    int integer = pk_atom_number_to_int(lisp, number, PK_NUMBER_CONV_STRICT);
    return (integer % 2) == 0;
}

PKAtomNumber *pk_atom_number_max_vec(Pocket lisp, PKAtoms vec) {
    PKAtomNumber *result = pk_atom_assert_number(lisp, vec.e[0]);
    for (size_t i = 1; i < vec.length; ++i) {
        PKAtomNumber *next = pk_atom_assert_number(lisp, vec.e[i]);
        if (pk_atom_number_gt(lisp, next, result)) {
            result = next;
        }
    }
    return result;
}

PKAtomNumber *pk_atom_number_min_vec(Pocket lisp, PKAtoms vec) {
    PKAtomNumber *result = pk_atom_assert_number(lisp, vec.e[0]);
    for (size_t i = 1; i < vec.length; ++i) {
        PKAtomNumber *next = pk_atom_assert_number(lisp, vec.e[i]);
        if (pk_atom_number_lt(lisp, next, result)) {
            result = next;
        }
    }
    return result;
}

PKAtomNumber *pk_atom_number_floor_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    int a = pk_num_to_int(lhs->num);
    int b = pk_num_to_int(rhs->num);
    int q = a / b;
    int r = a % b;
    if ((r != 0) && ((r < 0) != (b < 0))) {
        q -= 1;
    }
    return pk_atom_int(lisp, q);
}

PKAtomNumber *pk_atom_number_floor_remainder(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    int a = pk_num_to_int(lhs->num);
    int b = pk_num_to_int(rhs->num);
    int r = a % b;
    if ((r != 0) && ((r < 0) != (b < 0))) {
        r += b;
    }
    return pk_atom_int(lisp, r);
}

PKAtomNumber *pk_atom_number_truncate_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    int a = pk_num_to_int(lhs->num);
    int b = pk_num_to_int(rhs->num);
    return pk_atom_int(lisp, a / b);
}

PKAtomNumber *pk_atom_number_truncate_remainder(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    int a = pk_num_to_int(lhs->num);
    int b = pk_num_to_int(rhs->num);
    return pk_atom_int(lisp, a % b);
}

PKAtomNumber *pk_atom_number_lerp(Pocket lisp, PKAtomNumber *from, PKAtomNumber *to, PKAtomNumber *delta) {
    float a = pk_num_to_float(from->num);
    float b = pk_num_to_float(to->num);
    float t = pk_num_to_float(delta->num);
    return pk_atom_num(lisp, pk_num_float(a + ((b - a) * t)));
}
