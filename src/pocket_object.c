#include "pocket_internals.h"

PKAtom *pk_object_new(Pocket lisp, PKAtom *type) {
    PKAtomSymbol *asymbol = pk_atom_assert_symbol(lisp, type);

    PKAtom *atom = pk_atom_alloc(lisp);

    switch (asymbol->type_type) {
        case PK_TYPETY_LISP: {
            size_t length = asymbol->slots.count;
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
            void *data = pk_malloc(lisp, asymbol->owned_memory);
            memset(data, 0, asymbol->owned_memory);

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
            pk_writer_atom(lisp, &w, type);
            pk_errors(lisp, pk_writer_slice(&w));
            return lisp->cache.nil;
        }
    }

    return atom;
}

static bool pk_assert_lobject(Pocket lisp, PKAtom *atom) {
    if (atom->header.type != PK_ATOMTY_LOBJECT) {
        pk_error_type(lisp, PK_TYPE_LOBJECT, atom);
        return false;
    }
    return true;
}

static size_t pk_slot_index(Pocket lisp, PKAtom *object, PKAtom *slot) {
    PKAtomSymbol *type = pk_atom_assert_symbol(lisp, object->lobject.type);
    pk_atom_assert_symbol(lisp, slot);
    PKSlotmapSlot *found = pk_slotmap_get(lisp, &type->slots, slot);
    if (found == NULL) {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_cstr(lisp, &w, "no such slot: ");
        pk_writer_atom(lisp, &w, slot);
        pk_errors(lisp, pk_writer_slice(&w));
        return 0;
    }
    return found->index;
}

PKAtom *pk_slot_set_(Pocket lisp, PKAtom *object, PKAtom *slot, PKAtom *value) {
    (void)pk_assert_lobject(lisp, object);
    size_t index = pk_slot_index(lisp, object, slot);
    PKAtom *result = object->lobject.slots.e[index];
    object->lobject.slots.e[index] = value;
    return result;
}

PKAtom *pk_slot_get_(Pocket lisp, PKAtom *object, PKAtom *slot) {
    (void)pk_assert_lobject(lisp, object);
    size_t index = pk_slot_index(lisp, object, slot);
    return object->lobject.slots.e[index];
}

void *pk_cobject_data_(Pocket lisp, PKAtom *atom) {
    if (atom->header.type != PK_ATOMTY_COBJECT) {
        PKStr ty = pk_ident_atomty(atom->header.type);
        pk_errorf(lisp, "pk_cobject_data_: expected a cobject, got %.*s", (int)ty.length, ty.c);
        return NULL;
    } else {
        return atom->cobject.data;
    }
}

size_t pk_cobject_size_(Pocket lisp, PKAtom *atom) {
    if (atom->header.type != PK_ATOMTY_COBJECT) {
        PKStr ty = pk_ident_atomty(atom->header.type);
        pk_errorf(lisp, "pk_cobject_size_: expected a cobject, got %.*s", (int)ty.length, ty.c);
        return 0;
    }
    return pk_atom_assert_symbol(lisp, atom->cobject.type)->owned_memory;
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
