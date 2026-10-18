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
        pk_error(lisp); \
        return lisp->cache.nil; \
    } \
    if (!pk_atom_is_number(rhs)) { \
        pk_error(lisp); \
        return lisp->cache.nil; \
    } \
    switch (lhs->number.num_type) { \
        case PK_NUMTY_INT: { \
            switch (rhs->number.num_type) { \
                case PK_NUMTY_INT: return (PKAtom *)pk_atom_int(lisp, lhs->number.as.i op_ rhs->number.as.i); \
                case PK_NUMTY_FLOAT: return (PKAtom *)pk_atom_float(lisp, (float)lhs->number.as.i op_ rhs->number.as.f); \
                default: { \
                    pk_error(lisp); \
                    return lisp->cache.nil; \
                } \
            } \
        } \
        case PK_NUMTY_FLOAT: { \
            switch (rhs->number.num_type) { \
                case PK_NUMTY_INT: return (PKAtom *)pk_atom_float(lisp, lhs->number.as.f op_ (float)rhs->number.as.i); \
                case PK_NUMTY_FLOAT: return (PKAtom *)pk_atom_float(lisp, lhs->number.as.f op_ rhs->number.as.f); \
                default: { \
                    pk_error(lisp); \
                    return lisp->cache.nil; \
                } \
            } \
        } \
        default: { \
            pk_error(lisp); \
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
    float result = pk_atom_to_float(lisp, lhs) / pk_atom_to_float(lisp, rhs);
    return (PKAtom *)pk_atom_float(lisp, result);
}

PKAtom *pk_atom_mod(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    int a = pk_atom_to_int(lisp, lhs);
    int b = pk_atom_to_int(lisp, rhs);
    int result = a % b;
    return (PKAtom *)pk_atom_int(lisp, result);
}

int pk_atom_to_int(Pocket lisp, PKAtom *atom) {
    PKAtomNumber *number = pk_atom_assert_number(lisp, atom);
    switch (number->num_type) {
        case PK_NUMTY_INT: return atom->number.as.i;
        case PK_NUMTY_FLOAT: return (int)atom->number.as.f;
        default: return 0;
    }
}

float pk_atom_to_float(Pocket lisp, PKAtom *atom) {
    PKAtomNumber *number = pk_atom_assert_number(lisp, atom);
    switch (number->num_type) {
        case PK_NUMTY_INT: return (float)atom->number.as.i;
        case PK_NUMTY_FLOAT: return atom->number.as.f;
        default: return 0.0f;
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
                    pk_error(lisp); \
                    return false; \
                } \
            } \
        } \
        case PK_NUMTY_FLOAT: { \
            switch (nrhs->num_type) { \
                case PK_NUMTY_INT: return  nlhs->as.f op_ (float)nrhs->as.i; \
                case PK_NUMTY_FLOAT: return  nlhs->as.f op_ nrhs->as.f; \
                default: { \
                    pk_error(lisp); \
                    return false; \
                } \
            } \
        } \
        default: { \
            pk_error(lisp); \
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
        pk_error(lisp);
    }
    return (PKAtomNumber *)atom;
}
