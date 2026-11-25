#include "pocket_internals.h"

PKAtomBlob *pk_atom_blob_new_empty(Pocket lisp) {
    PKAtomBlob *result = (PKAtomBlob *)pk_atom_alloc(lisp);

    result->header.type = PK_ATOMTY_BLOB;
    result->bytes = PK_BLOB_EMPTY;

    return result;
}

PKAtomBlob *pk_atom_blob_nomemcpy(Pocket lisp, PKBlob blob) {
    PKAtomBlob *result = (PKAtomBlob *)pk_atom_alloc(lisp);

    result->header.type = PK_ATOMTY_BLOB;
    result->bytes = blob;

    return result;
}

PKAtomBlob *pk_atom_blob_alloc(Pocket lisp, size_t length) {
    if (length == 0) {
        return &PK_ATOM_EMPTY_BLOB;
    }

    PKBlob buffer = pk_blob_alloc(lisp, length);
    
    return pk_atom_blob_nomemcpy(lisp, buffer);
}

PKAtomBlob *pk_atom_blob(Pocket lisp, PKBlob blob) {
    PKBlob dupe = pk_blob_dupe(lisp, blob);
    return pk_atom_blob_nomemcpy(lisp, dupe);
}

PKAtomBlob *pk_atom_blob_cat(Pocket lisp, PKAtoms atoms) {
    pk_writer_reset(lisp);

    for (size_t i = 0; i < atoms.length; ++i) {
        switch (atoms.e[i]->header.type) {
            case PK_ATOMTY_BLOB: {
                pk_writer_bytes(lisp, ((PKAtomBlob *)atoms.e[i])->bytes);
                break;
            }
            case PK_ATOMTY_NUMBER: {
                pk_atom_number_to_bytes_trimmed_writer(lisp, (PKAtomNumber *)atoms.e[i]);
                break;
            }
            default: {
                pk_error_type2(lisp, atoms.e[i], PK_TYPE_BLOB, PK_TYPE_NUMBER);
                break;
            }
        }
    }

    PKAtomBlob *result = pk_atom_blob(lisp, pk_writer_slice(lisp));
    pk_writer_reset(lisp);
    return result;
}

PKAtom *pk_atom_blob_el(Pocket lisp, PKAtomBlob *blob, size_t index) {
    if (index >= blob->bytes.length) {
        pk_errorf(lisp, "index: %zu, longer than the blob length: %zu", index, blob->bytes.length);
        return &PK_ATOM_NIL;
    }
    return (PKAtom *)pk_atom_int(lisp, blob->bytes.e[index]);
}

size_t pk_atom_blob_length(Pocket lisp, PKAtomBlob *blob) {
    (void)lisp;
    return blob->bytes.length;
}

PKAtomString *pk_atom_blob_to_string(Pocket lisp, PKAtomBlob *blob) {
    PKStr str = (PKStr) {
        .c = (char *)blob->bytes.e,
        .length = blob->bytes.length,
    };
    return pk_atom_string(lisp, str);
}

PKAtomBlob *pk_atom_blob_reverse(Pocket lisp, PKBlob input) {
    PKBlob dupe = pk_blob_dupe(lisp, input);
    pk_blob_reverse_mut(dupe);
    return pk_atom_blob_nomemcpy(lisp, dupe);
}

PKBlob pk_blob_from(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_blob(atom)) {
        return ((PKAtomBlob *)atom)->bytes;
    } else if (pk_atom_is_number(atom)) {
        return pk_atom_number_to_bytes_trimmed(lisp, (PKAtomNumber *)atom)->bytes;
    } else {
        pk_error_type2(lisp, atom, PK_TYPE_NUMBER, PK_TYPE_BLOB);
        return PK_BLOB_EMPTY;
    }
}

