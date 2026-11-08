#include "pocket_internals.h"

PKAtomArray *pk_atom_array(Pocket lisp, PKAtoms init) {
    PKAtomArray *result = (PKAtomArray *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_ARRAY;
    result->atoms = PK_ARRAY_EMPTY;
    
    for (size_t i = 0; i < init.length; ++i) {
        pk_atom_array_append(lisp, result, init.e[i]);
    }
    
    return result;
}

void pk_atom_array_append(Pocket lisp, PKAtomArray *array, PKAtom *atom) {
    pk_array_push(lisp, &array->atoms, atom, 16);
}

PKAtom *pk_atom_array_el(Pocket lisp, PKAtomArray *array, size_t index) {
    PKArray *atoms = &array->atoms;
    if (index >= atoms->count) {
        pk_errorf(lisp, "array index: %zu, is out of bounds, array length is %zu", index, atoms->count);
        return lisp->cache.nil;
    }
    return atoms->e[index];
}

size_t pk_atom_array_length(Pocket lisp, PKAtomArray *array) {
    (void)lisp;
    return array->atoms.count;
}

PKAtomStringArray *pk_atom_string_array(Pocket lisp, PKStr init) {
    PKAtomStringArray *result = (PKAtomStringArray *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_STRING_ARRAY;
    result->string = PK_WRITER_EMPTY;

    if (init.length > 0) {
        pk_writer_string(lisp, &result->string, init);
    }
    
    return result;
}

void pk_atom_string_array_append_str(Pocket lisp, PKAtomStringArray *array, PKStr string) {
    pk_writer_string(lisp, &array->string, string);
}
    
void pk_atom_string_array_append(Pocket lisp, PKAtomStringArray *array, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            pk_atom_string_array_append_str(lisp, array, atom->string.str);
            break;
        }
        case PK_ATOMTY_STRING_ARRAY: {
            pk_atom_string_array_append_str(lisp, array, pk_writer_slice(&atom->string_array.string));
            break;
        }
        case PK_ATOMTY_CHARACTER: {
            pk_writer_char(lisp, &array->string, atom->character.c);
            break;
        }
        default: {
            pk_errors(lisp, pk_str("expected string type to append to string array"));
            break;
        }
    }
}

char pk_atom_string_array_el(Pocket lisp, PKAtomStringArray *array, size_t index) {
    PKStr str = pk_writer_slice(&array->string);
    if (index >= str.length) {
        pk_errorf(lisp, "string array index: %zu, is out of bounds, length is %zu", index, str.length);
        return '\0';
    }
    return str.c[index];
}

size_t pk_atom_string_array_length(Pocket lisp, PKAtomStringArray *array) {
    (void)lisp;
    return array->string.count;
}

PKAtomByteArray *pk_atom_byte_array(Pocket lisp, PKBlob init) {
    PKAtomByteArray *result = (PKAtomByteArray *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_BYTE_ARRAY;
    result->bytes = PK_BYTES_EMPTY;

    pk_atom_byte_array_append_blob(lisp, result, init);
    
    return result;
}

void pk_atom_byte_array_append_u8(Pocket lisp, PKAtomByteArray *array, pk_u8 byte) {
    pk_bytes_push(lisp, &array->bytes, byte, 16);
}

void pk_atom_byte_array_append_blob(Pocket lisp, PKAtomByteArray *array, PKBlob blob) {
    for (size_t i = 0; i < blob.length; ++i) {
        pk_atom_byte_array_append_u8(lisp, array, blob.e[i]);
    }
}

void pk_atom_byte_array_append(Pocket lisp, PKAtomByteArray *array, PKAtom *atom) {
    pk_atom_byte_array_append_u8(lisp, array, pk_atom_to_byte(lisp, atom));
}

pk_u8 pk_atom_byte_array_el(Pocket lisp, PKAtomByteArray *array, size_t index) {
    PKBytes *bytes = &array->bytes;
    if (index >= bytes->count) {
        pk_errorf(lisp, "byte array index: %zu, is out of bounds, length is %zu", index, bytes->count);
        return 0;
    }
    return bytes->e[index];
}

size_t pk_atom_byte_array_length(Pocket lisp, PKAtomByteArray *array) {
    (void)lisp;
    return array->bytes.count;
}

bool pk_atom_is_array(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_ARRAY;
}

bool pk_atom_is_string_array(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_STRING_ARRAY;
}

bool pk_atom_is_byte_array(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_BYTE_ARRAY;
}

PKAtomArray *pk_atom_assert_array(Pocket lisp, PKAtom *atom) {
    pk_atom_assert_type(lisp, atom, lisp->cache.type_array);
    return (PKAtomArray *)atom;
}

PKAtomByteArray *pk_atom_assert_byte_array(Pocket lisp, PKAtom *atom) {
    pk_atom_assert_type(lisp, atom, lisp->cache.type_byte_array);
    return (PKAtomByteArray *)atom;
}

PKAtomStringArray *pk_atom_assert_string_array(Pocket lisp, PKAtom *atom) {
    pk_atom_assert_type(lisp, atom, lisp->cache.type_string_array);
    return (PKAtomStringArray *)atom;
}

PKAtom *pk_atom_array_to_list(Pocket lisp, PKAtomArray *array) {
    PKAtoms atoms = pk_array_slice(&array->atoms);
    return pk_atom_list(lisp, atoms);
}

PKAtomString *pk_atom_string_array_to_string(Pocket lisp, PKAtomStringArray *array) {
    PKStr str = pk_writer_slice(&array->string);
    return pk_atom_string(lisp, str);
}

PKAtom *pk_atom_string_array_to_chars(Pocket lisp, PKAtomStringArray *array) {
    PKStr str = pk_writer_slice(&array->string);
    return pk_atom_to_list_chars(lisp, str);
}

PKAtomChar *pk_atom_string_array_to_char(Pocket lisp, PKAtomStringArray *array) {
    return pk_atom_char(lisp, pk_atom_str_to_char(lisp, pk_writer_slice(&array->string)));
}

pk_u8 pk_atom_byte_array_to_byte(Pocket lisp, PKAtomByteArray *array, size_t index) {
    return pk_atom_byte_array_el(lisp, array, index);
}

PKBlob pk_atom_byte_array_to_bytes(Pocket lisp, PKAtomByteArray *array) {
    (void)lisp;
    return pk_bytes_slice(&array->bytes);
}

PKAtom *pk_atom_byte_array_to_list(Pocket lisp, PKAtomByteArray *array) {
    PKBlob bytes = pk_bytes_slice(&array->bytes);
    return pk_atom_to_list_bytes(lisp, bytes);
}
