#include "pocket_internals.h"

// PKRes pk_atom_eval_quasiquote(Pocket lisp, PKAtom *atom, bool *spliced) {
//     switch (atom->tag.ty) {
//         case PKAtomTy_Cons: {
//             PKAtomCons *cons = (PKAtomCons *)atom;
//             if ((cons->car == (PKAtom *)lisp->cache.unquote) || (cons->car == (PKAtom *)lisp->cache.unquote_splice)) {
//                 PKAtomCons *a;
//                 pk_try(pk_atom_cast_cons(lisp, cons->cdr, &a));
//                 if (a->cdr != pk_atom_nil(lisp)) {
//                     return pk_error(lisp);
//                 }
//                 pk_try(pk_atom_eval(lisp, a->car));
//                 *spliced = (cons->car == (PKAtom *)lisp->cache.unquote_splice);
//                 return PK_Ok;
//             }
//             PKAtomCons *acc = NULL;
//             pk_cdolist(lisp, el, atom) {
//                 bool res;
//                 pk_try(pk_atom_eval_quasiquote(lisp, el, &res));
//                 PKAtom *pop;
//                 pk_try(pk_stack_pop(lisp, &pop));
//                 if (res) {
//                     PKAtomCons *head;
//                     pk_try(pk_atom_cast_cons(lisp, pop, &head));
//                     PKAtomCons *tail;
//                     pk_try(pk_atom_cons_tail(lisp, head, &tail));
//                     if (acc == NULL) {
//                         acc = tail;
//                         pk_try(pk_push(lisp, (PKAtom *)acc));
//                     } else {
//                         acc->cdr = (PKAtom *)head;
//                         acc = tail;
//                     }
//                 } else {
//                     PKAtomCons *next;
//                     pk_try(pk_atom_cons_car(lisp, pop, &next));
//                     if (acc == NULL) {
//                         acc = next;
//                         pk_try(pk_push(lisp, (PKAtom *)acc));
//                     } else {
//                         acc->cdr = (PKAtom *)next;
//                         acc = next;
//                     }
//                 }
//             }
//             if (acc == NULL) {
//                 pk_try(pk_push_nil(lisp));
//             }
//             break;
//         }
//         default: {
//             pk_try(pk_push(lisp, atom));
//             break;
//         }
//     }
//     *spliced = false;
//     return PK_Ok;
// }

// PKRes pk_atom_eval_special_form(Pocket lisp, PKAtomSymbol *symbol, PKAtom *body, PKAtom *expression, bool *output) {
//     *output = true;
//     if (symbol == lisp->cache.lambda) {
//         pk_try(pk_push(lisp, expression));
//     } else if (symbol == lisp->cache.quote) {
//         if (body->tag.ty != PKAtomTy_Cons) {
//             return pk_error(lisp);
//         }
//         PKAtomCons *body_cons = (PKAtomCons *)body;
//         if (!pk_atom_is_nil(body_cons->cdr)) {
//             return pk_error(lisp);
//         }
//         pk_try(pk_push(lisp, body_cons->car));
//     } else if (symbol == lisp->cache.quasiquote) {
//         if (body->tag.ty != PKAtomTy_Cons) {
//             return pk_error(lisp);
//         }
//         PKAtomCons *body_cons = (PKAtomCons *)body;
//         if (!pk_atom_is_nil(body_cons->cdr)) {
//             return pk_error(lisp);
//         }
//         bool _spliced;
//         pk_try(pk_atom_eval_quasiquote(lisp, body_cons->car, &_spliced));
//     } else if (symbol == lisp->cache.unquote) {
//         return pk_error(lisp);
//     } else if (symbol == lisp->cache.unquote_splice) {
//         return pk_error(lisp);
//     } else if (symbol == lisp->cache.progn) {
//         pk_try(pk_atom_evlist(lisp, body));
//     } else if (symbol == lisp->cache.while_sym) {
//         if (body->tag.ty != PKAtomTy_Cons) {
//             return pk_error(lisp);
//         }
//         PKAtomCons *body_cons = (PKAtomCons *)body;
//         PKAtom *cond = body_cons->car;
//         PKAtom *loop = body_cons->cdr;

//         pk_try(pk_push_nil(lisp));
//         for (;;) {
//             pk_try(pk_atom_eval(lisp, cond));
//             PKAtom *a;
//             pk_try(pk_stack_get(lisp, -1, &a));
//             if (pk_atom_is_nil(a)) {
//                 break;
//             }
//             pk_try(pk_pop(lisp));
//             pk_try(pk_atom_evlist(lisp, loop));
//             pk_try(pk_swap(lisp, -1, -2));
//             pk_try(pk_pop(lisp));
//         }
//     } else if (symbol == lisp->cache.if_sym) {
//         if (body->tag.ty != PKAtomTy_Cons) {
//             return pk_error(lisp);
//         }

