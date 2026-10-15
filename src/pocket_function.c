#include "pocket_internals.h"

PKAtom pk_cfun_cons(void *user_closure, PK_CALL call, void *opaque_fun, pk_u8 args, pk_u8 arity) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);

    if (call == PK_CALL_FUN) {
        atom->header.type = PK_ATOMTY_CFUN;
    } else {
        atom->header.type = PK_ATOMTY_CMACRO;
    }
    atom->cfun.user_closure = user_closure;
    atom->cfun.args = args;
    atom->cfun.arity = arity;
    atom->cfun.as.opaque = opaque_fun;
    return atom;
}

pk_u8 pk_cfun_arity_conv(PK_ARITY arity) {
    switch (arity) {
        case PK_ARITY_NORMAL: return PK_INT_ARITY_NORMAL;
        case PK_ARITY_OPTIONAL: return PK_INT_ARITY_OPTIONAL;
        case PK_ARITY_VARIADIC: return PK_INT_ARITY_VARIADIC;
        default: return PK_INT_ARITY_NORMAL;
    }
}

PKAtom pk_cfun0(void *user_closure, PK_CALL call, PKFun0 fun) {
    return pk_cfun_cons(user_closure, call, (void *)fun, 0, PK_INT_ARITY_NORMAL);
}

PKAtom pk_cfun1(void *user_closure, PK_CALL call, PKFun1 fun, PK_ARITY arity) {
    return pk_cfun_cons(user_closure, call, (void *)fun, 1, pk_cfun_arity_conv(arity));
}

PKAtom pk_cfun2(void *user_closure, PK_CALL call, PKFun2 fun, PK_ARITY arity) {
    return pk_cfun_cons(user_closure, call, (void *)fun, 2, pk_cfun_arity_conv(arity));
}

PKAtom pk_cfun3(void *user_closure, PK_CALL call, PKFun3 fun, PK_ARITY arity) {
    return pk_cfun_cons(user_closure, call, (void *)fun, 3, pk_cfun_arity_conv(arity));
}

PKAtom pk_lfun_exec(PKAtom lfun, size_t save) {
    PKVM vm = pk_vm_init(lfun, save);
    return pk_vm_exec(&vm);
}

bool pk_is_cfun(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_CFUN;
}

bool pk_is_lfun(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_LFUN;
}

bool pk_is_function(PKAtom atom) {
    return pk_is_cfun(atom) || pk_is_lfun(atom);
}

