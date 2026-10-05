#include "pocket_internals.h"

PKAtom pk_add(PKAtom lhs, PKAtom rhs) {
    if (!pk_is_number(lhs)) {
        return pk_error();
    }
    
    if (!pk_is_number(rhs)) {
        return pk_error();
    }
    
    return pk_int(lhs->number.as.i + rhs->number.as.i);
}
