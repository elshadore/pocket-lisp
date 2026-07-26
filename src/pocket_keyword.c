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

pk_bool pk_atom_is_keyword(PKAtom *atom) {
    return atom->tag.ty == PKAtomTy_Keyword;
}

PK_RES pk_atom_cast_keyword(Pocket lisp, PKAtom *atom, PKAtomKeyword **output) {
    if (atom->tag.ty != PKAtomTy_Keyword) return pk_error(lisp);
    *output = (PKAtomKeyword *)atom;
    return PK_OK;
}

pk_bool pk_atom_keyword_qeq(PKAtomKeyword *keyword, const char *comp) {
    size_t length = strlen(comp);
    return pk_string_eq(keyword->c, keyword->length, comp, length);
}
