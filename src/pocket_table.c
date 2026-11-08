#include "pocket_internals.h"

PKAtomTable *pk_atom_table(Pocket lisp) {
    PKAtomTable *result = (PKAtomTable *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_TABLE;
    result->map = PK_HASHMAP_EMPTY;
    
    return result;
}

PKAtomTable *pk_atom_table_cons(Pocket lisp, PKAtoms init) {
    if ((init.length % 2) != 0) {
        pk_errors(lisp, pk_str("table construction must be a list of key value pairs"));
    }
    PKAtomTable *result = pk_atom_table(lisp);
    
    size_t half = init.length / 2;
    for (size_t i = 0; i < half; ++i) {
        PKAtom *key = init.e[i * 2];
        PKAtom *value = init.e[(i * 2) + 1];
        pk_atom_table_put(lisp, result, key, value);
    }
    
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

PKAtom *pk_atom_table_rem(Pocket lisp, PKAtomTable *table, PKAtom *key) {
    PKAtom *result = pk_hashmap_remove(lisp, &table->map, key);
    if (pk_atom_is_undefined(result)) {
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
        pk_error_type(lisp, atom, lisp->cache.type_table);
        return NULL;
    }
}

size_t pk_atom_table_length(Pocket lisp, PKAtomTable *table) {
    (void)lisp;
    return table->map.count;
}

PKAtom *pk_atom_table_to_list(Pocket lisp, PKAtomTable *table) {
    return pk_atom_to_list_table(lisp, &table->map);
}
