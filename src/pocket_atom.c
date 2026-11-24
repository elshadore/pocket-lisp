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
    (void)lisp;

    if (boolean) {
        return &PK_ATOM_TRUE;
    } else {
        return &PK_ATOM_NIL;
    }
}

bool pk_atom_is_true(PKAtom *atom) {
    return atom->header.type != PK_ATOMTY_NIL;
}

bool pk_atom_is_undefined(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_UNDEFINED;
}

void pk_atom_set_immortal(PKAtom *atom) {
    if (!pk_atom_is_immortal(atom)) {
        atom->header.flags |= PK_ATOMFLAG_IMMORTAL;
    }
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
        case PK_ATOMTY_BLOB: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_CFUN: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_LFUN: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_CMACRO: return PK_ATOMKIND_VALUE;
        case PK_ATOMTY_LMACRO: return PK_ATOMKIND_VALUE;
        default: return PK_ATOMKIND_VALUE;
    }
}

PKAtom *pk_atom_clone_recursive(Pocket lisp, PKAtom *atom) {
    if (pk_atom_kind(atom) == PK_ATOMKIND_VALUE) {
        return atom;
    }

    PKAtom *have = pk_hashmap_get(lisp, &lisp->cloner, atom);
    if (!pk_atom_is_undefined(have)) {
        return have;
    }

    switch (atom->header.type) {
        case PK_ATOMTY_CONS: {
            PKAtomCons *result = pk_atom_cons(lisp, &PK_ATOM_NIL, &PK_ATOM_NIL);
            pk_hashmap_insert(lisp, &lisp->cloner, atom, (PKAtom *)result, PK_HASHMAP_INSERT_NORMAL);
            result->car = pk_atom_clone_recursive(lisp, atom->cons.car);

            PKAtom *iter = atom->cons.cdr;
            PKAtom *current = (PKAtom *)result;
            while (pk_atom_is_cons(iter) && pk_atom_is_undefined(pk_hashmap_get(lisp, &lisp->cloner, iter))) {
                PKAtomCons *next = pk_atom_cons(lisp, &PK_ATOM_NIL, &PK_ATOM_NIL);
                pk_hashmap_insert(lisp, &lisp->cloner, iter, (PKAtom *)next, PK_HASHMAP_INSERT_NORMAL);
                next->car = pk_atom_clone_recursive(lisp, iter->cons.car);
                current->cons.cdr = (PKAtom *)next;
                current = (PKAtom *)next;
                iter = iter->cons.cdr;
            }
            current->cons.cdr = pk_atom_clone_recursive(lisp, iter);
            return (PKAtom *)result;
        }
        case PK_ATOMTY_ARRAY: {
            PKAtomArray *result = pk_atom_array(lisp, PK_ATOMS_EMPTY);
            pk_hashmap_insert(lisp, &lisp->cloner, atom, (PKAtom *)result, PK_HASHMAP_INSERT_NORMAL);
            for (size_t i = 0; i < atom->array.atoms.count; ++i) {
                pk_atom_array_append(lisp, result, pk_atom_clone_recursive(lisp, atom->array.atoms.e[i]));
            }
            return (PKAtom *)result;
        }
        case PK_ATOMTY_OBJECT: {
            PKAtomObject *original = (PKAtomObject *)atom;
            PKAtomObject *result = NULL;
            if (original->reference == PK_TRUE) {
                result = pk_atom_object_reference(lisp, original->as.reference);
            } else if (original->user_data != NULL) {
                result = pk_atom_object_reference(lisp, original);
            } else {
                result = pk_atom_object(lisp, original->as.type, NULL, NULL);
            }
            pk_hashmap_insert(lisp, &lisp->cloner, atom, (PKAtom *)result, PK_HASHMAP_INSERT_NORMAL);
            PKHashmap *props = &original->props;
            for (size_t i = 0; i < props->capacity; ++i) {
                PKHashmapSlot *slot = &props->e[i];
                if (slot->state == PK_HASHMAP_SLOT_OCCUPIED) {
                    (void)pk_atom_object_put(lisp, result, slot->key, pk_atom_clone_recursive(lisp, slot->value));
                }
            }
            return (PKAtom *)result;
        }
        default: {
            pk_errors(lisp, pk_str("pk_atom_clone(), cannot clone this type, TODO: better error message for this"));
            return &PK_ATOM_NIL;
        }
    }
}

PKAtom *pk_atom_clone(Pocket lisp, PKAtom *atom) {
    pk_hashmap_deinit(lisp, &lisp->cloner);
    PKAtom *result = pk_atom_clone_recursive(lisp, atom);
    pk_hashmap_deinit(lisp, &lisp->cloner);
    return result;
}

