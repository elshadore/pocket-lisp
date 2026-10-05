#include "ulisp_internals.h"

ULAtom ul_add(ULAtom lhs, ULAtom rhs) {
    if (!ul_is_number(lhs)) {
        return ul_error();
    }
    
    if (!ul_is_number(rhs)) {
        return ul_error();
    }
    
    return ul_int(lhs->number.as.i + rhs->number.as.i);
}
