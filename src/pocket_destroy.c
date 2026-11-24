#include "pocket_internals.h"

void pk_atom_deinit(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            pk_free(lisp, atom->string.str.c);
            break;
        }
        case PK_ATOMTY_LFUN: {
            pk_free(lisp, atom->lfun.bc.e);
            pk_free(lisp, atom->lfun.atoms.e);
            pk_hashmap_deinit(lisp, &atom->lfun.props);
            break;
        }
        case PK_ATOMTY_LMACRO: {
            pk_free(lisp, atom->lmacro.bc.e);
            pk_free(lisp, atom->lmacro.atoms.e);
            pk_hashmap_deinit(lisp, &atom->lmacro.props);
            break;
        }
        case PK_ATOMTY_CFUN: {
            pk_hashmap_deinit(lisp, &atom->cfun.props);
            break;
        }
        case PK_ATOMTY_CMACRO: {
            pk_hashmap_deinit(lisp, &atom->cmacro.props);
            break;
        }
        case PK_ATOMTY_SYMBOL: {
            pk_hashmap_deinit(lisp, &atom->symbol.props);
            break;
        }
        case PK_ATOMTY_OBJECT: {
            PKAtomObject *object = (PKAtomObject *)atom;
            if (object->gc != NULL) {
                (object->gc)(object->user_data);
                object->user_data = NULL;
            }
            pk_hashmap_deinit(lisp, &object->props);
            break;
        }
        case PK_ATOMTY_BLOB: {
            pk_blob_free(lisp, &atom->blob.bytes);
            break;
        }
        case PK_ATOMTY_ARRAY: {
            pk_array_deinit(lisp, &atom->array.atoms);
            break;
        }
        default: break;
    }
}

PKAtom *pk_atom_destroy(Pocket lisp, PKAtom *atom) {
    pk_u8 type = atom->header.type;

    if (type == PK_ATOMTY_DESTROYED) {
        return atom;
    }

    if (pk_atom_is_immortal(atom)) {
        pk_errors(lisp, pk_str("cannot destroy immortal atoms"));
        return atom;
    }

    if (type == PK_ATOMTY_SYMBOL) {
        pk_errors(lisp, pk_str("cannot destroy symbols"));
    }

    pk_atom_deinit(lisp, atom);

    atom->header.type = PK_ATOMTY_DESTROYED;

    return atom;
}
