#include "pocket_internals.h"

#define pk_atom_number_op_template(op_) \
    switch (lhs->ty) { \
        case PKNumberTy_Int: { \
            int a = lhs->as.i; \
            switch (rhs->ty) { \
                case PKNumberTy_Int: { \
                    int b = rhs->as.i; \
                    int result = a op_ b; \
                    return pk_atom_int(lisp, result); \
                } \
                case PKNumberTy_Float: { \
                    float b = rhs->as.f; \
                    float result = (float)a op_ b; \
                    return pk_atom_int(lisp, result); \
                } \
            } \
        } \
        case PKNumberTy_Float: { \
            float a = lhs->as.f; \
            switch (rhs->ty) { \
                case PKNumberTy_Int: { \
                    int b = rhs->as.i; \
                    float result = a op_ (float)b; \
                    return pk_atom_int(lisp, result); \
                } \
                case PKNumberTy_Float: { \
                    float b = rhs->as.f; \
                    float result = a op_ b; \
                    return pk_atom_int(lisp, result); \
                } \
            } \
        } \
    }

PKAtomNumber *pk_number_add(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    pk_atom_number_op_template(+)
}

PKAtomNumber *pk_number_sub(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    pk_atom_number_op_template(-)
}

PKAtomNumber *pk_number_mul(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    pk_atom_number_op_template(*)
}

PKAtomNumber *pk_number_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    float result =  pk_number_to_float(lhs) / pk_number_to_float(rhs);
    return pk_atom_float(lisp, result);
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

PKAtomNumber *pk_number_mod(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    int a = pk_number_to_int(lhs);
    int b = pk_number_to_int(rhs);
    int result = a % b;
    return pk_atom_int(lisp, result);
}

#define pk_atom_number_logic_template(logic_) \
    switch (lhs->ty) { \
        case PKNumberTy_Int: { \
            int a = lhs->as.i; \
            switch (rhs->ty) { \
                case PKNumberTy_Int: { \
                    int b = rhs->as.i; \
                    bool result =  a logic_ b; \
                    return result; \
                } \
                case PKNumberTy_Float: { \
                    float b = rhs->as.f; \
                    bool result = a logic_ b; \
                    return result; \
                } \
            } \
            break; \
        } \
        case PKNumberTy_Float: { \
            float a = lhs->as.f; \
            switch (rhs->ty) { \
                case PKNumberTy_Int: { \
                    int b = rhs->as.i; \
                    bool result = a logic_ b; \
                    return result; \
                } \
                case PKNumberTy_Float: { \
                    float b = rhs->as.f; \
                    bool result = a logic_ b; \
                    return result; \
                } \
            } \
        } \
        break; \
    } \
    return false;


bool pk_number_lt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    (void)lisp;
    pk_atom_number_logic_template(<)
}

bool pk_number_lte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    (void)lisp;
    pk_atom_number_logic_template(<=)
}

bool pk_number_gt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    (void)lisp;
    pk_atom_number_logic_template(>)
}

bool pk_number_gte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    (void)lisp;
    pk_atom_number_logic_template(>=)
}

bool pk_number_eq(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs) {
    (void)lisp;
    pk_atom_number_logic_template(==)
}
