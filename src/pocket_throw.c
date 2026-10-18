#include "pocket_internals.h"

void pk_error_atom(Pocket lisp, PKAtom *message) {
    PKWriter w = PK_WRITER_EMPTY;
    pk_writer_string(lisp, &w, pk_str("error: "));
    pk_writer_atom(lisp, &w, message);
    pk_internal_println(lisp, pk_writer_slice(&w));
    pk_writer_deinit(lisp, &w);
}

void pk_errors(Pocket lisp, PKStr message) {
    (void)lisp;
    fprintf(stderr, "error: %.*s\n", (int)message.length, message.c);
    abort();
}

PK_PRINTF_FMT(2, 3) void pk_errorf(Pocket lisp, const char *fmt, ...) {
    PKWriter w = PK_WRITER_EMPTY;
    va_list args;

    va_start(args, fmt);
    pk_writer_vfmt(lisp, &w, fmt, args);
    va_end(args);

    pk_errors(lisp, pk_writer_slice(&w));
}

void pk_error_type(Pocket lisp, PK_TYPE expected, PKAtom *found) {
    PKWriter w = PK_WRITER_EMPTY;
    pk_writer_cstr(lisp, &w, "expected ");
    pk_writer_string(lisp, &w, pk_ident_type(expected));
    pk_writer_cstr(lisp, &w, ", got ");
    pk_writer_atom(lisp, &w, found);
    pk_errors(lisp, pk_writer_slice(&w));
    pk_writer_deinit(lisp, &w);
    abort();
}

void pk_error_oom(Pocket lisp) {
    pk_errors(lisp, pk_str("out of memory"));
    abort();
}

void pk_throw(Pocket lisp, int symbol, int value) {
    PKWriter w = PK_WRITER_EMPTY;
    pk_writer_cstr(lisp, &w, "throw ");
    pk_writer_atom(lisp, &w, pk_stack_get(lisp, symbol));
    pk_writer_cstr(lisp, &w, " ");
    pk_writer_atom(lisp, &w, pk_stack_get(lisp, value));
    pk_errors(lisp, pk_writer_slice(&w));
    pk_writer_deinit(lisp, &w);
    abort();
}
