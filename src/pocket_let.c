#include "pocket_internals.h"

PKRes pk_interp_let_bind(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.as.atom;
    switch (atom->tag.ty) {
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            PKAtom *let = cons->car;
            lisp->current_frame.as.atom = cons->cdr;
            switch (let->tag.ty) {
                case PKAtomTy_Symbol: {
                    pk_try(pk_push_nil(lisp));
                    return PK_Ok;
                }
                case PKAtomTy_Cons: {
                    PKAtomCons *a = (PKAtomCons *)let;
                    PKAtomCons *b = NULL;
                    pk_try(pk_atom_cast_cons(lisp, a->cdr, &b));
                    if (!pk_atom_is_nil(b->cdr)) {
                        return pk_error(lisp);
                    }
                    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Eval, (PKFrameData){.atom = b->car}));
                    return PK_Ok;
                }
                default: {
                    return pk_error(lisp);
                }
            }
        }
        case PKAtomTy_Nil: {
            pk_try(pk_ret_all(lisp));
            return PK_Ok;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

PKRes pk_interp_let_type(Pocket lisp, PKEnvTy ty) {
    PKAtom *atom = lisp->current_frame.as.atom;
    
    PKAtomCons *expr = NULL;
    pk_try(pk_atom_cast_cons(lisp, atom, &expr));
    PKAtom *iter = expr->car;
    
    PKAtoms atoms = pk_stack_slice(lisp);
    for (size_t i = 0; i < atoms.length; ++i) {
        PKAtomCons *let = NULL;
        pk_try(pk_atom_cast_cons(lisp, iter, &let));
        PKAtomSymbol *symbol = NULL;
        switch (let->car->tag.ty) {
            case PKAtomTy_Symbol: {
                symbol = (PKAtomSymbol *)let->car;
                break;
            }
            case PKAtomTy_Cons: {
                pk_try(pk_atom_cast_symbol(lisp, let->car->cons.car, &symbol));
                break;
            }
            default: {
                return pk_error(lisp);
            }
        }
        
        pk_try(pk_let_push(lisp, ty, symbol, atoms.e[i]));
        
        iter = let->cdr;
    }
    
    if (!pk_atom_is_nil(iter)) {
        return pk_error(lisp);
    }

    pk_frame_clear(lisp);
    pk_frame_steal(lisp, PKEvalMode_Ret, PK_FRAME_DATA_EMPTY);
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Evlist, (PKFrameData){.atom = expr->cdr}));
    
    return PK_Ok;
}
    
PKRes pk_interp_let(Pocket lisp) {
    return pk_interp_let_type(lisp, PKEnvTy_Var);
}

PKRes pk_interp_flet(Pocket lisp) {
    return pk_interp_let_type(lisp, PKEnvTy_Fun);
}
