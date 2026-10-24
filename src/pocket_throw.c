#include "pocket_internals.h"

void pk_error_writer(Pocket lisp, PKWriter *w) {
    PKAtomString *message = pk_atom_string(lisp, pk_writer_slice(w));
    pk_writer_deinit(lisp, w);
    pk_jmp_trigger(lisp, lisp->cache.error, (PKAtom *)message);
}

void pk_propagate(Pocket lisp) {
    if (lisp->throw_symbol == lisp->cache.nil) {
        pk_errors(lisp, pk_str("not in a propagate context"));
    } else {
        pk_jmp_trigger(lisp, (PKAtomSymbol *)lisp->throw_symbol, lisp->throw_value);
    }
}

void pk_error_atom(Pocket lisp, PKAtom *message) {
    pk_jmp_trigger(lisp, lisp->cache.error, message);
}

void pk_errors(Pocket lisp, PKStr message) {
    PKAtomString *string = pk_atom_string(lisp, message);
    pk_jmp_trigger(lisp, lisp->cache.error, (PKAtom *)string);
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
    pk_error_atom(lisp, (PKAtom *)lisp->cache.empty_string);
}

void pk_throw(Pocket lisp, int symbol, int value) {
    PKAtomSymbol *a = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, symbol));
    PKAtom *b = pk_stack_get(lisp, value);
    pk_jmp_trigger(lisp, a, b);
}
