#include "pocket_internals.h"

PKAtom pk_int(int integer) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    atom->header.type = PK_ATOMTY_NUMBER;
    atom->number.num_type = PK_NUMTY_INT;
    atom->number.as.i = integer;
    return atom;
}

PKAtom pk_float(float floater) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    atom->header.type = PK_ATOMTY_NUMBER;
    atom->number.num_type = PK_NUMTY_FLOAT;
    atom->number.as.f = floater;
    return atom;
}

#define PK_TEMPLATE_MATH_OP(op_) \
    if (!pk_is_number(lhs)) { \
        return pk_errors(pk_str("expected number")); \
    } \
    if (!pk_is_number(rhs)) { \
        return pk_errors(pk_str("expected number")); \
    } \
    switch (lhs->number.num_type) { \
        case PK_NUMTY_INT: { \
            switch (rhs->number.num_type) { \
                case PK_NUMTY_INT: return pk_int(lhs->number.as.i op_ rhs->number.as.i); \
                case PK_NUMTY_FLOAT: return pk_float((float)lhs->number.as.i op_ rhs->number.as.f); \
                default: return pk_errors(pk_str("expected number")); \
            } \
        } \
        case PK_NUMTY_FLOAT: { \
            switch (rhs->number.num_type) { \
                case PK_NUMTY_INT: return pk_float(lhs->number.as.f op_ (float)rhs->number.as.i); \
                case PK_NUMTY_FLOAT: return pk_float(lhs->number.as.f op_ rhs->number.as.f); \
                default: return pk_errors(pk_str("expected number")); \
            } \
        } \
        default: return pk_errors(pk_str("expected number")); \
    }

PKAtom pk_add(PKAtom lhs, PKAtom rhs) {
    PK_TEMPLATE_MATH_OP(+)
}

PKAtom pk_sub(PKAtom lhs, PKAtom rhs) {
    PK_TEMPLATE_MATH_OP(-)
}

PKAtom pk_mul(PKAtom lhs, PKAtom rhs) {
    PK_TEMPLATE_MATH_OP(*)
}

PKAtom pk_div(PKAtom lhs, PKAtom rhs) {
    if (!pk_is_number(lhs)) {
        return pk_errors(pk_str("expected number"));
    }
    if (!pk_is_number(rhs)) {
        return pk_errors(pk_str("expected number"));
    }
    
    float result = pk_to_float(lhs) / pk_to_float(rhs);
    
    return pk_float(result);
}

PKAtom pk_mod(PKAtom lhs, PKAtom rhs) {
    if (!pk_is_number(lhs)) {
        return pk_errors(pk_str("expected number"));
    }
    if (!pk_is_number(rhs)) {
        return pk_errors(pk_str("expected number"));
    }
    
    int a = pk_to_int(lhs);
    int b = pk_to_int(rhs);
    int result = a % b;
    return pk_int(result);
}

int pk_to_int(PKAtom atom) {
    if (!pk_is_number(atom)) {
        return 0;
    }
    switch (atom->number.num_type) {
        case PK_NUMTY_INT: return atom->number.as.i;
        case PK_NUMTY_FLOAT: return (int)atom->number.as.f;
        default: return 0;
    }
}

float pk_to_float(PKAtom atom) {
    if (!pk_is_number(atom)) {
        return 0.0f;
    }
    switch (atom->number.num_type) {
        case PK_NUMTY_INT: return (float)atom->number.as.i;
        case PK_NUMTY_FLOAT: return atom->number.as.f;
        default: return 0.0f;
    }
}

#define PK_TEMPLATE_MATH_LOGIC_OP(op_) \
    if (!pk_is_number(lhs)) { \
        return pk_errors(pk_str("expected number")); \
    } \
    if (!pk_is_number(rhs)) { \
        return pk_errors(pk_str("expected number")); \
    } \
    switch (lhs->number.num_type) { \
        case PK_NUMTY_INT: { \
            switch (rhs->number.num_type) { \
                case PK_NUMTY_INT: return pk_bool(lhs->number.as.i op_ rhs->number.as.i); \
                case PK_NUMTY_FLOAT: return pk_bool((float)lhs->number.as.i op_ rhs->number.as.f); \
                default: return pk_errors(pk_str("expected number")); \
            } \
        } \
        case PK_NUMTY_FLOAT: { \
            switch (rhs->number.num_type) { \
                case PK_NUMTY_INT: return pk_bool(lhs->number.as.f op_ (float)rhs->number.as.i); \
                case PK_NUMTY_FLOAT: return pk_bool(lhs->number.as.f op_ rhs->number.as.f); \
                default: return pk_errors(pk_str("expected number")); \
            } \
        } \
        default: return pk_errors(pk_str("expected number")); \
    }

PKAtom pk_lt(PKAtom lhs, PKAtom rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(<)
}

PKAtom pk_lte(PKAtom lhs, PKAtom rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(<=)
}

PKAtom pk_gt(PKAtom lhs, PKAtom rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(>)
}

PKAtom pk_gte(PKAtom lhs, PKAtom rhs) {
    PK_TEMPLATE_MATH_LOGIC_OP(>=)
}

bool pk_is_number(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_NUMBER;
}

bool pk_is_int(PKAtom atom) {
    if (!pk_is_number(atom)) return false;
    return atom->number.num_type == PK_NUMTY_INT;
}

bool pk_is_float(PKAtom atom) {
    if (!pk_is_number(atom)) return false;
    return atom->number.num_type == PK_NUMTY_FLOAT;
}

PKAtom pk_typecheck_number(PKAtom atom) {
    if (pk_is_number(atom)) {
        return pk_true();
    } else {
        return pk_errors(pk_str("expected number"));
    }
}
