#include "pocket_internals.h"

PK_RES pk_atom_macro_apply(Pocket lisp, PKAtom *function, PKAtom *args, PKAtom **output) {
    size_t acc = 0;
    PKAtom *iter = args;
    PK_RES result = PK_YIELD;
    PKCallConv call;

    pk_try(pk_frame_push(lisp, (PKAtom *)lisp->cache.macro_apply, 0));
    
    pk_defer(pk_push(lisp, function));

    while (!pk_atom_is_nil(iter)) {
        PKAtomCons *cons = NULL;
        pk_defer(pk_atom_cast_cons(lisp, iter, &cons));
        pk_defer(pk_push(lisp, cons->car));
        acc += 1;
        iter = cons->cdr;
    }

    pk_defer(pk_callconv(lisp, function, acc, PK_CALLFLAG_INSERT_RESULT | PK_CALLFLAG_MACRO_CALL, &call));

    pk_defer(pk_call(lisp, &call));

    *output = pk_stack_result(lisp);

    result = PK_OK;
    
    DEFER:
    
    pk_try(pk_frame_pop(lisp));
        
    return result;
}

PK_RES pk_atom_macroexpand_list(Pocket lisp, PKAtomCons *list, PKAtom **output, pk_bool *expanded) {
    PKAtom *iter = NULL;
    PKAtomCons *head = NULL;
    PKAtomCons *tail = NULL;
    PKAtom *result = NULL;

    pk_try(pk_atom_macroexpand_1(lisp, list->car, &result, expanded));
    pk_try(pk_atom_cons_car(lisp, result, &head));
    tail = head;
    iter = list->cdr;

    while (!pk_atom_is_nil(iter)) {
        PKAtomCons *cons = NULL;
        PKAtomCons *new_cons = NULL;
        PKAtom *result = NULL;
        
        pk_try(pk_atom_cast_cons(lisp, iter, &cons));
        pk_try(pk_atom_macroexpand_1(lisp, cons->car, &result, expanded));
        pk_try(pk_atom_cons_car(lisp, result, &new_cons));
        
        tail->cdr = (PKAtom *)new_cons;
        tail = new_cons;
        
        iter = cons->cdr;
    }

    *output = (PKAtom *)head;
    return PK_OK;
}

PK_RES pk_atom_macroexpand_let_args(Pocket lisp, PKAtom *args, PKAtom **output, pk_bool *expanded) {
    PKAtom *iter = args;
    
    PKAtomCons *head = NULL;
    PKAtomCons *tail = NULL;
    
    while (!pk_atom_is_nil(iter)) {
        PKAtomCons *a = NULL;
        PKAtomCons *b = NULL;
        PKAtomCons *c = NULL;
        PKAtomCons *d = NULL;
        PKAtomCons *e = NULL;
        PKAtomCons *next = NULL;
        PKAtom *result = NULL;
        
        pk_try(pk_atom_cast_cons(lisp, iter, &a));
        pk_try(pk_atom_cast_cons(lisp, a->car, &b));
        pk_try(pk_atom_cast_cons(lisp, b->cdr, &c));

        pk_try(pk_atom_assert_nil(lisp, c->cdr));
        
        pk_try(pk_atom_macroexpand_1(lisp, c->car, &result, expanded));
        pk_try(pk_atom_cons_car(lisp, result, &d));
        
        pk_try(pk_atom_cons(lisp, b->car, (PKAtom *)d, &e));
        pk_try(pk_atom_cons_car(lisp, (PKAtom *)e, &next));
        
        if (head == NULL) {
            head = next;
            tail = next;
        } else {
            tail->cdr = (PKAtom *)next;
            tail = next;
        }
                
        iter = a->cdr;
    }

    if (head == NULL) {
        *output = pk_atom_nil(lisp);
    } else {
        *output = (PKAtom *)head;
    }

    return PK_OK;
}
    
PK_RES pk_atom_macroexpand_let(Pocket lisp, PKAtomSymbol *let, PKAtom *body, PKAtom **output, pk_bool *expanded) {
    PKAtomCons *a = NULL;
    PKAtomCons *b = NULL;
    PKAtomCons *c = NULL;
    PKAtomCons *d = NULL;
    PKAtom *r_args = NULL;
    PKAtom *r_body = NULL;

    pk_try(pk_atom_cast_cons(lisp, body, &a));
    pk_try(pk_atom_cast_cons(lisp, a->cdr, &b));
    
    pk_try(pk_atom_macroexpand_let_args(lisp, a->car, &r_args, expanded));
    pk_try(pk_atom_macroexpand_list(lisp, b, &r_body, expanded));
    
    pk_try(pk_atom_cons(lisp, r_args, r_body, &c));
    pk_try(pk_atom_cons(lisp, (PKAtom *)let, (PKAtom *)c, &d));

    *output = (PKAtom *)d;
    
    return PK_OK;
}
    
PK_RES pk_atom_macroexpand_1(Pocket lisp, PKAtom *atom, PKAtom **output, pk_bool *expanded) {
    switch (atom->tag.ty) {
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            if (pk_atom_is_symbol(cons->car)) {
                PKAtom *lookup = NULL;
                PKAtomSymbol *symbol = (PKAtomSymbol *)cons->car;
                if ((symbol == lisp->cache.let_sym) ||
                    (symbol == lisp->cache.let_star) ||
                    (symbol == lisp->cache.flet_sym) ||
                    (symbol == lisp->cache.flet_star)
                ) {
                    pk_try(pk_atom_macroexpand_let(lisp, symbol, cons->cdr, output, expanded));
                    return PK_OK;
                } else if (pk_env_query(lisp, PKEnvTy_Fun, symbol, &lookup)) {
                    if (pk_atom_is_lmacro(lookup)) {
                        pk_try(pk_atom_macro_apply(lisp, lookup, cons->cdr, output));
                        *expanded = PK_TRUE;
                        return PK_OK;
                    }
                }
            }
            
            pk_try(pk_atom_macroexpand_list(lisp, cons, output, expanded));
            
            return PK_OK;
        }
        default: {
            *output = atom;
            return PK_OK;
        }
    }
}

PK_RES pk_atom_macroexpand(Pocket lisp, PKAtom *atom, PKAtom **output) {
    pk_bool expanded = PK_FALSE;
    PKAtom *result = atom;
    
    do {
        expanded = PK_FALSE;
        pk_try(pk_atom_macroexpand_1(lisp, result, &result, &expanded));
    } while (expanded);


    /*
    printf("MACRO-EXPAND: ");
    pk_try(pk_print_atom(lisp, atom));
    printf("MACRO-EXPAND-RESULT: ");
    pk_try(pk_print_atom(lisp, result));
    */
    
    *output = result;
    
    return PK_OK;
}
