#include "pocket_internals.h"

bool pk_macroexpand1_(Pocket lisp, int atom);

bool pk_macroexpand_list(Pocket lisp, int list);

bool pk_macroexpand_list(Pocket lisp, int list) {
    size_t abs = pk_sp_index(lisp, list);

    bool expanded = false;
    size_t count = 0;

    PKAtom *iter = lisp->stack.e[abs];
    while (pk_atom_is_cons(iter)) {
        pk_stack_push(lisp, iter->cons.car);
        expanded |= pk_macroexpand1_(lisp, -1);
        count += 1;
        iter = iter->cons.cdr;
    }

    if (pk_atom_is_nil(iter)) {
        pk_stack_op_list(lisp, count);
    } else {
        pk_stack_push(lisp, iter);
        expanded |= pk_macroexpand1_(lisp, -1);
        pk_stack_op_list_packed(lisp, count + 1);
    }

    lisp->stack.e[abs] = pk_stack_head(lisp);
    pk_pop(lisp);

    return expanded;
}

bool pk_macroexpand1_(Pocket lisp, int atom) {
    size_t abs = pk_sp_index(lisp, atom);

    PKAtom *a = lisp->stack.e[abs];
    if (!pk_atom_is_cons(a)) {
        return false;
    }

    PKAtom *head = a->cons.car;

    if (pk_atom_is_symbol(head)) {
        PKAtomSymbol *asymbol = (PKAtomSymbol *)head;
        PKAtom *get = pk_env_get(lisp, PK_ENV_FUN, asymbol);
        if (pk_atom_is_macro(get)) {
            pk_atom_apply(lisp, PK_CALL_MACRO, get, a->cons.cdr);
            lisp->stack.e[abs] = pk_stack_head(lisp);
            pk_pop(lisp);
            return true;
        }
    }

    return pk_macroexpand_list(lisp, atom);
}

bool pk_macroexpand_(Pocket lisp, int atom) {
    if (!pk_macroexpand1_(lisp, atom)) {
        return false;
    }
    for (;;) {
        if (!pk_macroexpand1_(lisp, atom)) {
            return true;
        }
    }
}

bool pk_atom_is_macro(PKAtom *atom) {
    return (atom->header.type == PK_ATOMTY_CMACRO) || (atom->header.type == PK_ATOMTY_LMACRO);
}
