#include "pocket_internals.h"

PK_RES pk_atom_keyword(Pocket lisp, const char *cstr, PKAtomKeyword **output) {
    size_t length = 0;
    length = strlen(cstr);
    return pk_atom_keywordn(lisp, cstr, length, output);
}

PK_RES pk_atom_keywordn(Pocket lisp, const char *string, size_t length, PKAtomKeyword **output) {
    char *dupe = NULL;
    pk_try(pk_string_dupe(lisp, string, length, &dupe));
    pk_try(pk_atom_keywordn_nomemcpy(lisp, dupe, length, output));
    return PK_OK;
}
    
PK_RES pk_atom_keywordn_nomemcpy(Pocket lisp, char *c, size_t length, PKAtomKeyword **output) {
    PKAtom *a = NULL;
    
    pk_try(pk_atom_alloc(lisp, &a));

    a->tag.ty = PKAtomTy_Keyword;
    a->keyword.c = c;
    a->keyword.length = length;

    *output = (PKAtomKeyword *)a;
    return PK_OK;
}

pk_bool pk_atom_keyword_eq(Pocket lisp, PKAtomKeyword *lhs, PKAtomKeyword *rhs) {
    (void)lisp;
    return pk_string_eq(lhs->c, lhs->length, rhs->c, rhs->length);
}
