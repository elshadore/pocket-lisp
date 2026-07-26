#include "pocket_internals.h"

PK_RES pk_atom_clone_rec(Pocket lisp, PKHashTable *ht, PKAtom *input, PKAtom **output) {
    PKAtom *duped = NULL;
    
    if (!pk_atom_is_cons(input)) {
        *output = input;
        return PK_OK;
    }
    
    if (!pk_hashtable_get(lisp, ht, input, &duped)) {
        PKAtomCons *cons = NULL;
        pk_try(pk_atom_cons(lisp, pk_atom_nil(lisp), pk_atom_nil(lisp), &cons));
        duped = (PKAtom *)cons;
        
        pk_try(pk_hashtable_put(lisp, ht, input, duped));

        pk_try(pk_atom_clone_rec(lisp, ht, input->cons.car, &cons->car));
        pk_try(pk_atom_clone_rec(lisp, ht, input->cons.cdr, &cons->cdr));
    }
    
    *output = duped;
    return PK_OK;
}
    
PK_RES pk_atom_clone(Pocket lisp, PKAtom *input, PKAtom **output) {
    PKHashTable ht = pk_hashtable_init();
    PK_RES result = pk_atom_clone_rec(lisp, &ht, input, output);
    pk_hashtable_deinit(lisp, &ht);
    return result;
}

PK_RES pk_atom_circular_rec(Pocket lisp, PKHashTable *ht, PKAtom *atom, pk_u8 *output) {
    PKAtomCons *cons = NULL;
    PKAtom *ignore = NULL;
    if (!pk_atom_is_cons(atom)) {
        return PK_OK;
    }
    cons = (PKAtomCons *)atom;
    if (pk_hashtable_get(lisp, ht, atom, &ignore)) {
        *output = PK_TRUE;
        return PK_OK;
    }
    pk_try(pk_hashtable_put(lisp, ht, atom, atom));
    
    pk_try(pk_atom_circular_rec(lisp, ht, cons->car, output));
    if (*output == PK_TRUE) {
        return PK_OK;
    }
    
    pk_try(pk_atom_circular_rec(lisp, ht, cons->cdr, output));
    if (*output == PK_TRUE) {
        return PK_OK;
    }

    return PK_OK;
}
    
PK_RES pk_atom_circular(Pocket lisp, PKAtom *atom, pk_u8 *output) {
    PKHashTable ht = pk_hashtable_init();
    PK_RES result = pk_atom_circular_rec(lisp, &ht, atom, output);
    pk_hashtable_deinit(lisp, &ht);
    return result;
}

PK_RES pk_atom_assert_non_circular(Pocket lisp, PKAtom *atom) {
    pk_u8 boolean = PK_FALSE;
    pk_try(pk_atom_circular(lisp, atom, &boolean));
    if (boolean) {
        return pk_error(lisp);
    }
    return PK_OK;
}
