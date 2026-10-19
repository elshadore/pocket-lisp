#include "pocket_internals.h"

void pk_throw_now(Pocket lisp) {
    if (lisp->current_jmp != NULL) {
        longjmp(lisp->current_jmp->buf, 1);
    }
    fprintf(stderr, "error: unprotected\n");
    abort();
}

void pk_error_writer(Pocket lisp, PKWriter *w) {
    lisp->throw_symbol = (PKAtom *)lisp->cache.error;
    lisp->throw_value = (PKAtom *)pk_atom_string(lisp, pk_writer_slice(w));
    pk_writer_deinit(lisp, w);
    pk_throw_now(lisp);
}

void pk_propagate(Pocket lisp) {
    if (lisp->throw_symbol == lisp->cache.nil) {
        pk_errors(lisp, pk_str("pk_propagate: no active throw"));
        return;
    }
    pk_throw_now(lisp);
}

PKAtom *pk_error_value(Pocket lisp) {
    if (lisp->throw_value == NULL) {
        return lisp->cache.nil;
    }
    return lisp->throw_value;
}

void pk_error_atom(Pocket lisp, PKAtom *message) {
    PKWriter w = PK_WRITER_EMPTY;
    pk_writer_string(lisp, &w, pk_str("error: "));
    pk_writer_atom(lisp, &w, message);
    pk_internal_println(lisp, pk_writer_slice(&w));
    pk_writer_deinit(lisp, &w);
}

void pk_errors(Pocket lisp, PKStr message) {
    pk_internal_println(lisp, message);
    pk_throw_now(lisp);
}

PK_PRINTF_FMT(2, 3) void pk_errorf(Pocket lisp, const char *fmt, ...) {
    PKWriter w = PK_WRITER_EMPTY;
    va_list args;

    va_start(args, fmt);
    pk_writer_vfmt(lisp, &w, fmt, args);
    va_end(args);

    pk_error_writer(lisp, &w);
}

void pk_error_type(Pocket lisp, PK_TYPE expected, PKAtom *found) {
    PKWriter w = PK_WRITER_EMPTY;
    pk_writer_cstr(lisp, &w, "expected ");
    pk_writer_string(lisp, &w, pk_ident_type(expected));
    pk_writer_cstr(lisp, &w, ", got ");
    pk_writer_atom(lisp, &w, found);
    pk_error_writer(lisp, &w);
}

void pk_error_oom(Pocket lisp) {
    pk_internal_println(lisp, pk_str("error: out of memory"));
    pk_throw_now(lisp);
}

void pk_throw(Pocket lisp, int symbol, int value) {
    lisp->throw_symbol = pk_stack_get(lisp, symbol);
    lisp->throw_value = pk_stack_get(lisp, value);

    PKWriter w = PK_WRITER_EMPTY;
    pk_writer_cstr(lisp, &w, "throw ");
    pk_writer_atom(lisp, &w, lisp->throw_symbol);
    pk_writer_cstr(lisp, &w, " ");
    pk_writer_atom(lisp, &w, lisp->throw_value);
    pk_internal_println(lisp, pk_writer_slice(&w));
    pk_writer_deinit(lisp, &w);

    pk_throw_now(lisp);
}
