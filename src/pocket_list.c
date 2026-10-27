#include "pocket_internals.h"

PKAtomCons *pk_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr) {
    PKAtomCons *result = (PKAtomCons *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_CONS;
    result->car = car;
    result->cdr = cdr;
    
    return result;
}

PKAtom *pk_atom_car(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_NIL: return lisp->cache.nil;
        case PK_ATOMTY_CONS: return atom->cons.car;
        default:{
            PKStr ty = pk_ident_atomty(atom->header.type);
            pk_errorf(lisp, "pk_atom_car: expected a cons or nil, got %.*s", (int)ty.length, ty.c);
            return lisp->cache.nil;
        }
    }
}

PKAtom *pk_atom_cdr(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_NIL: return lisp->cache.nil;
        case PK_ATOMTY_CONS: return atom->cons.cdr;
        default:{
            PKStr ty = pk_ident_atomty(atom->header.type);
            pk_errorf(lisp, "pk_atom_cdr: expected a cons or nil, got %.*s", (int)ty.length, ty.c);
            return lisp->cache.nil;
        }
    }
}

PKAtom *pk_atom_car_strict(Pocket lisp, PKAtom *atom) {
    PKAtomCons *cons = pk_atom_assert_cons(lisp, atom);
    return cons->car;
}

PKAtom *pk_atom_cdr_strict(Pocket lisp, PKAtom *atom) {
    PKAtomCons *cons = pk_atom_assert_cons(lisp, atom);
    return cons->cdr;
}

PKAtom *pk_atom_set_car(Pocket lisp, PKAtom *atom, PKAtom *new_car) {
    PKAtomCons *cons = pk_atom_assert_cons(lisp, atom);
    PKAtom *result = cons->car;
    cons->car = new_car;
    return result;
}

PKAtom *pk_atom_set_cdr(Pocket lisp, PKAtom *atom, PKAtom *new_cdr) {
    PKAtomCons *cons = pk_atom_assert_cons(lisp, atom);
    PKAtom *result = cons->cdr;
    cons->cdr = new_cdr;
    return result;
}

PKAtomCons *pk_atom_list1(Pocket lisp, PKAtom *first) {
    return pk_atom_cons(lisp, first, lisp->cache.nil);
}
    
PKAtomCons *pk_atom_list2(Pocket lisp, PKAtom *first, PKAtom *second) {
    PKAtomCons *b = pk_atom_cons(lisp, second, lisp->cache.nil);
    PKAtomCons *a = pk_atom_cons(lisp, first, (PKAtom *)b);
    return a;
}

PKAtomCons *pk_atom_list3(Pocket lisp, PKAtom *first, PKAtom *second, PKAtom *third) {
    PKAtomCons *c = pk_atom_cons(lisp, third, lisp->cache.nil);
    PKAtomCons *b = pk_atom_cons(lisp, second, (PKAtom *)c);
    PKAtomCons *a = pk_atom_cons(lisp, first, (PKAtom *)b);
    return a;
}

PKAtom *pk_atom_list(Pocket lisp, PKAtoms atoms) {
    if (atoms.length == 0) {
        return lisp->cache.nil;
    }
    
    PKAccum acc = pk_accum_new(lisp);
    for (size_t i = 0; i < atoms.length; ++i) {
        pk_accum_push(&acc, atoms.e[i]);
    }
    
    return acc.head;
}

PKAtom *pk_atom_list_rev(Pocket lisp, PKAtoms atoms) {
    if (atoms.length == 0) {
        return lisp->cache.nil;
    }
    
    PKAccum acc = pk_accum_new(lisp);
    for (size_t i = 0; i < atoms.length; ++i) {
        size_t index = atoms.length - i - 1;
        pk_accum_push(&acc, atoms.e[index]);
    }
    
    return acc.head;
}

PKAtom *pk_atom_list_packed(Pocket lisp, PKAtoms atoms) {
    if (atoms.length == 0) {
        return lisp->cache.nil;
    }
    
    PKAccum acc = pk_accum_new(lisp);
    for (size_t i = 0; i < (atoms.length - 1); ++i) {
        pk_accum_push(&acc, atoms.e[i]);
    }
    if (atoms.length > 1) {
        acc.tail->cons.cdr = atoms.e[atoms.length - 1];
    }
    return acc.head;
}

PKAtom *pk_atom_list_rev_packed(Pocket lisp, PKAtoms atoms) {
    if (atoms.length == 0) {
        return lisp->cache.nil;
    }
    
    PKAccum acc = pk_accum_new(lisp);
    for (size_t i = 0; i < (atoms.length - 1); ++i) {
        size_t index = atoms.length - i - 1;
        pk_accum_push(&acc, atoms.e[index]);
    }
    if (atoms.length > 1) {
        acc.tail->cons.cdr = atoms.e[0];
    }
    return acc.head;
}

