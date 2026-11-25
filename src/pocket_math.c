#include "pocket_internals.h"

#define PK_TEMPLATE_MATH_OP(op_) \
    if (!pk_atom_is_number(lhs)) { \
        PKStr ty_ = pk_ident_atomty(lhs->header.type); \
        pk_errorf(lisp, "math op " #op_ ": lhs expected a number, got %.*s", (int)ty_.length, ty_.c); \
        return &PK_ATOM_NIL; \
    } \
    if (!pk_atom_is_number(rhs)) { \
        PKStr ty_ = pk_ident_atomty(rhs->header.type); \
        pk_errorf(lisp, "math op " #op_ ": rhs expected a number, got %.*s", (int)ty_.length, ty_.c); \
        return &PK_ATOM_NIL; \
    } \
    switch (lhs->number.num.num_type) { \
        case PK_NUMTY_INT: { \
            switch (rhs->number.num.num_type) { \
                case PK_NUMTY_INT: return (PKAtom *)pk_atom_int(lisp, lhs->number.num.as.i op_ rhs->number.num.as.i); \
                case PK_NUMTY_FLOAT: return (PKAtom *)pk_atom_float(lisp, (float)lhs->number.num.as.i op_ rhs->number.num.as.f); \
                default: { \
                    pk_errorf(lisp, "math op " #op_ ": rhs has unknown number type %u", rhs->number.num.num_type); \
                    return &PK_ATOM_NIL; \
                } \
            } \
        } \
        case PK_NUMTY_FLOAT: { \
            switch (rhs->number.num.num_type) { \
                case PK_NUMTY_INT: return (PKAtom *)pk_atom_float(lisp, lhs->number.num.as.f op_ (float)rhs->number.num.as.i); \
                case PK_NUMTY_FLOAT: return (PKAtom *)pk_atom_float(lisp, lhs->number.num.as.f op_ rhs->number.num.as.f); \
                default: { \
                    pk_errorf(lisp, "math op " #op_ ": rhs has unknown number type %u", rhs->number.num.num_type); \
                    return &PK_ATOM_NIL; \
                } \
            } \
        } \
        default: { \
            pk_errorf(lisp, "math op " #op_ ": lhs has unknown number type %u", lhs->number.num.num_type); \
            return &PK_ATOM_NIL; \
        } \
    }

PKAtom *pk_atom_add(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    PK_TEMPLATE_MATH_OP(+)
}

PKAtom *pk_atom_sub(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    PK_TEMPLATE_MATH_OP(-)
}

PKAtom *pk_atom_mul(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    PK_TEMPLATE_MATH_OP(*)
}

PKAtom *pk_atom_div(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    float result = pk_atom_to_float(lisp, lhs, PK_NUMBER_CONV_FLOAT_CONV) / pk_atom_to_float(lisp, rhs, PK_NUMBER_CONV_FLOAT_CONV);
    return (PKAtom *)pk_atom_float(lisp, result);
}

PKAtom *pk_atom_mod(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    int a = pk_atom_to_int(lisp, lhs, PK_NUMBER_CONV_FLOAT_CONV);
    int b = pk_atom_to_int(lisp, rhs, PK_NUMBER_CONV_FLOAT_CONV);
    int result = a % b;
    return (PKAtom *)pk_atom_int(lisp, result);
}

int pk_atom_to_int(Pocket lisp, PKAtom *atom, PK_NUMBER_CONV conv) {
    PKAtomNumber *number = pk_atom_assert_number(lisp, atom);
    switch (number->num.num_type) {
        case PK_NUMTY_INT: return atom->number.num.as.i;
        case PK_NUMTY_FLOAT: {
            if (conv == PK_NUMBER_CONV_STRICT) {
                pk_errors(lisp, pk_str("pk_atom_to_int: float given, strict conversion does not allow float to int"));
                return 0;
            }
            return (int)atom->number.num.as.f;
        }
        default: return 0;
    }
}

size_t pk_atom_to_size_t(Pocket lisp, PKAtom *atom) {
    PKAtomNumber *number = pk_atom_assert_number(lisp, atom);
    switch (number->num.num_type) {
        case PK_NUMTY_INT: {
            if (number->num.as.i < 0) {
                pk_errorf(lisp, "integer: %d, is negative, expected a size_t", number->num.as.i);
                return 0;
            } else {
                return (size_t)number->num.as.i;
            }
        }
        case PK_NUMTY_FLOAT: {
            pk_errors(lisp, pk_str("cannot convert number from a float"));
            return 0;
        }
        default: return 0;
    }
}

float pk_atom_to_float(Pocket lisp, PKAtom *atom, PK_NUMBER_CONV conv) {
    PKAtomNumber *number = pk_atom_assert_number(lisp, atom);
    switch (number->num.num_type) {
        case PK_NUMTY_INT: {
            if (conv == PK_NUMBER_CONV_STRICT) {
                pk_errors(lisp, pk_str("pk_atom_to_float: int given, strict conversion does not allow int to float"));
                return 0.0f;
            }
            return (float)atom->number.num.as.i;
        }
        case PK_NUMTY_FLOAT: return atom->number.num.as.f;
        default: return 0.0f;
    }
}

int pk_atom_as_int(PKAtom *atom) {
    if (!pk_atom_is_number(atom)) {
        return 0;
    }
    switch (atom->number.num.num_type) {
        case PK_NUMTY_INT: return atom->number.num.as.i;
        case PK_NUMTY_FLOAT: return (int)atom->number.num.as.f;
        default: return 0;
    }
}

float pk_atom_as_float(PKAtom *atom) {
    if (!pk_atom_is_number(atom)) {
        return 0.0f;
    }
    switch (atom->number.num.num_type) {
        case PK_NUMTY_INT: return (float)atom->number.num.as.i;
        case PK_NUMTY_FLOAT: return atom->number.num.as.f;
        default: return 0.0f;
    }
}

#define PK_TEMPLATE_MATH_LOGIC_OP(op_) \
    PKAtomNumber *nlhs = pk_atom_assert_number(lisp, lhs); \
    PKAtomNumber *nrhs = pk_atom_assert_number(lisp, rhs); \
    switch (nlhs->num.num_type) { \
        case PK_NUMTY_INT: { \
            switch (nrhs->num.num_type) { \
                case PK_NUMTY_INT: return  nlhs->num.as.i op_ nrhs->num.as.i; \
                case PK_NUMTY_FLOAT: return  (float)nlhs->num.as.i op_ nrhs->num.as.f; \
                default: { \
                    pk_errorf(lisp, "logic op " #op_ ": rhs has unknown number type %u", nrhs->num.num_type); \
                    return false; \
                } \
            } \
        } \
        case PK_NUMTY_FLOAT: { \
            switch (nrhs->num.num_type) { \
                case PK_NUMTY_INT: return  nlhs->num.as.f op_ (float)nrhs->num.as.i; \
                case PK_NUMTY_FLOAT: return  nlhs->num.as.f op_ nrhs->num.as.f; \
                default: { \
                    pk_errorf(lisp, "logic op " #op_ ": rhs has unknown number type %u", nrhs->num.num_type); \
                    return false; \
                } \
            } \
        } \
        default: { \
            pk_errorf(lisp, "logic op " #op_ ": lhs has unknown number type %u", nlhs->num.num_type); \
            return false; \
        } \
    }

bool pk_atom_lt(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(<)
}

bool pk_atom_lte(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(<=)
}

bool pk_atom_gt(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(>)
}

bool pk_atom_gte(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(>=)
}
