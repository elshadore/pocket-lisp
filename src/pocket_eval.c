#include "pocket_internals.h"

PKAtom pk_eval(PKAtom expr) {
    PKAtom fun = pk_compile(expr);
    pk_propagate(fun);
    return pk_funcall0(fun);
}
