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
