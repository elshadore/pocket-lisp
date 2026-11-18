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

PKAtomKind pk_atom_kind(PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_CONS: return PK_ATOMKIND_OBJECT;
        case PK_ATOMTY_ARRAY: return PK_ATOMKIND_OBJECT;
        case PK_ATOMTY_OBJECT: return PK_ATOMKIND_OBJECT;
        case PK_ATOMTY_DESTROYED: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_UNDEFINED: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_NIL: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_TRUE: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_NUMBER: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_STRING: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_SYMBOL: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_KEYWORD: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_CFUN: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_LFUN: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_CMACRO: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_LMACRO: return PK_ATOMKIND_VALUE;
        default: return PK_ATOMKIND_VALUE;
    }
}

PKAtom *pk_atom_clone_rec_(Pocket lisp, PKHashmap *memo, PKAtom *atom) {
    if (pk_atom_kind(atom) == PK_ATOMKIND_VALUE) {
        return atom;
    }

    PKAtom *have = pk_hashmap_get(lisp, memo, atom);
    if (!pk_atom_is_undefined(have)) {
        return have;
    }

    switch (atom->header.type) {
        case PK_ATOMTY_CONS: {
            PKAtomCons *result = pk_atom_cons(lisp, lisp->cache.nil, lisp->cache.nil);
            pk_hashmap_insert(lisp, memo, atom, (PKAtom *)result, PK_HASHMAP_INSERT_NORMAL);
            result->car = pk_atom_clone_rec_(lisp, memo, atom->cons.car);

            PKAtom *iter = atom->cons.cdr;
            PKAtom *current = (PKAtom *)result;
            while (pk_atom_is_cons(iter) && pk_atom_is_undefined(pk_hashmap_get(lisp, memo, iter))) {
                PKAtomCons *next = pk_atom_cons(lisp, lisp->cache.nil, lisp->cache.nil);
                pk_hashmap_insert(lisp, memo, iter, (PKAtom *)next, PK_HASHMAP_INSERT_NORMAL);
                next->car = pk_atom_clone_rec_(lisp, memo, iter->cons.car);
                current->cons.cdr = (PKAtom *)next;
                current = (PKAtom *)next;
                iter = iter->cons.cdr;
            }
            current->cons.cdr = pk_atom_clone_rec_(lisp, memo, iter);
            return (PKAtom *)result;
        }
        case PK_ATOMTY_ARRAY: {
            PKAtomArray *result = pk_atom_array(lisp, PK_ATOMS_EMPTY);
            pk_hashmap_insert(lisp, memo, atom, (PKAtom *)result, PK_HASHMAP_INSERT_NORMAL);
            for (size_t i = 0; i < atom->array.atoms.count; ++i) {
                pk_atom_array_append(lisp, result, pk_atom_clone_rec_(lisp, memo, atom->array.atoms.e[i]));
            }
            return (PKAtom *)result;
        }
        case PK_ATOMTY_OBJECT: {
            pk_errors(lisp, pk_str("pk_atom_clone(), TODO: thing about object cloning"));
            return lisp->cache.nil;
        }
        default: {
            pk_errors(lisp, pk_str("pk_atom_clone(), cannot clone this type, TODO: better error message for this"));
            return lisp->cache.nil;
        }
    }
}

typedef struct PKAtomCloneData_ {
    PKHashmap *memo;
    PKAtom *atom;
} PKAtomCloneData;

void pk_atom_clone_body(void *user_closure, Pocket lisp) {
    PKAtomCloneData *data = (PKAtomCloneData *)user_closure;
    pk_stack_push(lisp, pk_atom_clone_rec_(lisp, data->memo, data->atom));
}

PKAtom *pk_atom_clone(Pocket lisp, PKAtom *atom) {
    PKHashmap memo = PK_HASHMAP_EMPTY;
    PKAtomCloneData data = (PKAtomCloneData) {
        .memo = &memo,
        .atom = atom,
    };

    PK_RES result = pk_protect(lisp, &data, pk_atom_clone_body, 0);
    pk_hashmap_deinit(lisp, &memo);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }

    PKAtom *out = pk_stack_head_pop(lisp);
    return out;
}

