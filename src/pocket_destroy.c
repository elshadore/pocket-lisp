#include "pocket_internals.h"

void pk_atom_deinit(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            pk_free(lisp, atom->string.str.c);
            break;
        }
        case PK_ATOMTY_KEYWORD: {
            pk_free(lisp, atom->keyword.id.c);
            break;
        }
        case PK_ATOMTY_LFUN: {
            pk_free(lisp, atom->lfun.bc.e);
            pk_free(lisp, atom->lfun.atoms.e);
            break;
        }
        case PK_ATOMTY_LMACRO: {
            pk_free(lisp, atom->lmacro.bc.e);
            pk_free(lisp, atom->lmacro.atoms.e);
            break;
        }
        case PK_ATOMTY_SYMBOL: {
            break;
        }
        case PK_ATOMTY_OBJECT: {
            pk_errors(lisp, pk_str("pk_atom_deinit(): TODO: object destroy"));
            break;
        }
        case PK_ATOMTY_ARRAY: {
            pk_array_deinit(lisp, &atom->array.atoms);
            break;
        }
        default: break;
    }
}

bool pk_atom_is_immortal(Pocket lisp, PKAtom *atom) {
    return (atom == lisp->cache.nil)
        || (atom == lisp->cache.atom_true)
        || (atom == lisp->cache.undefined)
        || (atom == (PKAtom *)lisp->cache.empty_string)
        || (atom == (PKAtom *)lisp->cache.out_of_memory);
}

PKAtom *pk_atom_destroy(Pocket lisp, PKAtom *atom) {
    pk_u8 type = atom->header.type;

    if (type == PK_ATOMTY_DESTROYED) {
        return atom;
    }

    if (pk_atom_is_immortal(lisp, atom)) {
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
