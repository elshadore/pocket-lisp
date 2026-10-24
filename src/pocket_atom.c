#include "pocket_internals.h"

PKAtom *pk_atom_new_nil(Pocket lisp) {
    PKAtom *result = pk_atom_alloc(lisp);
    result->header.type = PK_ATOMTY_NIL;
    return result;
}

PKAtom *pk_atom_new_true(Pocket lisp) {
    PKAtom *result = pk_atom_alloc(lisp);
    result->header.type = PK_ATOMTY_TRUE;
    return result;
}

PKAtom *pk_atom_new_undefined(Pocket lisp) {
    PKAtom *result = pk_atom_alloc(lisp);
    result->header.type = PK_ATOMTY_UNDEFINED;
    return result;
}

PKAtom *pk_atom_bool(Pocket lisp, bool boolean) {
    if (boolean) {
        return lisp->cache.atom_true;
    } else {
        return lisp->cache.nil;
    }
}

bool pk_atom_is_true(PKAtom *atom) {
    return atom->header.type != PK_ATOMTY_NIL;
}

bool pk_atom_is_undefined(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_UNDEFINED;
}

bool pk_atom_is_type(PKAtom *atom, PK_TYPE type) {
    if (type == PK_TYPE_TRUE) {
        if (atom->header.type == PK_ATOMTY_NIL) {
            return false;
        } else {
            return true;
        }
    }
    switch (atom->header.type) {
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
        case PK_ATOMTY_CHARACTER: {
            return type == PK_TYPE_CHARACTER;
        }
        default: return false;
    }
}

PKAtom *pk_atom_reverse(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            PKStr dupe = pk_str_dupe(lisp, atom->string.str);
            pk_str_reverse_mut(dupe);
            return (PKAtom *)pk_atom_string_nomemcpy(lisp, dupe);
        }
        case PK_ATOMTY_CONS: {
            PKAtom *acc = lisp->cache.nil;
            PKAtom *iter = atom;
            while (pk_atom_is_cons(iter)) {
                acc = (PKAtom *)pk_atom_cons(lisp, iter->cons.car, acc);
                iter = iter->cons.cdr;
            }
            return acc;
        }
        case PK_ATOMTY_NIL: return lisp->cache.nil;
        default: {
            PKStr ty = pk_ident_atomty(atom->header.type);
            pk_errorf(lisp, "pk_atom_reverse: expected a string or cons, got %.*s", (int)ty.length, ty.c);
            return lisp->cache.nil;
        }
    }
}

bool pk_atom_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    (void)lisp;
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

bool pk_atom_object_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    (void)lisp;
    return lhs == rhs;
}

bool pk_atom_struct_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    (void)lisp;
    (void)lhs;
    (void)rhs;
    pk_errors(lisp, pk_str("TODO: implement pk_atom_struct_eq()"));
    return false;
}

size_t pk_atom_hash(PKAtom *atom) {
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
