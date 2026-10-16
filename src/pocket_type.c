#include "pocket_internals.h"

static PKAtom pk_error_message_(PKStr prefix, PKAtom atom) {
    PKWriter w = PK_WRITER_EMPTY;
    pk_propagate(pk_writer_string(&w, prefix));
    pk_propagate(pk_writer_atom(&w, atom));
    PKAtom message = pk_string(pk_writer_slice(&w));
    pk_writer_deinit(&w);
    pk_propagate(message);
    PKAtom error = pk_error(message);
    pk_ref_dec(message);
    return error;
}

static PKAtom pk_typecheck_new_type(PKAtom symbol) {
    pk_propagate(pk_typecheck_symbol(symbol));
    if (symbol->symbol.type_type != PK_TYPETY_NOTYPE) {
        return pk_error_message_(pk_str("type already defined: "), symbol);
    }
    return pk_true();
}

PKAtom pk_deftype_lisp(PKAtom symbol, PKAtoms slots) {
    pk_propagate(pk_typecheck_new_type(symbol));

    for (size_t i = 0; i < slots.length; ++i) {
        pk_propagate(pk_typecheck_symbol(slots.e[i]));
        for (size_t j = 0; j < i; ++j) {
            if (pk_eq(slots.e[i], slots.e[j])) {
                return pk_error_message_(pk_str("slot already defined: "), slots.e[i]);
            }
        }
    }

    pk_propagate(pk_slotmap_reserve(&symbol->symbol.slots, slots.length));

    for (size_t i = 0; i < slots.length; ++i) {
        size_t index = 0;
        pk_propagate(pk_slotmap_insert(&symbol->symbol.slots, slots.e[i], &index));
    }

    symbol->symbol.type_type = PK_TYPETY_LISP;
    symbol->symbol.owned_memory = 0;

    return pk_true();
}

PKAtom pk_deftype_c_owned(PKAtom symbol, size_t size) {
    pk_propagate(pk_typecheck_new_type(symbol));

    symbol->symbol.type_type = PK_TYPETY_C_OWNED;
    symbol->symbol.owned_memory = size;

    return pk_true();
}

PKAtom pk_deftype_c_user(PKAtom symbol) {
    pk_propagate(pk_typecheck_new_type(symbol));

    symbol->symbol.type_type = PK_TYPETY_C_USER;
    symbol->symbol.owned_memory = 0;

    return pk_true();
}

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
                e[i] = pk_undefined();
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
            return pk_error_message_(pk_str("not a type: "), type);
        }
    }
}

static PKAtom pk_typecheck_object(PKAtom object) {
    pk_u8 ty = object->header.type;
    if ((ty != PK_ATOMTY_LOBJECT) && (ty != PK_ATOMTY_COBJECT)) {
        return pk_error_type(PK_TYPE_OBJECT, object);
    }
    return pk_true();
}

static PKAtom pk_typecheck_slots(PKAtom object, PKAtom slot, size_t *index) {
    if (object->header.type != PK_ATOMTY_LOBJECT) {
        return pk_error_message_(pk_str("type has no slots: "), object->cobject.type);
    }
    if (!pk_slotmap_index_of(&object->lobject.type->symbol.slots, slot, index)) {
        return pk_error_message_(pk_str("no such slot: "), slot);
    }
    return pk_true();
}

PKAtom pk_slot_set(PKAtom object, PKAtom slot, PKAtom value) {
    pk_propagate(pk_typecheck_object(object));
    pk_propagate(pk_typecheck_symbol(slot));

    size_t index = 0;
    pk_propagate(pk_typecheck_slots(object, slot, &index));

    pk_propagate(pk_ref_inc(value));

    PKAtom old_value = object->lobject.slots.e[index];
    object->lobject.slots.e[index] = value;
    pk_ref_dec(old_value);

    return pk_true();
}

PKAtom pk_slot_get(PKAtom object, PKAtom slot) {
    pk_propagate(pk_typecheck_object(object));
    pk_propagate(pk_typecheck_symbol(slot));

    size_t index = 0;
    pk_propagate(pk_typecheck_slots(object, slot, &index));

    pk_propagate(pk_ref_inc(object->lobject.slots.e[index]));

    return object->lobject.slots.e[index];
}

void *pk_cobject_data(PKAtom atom) {
    if (atom->header.type != PK_ATOMTY_COBJECT) {
        return NULL;
    }
    return atom->cobject.data;
}

size_t pk_cobject_size(PKAtom atom) {
    if (atom->header.type != PK_ATOMTY_COBJECT) {
        return 0;
    }
    return atom->cobject.type->symbol.owned_memory;
}