PKAtom *pk_atom_blob_split(Pocket lisp, PKAtomBlob *blob, PKAtom *pattern) {
    PKBlob source = blob->bytes;
    PKBlob sep = pk_blob_from(lisp, pattern);
    
    if (sep.length == 0) {
        return (PKAtom *)pk_atom_list1(lisp, (PKAtom *)blob);
    }

    PKAccum acc = pk_accum_new(lisp);
    size_t start = 0;
    size_t i = 0;

    while (i + sep.length <= source.length) {
        PKBlob window = {
            .e = source.e + i,
            .length = sep.length,
        };
        if (pk_blob_eq(window, sep)) {
            if (i > start) {
                pk_accum_push(&acc, (PKAtom *)pk_atom_blob(lisp, (PKBlob) {
                    .e = source.e + start,
                    .length = i - start,
                }));
            }
            i += sep.length;
            start = i;
        } else {
            ++i;
        }
    }

    if (start < source.length) {
        pk_accum_push(&acc, (PKAtom *)pk_atom_blob(lisp, (PKBlob) {
            .e = source.e + start,
            .length = source.length - start,
        }));
    }

    return acc.head;
}

bool pk_atom_blob_eq(Pocket lisp, PKAtomBlob *lhs, PKAtomBlob *rhs) {
    (void)lisp;
    return pk_blob_eq(lhs->bytes, rhs->bytes);
}

bool pk_atom_is_blob(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_BLOB;
}

PKAtomBlob *pk_atom_assert_blob(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_blob(atom)) {
        pk_error_type(lisp, atom, PK_TYPE_BLOB);
    }
    return (PKAtomBlob *)atom;
}

PKAtomBlob *pk_atom_blob_make(Pocket lisp, size_t count, PKAtom *fill) {
    if (count == 0) {
        return &PK_ATOM_EMPTY_BLOB;
    }

    PKBlob bytes = pk_blob_from(lisp, fill);

    pk_writer_reset(lisp);

    for (size_t i = 0; i < count; ++i) {
        pk_writer_bytes(lisp, bytes);
    }

    PKAtomBlob *result = pk_atom_blob(lisp, pk_writer_slice(lisp));
    pk_writer_reset(lisp);
    return result;
}

PKAtomBlob *pk_atom_blob_map(Pocket lisp, PKAtomBlob *blob, PKAtom *function) {
    size_t count = blob->bytes.length;
    PKAtom **mapped = pk_realloc(lisp, NULL, count * sizeof(PKAtom *));

    for (size_t i = 0; i < count; ++i) {
        PKAtom *element = (PKAtom *)pk_atom_byte(lisp, blob->bytes.e[i]);
        pk_stack_push(lisp, element);
        pk_stack_push(lisp, function);
        pk_funcall(lisp, PK_CALL_FUN, 1);
        mapped[i] = pk_stack_head_pop(lisp);
    }

    PKAtoms atoms = {
        .e = mapped,
        .length = count,
    };
    PKAtomBlob *result = pk_atom_blob_cat(lisp, atoms);
    pk_free(lisp, mapped);
    return result;
}

PKAtomBlob *pk_atom_blob_filter(Pocket lisp, PKAtomBlob *blob, PKAtom *function) {
    size_t count = 0;
    PKAtom **kept = pk_realloc(lisp, NULL, blob->bytes.length * sizeof(PKAtom *));

    for (size_t i = 0; i < blob->bytes.length; ++i) {
        pk_u8 byte = blob->bytes.e[i];
        PKAtom *element = (PKAtom *)pk_atom_byte(lisp, byte);
        pk_stack_push(lisp, element);
        pk_stack_push(lisp, function);
        pk_funcall(lisp, PK_CALL_FUN, 1);
        PKAtom *result = pk_stack_head_pop(lisp);
        if (pk_atom_is_true(result)) {
            kept[count++] = element;
        }
    }

    PKAtoms atoms = {
        .e = kept,
        .length = count,
    };
    PKAtomBlob *result = pk_atom_blob_cat(lisp, atoms);
    pk_free(lisp, kept);
    return result;
}