bool pk_atom_is_circular_recursive(Pocket lisp, PKAtom *atom) {
    if (pk_atom_kind(atom) != PK_ATOMKIND_OBJECT) {
        return false;
    }

    PKAtom *state = pk_hashmap_get(lisp, &lisp->cloner, atom);
    if (!pk_atom_is_undefined(state)) {
        return !pk_atom_is_true(state);
    }

    pk_hashmap_insert(lisp, &lisp->cloner, atom, &PK_ATOM_NIL, PK_HASHMAP_INSERT_NORMAL);
    bool result = false;

    switch (atom->header.type) {
        case PK_ATOMTY_CONS: {
            result = pk_atom_is_circular_recursive(lisp, atom->cons.car)
                || pk_atom_is_circular_recursive(lisp, atom->cons.cdr);
            break;
        }
        case PK_ATOMTY_ARRAY: {
            PKArray *array = &atom->array.atoms;
            for (size_t i = 0; !result && (i < array->count); ++i) {
                result = pk_atom_is_circular_recursive(lisp, array->e[i]);
            }
            break;
        }
        case PK_ATOMTY_OBJECT: {
            PKHashmap *props = &atom->object.props;
            for (size_t i = 0; !result && (i < props->capacity); ++i) {
                PKHashmapSlot *slot = &props->e[i];
                if (slot->state == PK_HASHMAP_SLOT_OCCUPIED) {
                    result = pk_atom_is_circular_recursive(lisp, slot->value);
                }
            }
            break;
        }
        default: break;
    }

    pk_hashmap_insert(lisp, &lisp->cloner, atom, &PK_ATOM_TRUE, PK_HASHMAP_INSERT_NORMAL);
    return result;
}

bool pk_atom_is_circular(Pocket lisp, PKAtom *atom) {
    pk_hashmap_deinit(lisp, &lisp->cloner);
    bool result = pk_atom_is_circular_recursive(lisp, atom);
    pk_hashmap_deinit(lisp, &lisp->cloner);
    return result;
}

bool pk_atom_text(PKAtom *atom, PKStr *text) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            *text = atom->string.str;
            return true;
        }
        case PK_ATOMTY_SYMBOL: {
            *text = atom->symbol.name->str;
            return true;
        }
        case PK_ATOMTY_KEYWORD: {
            *text = atom->keyword.name->str;
            return true;
        }
        default: return false;
    }
}

bool pk_atom_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    (void)lisp;
    if (lhs == rhs) {
        return true;
    }

    // TODO: think about this.
    /* PKStr ltext = PK_STR_EMPTY; */
    /* PKStr rtext = PK_STR_EMPTY; */
    /* if (pk_atom_text(lhs, &ltext) && pk_atom_text(rhs, &rtext)) { */
    /*     return pk_str_eq(ltext, rtext); */
    /* } */

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
        default: return false;
    }
}

bool pk_atom_eq_atoms(Pocket lisp, PKAtoms atoms) {
    if (atoms.length < 2) {
        pk_errorf(lisp, "pk_atom_eq_atoms: expected at least 2 atoms, got %zu", atoms.length);
        return false;
    }

    for (size_t i = 0; i < atoms.length - 1; ++i) {
        if (!pk_atom_eq(lisp, atoms.e[i], atoms.e[i + 1])) {
            return false;
        }
    }

    return true;
}

bool pk_atom_object_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    (void)lisp;
    return lhs == rhs;
}

bool pk_atom_object_eq_atoms(Pocket lisp, PKAtoms atoms) {
    if (atoms.length < 2) {
        pk_errorf(lisp, "pk_atom_object_eq_atoms: expected at least 2 atoms, got %zu", atoms.length);
        return false;
    }

    for (size_t i = 0; i < atoms.length - 1; ++i) {
        if (!pk_atom_object_eq(lisp, atoms.e[i], atoms.e[i + 1])) {
            return false;
        }
    }

    return true;
}

