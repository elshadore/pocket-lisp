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

bool pk_is_true(PKAtom atom) {
    return atom->header.type != PK_ATOMTY_NIL;
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
        case PK_ATOMTY_LOBJECT: {
            return (type == PK_TYPE_OBJECT) || (type == PK_TYPE_LOBJECT);
        }
        case PK_ATOMTY_COBJECT: {
            return (type == PK_TYPE_OBJECT) || (type == PK_TYPE_COBJECT);
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
            return pk_hash_djb2(atom->string.str);
        }
        case PK_ATOMTY_KEYWORD: {
            return pk_hash_djb2(atom->keyword.id);
        }
        case PK_ATOMTY_NUMBER: {
            if (atom->number.num_type == PK_NUMTY_FLOAT) {
                pk_u32 bits = 0;
                memcpy(&bits, &atom->number.as.f, sizeof(pk_u32));
                if (atom->number.as.f == 0.0f) {
                    bits = 0;
                }
                PKStr mem = (PKStr) {
                    .c = (char *)&bits,
                    .length = sizeof(pk_u32)
                };
                return pk_hash_djb2(mem);
            } else {
                PKStr mem = (PKStr) {
                    .c = (char *)&atom->number.as.i,
                    .length = sizeof(int)
                };
                return pk_hash_djb2(mem);
            }
        }
        default: {
            return pk_hash_pointer(atom);
        }
    }
}
