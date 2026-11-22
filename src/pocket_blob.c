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

bool pk_atom_is_blob(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_BLOB;
}

PKAtomBlob *pk_atom_assert_blob(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_blob(atom)) {
        pk_error_type(lisp, atom, PK_TYPE_BLOB);
    }
    return (PKAtomBlob *)atom;
}