bool pk_atom_is_circular_rec_(Pocket lisp, PKHashmap *visited, PKAtom *atom) {
    if (pk_atom_kind(atom) != PK_ATOMKIND_OBJECT) {
        return false;
    }

    PKAtom *state = pk_hashmap_get(lisp, visited, atom);
    if (!pk_atom_is_undefined(state)) {
        return !pk_atom_is_true(state);
    }

    pk_hashmap_insert(lisp, visited, atom, lisp->cache.nil, PK_HASHMAP_INSERT_NORMAL);
    bool result = false;

    switch (atom->header.type) {
        case PK_ATOMTY_CONS: {
            result = pk_atom_is_circular_rec_(lisp, visited, atom->cons.car)
                || pk_atom_is_circular_rec_(lisp, visited, atom->cons.cdr);
            break;
        }
        case PK_ATOMTY_ARRAY: {
            PKArray *array = &atom->array.atoms;
            for (size_t i = 0; !result && (i < array->count); ++i) {
                result = pk_atom_is_circular_rec_(lisp, visited, array->e[i]);
            }
            break;
        }
        case PK_ATOMTY_OBJECT: {
            pk_errors(lisp, pk_str("pk_atom_is_circular(): TODO: implement object is circular logic"));
            break;
        }
        default: break;
    }

    pk_hashmap_insert(lisp, visited, atom, lisp->cache.atom_true, PK_HASHMAP_INSERT_NORMAL);
    return result;
}

typedef struct PKAtomCircularData_ {
    PKHashmap *visited;
    PKAtom *atom;
} PKAtomCircularData;

void pk_atom_is_circular_body(void *user_closure, Pocket lisp) {
    PKAtomCircularData *data = (PKAtomCircularData *)user_closure;
    bool result = pk_atom_is_circular_rec_(lisp, data->visited, data->atom);
    pk_stack_push(lisp, result ? (PKAtom *)lisp->cache.atom_true : lisp->cache.nil);
}

bool pk_atom_is_circular(Pocket lisp, PKAtom *atom) {
    PKHashmap visited = PK_HASHMAP_EMPTY;
    PKAtomCircularData data = (PKAtomCircularData) {
        .visited = &visited,
        .atom = atom,
    };

    PK_RES result = pk_protect(lisp, &data, pk_atom_is_circular_body, 0);
    pk_hashmap_deinit(lisp, &visited);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }

    PKAtom *out = pk_stack_head_pop(lisp);
    return pk_atom_is_true(out);
}

bool pk_atom_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    (void)lisp;
    if (lhs == rhs) {
        return true;
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

bool pk_atom_text_qeq(Pocket lisp, PKAtom *text, PKStr cmp) {
    (void)lisp;
    switch (text->header.type) {
        case PK_ATOMTY_STRING: return pk_str_eq(text->string.str, cmp);
        case PK_ATOMTY_KEYWORD: return pk_str_eq(text->keyword.id, cmp);
        case PK_ATOMTY_SYMBOL: return pk_str_eq(text->symbol.name->str, cmp);
        default: return false;
    }
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

PK_TYPE pk_atom_typeof(PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_DESTROYED: return PK_TYPE_DESTROYED;
        case PK_ATOMTY_NIL: return PK_TYPE_NIL;
        case PK_ATOMTY_TRUE: return PK_TYPE_TRUE;
        case PK_ATOMTY_NUMBER: return PK_TYPE_NUMBER;
        case PK_ATOMTY_STRING: return PK_TYPE_STRING;
        case PK_ATOMTY_SYMBOL: return PK_TYPE_SYMBOL;
        case PK_ATOMTY_KEYWORD: return PK_TYPE_KEYWORD;
        case PK_ATOMTY_CONS: return PK_TYPE_CONS;
        case PK_ATOMTY_ARRAY: return PK_TYPE_ARRAY;
        case PK_ATOMTY_OBJECT: return PK_TYPE_OBJECT;
        case PK_ATOMTY_CFUN: return PK_TYPE_FUNCTION;
        case PK_ATOMTY_LFUN: return PK_TYPE_FUNCTION;
        case PK_ATOMTY_CMACRO: return PK_TYPE_MACRO;
        case PK_ATOMTY_LMACRO: return PK_TYPE_MACRO;
        default: return PK_TYPE_UNKNOWN;
    }
}

bool pk_atom_is_type(PKAtom *atom, PK_TYPE type) {
    return pk_atom_typeof(atom) == type;
}
