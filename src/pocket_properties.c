#include "pocket_internals.h"

PKAtom *pk_atom_property_put(Pocket lisp, PKAtom *object, PKHashmap *props, PKAtom *key, PKAtom *value) {
    if (pk_atom_is_nil(value)) {
        (void)pk_hashmap_remove(lisp, props, key);
    } else {
        (void)pk_hashmap_insert(lisp, props, key, value, PK_HASHMAP_INSERT_NORMAL);
    }
    return object;
}

PKAtom *pk_atom_property_remove(Pocket lisp, PKHashmap *props, PKAtom *key) {
    PKAtom *result = pk_hashmap_remove(lisp, props, key);
    if (pk_atom_is_undefined(result)) {
        return lisp->cache.nil;
    } else {
        return result;
    }
}

PKAtom *pk_atom_property_get(Pocket lisp, PKHashmap *props, PKAtom *key) {
    PKAtom *result = pk_hashmap_get(lisp, props, key);
    if (pk_atom_is_undefined(result)) {
        return lisp->cache.nil;
    } else {
        return result;
    }
}

bool pk_atom_property_is_property(Pocket lisp, PKHashmap *props, PKAtom *key) {
    PKAtom *result = pk_hashmap_get(lisp, props, key);
    if (pk_atom_is_undefined(result)) {
        return false;
    } else {
        return true;
    }
}

PKAtom *pk_atom_property_to_plist(Pocket lisp, PKHashmap *props) {
    return pk_hashmap_to_plist(lisp, props);
}
