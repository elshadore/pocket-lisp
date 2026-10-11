#include "pocket_internals.h"

PKAtom pk_macroexpand_list(PKAtom list, bool *expanded) {
    PKAccum acc = pk_accum_new();

    PKAtom iter = list;
    while (pk_is_cons(iter)) {
        PKAtom atom = pk_macroexpand1_(iter->cons.car, expanded);
        pk_propagate(pk_accum_push(&acc, atom));
        iter = iter->cons.cdr;
    }

    return acc.head;
}

PKAtom pk_macroexpand_let_args(PKAtom args, bool *expanded) {
    PKAccum acc = pk_accum_new();
    
    PKAtom iter = args;
    while (pk_is_cons(iter)) {
        PKAtom a = iter->cons.car;
        pk_propagate(pk_typecheck_cons(a));
        PKAtom b = a->cons.cdr;
        pk_propagate(pk_typecheck_cons(b));
        pk_propagate(pk_typecheck_nil(b->cons.cdr));
        
        PKAtom atom = pk_macroexpand1_(b->cons.car, expanded);
        pk_propagate(atom);

        PKAtom result = pk_list2(a->cons.car, atom);
        
        pk_propagate(result);
        pk_propagate(pk_accum_push(&acc, result));
        
        iter = iter->cons.cdr;
    }

    return acc.head;
}
    
PKAtom pk_macroexpand_let(PKAtom let, PKAtom body, bool *expanded) {
    PKAtom args = pk_macroexpand_let_args(body->cons.car, expanded);
    pk_propagate(args);
    PKAtom rbody = pk_macroexpand_list(body->cons.cdr, expanded);
    pk_propagate(rbody);
    
    PKAccum acc = pk_accum_new();
    pk_propagate(pk_accum_push(&acc, let));
    pk_propagate(pk_accum_push(&acc, args));
    PKAtom iter = rbody;
    while (pk_is_cons(iter)) {
        pk_propagate(pk_accum_push(&acc, iter->cons.car));
        iter = iter->cons.cdr;
    }
    return acc.head;
}
    
PKAtom pk_macroexpand1_(PKAtom atom, bool *expanded) {
    if (!pk_is_cons(atom)) {
        return atom;
    }
    PKAtom head = atom->cons.car;
    if (pk_is_symbol(head)) {
        if ((head == PK_CORE.cache.let) ||
            (head == PK_CORE.cache.let_star) ||
            (head == PK_CORE.cache.flet) ||
            (head == PK_CORE.cache.flet_star)
        ) {
            return pk_macroexpand_let(head, atom->cons.cdr, expanded);
        }
        PKAtom get = pk_get(PK_ENV_FUN, head);
        if (pk_is_macro(get)) {
            *expanded = true;
            return pk_apply(PK_CALL_MACRO, get, atom->cons.cdr);
        }
    }
    
    return pk_macroexpand_list(atom, expanded);
}

PKAtom pk_macroexpand1(PKAtom atom) {
    bool ignore = false;
    return pk_macroexpand1_(atom, &ignore);
}
    
PKAtom pk_macroexpand(PKAtom atom) {
    PKAtom result = atom;
    for(;;) {
        bool expanded = false;
        result = pk_macroexpand1_(result, &expanded);
        if (!expanded) {
            break;
        }
    }
    return result;
}
