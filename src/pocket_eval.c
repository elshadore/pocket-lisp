#include "pocket_internals.h"

PK_RES pk_atom_apply_atom(Pocket lisp, PK_CALL call, PKAtom *fun, PKAtom *args) {
    PKAtoms atoms = {
        .e = &args,
        .length = 1,
    };
    return pk_atom_apply(lisp, call, fun, atoms);
}
    
PK_RES pk_atom_apply(Pocket lisp, PK_CALL call, PKAtom *fun, PKAtoms args) {
    size_t total = args.length > 0 ? args.length - 1 : 0;
    if (args.length > 0) {
        PKAtom *iter = args.e[args.length - 1];
        while (pk_atom_is_cons(iter)) {
            total += 1;
            iter = iter->cons.cdr;
        }
    }

    if (total > UINT8_MAX) {
        pk_errorf(lisp, "pk_atom_apply: more than %u arguments", UINT8_MAX);
        return PK_YIELDED;
    }

    PKAtom **values = NULL;
    if (args.length > 0) {
        values = pk_malloc(lisp, args.length * sizeof(PKAtom *));
        memcpy(values, args.e, args.length * sizeof(PKAtom *));
    }

    for (size_t i = 0; i + 1 < args.length; ++i) {
        pk_stack_push(lisp, values[i]);
    }
    if (args.length > 0) {
        PKAtom *iter = values[args.length - 1];
        while (pk_atom_is_cons(iter)) {
            pk_stack_push(lisp, iter->cons.car);
            iter = iter->cons.cdr;
        }
    }
    pk_free(lisp, values);

    pk_stack_push(lisp, fun);

    return pk_int_funcall(lisp, call, (pk_u8)total);
}

PK_RES pk_atom_eval_(Pocket lisp, PKAtom *expr) {
    pk_stack_push(lisp, expr);
    (void)pk_macroexpand_(lisp, -1);
    pk_stack_set(lisp, -1, pk_atom_compile(lisp, PK_CALL_FUN, pk_stack_get(lisp, -1), PK_MODE_EXPRESSION));
    return pk_int_funcall(lisp, PK_CALL_FUN, 0);
}

PK_RES pk_atom_evlist_(Pocket lisp, PKAtom *exprlist) {
    pk_frame_push(lisp, exprlist, 0);
    pk_stack_push(lisp, exprlist);
    PKAtom *iter = exprlist;
    while (pk_atom_is_cons(iter)) {
        PK_RES res = pk_atom_eval_(lisp, iter->cons.car);
        if (res == PK_YIELDED) {
            return res;
        }
        iter = iter->cons.cdr;
    }
    pk_frame_unwind(lisp);
    pk_stack_push(lisp, &PK_ATOM_NIL);

    return PK_OK;
}

PK_RES pk_atom_eval(Pocket lisp, PKAtom *expr, PK_MODE mode) {
    if (mode == PK_MODE_EXPRESSION) {
        return pk_atom_eval_(lisp, expr);
    } else {
        pk_pop(lisp);
        return pk_atom_evlist_(lisp, expr);
    }
}
