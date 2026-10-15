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

PKAtom pk_undefined(void) {
    return &PK_ATOM_UNDEFINED;
}

PKAtom pk_true() {
    return &PK_ATOM_TRUE;
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
        pk_propagate(pk_ref_inc(interned));
        return interned;
    }
    
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);

    PKAtom name = pk_string(string);
    pk_propagate(name);

    atom->header.type = PK_ATOMTY_SYMBOL;
    atom->symbol.var = pk_undefined();
    atom->symbol.fun = pk_undefined();
    atom->symbol.prop = pk_undefined();
    atom->symbol.name = name;

    pk_propagate(pk_hashmap_insert(&PK_CORE.symbolizer, name, atom));
    
    return atom;
}

PKAtom pk_symbolc(char *csymbol) {
    PKStr symbol = pk_str_from_cstr(csymbol);
    return pk_symbol(symbol);
}

PKAtom pk_keyword(PKStr keyword) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);

    PKStr dupe = PK_STR_EMPTY;
    pk_propagate(pk_str_dupe(keyword, &dupe));

    atom->header.type = PK_ATOMTY_KEYWORD;
    atom->keyword.id = dupe;

    return atom;
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

bool pk_is_undefined(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_UNDEFINED;
}

bool pk_is_type(PKAtom atom, PK_TYPE type) {
    if (type == PK_TYPE_TRUE) {
        if (atom->header.type == PK_ATOMTY_NIL) {
            return false;
        } else {
            return true;
        }
    }
    switch (atom->header.type) {
        case PK_ATOMTY_OOM: {
            return (type == PK_TYPE_THROW) || (type == PK_TYPE_ERROR);
        }
        case PK_ATOMTY_THROW: {
            return type == PK_TYPE_THROW;
        }
        case PK_ATOMTY_NIL: {
            return (type == PK_TYPE_NIL) || (type == PK_TYPE_LIST);
        }
        case PK_ATOMTY_NUMBER: {
            switch (type) {
                case PK_TYPE_NUMBER: return true;
                case PK_TYPE_INTEGER: return atom->number.num_type == PK_NUMTY_INT;
                case PK_TYPE_FLOAT: return atom->number.num_type == PK_NUMTY_FLOAT;
                default: return false;
            }
        }
        case PK_ATOMTY_STRING: {
            return type == PK_TYPE_STRING;
        }
        case PK_ATOMTY_SYMBOL: {
            return type == PK_TYPE_SYMBOL;
        }
        case PK_ATOMTY_KEYWORD: {
            return type == PK_TYPE_KEYWORD;
        }
        case PK_ATOMTY_CONS: {
            return (type == PK_TYPE_CONS) || (type == PK_TYPE_LIST);
        }
        case PK_ATOMTY_CFUN: {
            return (type == PK_TYPE_FUNCTION) || (type == PK_TYPE_CFUN);
        }
        case PK_ATOMTY_LFUN: {
            return (type == PK_TYPE_FUNCTION) || (type == PK_TYPE_LFUN);
        }
        case PK_ATOMTY_CMACRO: {
            return (type == PK_TYPE_MACRO) || (type == PK_TYPE_CMACRO);
        }
        case PK_ATOMTY_LMACRO: {
            return (type == PK_TYPE_MACRO) || (type == PK_TYPE_LMACRO);
        }
        default: return false;
    }
}

PKAtom pk_typecheck(PKAtom atom, PK_TYPE check) {
    if (pk_is_type(atom, check)) {
        return pk_true();
    } else {
        return pk_error_type(check, atom);
    }
}

PKAtom pk_typecheck_symbol(PKAtom atom) {
    return pk_typecheck(atom, PK_TYPE_SYMBOL);
}

PKAtom pk_typecheck_keyword(PKAtom atom) {
    return pk_typecheck(atom, PK_TYPE_KEYWORD);
}

PKAtom pk_reverse(PKAtom atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            PKStr dupe = PK_STR_EMPTY;
            pk_propagate(pk_str_dupe(atom->string.str, &dupe));
            pk_str_reverse_mut(dupe);
            return pk_string_nomemcpy(dupe);
        }
        case PK_ATOMTY_CONS: {
            PKAtom acc = pk_nil();
            PKAtom iter = atom;
            while (pk_is_cons(iter)) {
                PKAtom a = pk_cons(iter->cons.car, acc);
                pk_propagate(a);
                pk_ref_dec(acc);
                acc = a;
                iter = iter->cons.cdr;
            }
            return acc;
        }
        case PK_ATOMTY_NIL: return pk_nil();
        default: return pk_error_type(PK_TYPE_SEQ, atom);
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
