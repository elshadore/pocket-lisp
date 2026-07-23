#include "pocket_internals.h"

PKAtom *pk_atom_nil(Pocket lisp) {
    return lisp->cache.nil;
}

PKRes pk_atom_nil_new(Pocket lisp, PKAtom **output) {
    PKAtom *a = NULL;
    pk_try(pk_atom_alloc(lisp, &a));
    
    a->tag.ty = PKAtomTy_Nil;
    
    *output = a;
    return PK_Ok;
}

PKAtom *pk_atom_t(Pocket lisp) {
    return (PKAtom *)lisp->cache.t;
}

PKRes pk_atom_cfunc(Pocket lisp, void *user_closure, PKFn fn, PKFuncArity arity, PKAtomCFunc **output) {
    PKAtom *a = NULL;
    pk_try(pk_atom_alloc(lisp, &a));
    
    a->tag.ty = PKAtomTy_CFunc;
    a->cfunc.user_closure = user_closure;
    a->cfunc.fn = fn;
    a->cfunc.arity = arity;
    
    *output = (PKAtomCFunc *)a;
    return PK_Ok;
}

PKRes pk_atom_cast_cfunc(Pocket lisp, PKAtom *atom, PKAtomCFunc **output) {
    if (atom->tag.ty != PKAtomTy_CFunc) return pk_error(lisp);
    *output = (PKAtomCFunc *)atom;
    return PK_Ok;
}

PKRes pk_atom_cast_lfunc(Pocket lisp, PKAtom *atom, PKAtomLFunc **output) {
    if (atom->tag.ty != PKAtomTy_LFunc) return pk_error(lisp);
    *output = (PKAtomLFunc *)atom;
    return PK_Ok;
}

pk_bool pk_atom_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    if (lhs == rhs) return PK_TRUE;
    if (lhs->tag.ty != rhs->tag.ty) return PK_FALSE;
    switch (lhs->tag.ty) {
        case PKAtomTy_Number: {
            pk_bool result = PK_FALSE;
            pk_number_eq(lisp, (PKAtomNumber *)lhs, (PKAtomNumber *)rhs, &result);
            return result;
        }
        case PKAtomTy_String: {
            return pk_atom_string_eq(lisp, (PKAtomString *)lhs, (PKAtomString *)rhs);
        }
        default: return PK_FALSE;
    }
}

pk_bool pk_atom_is_nil(PKAtom *atom) {
    return atom->tag.ty == PKAtomTy_Nil;
}

PKRes pk_atom_assert_nil(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_nil(atom)) {
        return pk_error(lisp);
    }
    return PK_Ok;
}

pk_bool pk_atom_is_true(PKAtom *atom) {
    return !pk_atom_is_nil(atom);
}

