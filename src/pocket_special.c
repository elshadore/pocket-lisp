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

PKRes pk_interp_let_eval(Pocket lisp) {
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
        
        pk_try(pk_let_push(lisp, PKEnvTy_Var, symbol, atoms.e[i]));
        
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

PKRes pk_interp_if(Pocket lisp) {
    PKAtom *atom = NULL;
    pk_try(pk_stack_head(lisp, &atom));
    pk_try(pk_pop(lisp));
    
    PKTuple cond = lisp->current_frame.as.t;
    lisp->current_frame.mode = PKEvalMode_Eval;
    if (pk_atom_is_true(atom)) {
        lisp->current_frame.as.atom = cond.a;
    } else {
        lisp->current_frame.as.atom = cond.b;
    }
    return PK_Ok;
}

PKRes pk_interp_while(Pocket lisp) {
    lisp->current_frame.mode = PKEvalMode_While_2;
    pk_frame_clear(lisp);
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Eval, (PKFrameData){.atom = lisp->current_frame.as.t.a}));
    return PK_Ok;
}

PKRes pk_interp_while_2(Pocket lisp) {
    PKAtom *atom = NULL;
    pk_try(pk_stack_head(lisp, &atom));
    pk_try(pk_pop(lisp));
    
    if (pk_atom_is_true(atom)) {
        lisp->current_frame.mode = PKEvalMode_While;
        pk_try(pk_frame_push(lisp, 0, PKEvalMode_Evlist, (PKFrameData){.atom = lisp->current_frame.as.t.b}));
    } else {
        pk_try(pk_ret_nil(lisp));
    }
    return PK_Ok;
}

PKRes pk_process_special_form(Pocket lisp, PKAtomSymbol *symbol, PKAtom *rest, PKAtom *expression, bool *is_special) {
    if (symbol == lisp->cache.lambda) {
        *is_special = true;
        // TODO: validate lambda structure in its creation.
        pk_try(pk_process_quote(lisp, expression));
    } else if (symbol == lisp->cache.quote) {
        *is_special = true;
        PKAtomCons *cons = NULL;
        pk_try(pk_atom_cast_cons(lisp, rest, &cons));
        if (!pk_atom_is_nil(cons->cdr)) {
            pk_error(lisp);
        }
        pk_try(pk_process_quote(lisp, cons->car));
    } else if (symbol == lisp->cache.if_sym) {
        *is_special = true;
        
        PKAtomCons *a = NULL;
        pk_try(pk_atom_cast_cons(lisp, rest, &a));
        PKAtomCons *b = NULL;
        pk_try(pk_atom_cast_cons(lisp, a->cdr, &b));
        PKAtomCons *c = NULL;
        pk_try(pk_atom_cast_cons(lisp, b->cdr, &c));
        if (!pk_atom_is_nil(c->cdr)) {
            pk_error(lisp);
        }
        
        PKTuple cond = (PKTuple) {
            .a = b->car,
            .b = c->car,
        };

        pk_try(pk_frame_push(lisp, 0, PKEvalMode_If, (PKFrameData){.t = cond}));
        pk_try(pk_frame_push(lisp, 0, PKEvalMode_Eval, (PKFrameData){.atom = a->car}));
        
    } else if (symbol == lisp->cache.progn) {
        *is_special = true;
        pk_try(pk_frame_push(lisp, 0, PKEvalMode_Evlist, (PKFrameData){.atom = rest}));
        
    } else if (symbol == lisp->cache.while_sym) {
        *is_special = true;
        
        PKAtomCons *cons = NULL;
        pk_try(pk_atom_cast_cons(lisp, rest, &cons));
        
        PKTuple cond = (PKTuple) {
            .a = cons->car,
            .b = cons->cdr,
        };
        
        pk_try(pk_frame_push(lisp, 0, PKEvalMode_While, (PKFrameData){.t = cond}));
        
    } else if (symbol == lisp->cache.let_sym) {
        *is_special = true;
        
        PKAtomCons *cons = NULL;
        pk_try(pk_atom_cast_cons(lisp, rest, &cons));
        
        pk_try(pk_frame_push(lisp, 0, PKEvalMode_Let_Eval, (PKFrameData){.atom = rest}));
        pk_try(pk_frame_push(lisp, 0, PKEvalMode_Let_Bind, (PKFrameData){.atom = cons->car}));
        
    } else {
        *is_special = false;
    }
    return PK_Ok;
}
