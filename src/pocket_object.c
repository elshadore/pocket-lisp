#include "pocket_internals.h"

PKAtom pk_object_new(PKAtom type) {
    pk_propagate(pk_typecheck_symbol(type));

    switch (type->symbol.type_type) {
        case PK_TYPETY_LISP: {
            size_t length = type->symbol.slots.count;
            PKAtom *e = (PKAtom *)pk_malloc(length * sizeof(PKAtom));
            if (e == NULL) {
                return pk_error_oom();
            }
            for (size_t i = 0; i < length; ++i) {
                e[i] = pk_nil();
            }

            PKAtom atom = pk_atom_alloc();
            pk_propagate(atom);

            pk_propagate(pk_ref_inc(type));
            atom->header.type = PK_ATOMTY_LOBJECT;
            atom->lobject.type = type;
            atom->lobject.slots = (PKAtoms) {
                .e = e,
                .length = length,
            };

            return atom;
        }
        case PK_TYPETY_C_OWNED: {
            void *data = pk_malloc(type->symbol.owned_memory);
            if (data == NULL) {
                return pk_error_oom();
            }
            memset(data, 0, type->symbol.owned_memory);

            PKAtom atom = pk_atom_alloc();
            if (pk_is_throwing(atom)) {
                pk_free(data);
                return atom;
            }

            pk_propagate(pk_ref_inc(type));
            atom->header.type = PK_ATOMTY_COBJECT;
            atom->cobject.type = type;
            atom->cobject.data = data;

            return atom;
        }
        case PK_TYPETY_C_USER: {
            PKAtom atom = pk_atom_alloc();
            pk_propagate(atom);

            pk_propagate(pk_ref_inc(type));
            atom->header.type = PK_ATOMTY_COBJECT;
            atom->cobject.type = type;
            atom->cobject.data = NULL;

            return atom;
        }
        default: {
            return pk_error_message(pk_str("not a type: "), type);
        }
    }
}

PKAtom pk_slot_index(PKAtom object, PKAtom slot, size_t *index) {
    if (!pk_slotmap_index_of(&object->lobject.type->symbol.slots, slot, index)) {
        return pk_error_message(pk_str("no such slot: "), slot);
    }
    return pk_true();
}

PKAtom pk_slot_set(PKAtom object, PKAtom slot, PKAtom value) {
    pk_propagate(pk_typecheck_lobject(object));
    pk_propagate(pk_typecheck_symbol(slot));

    size_t index = 0;
    pk_propagate(pk_slot_index(object, slot, &index));

    pk_propagate(pk_ref_inc(value));

    PKAtom result = object->lobject.slots.e[index];
    object->lobject.slots.e[index] = value;
    return result;
}

PKAtom pk_slot_get(PKAtom object, PKAtom slot) {
    pk_propagate(pk_typecheck_lobject(object));
    pk_propagate(pk_typecheck_symbol(slot));

    size_t index = 0;
    pk_propagate(pk_slot_index(object, slot, &index));

    pk_propagate(pk_ref_inc(object->lobject.slots.e[index]));

    return object->lobject.slots.e[index];
}

void *pk_cobject_data(PKAtom atom) {
    if (atom->header.type != PK_ATOMTY_COBJECT) {
        return NULL;
    } else {
        return atom->cobject.data;
    }
}

size_t pk_cobject_size(PKAtom atom) {
    if (atom->header.type != PK_ATOMTY_COBJECT) {
        return 0;
    } else {
        return atom->cobject.type->symbol.owned_memory;
    }
}

bool pk_is_object(PKAtom object) {
    return pk_is_type(object, PK_TYPE_OBJECT);
}

bool pk_is_cobject(PKAtom object) {
    return pk_is_type(object, PK_TYPE_COBJECT);
}

bool pk_is_lobject(PKAtom object) {
    return pk_is_type(object, PK_TYPE_LOBJECT);
}

PKAtom pk_typecheck_object(PKAtom object) {
    return pk_typecheck(object, PK_TYPE_OBJECT);
}

PKAtom pk_typecheck_cobject(PKAtom object) {
    return pk_typecheck(object, PK_TYPE_COBJECT);
}

PKAtom pk_typecheck_lobject(PKAtom object) {
    return pk_typecheck(object, PK_TYPE_LOBJECT);
}
