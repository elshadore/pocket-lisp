#include "pocket_internals.h"

PK_RES pk_atom_int(Pocket lisp, int value, PKAtomNumber **output) {
    PKAtom *a = NULL;
    pk_try(pk_atom_alloc(lisp, &a));
    
    a->tag.ty = PKAtomTy_Number;
    a->number.ty = PKNumberTy_Int;
    a->number.as.i = value;
    
    *output = (PKAtomNumber *)a;
    return PK_OK;
}

PK_RES pk_atom_float(Pocket lisp, float value, PKAtomNumber **output) {
    PKAtom *a = NULL;
    pk_try(pk_atom_alloc(lisp, &a));
    
    a->tag.ty = PKAtomTy_Number;
    a->number.ty = PKNumberTy_Float;
    a->number.as.f = value;
    
    *output = (PKAtomNumber *)a;
    return PK_OK;
}

PK_RES pk_atom_cast_number(Pocket lisp, PKAtom *atom, PKAtomNumber **output) {
    if (atom->tag.ty != PKAtomTy_Number) return pk_error(lisp);
    *output = (PKAtomNumber *)atom;
    return PK_OK;
}

#define PK_ATOM_NUMBER_OP_TEMPLATE(op_) \
    switch (lhs->ty) { \
        case PKNumberTy_Int: { \
            int a = lhs->as.i; \
            switch (rhs->ty) { \
                case PKNumberTy_Int: { \
                    int b = rhs->as.i; \
                    int result = a op_ b; \
                    pk_try(pk_atom_int(lisp, (int)result, output)); \
                    return PK_OK; \
                } \
                case PKNumberTy_Float: { \
                    float b = rhs->as.f; \
                    float result = (float)a op_ b; \
                    pk_try(pk_atom_int(lisp, (int)result, output)); \
                    return PK_OK; \
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
                    pk_try(pk_atom_int(lisp, (int)result, output)); \
                    return PK_OK; \
                } \
                case PKNumberTy_Float: { \
                    float b = rhs->as.f; \
                    float result = a op_ b; \
                    pk_try(pk_atom_int(lisp, (int)result, output)); \
                    return PK_OK; \
                } \
            } \
            break; \
        } \
    }

PK_RES pk_number_add(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output) {
    PK_ATOM_NUMBER_OP_TEMPLATE(+)
    return pk_error(lisp);
}

PK_RES pk_number_sub(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output) {
    PK_ATOM_NUMBER_OP_TEMPLATE(-)
    return pk_error(lisp);
}

PK_RES pk_number_mul(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output) {
    PK_ATOM_NUMBER_OP_TEMPLATE(*)
    return pk_error(lisp);
}

PK_RES pk_number_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output) {
    float result = pk_number_to_float(lhs) / pk_number_to_float(rhs);
    pk_try(pk_atom_float(lisp, result, output));
    return PK_OK;
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

PK_RES pk_number_mod(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output) {
    int a = pk_number_to_int(lhs);
    int b = pk_number_to_int(rhs);
    int result = a % b;
    pk_try(pk_atom_int(lisp, result, output));
    return PK_OK;
}

#define PK_ATOM_NUMBER_LOGIC_TEMPLATE(logic_) \
    switch (lhs->ty) { \
        case PKNumberTy_Int: { \
            int a = lhs->as.i; \
            switch (rhs->ty) { \
                case PKNumberTy_Int: { \
                    int b = rhs->as.i; \
                    *output = a logic_ b; \
                    return PK_OK; \
                } \
                case PKNumberTy_Float: { \
                    float b = rhs->as.f; \
                    *output = (float)a logic_ b; \
                    return PK_OK; \
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
                    return PK_OK; \
                } \
                case PKNumberTy_Float: { \
                    float b = rhs->as.f; \
                    *output = a logic_ b; \
                    return PK_OK; \
                } \
            } \
            break; \
        } \
    }

PK_RES pk_number_lt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output) {
    (void)lisp;
    PK_ATOM_NUMBER_LOGIC_TEMPLATE(<)
    return pk_error(lisp);
}

PK_RES pk_number_lte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output) {
    (void)lisp;
    PK_ATOM_NUMBER_LOGIC_TEMPLATE(<=)
    return pk_error(lisp);
}

PK_RES pk_number_gt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output) {
    (void)lisp;
    PK_ATOM_NUMBER_LOGIC_TEMPLATE(>)
    return pk_error(lisp);
}

PK_RES pk_number_gte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output) {
    (void)lisp;
    PK_ATOM_NUMBER_LOGIC_TEMPLATE(>=)
    return pk_error(lisp);
}

PK_RES pk_number_eq(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output) {
    (void)lisp;
    PK_ATOM_NUMBER_LOGIC_TEMPLATE(==)
    return pk_error(lisp);
}
