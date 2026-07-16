#include "pocket_internals.h"

PKRes pk_interp_eval(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.as.atom;
    switch (atom->tag.ty) {
        case PKAtomTy_Nil:
        case PKAtomTy_Number:
        case PKAtomTy_String:
        case PKAtomTy_CFunc: {
            pk_try(pk_push(lisp, atom));
            pk_try(pk_ret_top(lisp));
            return PK_Ok;
        }
        case PKAtomTy_Symbol: {
            PKAtom *value;
            pk_try(pk_env_get(lisp, PKEnvTy_Var, (PKAtomSymbol *)atom, &value));
            pk_try(pk_push(lisp, value));
            pk_try(pk_ret_top(lisp));
            return PK_Ok;
        }
        case PKAtomTy_Cons: {
            pk_try(pk_ret_all(lisp));
            PKAtomCons *form = (PKAtomCons *)atom;
            if (pk_atom_is_symbol(form->car)) {
                bool is_special = false;
                pk_try(pk_interp_special_form(lisp, (PKAtomSymbol *)form->car, form->cdr, (PKAtom *)form, &is_special));
                if (is_special) {
                    return PK_Ok;
                }
            }
            pk_try(pk_frame_push(lisp, 0, PKEvalMode_Apply, (PKFrameData){.atom = form->car}));
            pk_try(pk_frame_push(lisp, 0, PKEvalMode_Evargs, (PKFrameData){.atom = form->cdr}));
            return PK_Ok;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

PKRes pk_interp_apply(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.as.atom;
    if (pk_atom_is_symbol(atom)) {
        PKAtomSymbol *sym = (PKAtomSymbol *)atom;
        pk_try(pk_env_get(lisp, PKEnvTy_Fun, sym, &atom));
    }
    
    size_t length = pk_frame_length(lisp);
    PKCallConv call = (PKCallConv){0};
    pk_try(pk_callconv(lisp, atom, length, &call));
    
    switch (call.ty) {
        case PKFuncTy_CFunc: {
            pk_try((call.as.c.fn)(call.as.c.user_closure, lisp));
            pk_try(pk_ret_top(lisp));
            return PK_Ok;
        }
        case PKFuncTy_Lambda: {
            PKAtom *args = call.as.lisp.args;
            PKAtoms stack = pk_stack_slice(lisp);
            pk_try(pk_bind_lambda_list(lisp, args, stack));
            pk_frame_steal(lisp, PKEvalMode_Ret, PK_FRAME_DATA_EMPTY);
            pk_try(pk_frame_push(lisp, 0, PKEvalMode_Evlist, (PKFrameData){.atom = call.as.lisp.body}));
            return PK_Ok;
        }
    }
}

PKRes pk_interp_evlist(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.as.atom;
    switch (atom->tag.ty) {
        case PKAtomTy_Nil: {
            pk_try(pk_push_nil(lisp));
            pk_try(pk_ret_top(lisp));
            return PK_Ok;
        }
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            lisp->current_frame.mode = PKEvalMode_Evlist_2;
            lisp->current_frame.as.cons = cons;
            pk_try(pk_push_nil(lisp));
            return PK_Ok;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

PKRes pk_interp_evlist_2(Pocket lisp) {
    PKAtomCons *cons = lisp->current_frame.as.cons;
    pk_frame_clear(lisp);
    switch (cons->cdr->tag.ty) {
        case PKAtomTy_Cons: {
            lisp->current_frame.as.cons = (PKAtomCons *)cons->cdr;
            pk_try(pk_frame_push(lisp, 0, PKEvalMode_Eval, (PKFrameData){.atom = cons->car}));
            return PK_Ok;
        }
        case PKAtomTy_Nil: {
            lisp->current_frame.mode = PKEvalMode_Eval;
            lisp->current_frame.as.atom = cons->car;
            return PK_Ok;
        }
        default: {
            return pk_error(lisp);
        }
    }
}

PKRes pk_interp_evargs(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.as.atom;
    switch (atom->tag.ty) {
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            lisp->current_frame.as.atom = cons->cdr;
            pk_try(pk_frame_push(lisp, 0, PKEvalMode_Eval, (PKFrameData){.atom = cons->car}));
            return PK_Ok;
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

PKRes pk_interp(Pocket lisp, size_t stop) {
    while (lisp->frames.count > stop) {
        // (void)pk_trace_dump(lisp, "interp");
        switch (lisp->current_frame.mode) {
            case PKEvalMode_Root: {
                return pk_error(lisp);
            }
            case PKEvalMode_User: {
                return pk_error(lisp);
            }
            case PKEvalMode_Ret: {
                pk_try(pk_ret_top(lisp));
                break;
            }
            case PKEvalMode_Eval: {
                pk_try(pk_interp_eval(lisp));
                break;
            }
            case PKEvalMode_Evlist: {
                pk_try(pk_interp_evlist(lisp));
                break;
            }
            case PKEvalMode_Evlist_2: {
                pk_try(pk_interp_evlist_2(lisp));
                break;
            }
            case PKEvalMode_Evargs: {
                pk_try(pk_interp_evargs(lisp));
                break;
            }
            case PKEvalMode_Apply: {
                pk_try(pk_interp_apply(lisp));
                break;
            }
            case PKEvalMode_Clone: {
                pk_try(pk_interp_clone(lisp));
                break;
            }
            case PKEvalMode_If: {
                pk_try(pk_interp_if(lisp));
                break;
            }
            case PKEvalMode_While: {
                pk_try(pk_interp_while(lisp));
                break;
            }
            case PKEvalMode_While_2: {
                pk_try(pk_interp_while_2(lisp));
                break;
            }
            case PKEvalMode_Let_Eval: {
                pk_try(pk_interp_let_eval(lisp));
                break;
            }
            case PKEvalMode_Let_Bind: {
                pk_try(pk_interp_let_bind(lisp));
                break;
            }
            case PKEvalMode_Read_Mode: {
                pk_try(pk_interp_read_mode(lisp));
                break;
            }
            case PKEvalMode_Read_Atom: {
                pk_try(pk_interp_read_atom(lisp));
                break;
            }
            case PKEvalMode_Read_All: {
                pk_try(pk_interp_read_all(lisp));
                break;
            }
            case PKEvalMode_Read_All_2: {
                pk_try(pk_interp_read_all_2(lisp));
                break;
            }
            case PKEvalMode_Read_Append: {
                pk_try(pk_interp_read_append(lisp));
                break;
            }
            case PKEvalMode_Read_Cons: {
                pk_try(pk_interp_cons(lisp));
                break;
            }
            case PKEvalMode_Read_Cons_2: {
                pk_try(pk_interp_cons_2(lisp));
                break;
            }
            case PKEvalMode_Read_Cons_3: {
                pk_try(pk_interp_cons_3(lisp));
                break;
            }
        }
    }
    // (void)pk_trace_dump(lisp, "result");
    return PK_Ok;
}
