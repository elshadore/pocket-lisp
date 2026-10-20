#include "pocket_internals.h"

PKAtomArray *pk_atom_array(Pocket lisp, PKAtoms init) {
    PKAtomArray *result = (PKAtomArray *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_ARRAY;
    result->a = PK_ARRAY_EMPTY;
    
    for (size_t i = 0; i < init.length; ++i) {
        pk_atom_array_append(lisp, result, init.e[i]);
    }
    
    return result;
}

void pk_atom_array_append(Pocket lisp, PKAtomArray *array, PKAtom *atom) {
    pk_array_push(lisp, &array->a, atom, 16);
}

PKAtomStringArray *pk_atom_string_array(Pocket lisp, PKStr init) {
    PKAtomStringArray *result = (PKAtomStringArray *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_ARRAY;
    result->w = PK_WRITER_EMPTY;

    if (init.length > 0) {
        pk_writer_string(lisp, &result->w, init);
    }
    
    return result;
}

void pk_atom_string_array_append_str(Pocket lisp, PKAtomStringArray *array, PKStr string) {
    pk_writer_string(lisp, &array->w, string);
}
    
void pk_atom_string_array_append(Pocket lisp, PKAtomStringArray *array, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            pk_atom_string_array_append_str(lisp, array, atom->string.str);
            break;
        }
        case PK_ATOMTY_STRING_ARRAY: {
            pk_atom_string_array_append_str(lisp, array, pk_writer_slice(&atom->string_array.w));
            break;
        }
        case PK_ATOMTY_CHARACTER: {
            pk_writer_char(lisp, &array->w, atom->character.c);
            break;
        }
        default: {
            pk_errors(lisp, pk_str("expected string type to append to string array"));
            break;
        }
    }
}

PKAtomByteArray *pk_atom_byte_array(Pocket lisp, PKBlob init) {
    PKAtomByteArray *result = (PKAtomByteArray *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_ARRAY;
    result->b = PK_BYTES_EMPTY;

    pk_atom_byte_array_append_blob(lisp, result, init);
    
    return result;
}

void pk_atom_byte_array_append_u8(Pocket lisp, PKAtomByteArray *array, pk_u8 byte) {
    pk_bytes_push(lisp, &array->b, byte, 16);
}

void pk_atom_byte_array_append_blob(Pocket lisp, PKAtomByteArray *array, PKBlob blob) {
    for (size_t i = 0; i < blob.length; ++i) {
        pk_atom_byte_array_append_u8(lisp, array, blob.e[i]);
    }
}

void pk_atom_byte_array_append(Pocket lisp, PKAtomByteArray *array, PKAtom *atom) {
    pk_atom_byte_array_append_u8(lisp, array, pk_atom_to_byte(lisp, atom));
}

void pk_atom_append(Pocket lisp, PKAtom *array, PKAtom *atom) {
    switch (array->header.type) {
        case PK_ATOMTY_ARRAY: {
            pk_atom_array_append(lisp, (PKAtomArray *)array, atom);
            break;
        }
        case PK_ATOMTY_STRING_ARRAY: {
            pk_atom_string_array_append(lisp, (PKAtomStringArray *)array, atom);
            break;
        }
        case PK_ATOMTY_BYTE_ARRAY: {
            pk_atom_byte_array_append(lisp, (PKAtomByteArray *)array, atom);
            break;
        }
        default: {
            pk_error_type(lisp, PK_TYPE_SEQ, array);
            break;
        }
    }
}
