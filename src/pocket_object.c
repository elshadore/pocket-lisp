#include "pocket_internals.h"

PKAtomObject *pk_atom_object(Pocket lisp, PKAtomSymbol *type, void *user_data, PKGCFun gc) {
    PKAtomObject *result = (PKAtomObject *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_OBJECT;
    result->type = type;
    result->user_data = user_data;
    result->gc = gc;
    result->props = PK_HASHMAP_EMPTY;
    
    return result;
}

PKAtomObject *pk_atom_object_cons(Pocket lisp, PKAtomSymbol *type, void *user_data, PKGCFun gc, PKAtoms init) {
    if ((init.length % 2) != 0) {
        pk_errors(lisp, pk_str("object construction must be a list of key value pairs"));
    }
    PKAtomObject *result = pk_atom_object(lisp, type, user_data, gc);
    
    size_t half = init.length / 2;
    for (size_t i = 0; i < half; ++i) {
        PKAtom *key = init.e[i * 2];
        PKAtom *value = init.e[(i * 2) + 1];
        pk_atom_object_put(lisp, result, key, value);
    }
    
    return result;
}

void *pk_atom_object_data(Pocket lisp, PKAtomObject *object, PKAtom *typecheck) {
    if (!pk_atom_is_nil(typecheck)) {
        PKAtomSymbol *type = pk_atom_assert_symbol(lisp, typecheck);
        if (type != object->type) {
            pk_errors(lisp, pk_str("object typecheck failed"));
        }
    }
    if (object->user_data == NULL) {
        pk_errors(lisp, pk_str("object has no userdata"));
    }
    return object->user_data;
}

PKAtom *pk_atom_object_put(Pocket lisp, PKAtomObject *object, PKAtom *key, PKAtom *value) {
    if (pk_atom_is_nil(value)) {
        (void)pk_hashmap_remove(lisp, &object->props, key);
    } else {
        (void)pk_hashmap_insert(lisp, &object->props, key, value, PK_HASHMAP_INSERT_NORMAL);
    }
    return (PKAtom *)object;
}

PKAtom *pk_atom_object_remove(Pocket lisp, PKAtomObject *object, PKAtom *key) {
    PKAtom *result = pk_hashmap_remove(lisp, &object->props, key);
    if (pk_atom_is_undefined(result)) {
        return lisp->cache.nil;
    } else {
        return result;
    }
}

PKAtom *pk_atom_object_get(Pocket lisp, PKAtomObject *object, PKAtom *key) {
    PKAtom *result = pk_hashmap_get(lisp, &object->props, key);
    if (pk_atom_is_undefined(result)) {
        return lisp->cache.nil;
    } else {
        return result;
    }
}

bool pk_atom_object_is_property(Pocket lisp, PKAtomObject *object, PKAtom *key) {
    PKAtom *result = pk_hashmap_get(lisp, &object->props, key);
    if (pk_atom_is_undefined(result)) {
        return false;
    } else {
        return true;
    }
}

bool pk_atom_is_object(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_OBJECT;
}

PKAtomObject *pk_atom_assert_object(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_object(atom)) {
        return (PKAtomObject *)atom;
    } else {
        pk_error_type(lisp, atom, lisp->cache.type_object);
        return NULL;
    }
}

size_t pk_atom_object_length(Pocket lisp, PKAtomObject *object) {
    (void)lisp;
    return object->props.count;
}

PKAtom *pk_atom_object_to_plist(Pocket lisp, PKAtomObject *object) {
    return pk_hashmap_to_plist(lisp, &object->props);
}
PKAtom *pk_atom_object_to_alist(Pocket lisp, PKAtomObject *object) {
    return pk_hashmap_to_alist(lisp, &object->props);
}
