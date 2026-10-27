#include "pocket_internals.h"

PKAtomTable * pk_atom_table(Pocket lisp) {
    PKAtomTable *result = (PKAtomTable *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_TABLE;
    result->map = PK_HASHMAP_EMPTY;
    
    return result;
}

PKAtom *pk_atom_table_put(Pocket lisp, PKAtomTable *table, PKAtom *key, PKAtom *value) {
    PKAtom *result = pk_hashmap_insert(lisp, &table->map, key, value, PK_HASHMAP_INSERT_NORMAL);
    if (pk_atom_is_undefined(result)) {
        return lisp->cache.nil;
    } else {
        return result;
    }
}

PKAtom *pk_atom_table_put_unique(Pocket lisp, PKAtomTable *table, PKAtom *key, PKAtom *value) {
    PKAtom *result = pk_hashmap_insert(lisp, &table->map, key, value, PK_HASHMAP_INSERT_UNIQUE);
    if (pk_atom_is_undefined(result)) {
        return lisp->cache.nil;
    } else {
        return result;
    }
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

PKAtom *pk_atom_table_find_strict(Pocket lisp, PKAtomTable *table, PKAtom *key) {
    PKAtom *result = pk_hashmap_get(lisp, &table->map, key);
    if (pk_atom_is_undefined(result)) {
        pk_errors(lisp, pk_str("key was not defined in table"));
        return lisp->cache.nil;
    } else {
        return result;
    }
}

bool pk_atom_table_is_member(Pocket lisp, PKAtomTable *table, PKAtom *key) {
    PKAtom *result = pk_hashmap_get(lisp, &table->map, key);
    if (pk_atom_is_undefined(result)) {
        return false;
    } else {
        return true;
    }
}

bool pk_atom_is_table(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_TABLE;
}

PKAtomTable *pk_atom_assert_table(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_table(atom)) {
        return (PKAtomTable *)atom;
    } else {
        pk_error_type(lisp, PK_TYPE_TABLE, atom);
        return NULL;
    }
}
