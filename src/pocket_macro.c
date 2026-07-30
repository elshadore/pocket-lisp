#include "pocket_internals.h"

PK_RES pk_atom_macro_apply(Pocket lisp, PKAtom *function, PKAtom *args, PKAtom **output) {
    size_t acc = 0;
    PKAtom *iter = args;
    PK_RES result = PK_YIELD;
    PKCallConv call;
    
    pk_try(pk_frame_push(lisp, 0));
    
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

PK_RES pk_atom_macroexpand_1(Pocket lisp, PKAtom *atom, PKAtom **output);
    
PK_RES pk_atom_macroexpand_list(Pocket lisp, PKAtomCons *list, PKAtom **output) {
    PKAtom *iter = NULL;
    PKAtomCons *head = NULL;
    PKAtomCons *tail = NULL;
    PKAtom *result = NULL;

    pk_try(pk_atom_macroexpand_1(lisp, list->car, &result));
    pk_try(pk_atom_cons_car(lisp, result, &head));
    tail = head;
    iter = list->cdr;

    while (!pk_atom_is_nil(iter)) {
        PKAtomCons *cons = NULL;
        PKAtomCons *new_cons = NULL;
        PKAtom *result = NULL;
        
        pk_try(pk_atom_cast_cons(lisp, iter, &cons));
        pk_try(pk_atom_macroexpand_1(lisp, cons->car, &result));
        pk_try(pk_atom_cons_car(lisp, result, &new_cons));
        
        tail->cdr = (PKAtom *)new_cons;
        tail = new_cons;
        
        iter = cons->cdr;
    }

    *output = (PKAtom *)head;
    return PK_OK;
}

PK_RES pk_atom_macroexpand_1(Pocket lisp, PKAtom *atom, PKAtom **output) {
    switch (atom->tag.ty) {
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            if (pk_atom_is_symbol(cons->car)) {
                PKAtom *lookup = NULL;
                if (pk_env_query(lisp, PKEnvTy_Fun, (PKAtomSymbol *)cons->car, &lookup)) {
                    if (pk_atom_is_lmacro(lookup)) {
                        pk_try(pk_atom_macro_apply(lisp, lookup, cons->cdr, output));
                        return PK_OK;
                    }
                }
            }
            
            pk_try(pk_atom_macroexpand_list(lisp, cons, output));
            
            return PK_OK;
        }
        default: {
            *output = atom;
            return PK_OK;
        }
    }
}

PK_RES pk_atom_macroexpand(Pocket lisp, PKAtom *atom, PKAtom **output) {
    pk_try(pk_atom_macroexpand_1(lisp, atom, output));
    return PK_OK;
}
