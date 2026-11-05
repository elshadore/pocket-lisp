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

bool pk_atom_is_blob(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_BLOB;
}

PKAtomBlob *pk_atom_assert_blob(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_blob(atom)) {
        pk_error_type(lisp, atom, lisp->cache.type_blob);
    }
    return (PKAtomBlob *)atom;
}
