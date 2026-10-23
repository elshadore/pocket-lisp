#include "pocket_internals.h"

PKAtomTable * pk_atom_table(Pocket lisp) {
    PKAtomTable *result = (PKAtomTable *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_TABLE;
    result->map = PK_HASHMAP_EMPTY;
    
    return result;
}

PKAtom *pk_atom_table_put(Pocket lisp, PKAtomTable *table, PKAtom *key, PKAtom *value) {
    return pk_hashmap_insert(lisp, &table->map, key, value, PK_HASHMAP_INSERT_NORMAL);
}

PKAtom *pk_atom_table_put_unique(Pocket lisp, PKAtomTable *table, PKAtom *key, PKAtom *value) {
    return pk_hashmap_insert(lisp, &table->map, key, value, PK_HASHMAP_INSERT_UNIQUE);
}

PKAtom *pk_atom_table_rem(Pocket lisp, PKAtomTable *table, PKAtom *key) {
    PKAtom *result = pk_hashmap_remove(lisp, &table->map, key);
    if (pk_atom_is_undefined(result)) {
        return lisp->cache.nil;
    } else {
        return result;
    }
}

PKAtom *pk_atom_table_rem_strict(Pocket lisp, PKAtomTable *table, PKAtom *key) {
    PKAtom *result = pk_hashmap_remove(lisp, &table->map, key);
    if (pk_atom_is_undefined(result)) {
        pk_errors(lisp, pk_str("key was not defined in table"));
        return lisp->cache.nil;
    } else {
        return result;
    }
}

PKAtom *pk_atom_table_find(Pocket lisp, PKAtomTable *table, PKAtom *key) {
    PKAtom *result = pk_hashmap_get(lisp, &table->map, key);
    if (pk_atom_is_undefined(result)) {
        return lisp->cache.nil;
    } else {
        return result;
    }
}

PKAtom *pk_atom_table_find_raw(Pocket lisp, PKAtomTable *table, PKAtom *key) {
    return pk_hashmap_get(lisp, &table->map, key);
}

PKAtom *pk_atom_table_strict(Pocket lisp, PKAtomTable *table, PKAtom *key) {
    PKAtom *result = pk_hashmap_get(lisp, &table->map, key);
    if (pk_atom_is_undefined(result)) {
        pk_errors(lisp, pk_str("key was not defined in table"));
        return lisp->cache.nil;
    } else {
        return result;
    }
}
