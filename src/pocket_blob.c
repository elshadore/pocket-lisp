#include "pocket_internals.h"

typedef struct PKAtomBlobCatData_ {
    PKBytes bytes;
    PKAtoms atoms;
} PKAtomBlobCatData;

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

PKAtomBlob *pk_atom_blob(Pocket lisp, PKBlob blob) {
    PKBlob dupe = pk_blob_dupe(lisp, blob);
    return pk_atom_blob_nomemcpy(lisp, dupe);
}

pk_u8 pk_atom_blob_el(Pocket lisp, PKAtomBlob *blob, size_t index) {
    if (index >= blob->bytes.length) {
        pk_errorf(lisp, "index: %zu, longer than the blob length: %zu", index, blob->bytes.length);
        return '\0';
    }
    return blob->bytes.e[index];
}

size_t pk_atom_blob_length(Pocket lisp, PKAtomBlob *blob) {
    (void)lisp;
    return blob->bytes.length;
}

void pk_atom_blob_cat_body(void *user_closure, Pocket lisp) {
    PKAtomBlobCatData *data = (PKAtomBlobCatData *)user_closure;

    for (size_t i = 0; i < data->atoms.length; ++i) {
        PKBlob part = pk_atom_as_bytes(lisp, data->atoms.e[i]);
        for (size_t j = 0; j < part.length; ++j) {
            pk_bytes_push(lisp, &data->bytes, part.e[j], 8);
        }
    }

    pk_stack_push(lisp, (PKAtom *)pk_atom_blob(lisp, pk_bytes_slice(&data->bytes)));
    data->bytes = PK_BYTES_EMPTY;
}

PKAtomBlob *pk_atom_blob_cat(Pocket lisp, PKAtoms atoms) {
    PKAtomBlobCatData data = (PKAtomBlobCatData) {
        .bytes = PK_BYTES_EMPTY,
        .atoms = atoms,
    };

    PK_RES result = pk_protect(lisp, &data, pk_atom_blob_cat_body, 0);

    pk_bytes_deinit(lisp, &data.bytes);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }

    return (PKAtomBlob *)pk_stack_head_pop(lisp);
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
        pk_error_type(lisp, atom, lisp->cache.type_blob);
    }
    return (PKAtomBlob *)atom;
}
