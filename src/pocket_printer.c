#include "pocket_internals.h"

void pk_puts(Pocket lisp, char *c, size_t length) {
    (lisp->print)(lisp->user_env, c, length);
}

void pk_print(Pocket lisp, char *c, size_t length) {
    pk_puts(lisp, c, length);
    pk_newline(lisp);
}

void pk_newline(Pocket lisp) {
    pk_puts(lisp, "\n", 1);
}

PK_RES pk_print_atom(Pocket lisp, PKAtom *atom) {
    PKWriter w = pk_writer_init(lisp);
    PK_RES result = PK_YIELD;
    
    pk_defer(pk_writer_atom(&w, atom));
    pk_writer_print(&w);
    result = PK_OK;
    
    DEFER:
    pk_writer_deinit(&w);
    return result;
}

PK_RES pk_puts_atom(Pocket lisp, PKAtom *atom) {
    PKWriter w = pk_writer_init(lisp);
    PK_RES result = PK_YIELD;
    
    pk_defer(pk_writer_atom(&w, atom));
    pk_writer_puts(&w);
    result = PK_OK;
    
    DEFER:
    pk_writer_deinit(&w);
    return result;
}
