#include "pocket_internals.h"

PKAtomNumber *pk_atom_int(Pocket lisp, int integer) {
    if ((integer >= 0) && (integer < PK_ATOM_NUMBER_CACHE_LENGTH)) {
        return &PK_ATOM_NUMBER_CACHE[integer];
    }

    PKAtomNumber *atom = (PKAtomNumber *)pk_atom_alloc(lisp);
    atom->header.type = PK_ATOMTY_NUMBER;
    atom->num.num_type = PK_NUMTY_INT;
    atom->num.as.i = integer;
    return atom;
}

PKAtomNumber *pk_atom_size_t(Pocket lisp, size_t value) {
    if (value > INT_MAX) {
        pk_errorf(lisp, "cannot create an integer of size: %zu", value);
    }
    return pk_atom_int(lisp, (int)value);
}
    
PKAtomNumber *pk_atom_float(Pocket lisp, float floater) {
    PKAtomNumber *atom = (PKAtomNumber *)pk_atom_alloc(lisp);
    atom->header.type = PK_ATOMTY_NUMBER;
    atom->num.num_type = PK_NUMTY_FLOAT;
    atom->num.as.f = floater;
    return atom;
}

PKAtomNumber *pk_atom_byte(Pocket lisp, pk_u8 byte) {
    return pk_atom_int(lisp, (int)byte);
}

PKBlob pk_atom_number_to_bytes(PKAtomNumber *number) {
    switch (number->num.num_type) {
        case PK_NUMTY_INT: {
            return (PKBlob) {
                .e = (pk_u8 *)&number->num.as.i,
                .length = sizeof(number->num.as.i),
            };
        }
        case PK_NUMTY_FLOAT: {
            return (PKBlob) {
                .e = (pk_u8 *)&number->num.as.f,
                .length = sizeof(number->num.as.f),
            };
        }
        default: {
            return PK_BLOB_EMPTY;
        }
    }
}

void pk_atom_number_to_bytes_trimmed_writer(Pocket lisp, PKAtomNumber *number) {
    switch (number->num.num_type) {
        case PK_NUMTY_INT: {
            pk_u32 bits = (pk_u32)number->num.as.i;
            size_t length = sizeof(bits);

            while ((length > 1) && (((bits >> ((length - 1) * 8)) & 0xff) == 0)) {
                --length;
            }

            for (size_t i = length; i > 0; --i) {
                pk_writer_byte(lisp, (pk_u8)(bits >> ((i - 1) * 8)));
            }
            break;
        }
        default: {
            pk_errors(lisp, pk_str("pk_atom_number_to_bytes_trimmed: floats are not supported"));
            break;
        }
    }
}

PKAtomBlob *pk_atom_number_to_bytes_trimmed(Pocket lisp, PKAtomNumber *number) {
    pk_writer_reset(lisp);
    pk_atom_number_to_bytes_trimmed_writer(lisp, number);
    PKAtomBlob *result = pk_atom_blob(lisp, pk_writer_slice(lisp));
    pk_writer_reset(lisp);
    return result;
}

bool pk_atom_is_number(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_NUMBER;
}

bool pk_atom_is_int(PKAtom *atom) {
    if (!pk_atom_is_number(atom)) return false;
    return atom->number.num.num_type == PK_NUMTY_INT;
}

bool pk_atom_is_byte(PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_BLOB: {
            return atom->blob.bytes.length == 1;
        }
        case PK_ATOMTY_NUMBER: {
            if (!pk_atom_is_int(atom)) {
                return false;
            }
            int i = atom->number.num.as.i;
            return i >= 0 && i <= UCHAR_MAX;
        }
        default: return false;
    }
}

bool pk_atom_is_float(PKAtom *atom) {
    if (!pk_atom_is_number(atom)) return false;
    return atom->number.num.num_type == PK_NUMTY_FLOAT;
}

PKAtomNumber *pk_atom_assert_number(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_number(atom)) {
        PKStr ty = pk_ident_atomty(atom->header.type);
        pk_errorf(lisp, "pk_atom_assert_number: expected a number, got %.*s", (int)ty.length, ty.c);
    }
    return (PKAtomNumber *)atom;
}
