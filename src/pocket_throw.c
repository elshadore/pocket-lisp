#include "pocket_internals.h"

PK_RES pk_error_impl(Pocket lisp, const char *file, int line) {
    fprintf(stderr, "ERROR: %s:%d\n\n", file, line);
    (void)pk_dump_trace(lisp, "error");
    (void)pk_dump_env(lisp, "error");
    pk_atom_throw(lisp, lisp->cache.error);
    return PK_YIELD;
}

void pk_abort(Pocket lisp, const char *message) {
    (void)lisp;
    fprintf(stderr, "ABORT: %s\n", message);
}

void pk_atom_throw(Pocket lisp, PKAtomSymbol *symbol) {
    lisp->throwing = symbol;
}

pk_bool pk_atom_is_throwing(Pocket lisp, PKAtomSymbol *symbol) {
    if (pk_atom_throwing(lisp) == symbol) {
        return PK_TRUE;
    } else {
        return PK_FALSE;
    }
}

void pk_catch_all(Pocket lisp) {
    lisp->throwing = lisp->cache.error;
}

