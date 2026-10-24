#include "pocket_internals.h"

PKAtom *pk_atom_object_new(Pocket lisp, PKAtomSymbol *type) {
    PKAtom *atom = pk_atom_alloc(lisp);

    switch (type->type_type) {
        case PK_TYPETY_LISP: {
            size_t length = type->slots.count;
            PKAtom **e = (PKAtom **)pk_malloc(lisp, length * sizeof(PKAtom *));
            for (size_t i = 0; i < length; ++i) {
                e[i] = lisp->cache.nil;
            }

            atom->header.type = PK_ATOMTY_LOBJECT;
            atom->lobject.type = type;
            atom->lobject.slots = (PKAtoms) {
                .e = e,
                .length = length,
            };
            break;
        }
        case PK_TYPETY_C_OWNED: {
            void *data = pk_malloc(lisp, type->owned_memory);
            memset(data, 0, type->owned_memory);

            atom->header.type = PK_ATOMTY_COBJECT;
            atom->cobject.type = type;
            atom->cobject.data = data;
            break;
        }
        case PK_TYPETY_C_USER: {
            atom->header.type = PK_ATOMTY_COBJECT;
            atom->cobject.type = type;
            atom->cobject.data = NULL;
            break;
        }
        default: {
            PKWriter w = PK_WRITER_EMPTY;
            pk_writer_cstr(lisp, &w, "not a type: ");
            pk_writer_atom(lisp, &w, (PKAtom *)type);
            pk_error_writer(lisp, &w);
            return lisp->cache.nil;
        }
    }

    return atom;
}

size_t pk_atom_slot_index(Pocket lisp, PKAtomLObject *object, PKAtomSymbol *slot) {
    PKSlotmapSlot *found = pk_slotmap_get(lisp, &object->type->slots, (PKAtom *)slot);
    if (found == NULL) {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_cstr(lisp, &w, "no such slot: ");
        pk_writer_atom(lisp, &w, (PKAtom *)slot);
        pk_error_writer(lisp, &w);
        return 0;
    }
    return found->index;
}

PKAtom *pk_atom_slot_set(Pocket lisp, PKAtomLObject *object, PKAtomSymbol *slot, PKAtom *value) {
    size_t index = pk_atom_slot_index(lisp, object, slot);
    PKAtom *result = object->slots.e[index];
    object->slots.e[index] = value;
    return result;
}

PKAtom *pk_atom_slot_get(Pocket lisp, PKAtomLObject *object, PKAtomSymbol *slot) {
    size_t index = pk_atom_slot_index(lisp, object, slot);
    return object->slots.e[index];
}

void *pk_atom_cobject_data(Pocket lisp, PKAtomCObject *object) {
    (void)lisp;
    return object->data;
}

size_t pk_atom_cobject_size(Pocket lisp, PKAtomCObject *object) {
    (void)lisp;
    return object->type->owned_memory;
}

bool pk_atom_is_object(PKAtom *atom) {
    return (atom->header.type == PK_ATOMTY_LOBJECT) || (atom->header.type == PK_ATOMTY_COBJECT);
}

bool pk_atom_is_cobject(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_COBJECT;
}

bool pk_atom_is_lobject(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_LOBJECT;
}

PKAtomCObject *pk_atom_assert_cobject(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_lobject(atom)) {
        return (PKAtomCObject *)atom;
    } else {
        pk_error_type(lisp, PK_TYPE_LOBJECT, atom);
        return (PKAtomCObject *)lisp->cache.nil;
    }
}

PKAtomLObject *pk_atom_assert_lobject(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_lobject(atom)) {
        return (PKAtomLObject *)atom;
    } else {
        pk_error_type(lisp, PK_TYPE_LOBJECT, atom);
        return (PKAtomLObject *)lisp->cache.nil;
    }
}

