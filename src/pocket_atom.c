#include "pocket_internals.h"

PKAtomData PK_ATOM_UNDEFINED = (PKAtomData) {
    .header = (PKAtomHeader) {
        .type = PK_ATOMTY_UNDEFINED,
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

PKAtom pk_undefined(void) {
    return &PK_ATOM_UNDEFINED;
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

PKAtom pk_bool(bool boolean) {
    if (boolean) {
        return pk_true();
    } else {
        return pk_nil();
    }
}

PKAtom pk_symbol(PKStr string) {
    PKAtom interned = pk_hashmap_get_str(&PK_CORE.symbolizer, string);
    if (!pk_is_undefined(interned)) {
        return interned;
    }
    
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);

    PKAtom name = pk_string(string);
    pk_propagate(name);

    atom->header.type = PK_ATOMTY_SYMBOL;
    atom->symbol.var = pk_nil();
    atom->symbol.fun = pk_nil();
    atom->symbol.prop = pk_nil();
    atom->symbol.name = name;

    pk_propagate(pk_hashmap_insert(&PK_CORE.symbolizer, name, atom));
    
    return atom;
}

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

bool pk_is_true(PKAtom atom) {
    return atom->header.type != PK_ATOMTY_NIL;
}

bool pk_is_symbol(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_SYMBOL;
}

bool pk_is_keyword(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_KEYWORD;
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

bool pk_is_lfun(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_LFUN;
}

bool pk_is_function(PKAtom atom) {
    return pk_is_cfun(atom) || pk_is_lfun(atom);
}

bool pk_is_macro(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_CMACRO || atom->header.type == PK_ATOMTY_LMACRO;
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

size_t pk_hash(PKAtom atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            PKStr str = atom->string.str;
            return pk_hash_djb2(str.c, str.length);
        }
        case PK_ATOMTY_KEYWORD: {
            PKStr id = atom->keyword.id;
            return pk_hash_djb2(id.c, id.length);
        }
        case PK_ATOMTY_NUMBER: {
            if (atom->number.num_type == PK_NUMTY_FLOAT) {
                pk_u32 bits = 0;
                memcpy(&bits, &atom->number.as.f, sizeof(pk_u32));
                if (atom->number.as.f == 0.0f) {
                    bits = 0;
                }
                return pk_hash_djb2((const char *)&bits, sizeof(pk_u32));
            } else {
                return pk_hash_djb2((const char *)&atom->number.as.i, sizeof(int));
            }
        }
        default: {
            return pk_hash_pointer(atom);
        }
    }
}