PKAtom *pk_atom_list_flatten(Pocket lisp, PKAtoms lists) {
    if (lists.length == 0) {
        return lisp->cache.nil;
    }
    
    PKAccum acc = pk_accum_new(lisp);
    for (size_t i = 0; i < lists.length; ++i) {
        PKAtom *atom = lists.e[i];
        switch (atom->header.type) {
            case PK_ATOMTY_NIL: {
                continue;
            }
            case PK_ATOMTY_CONS: {
                PKAtom *iter = atom;
                while (pk_atom_is_cons(iter)) {
                    pk_accum_push(&acc, iter->cons.car);
                    iter = iter->cons.cdr;
                }
                break;
            }
            default: {
                PKStr ty = pk_ident_atomty(atom->header.type);
                pk_errorf(lisp, "pk_atom_list_flatten: expected a list, got %.*s", (int)ty.length, ty.c);
                break;
            }
        }
    }
    return acc.head;
}

PKAtom *pk_atom_to_list_bytes(Pocket lisp, PKBlob bytes) {
    PKAccum acc = pk_accum_new(lisp);
    for (size_t i = 0; i < bytes.length; ++i) {
        pk_accum_push(&acc, (PKAtom *)pk_atom_byte(lisp, bytes.e[i]));
    }
    return acc.head;
}

PKAtom *pk_atom_to_list_chars(Pocket lisp, PKStr string) {
    PKAccum acc = pk_accum_new(lisp);
    for (size_t i = 0; i < string.length; ++i) {
        pk_accum_push(&acc, (PKAtom *)pk_atom_char(lisp, string.c[i]));
    }
    return acc.head;
}

PKAtom *pk_atom_to_list_table(Pocket lisp, PKHashmap *map) {
    PKAccum acc = pk_accum_new(lisp);
    for (size_t i = 0; i < map->capacity; ++i) {
        PKHashmapSlot *slot = &map->e[i];
        if (slot->state == PK_HASHMAP_SLOT_OCCUPIED) {
            pk_accum_push(&acc, (PKAtom *)pk_atom_cons(lisp, slot->key, slot->value));
        }
    }
    return acc.head;
}

PKAtom *pk_atom_to_list(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_NIL: {
            return lisp->cache.nil;
        }
        case PK_ATOMTY_CONS: {
            return atom;
        }
        case PK_ATOMTY_ARRAY: {
            PKAtoms atoms = pk_array_slice(&atom->array.atoms);
            return pk_atom_list(lisp, atoms);
        }
        case PK_ATOMTY_STRING: {
            PKStr str = atom->string.str;
            return pk_atom_to_list_chars(lisp, str);
        }
        case PK_ATOMTY_STRING_ARRAY: {
            PKStr str = pk_writer_slice(&atom->string_array.string);
            return pk_atom_to_list_chars(lisp, str);
        }
        case PK_ATOMTY_TABLE: {
            PKHashmap *map = &atom->table.map;
            return pk_atom_to_list_table(lisp, map);
        }
        case PK_ATOMTY_BLOB: {
            PKBlob bytes = atom->blob.bytes;
            return pk_atom_to_list_bytes(lisp, bytes);
        }
        case PK_ATOMTY_BYTE_ARRAY: {
            PKBlob bytes = pk_bytes_slice(&atom->byte_array.bytes);
            return pk_atom_to_list_bytes(lisp, bytes);
        }
        default: {
            PKStr ty = pk_ident_atomty(atom->header.type);
            pk_errorf(lisp, "pk_atom_to_list: unsupported type %.*s", (int)ty.length, ty.c);
            return lisp->cache.nil;
        }
    }
}

void pk_atom_list_to_array(Pocket lisp, PKAtom *list, PKArray *array) {
    PKAtom *iter = list;
    while (pk_atom_is_cons(iter)) {
        pk_array_push(lisp, array, iter->cons.car, 8);
        iter = iter->cons.cdr;
    }
}

bool pk_atom_is_nil(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_NIL;
}

bool pk_atom_is_cons(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_CONS;
}

PKAtom *pk_atom_assert_nil(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_nil(atom)) {
        return atom;
    } else {
        PKStr ty = pk_ident_atomty(atom->header.type);
        pk_errorf(lisp, "pk_atom_assert_nil: expected nil, got %.*s", (int)ty.length, ty.c);
        return atom;
    }
}

PKAtomCons *pk_atom_assert_cons(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_cons(atom)) {
        return (PKAtomCons *)atom;
    } else {
        PKStr ty = pk_ident_atomty(atom->header.type);
        pk_errorf(lisp, "pk_atom_assert_cons: expected a cons, got %.*s", (int)ty.length, ty.c);
        return NULL;
    }
}
