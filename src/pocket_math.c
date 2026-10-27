#include "pocket_internals.h"

PKAtomNumber *pk_atom_int(Pocket lisp, int integer) {
    PKAtomNumber *atom = (PKAtomNumber *)pk_atom_alloc(lisp);
    atom->header.type = PK_ATOMTY_NUMBER;
    atom->num_type = PK_NUMTY_INT;
    atom->as.i = integer;
    return atom;
}

PKAtomNumber *pk_atom_float(Pocket lisp, float floater) {
    PKAtomNumber *atom = (PKAtomNumber *)pk_atom_alloc(lisp);
    atom->header.type = PK_ATOMTY_NUMBER;
    atom->num_type = PK_NUMTY_FLOAT;
    atom->as.f = floater;
    return atom;
}

#define PK_TEMPLATE_MATH_OP(op_) \
    if (!pk_atom_is_number(lhs)) { \
        PKStr ty_ = pk_ident_atomty(lhs->header.type); \
        pk_errorf(lisp, "math op " #op_ ": lhs expected a number, got %.*s", (int)ty_.length, ty_.c); \
        return lisp->cache.nil; \
    } \
    if (!pk_atom_is_number(rhs)) { \
        PKStr ty_ = pk_ident_atomty(rhs->header.type); \
        pk_errorf(lisp, "math op " #op_ ": rhs expected a number, got %.*s", (int)ty_.length, ty_.c); \
        return lisp->cache.nil; \
    } \
    switch (lhs->number.num_type) { \
        case PK_NUMTY_INT: { \
            switch (rhs->number.num_type) { \
                case PK_NUMTY_INT: return (PKAtom *)pk_atom_int(lisp, lhs->number.as.i op_ rhs->number.as.i); \
                case PK_NUMTY_FLOAT: return (PKAtom *)pk_atom_float(lisp, (float)lhs->number.as.i op_ rhs->number.as.f); \
                default: { \
                    pk_errorf(lisp, "math op " #op_ ": rhs has unknown number type %u", rhs->number.num_type); \
                    return lisp->cache.nil; \
                } \
            } \
        } \
        case PK_NUMTY_FLOAT: { \
            switch (rhs->number.num_type) { \
                case PK_NUMTY_INT: return (PKAtom *)pk_atom_float(lisp, lhs->number.as.f op_ (float)rhs->number.as.i); \
                case PK_NUMTY_FLOAT: return (PKAtom *)pk_atom_float(lisp, lhs->number.as.f op_ rhs->number.as.f); \
                default: { \
                    pk_errorf(lisp, "math op " #op_ ": rhs has unknown number type %u", rhs->number.num_type); \
                    return lisp->cache.nil; \
                } \
            } \
        } \
        default: { \
            pk_errorf(lisp, "math op " #op_ ": lhs has unknown number type %u", lhs->number.num_type); \
            return lisp->cache.nil; \
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
    switch (number->num_type) {
        case PK_NUMTY_INT: return atom->number.as.i;
        case PK_NUMTY_FLOAT: {
            if (conv == PK_NUMBER_CONV_STRICT) {
                pk_errors(lisp, pk_str("pk_atom_to_int: float given, strict conversion does not allow float to int"));
                return 0;
            }
            return (int)atom->number.as.f;
        }
        default: return 0;
    }
}

size_t pk_atom_to_size_t(Pocket lisp, PKAtom *atom, PK_NUMBER_CONV conv) {
    PKAtomNumber *number = pk_atom_assert_number(lisp, atom);
    switch (number->num_type) {
        case PK_NUMTY_INT: {
            if (number->as.i < 0) {
                pk_errorf(lisp, "integer: %d, is negative, expected a size_t", number->as.i);
                return 0;
            } else {
                return (size_t)number->as.i;
            }
        }
        case PK_NUMTY_FLOAT: {
            if (conv == PK_NUMBER_CONV_STRICT) {
                pk_errors(lisp, pk_str("pk_atom_to_size_t: float given, strict conversion does not allow float to size_t"));
                return 0;
            }
            if (number->as.f < 0.0f) {
                pk_errorf(lisp, "float: %f, is negative, expected a size_t", number->as.f);
                return 0;
            } else {
                return (size_t)number->as.f;
            }
        }
        default: return 0;
    }
}

float pk_atom_to_float(Pocket lisp, PKAtom *atom, PK_NUMBER_CONV conv) {
    PKAtomNumber *number = pk_atom_assert_number(lisp, atom);
    switch (number->num_type) {
        case PK_NUMTY_INT: {
            if (conv == PK_NUMBER_CONV_STRICT) {
                pk_errors(lisp, pk_str("pk_atom_to_float: int given, strict conversion does not allow int to float"));
                return 0.0f;
            }
            return (float)atom->number.as.i;
        }
        case PK_NUMTY_FLOAT: return atom->number.as.f;
        default: return 0.0f;
    }
}

pk_u8 pk_atom_to_byte(Pocket lisp, PKAtom *atom, PK_NUMBER_CONV conv) {
    PKAtomNumber *number = pk_atom_assert_number(lisp, atom);
    (void)conv;
    switch (number->num_type) {
        case PK_NUMTY_INT: {
            if(number->as.i < 0 || number->as.i > UINT8_MAX) {
                pk_errorf(lisp, "integer: %d, is outside byte range 0..255", number->as.i);
                return 0;
            } else {
                return (pk_u8)number->as.i;
            }
        }
        default: {
            pk_errors(lisp, pk_str("cannot convert a float to a byte"));
            return 0;
        }
    }
}

int pk_atom_as_int(PKAtom *atom) {
    if (!pk_atom_is_number(atom)) {
        return 0;
    }
    switch (atom->number.num_type) {
        case PK_NUMTY_INT: return atom->number.as.i;
        case PK_NUMTY_FLOAT: return (int)atom->number.as.f;
        default: return 0;
    }
}

float pk_atom_as_float(PKAtom *atom) {
    if (!pk_atom_is_number(atom)) {
        return 0.0f;
    }
    switch (atom->number.num_type) {
        case PK_NUMTY_INT: return (float)atom->number.as.i;
        case PK_NUMTY_FLOAT: return atom->number.as.f;
        default: return 0.0f;
    }
}

#define PK_TEMPLATE_MATH_LOGIC_OP(op_) \
    PKAtomNumber *nlhs = pk_atom_assert_number(lisp, lhs); \
    PKAtomNumber *nrhs = pk_atom_assert_number(lisp, rhs); \
    switch (nlhs->num_type) { \
        case PK_NUMTY_INT: { \
            switch (nrhs->num_type) { \
                case PK_NUMTY_INT: return  nlhs->as.i op_ nrhs->as.i; \
                case PK_NUMTY_FLOAT: return  (float)nlhs->as.i op_ nrhs->as.f; \
                default: { \
                    pk_errorf(lisp, "logic op " #op_ ": rhs has unknown number type %u", nrhs->num_type); \
                    return false; \
                } \
            } \
        } \
        case PK_NUMTY_FLOAT: { \
            switch (nrhs->num_type) { \
                case PK_NUMTY_INT: return  nlhs->as.f op_ (float)nrhs->as.i; \
                case PK_NUMTY_FLOAT: return  nlhs->as.f op_ nrhs->as.f; \
                default: { \
                    pk_errorf(lisp, "logic op " #op_ ": rhs has unknown number type %u", nrhs->num_type); \
                    return false; \
                } \
            } \
        } \
        default: { \
            pk_errorf(lisp, "logic op " #op_ ": lhs has unknown number type %u", nlhs->num_type); \
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

bool pk_atom_is_number(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_NUMBER;
}

bool pk_atom_is_int(PKAtom *atom) {
    if (!pk_atom_is_number(atom)) return false;
    return atom->number.num_type == PK_NUMTY_INT;
}

bool pk_atom_is_float(PKAtom *atom) {
    if (!pk_atom_is_number(atom)) return false;
    return atom->number.num_type == PK_NUMTY_FLOAT;
}

PKAtomNumber *pk_atom_assert_number(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_number(atom)) {
        PKStr ty = pk_ident_atomty(atom->header.type);
        pk_errorf(lisp, "pk_atom_assert_number: expected a number, got %.*s", (int)ty.length, ty.c);
    }
    return (PKAtomNumber *)atom;
}