bool pk_atom_struct_eq_recursive(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    if (lhs == rhs) {
        return true;
    }

    if (lhs->header.type != rhs->header.type) {
        return false;
    }

    if (pk_atom_kind(lhs) == PK_ATOMKIND_VALUE) {
        return pk_atom_eq(lisp, lhs, rhs);
    }

    switch (lhs->header.type) {
        case PK_ATOMTY_CONS: {
            PKAtom *seen = pk_hashmap_get(lisp, &lisp->cloner, lhs);
            if (!pk_atom_is_undefined(seen)) {
                return seen == rhs;
            }
            pk_hashmap_insert(lisp, &lisp->cloner, lhs, rhs, PK_HASHMAP_INSERT_NORMAL);
            bool result = pk_atom_struct_eq_recursive(lisp, lhs->cons.car, rhs->cons.car)
                && pk_atom_struct_eq_recursive(lisp, lhs->cons.cdr, rhs->cons.cdr);
            pk_hashmap_remove(lisp, &lisp->cloner, lhs);
            return result;
        }
        case PK_ATOMTY_ARRAY: {
            PKArray *larray = &lhs->array.atoms;
            PKArray *rarray = &rhs->array.atoms;
            if (larray->count != rarray->count) {
                return false;
            }
            PKAtom *seen = pk_hashmap_get(lisp, &lisp->cloner, lhs);
            if (!pk_atom_is_undefined(seen)) {
                return seen == rhs;
            }
            pk_hashmap_insert(lisp, &lisp->cloner, lhs, rhs, PK_HASHMAP_INSERT_NORMAL);
            bool result = true;
            for (size_t i = 0; result && (i < larray->count); ++i) {
                result = pk_atom_struct_eq_recursive(lisp, larray->e[i], rarray->e[i]);
            }
            pk_hashmap_remove(lisp, &lisp->cloner, lhs);
            return result;
        }
        case PK_ATOMTY_OBJECT: {
            PKAtomObject *lobject = (PKAtomObject *)lhs;
            PKAtomObject *robject = (PKAtomObject *)rhs;
            if (pk_atom_object_deref(lobject)->as.type != pk_atom_object_deref(robject)->as.type) {
                return false;
            }
            PKHashmap *lprops = &lobject->props;
            PKHashmap *rprops = &robject->props;
            if (lprops->count != rprops->count) {
                return false;
            }
            PKAtom *seen = pk_hashmap_get(lisp, &lisp->cloner, lhs);
            if (!pk_atom_is_undefined(seen)) {
                return seen == rhs;
            }
            pk_hashmap_insert(lisp, &lisp->cloner, lhs, rhs, PK_HASHMAP_INSERT_NORMAL);
            bool result = true;
            for (size_t i = 0; result && (i < lprops->capacity); ++i) {
                PKHashmapSlot *lslot = &lprops->e[i];
                if (lslot->state != PK_HASHMAP_SLOT_OCCUPIED) {
                    continue;
                }
                bool found = false;
                for (size_t j = 0; !found && (j < rprops->capacity); ++j) {
                    PKHashmapSlot *rslot = &rprops->e[j];
                    if (rslot->state != PK_HASHMAP_SLOT_OCCUPIED) {
                        continue;
                    }
                    if (pk_atom_struct_eq_recursive(lisp, lslot->key, rslot->key)) {
                        found = pk_atom_struct_eq_recursive(lisp, lslot->value, rslot->value);
                    }
                }
                result = found;
            }
            pk_hashmap_remove(lisp, &lisp->cloner, lhs);
            return result;
        }
        default: {
            return false;
        }
    }
}

bool pk_atom_struct_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    pk_hashmap_deinit(lisp, &lisp->cloner);
    bool result = pk_atom_struct_eq_recursive(lisp, lhs, rhs);
    pk_hashmap_deinit(lisp, &lisp->cloner);
    return result;
}

bool pk_atom_struct_eq_atoms(Pocket lisp, PKAtoms atoms) {
    if (atoms.length < 2) {
        pk_errorf(lisp, "pk_atom_struct_eq_atoms: expected at least 2 atoms, got %zu", atoms.length);
        return false;
    }

    for (size_t i = 0; i < atoms.length - 1; ++i) {
        if (!pk_atom_struct_eq(lisp, atoms.e[i], atoms.e[i + 1])) {
            return false;
        }
    }

    return true;
}

bool pk_atom_text_qeq(Pocket lisp, PKAtom *text, PKStr cmp) {
    (void)lisp;
    PKStr string = PK_STR_EMPTY;
    if (pk_atom_text(text, &string)) {
        return pk_str_eq(string, cmp);
    } else {
        return false;
    }
}

size_t pk_atom_hash(PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            return pk_hash_djb2(atom->string.str);
        }
        case PK_ATOMTY_KEYWORD: {
            return pk_hash_djb2(atom->keyword.name->str);
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
        case PK_ATOMTY_BLOB: return PK_TYPE_BLOB;
        default: return PK_TYPE_UNKNOWN;
    }
}

bool pk_atom_is_type(PKAtom *atom, PK_TYPE type) {
    return pk_atom_typeof(atom) == type;
}