//         PKAtomCons *body_cons = (PKAtomCons *)body;
//         PKAtomCons *then_cons;
//         pk_try(pk_atom_cast_cons(lisp, body_cons->cdr, &then_cons));
//         PKAtomCons *else_cons;
//         pk_try(pk_atom_cast_cons(lisp, then_cons->cdr, &else_cons));
//         if (!pk_atom_is_nil(else_cons->cdr)) return pk_error(lisp);

//         pk_try(pk_atom_eval(lisp, body_cons->car));

//         PKAtom *cond;
//         pk_try(pk_stack_pop(lisp, &cond));

//         if (pk_atom_is_true(cond)) {
//             pk_try(pk_atom_eval(lisp, then_cons->car));
//         } else {
//             pk_try(pk_atom_eval(lisp, else_cons->car));
//         }
//     } else if (symbol == lisp->cache.let_sym) {
//         if (body->tag.ty != PKAtomTy_Cons) {
//             return pk_error(lisp);
//         }

//         PKAtomCons *body_cons = (PKAtomCons *)body;
//         PKAtom *bindings = body_cons->car;
//         PKAtom *body_forms = body_cons->cdr;

//         size_t count = 0;
//         pk_cdolist(lisp, binding, bindings) {
//             PKAtomCons *pair;
//             pk_try(pk_atom_cast_cons(lisp, binding, &pair));
//             PKAtomCons *val;
//             pk_try(pk_atom_cast_cons(lisp, pair->cdr, &val));
//             if (!pk_atom_is_nil(val->cdr)) return pk_error(lisp);
//             pk_try(pk_atom_eval(lisp, val->car));
//             count++;
//         }

//         int top = pk_get_top(lisp);
//         size_t i = 0;
//         pk_cdolist(lisp, binding, bindings) {
//             PKAtomCons *pair;
//             pk_try(pk_atom_cast_cons(lisp, binding, &pair));
//             PKAtomSymbol *sym;
//             pk_try(pk_atom_cast_symbol(lisp, pair->car, &sym));
//             PKAtom *value;
//             pk_try(pk_stack_get(lisp, (int)(top - count + i + 1), &value));
//             pk_try(pk_let_push(lisp, PKEnvTy_Var, sym, value));
//             i++;
//         }

//         pk_try(pk_popn(lisp, (int)count));
//         pk_try(pk_atom_evlist(lisp, body_forms));
//         pk_try(pk_let_pop(lisp, count));
//     } else if (symbol == lisp->cache.let_star) {
//         if (body->tag.ty != PKAtomTy_Cons) {
//             return pk_error(lisp);
//         }

//         PKAtomCons *body_cons = (PKAtomCons *)body;
//         PKAtom *bindings = body_cons->car;
//         PKAtom *body_forms = body_cons->cdr;

//         size_t count = 0;
//         pk_cdolist(lisp, binding, bindings) {
//             PKAtomCons *pair;
//             pk_try(pk_atom_cast_cons(lisp, binding, &pair));
//             PKAtomSymbol *sym;
//             pk_try(pk_atom_cast_symbol(lisp, pair->car, &sym));
//             PKAtomCons *val;
//             pk_try(pk_atom_cast_cons(lisp, pair->cdr, &val));
//             if (!pk_atom_is_nil(val->cdr)) return pk_error(lisp);
//             pk_try(pk_atom_eval(lisp, val->car));
//             PKAtom *value;
//             pk_try(pk_stack_pop(lisp, &value));
//             pk_try(pk_let_push(lisp, PKEnvTy_Var, sym, value));
//             count++;
//         }

//         pk_try(pk_atom_evlist(lisp, body_forms));
//         pk_try(pk_let_pop(lisp, count));
//     } else {
//         *output = false;
//     }
//     return PK_Ok;
// }

PKRes pk_interp_quote(Pocket lisp) {
    return pk_ret_this(lisp, lisp->current_frame.as.atom);
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
        pk_try(pk_frame_push(lisp, 0, PKEvalMode_Eval, (PKFrameData){.atom = lisp->current_frame.as.t.b}));
    } else {
        pk_try(pk_ret_nil(lisp));
    }
    return PK_Ok;
}

PKRes pk_interp_special_form(Pocket lisp, PKAtomSymbol *symbol, PKAtom *rest, PKAtom *expression, bool *is_special) {
    if (symbol == lisp->cache.lambda) {
        *is_special = true;
        // TODO: validate lambda structure in its creation.
        pk_try(pk_frame_push(lisp, 0, PKEvalMode_Quote, (PKFrameData){.atom = expression}));
    } else if (symbol == lisp->cache.quote) {
        *is_special = true;
        PKAtomCons *cons = NULL;
        pk_try(pk_atom_cast_cons(lisp, rest, &cons));
        if (!pk_atom_is_nil(cons->cdr)) {
            pk_error(lisp);
        }
        pk_try(pk_frame_push(lisp, 0, PKEvalMode_Quote, (PKFrameData){.atom = cons->car}));
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
        
    } else {
        *is_special = false;
    }
    return PK_Ok;
}
