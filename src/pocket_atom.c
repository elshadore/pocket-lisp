#include "pocket_internals.h"

PKAtom *pk_atom_nil(Pocket lisp) {
    return lisp->cache.nil;
}

PKRes pk_atom_nil_new(Pocket lisp, PKAtom **output) {
    PKAtom *atom;
    pk_try(pk_atom_alloc(lisp, &atom));
    atom->tag = (PKAtomTag){ .ty = PKAtomTy_Nil, .marked = false };
    *output = atom;
    return PK_Ok;
}

PKAtom *pk_atom_t(Pocket lisp) {
    return (PKAtom *)lisp->cache.t;
}

PKRes pk_atom_cfunc(Pocket lisp, void *user_closure, PKFn fn, PKFuncArity arity, PKAtomCFunc **output) {
    PKAtom *a;
    pk_try(pk_atom_alloc(lisp, &a));
    PKAtomCFunc *atom = (PKAtomCFunc *)a;
    *atom = (PKAtomCFunc) {
        .tag.ty = PKAtomTy_CFunc,
        .tag.marked = false,
        .user_closure = user_closure,
        .fn = fn,
        .arity = arity,
    };
    *output = atom;
    return PK_Ok;
}

PKRes pk_atom_cast_cfunc(Pocket lisp, PKAtom *atom, PKAtomCFunc **output) {
    if (atom->tag.ty != PKAtomTy_CFunc) return pk_error(lisp);
    *output = (PKAtomCFunc *)atom;
    return PK_Ok;
}

bool pk_atom_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs) {
    if (lhs == rhs) return true;
    if (lhs->tag.ty != rhs->tag.ty) return false;
    switch (lhs->tag.ty) {
        case PKAtomTy_Number: {
            bool result;
            pk_number_eq(lisp, (PKAtomNumber *)lhs, (PKAtomNumber *)rhs, &result);
            return result;
        }
        case PKAtomTy_String: {
            return pk_atom_string_eq(lisp, (PKAtomString *)lhs, (PKAtomString *)rhs);
        }
        default: return false;
    }
}

bool pk_atom_is_nil(PKAtom *atom) {
    return atom->tag.ty == PKAtomTy_Nil;
}

bool pk_atom_is_true(PKAtom *atom) {
    return !pk_atom_is_nil(atom);
}

PKRes pk_interp_clone(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.as.atom;
    switch (atom->tag.ty) {
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            lisp->current_frame.as.atom = cons->cdr;
            pk_frame_push(lisp, 0, PKEvalMode_Clone, (PKFrameData){.atom = cons->car});
            return PK_Ok;
        }
        default: {
            PKAtoms slice = pk_stack_slice(lisp);
            
            PKAtom *result = NULL;
            if (slice.length == 0) {
                result = atom;
            } else {
                if (pk_atom_is_nil(atom)) {
                    pk_try(pk_slice_list(lisp, slice, &result));
                } else {
                    pk_try(pk_push(lisp, atom));
                    pk_try(pk_slice_list_tailed(lisp, pk_stack_slice(lisp), &result));
                }
            }
            pk_try(pk_ret_this(lisp, result));
            return PK_Ok;
        }
    }
}
