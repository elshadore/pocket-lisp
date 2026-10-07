#include "pocket_internals.h"

PKAtomData PK_ATOM_UNDEFINED = (PKAtomData) {
    .header = (PKAtomHeader) {
        .type = PK_ATOMTY_UNDEFINED,
        .flags = PK_ATOMFLAG_IMMORTAL,
        .refcount = 0,
    }
};

PKAtomData PK_ATOM_NIL = (PKAtomData) {
    .header = (PKAtomHeader) {
        .type = PK_ATOMTY_NIL,
        .flags = PK_ATOMFLAG_IMMORTAL,
        .refcount = 0,
    }
};

PKAtomData PK_ATOM_TRUE = (PKAtomData) {
    .header = (PKAtomHeader) {
        .type = PK_ATOMTY_TRUE,
        .flags = PK_ATOMFLAG_IMMORTAL,
        .refcount = 0,
    }
};

PKAtomData PK_ATOM_ERROR = (PKAtomData) {
    .header = (PKAtomHeader) {
        .type = PK_ATOMTY_ERROR,
        .flags = PK_ATOMFLAG_IMMORTAL,
        .refcount = 0,
    }
};

PKAtomData PK_ATOM_STRING_EMPTY = (PKAtomData) {
    .string = (PKAtomString) {
        .header.type = PK_ATOMTY_STRING,
        .header.flags = PK_ATOMFLAG_IMMORTAL,
        .header.refcount = 0,
        .str = PK_STR_EMPTY,
    }
};

PKAtom pk_undefined(void) {
    return &PK_ATOM_UNDEFINED;
}

PKAtom pk_nil() {
    return &PK_ATOM_NIL;
}

PKAtom pk_true() {
    return &PK_ATOM_TRUE;
}

PKAtom pk_error(void) {
    return &PK_ATOM_ERROR;
}
    
PKAtom pk_error_oom(void) {
    return &PK_ATOM_ERROR;
}

PKAtom pk_string_empty(void) {
    return &PK_ATOM_STRING_EMPTY;
}

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

PKAtom pk_string(PKStr string) {
    if (string.length == 0) {
        return pk_string_empty();
    }

    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    
    PKStr dupe = PK_STR_EMPTY;
    pk_propagate(pk_str_dupe(string, &dupe));
    
    atom->header.type = PK_ATOMTY_STRING;
    atom->string.str = dupe;
    
    return atom;
}

PKAtom pk_symbol(PKStr string) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);

    PKAtom name = pk_string(string);
    pk_propagate(name);

    atom->header.type = PK_ATOMTY_SYMBOL;
    atom->symbol.var = pk_nil();
    atom->symbol.fun = pk_nil();
    atom->symbol.prop = pk_nil();
    atom->symbol.name = name;
    
    return atom;
}

PKAtom pk_cons(PKAtom car, PKAtom cdr) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    
    atom->header.type = PK_ATOMTY_CONS;
    atom->cons.car = car;
    atom->cons.cdr = cdr;
    
    return atom;
}

PKAtom pk_list2(PKAtom first, PKAtom second) {
    PKAtom b = pk_cons(second, pk_nil());
    pk_propagate(b);
    PKAtom a = pk_cons(first, b);
    pk_propagate(a);
    return a;
}

PKAtom pk_cfun_cons(void *user_closure, void *opaque_fun, pk_u8 args, pk_u8 arity) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);

    atom->header.type = PK_ATOMTY_CFUN;
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

PKAtom pk_cfun0(void *user_closure, PKFun0 fun) {
    return pk_cfun_cons(user_closure, (void *)fun, 0, PK_INT_ARITY_NORMAL);
}

PKAtom pk_cfun1(void *user_closure, PKFun1 fun, PK_ARITY arity) {
    return pk_cfun_cons(user_closure, (void *)fun, 0, pk_cfun_arity_conv(arity));
}

PKAtom pk_cfun2(void *user_closure, PKFun2 fun, PK_ARITY arity) {
    return pk_cfun_cons(user_closure, (void *)fun, 0, pk_cfun_arity_conv(arity));
}

PKAtom pk_cfun3(void *user_closure, PKFun3 fun, PK_ARITY arity) {
    return pk_cfun_cons(user_closure, (void *)fun, 0, pk_cfun_arity_conv(arity));
}

bool pk_string_qeq(PKAtom string, PKStr comp) {
    if (!pk_is_string(string)) {
        return false;
    }
    return pk_str_eq(string->string.str, comp);
}

bool pk_symbol_qeq(PKAtom symbol, PKStr comp) {
    if (!pk_is_symbol(symbol)) {
        return false;
    }
    return pk_str_eq(symbol->symbol.name->string.str, comp);
}

bool pk_keyword_qeq(PKAtom keyword, PKStr comp) {
    if (!pk_is_keyword(keyword)) {
        return false;
    }
    return pk_str_eq(keyword->keyword.id, comp);
}

bool pk_is_nil(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_NIL;
}

bool pk_is_true(PKAtom atom) {
    return atom->header.type != PK_ATOMTY_NIL;
}

bool pk_is_number(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_NUMBER;
}

bool pk_is_string(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_STRING;
}

bool pk_is_symbol(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_SYMBOL;
}

bool pk_is_keyword(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_KEYWORD;
}

bool pk_is_cons(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_CONS;
}

bool pk_is_error(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_ERROR;
}

bool pk_is_undefined(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_UNDEFINED;
}

bool pk_is_cfun(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_CFUN;
}

PKAtom pk_typecheck_nil(PKAtom atom) {
    if (pk_is_nil(atom)) {
        return pk_true();
    } else {
        return pk_error();
    }
}

PKAtom pk_typecheck_number(PKAtom atom) {
    if (pk_is_number(atom)) {
        return pk_true();
    } else {
        return pk_error();
    }
}

PKAtom pk_typecheck_string(PKAtom atom) {
    if (pk_is_string(atom)) {
        return pk_true();
    } else {
        return pk_error();
    }
}

PKAtom pk_typecheck_symbol(PKAtom atom) {
    if (pk_is_symbol(atom)) {
        return pk_true();
    } else {
        return pk_error();
    }
}

PKAtom pk_typecheck_keyword(PKAtom atom) {
    if (pk_is_keyword(atom)) {
        return pk_true();
    } else {
        return pk_error();
    }
}

PKAtom pk_typecheck_cons(PKAtom atom) {
    if (pk_is_cons(atom)) {
        return pk_true();
    } else {
        return pk_error();
    }
}

bool pk_eq(PKAtom lhs, PKAtom rhs) {
    if (lhs == rhs) {
        return true;
    }
    if (lhs->header.type != rhs->header.type) {
        return false;
    }
    switch (lhs->header.type) {
        case PK_ATOMTY_NUMBER: {
            if (lhs->number.num_type != rhs->number.num_type) {
                return false;
            }
            switch (lhs->number.num_type) {
                case PK_NUMTY_INT: {
                    return lhs->number.as.i == rhs->number.as.i;
                }
                case PK_NUMTY_FLOAT: {
                    return lhs->number.as.f == rhs->number.as.f;
                }
                default: return false;
            }
        }
        case PK_ATOMTY_KEYWORD: {
            return pk_str_eq(lhs->keyword.id, rhs->keyword.id);
        }
        case PK_ATOMTY_STRING: {
            return pk_str_eq(lhs->string.str, rhs->string.str);
        }
        default: return false;
    }
}
