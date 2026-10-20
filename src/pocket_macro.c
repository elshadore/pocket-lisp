#include "pocket_internals.h"

bool pk_macroexpand1_(Pocket lisp, int atom);

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

bool pk_macroexpand_is_literal(Pocket lisp, PKAtom *head) {
    return (head == (PKAtom *)lisp->cache.quote) ||
           (head == (PKAtom *)lisp->cache.quasiquote);
}

bool pk_macroexpand_is_let(Pocket lisp, PKAtom *head) {
    return (head == (PKAtom *)lisp->cache.let) ||
           (head == (PKAtom *)lisp->cache.let_star) ||
           (head == (PKAtom *)lisp->cache.flet) ||
           (head == (PKAtom *)lisp->cache.flet_star);
}

bool pk_macroexpand_is_lambda(Pocket lisp, PKAtom *head) {
    return (head == (PKAtom *)lisp->cache.lambda) ||
           (head == (PKAtom *)lisp->cache.macro);
}

bool pk_macroexpand_is_def(Pocket lisp, PKAtom *head) {
    return (head == (PKAtom *)lisp->cache.defun) ||
           (head == (PKAtom *)lisp->cache.defmacro);
}

bool pk_macroexpand_special_(Pocket lisp, size_t abs, PKAtom *head) {
    PKAtom *form = lisp->stack.e[abs];
    PKAtomCons *rest = pk_atom_assert_cons(lisp, form->cons.cdr);

    PKAtom *data = rest->car;
    PKAtom *body = rest->cdr;

    bool is_lambda = pk_macroexpand_is_lambda(lisp, head);
    bool is_def = pk_macroexpand_is_def(lisp, head);
    PKAtom *def_symbol = NULL;

    if (is_def) {
        PKAtomCons *def = pk_atom_assert_cons(lisp, body);
        def_symbol = data;
        data = def->car;
        body = def->cdr;
        is_lambda = true;
    }

    bool expanded = false;
    size_t count = 0;

    pk_stack_push(lisp, head);
    count += 1;

    if (is_def) {
        pk_stack_push(lisp, def_symbol);
        count += 1;
    }

    if (is_lambda) {
        pk_stack_push(lisp, data);
        count += 1;
    } else {
        PKAtom *iter = (PKAtom *)pk_atom_assert_cons(lisp, data);

        size_t pairs = 0;
        while (pk_atom_is_cons(iter)) {
            PKAtom *binding = iter->cons.car;

            if (pk_atom_is_cons(binding)) {
                PKAtomCons *b = (PKAtomCons *)binding;

                pk_stack_push(lisp, b->car);

                if (pk_atom_is_cons(b->cdr)) {
                    pk_stack_push(lisp, b->cdr->cons.car);
                    expanded |= pk_macroexpand1_(lisp, -1);
                } else {
                    pk_stack_push(lisp, lisp->cache.nil);
                }
            } else {
                pk_stack_push(lisp, binding);
                pk_stack_push(lisp, lisp->cache.nil);
            }

            pk_stack_op_list(lisp, 2);

            pairs += 1;
            iter = iter->cons.cdr;
        }

        if (pairs == 0) {
            pk_stack_push(lisp, lisp->cache.nil);
        } else {
            pk_stack_op_list(lisp, pairs);
        }

        count += 1;
    }

    size_t forms = 0;
    while (pk_atom_is_cons(body)) {
        pk_stack_push(lisp, body->cons.car);
        expanded |= pk_macroexpand1_(lisp, -1);
        forms += 1;
        body = body->cons.cdr;
    }

    if (!pk_atom_is_nil(body)) {
        pk_stack_push(lisp, body);
        expanded |= pk_macroexpand1_(lisp, -1);
        forms += 1;
    }

    if (forms == 0) {
        pk_stack_push(lisp, lisp->cache.nil);
        forms = 1;
    }

    count += forms;

    pk_stack_op_list(lisp, count);

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
        if (pk_macroexpand_is_literal(lisp, head)) {
            return false;
        }

        if (pk_macroexpand_is_let(lisp, head) || pk_macroexpand_is_lambda(lisp, head) ||
            pk_macroexpand_is_def(lisp, head)) {
            return pk_macroexpand_special_(lisp, abs, head);
        }

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