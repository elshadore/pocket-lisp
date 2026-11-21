#include "pocket_internals.h"

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

void pk_error_writer(Pocket lisp) {
    pk_errors(lisp, pk_writer_str(lisp));
}

PK_PRINTF_FMT(2, 3) void pk_errorf(Pocket lisp, const char *fmt, ...) {
    pk_writer_reset(lisp);
    
    va_list args;

    va_start(args, fmt);
    pk_writer_vfmt(lisp, fmt, args);
    va_end(args);

    pk_error_writer(lisp);
}

void pk_error_type(Pocket lisp, PKAtom *found, PK_TYPE expected) {
    pk_writer_reset(lisp);
    pk_writer_string(lisp, pk_str("expected "));
    pk_writer_string(lisp, pk_ident_type(expected));
    pk_writer_string(lisp, pk_str(", got "));
    pk_writer_atom(lisp, found);
    pk_error_writer(lisp);
}

void pk_error_type2(Pocket lisp, PKAtom *found, PK_TYPE expected1, PK_TYPE expected2) {
    pk_writer_reset(lisp);
    pk_writer_string(lisp, pk_str("expected "));
    pk_writer_string(lisp, pk_ident_type(expected1));
    pk_writer_string(lisp, pk_str(", "));
    pk_writer_string(lisp, pk_ident_type(expected2));
    pk_writer_string(lisp, pk_str(", got "));
    pk_writer_atom(lisp, found);
    pk_error_writer(lisp);
}

void pk_error_type3(Pocket lisp, PKAtom *found, PK_TYPE expected1, PK_TYPE expected2, PK_TYPE expected3) {
    pk_writer_reset(lisp);
    pk_writer_string(lisp, pk_str("expected "));
    pk_writer_string(lisp, pk_ident_type(expected1));
    pk_writer_string(lisp, pk_str(", "));
    pk_writer_string(lisp, pk_ident_type(expected2));
    pk_writer_string(lisp, pk_str(", "));
    pk_writer_string(lisp, pk_ident_type(expected3));
    pk_writer_string(lisp, pk_str(", got "));
    pk_writer_atom(lisp, found);
    pk_error_writer(lisp);
}

void pk_error_type4(Pocket lisp, PKAtom *found, PK_TYPE expected1, PK_TYPE expected2, PK_TYPE expected3, PK_TYPE expected4) {
    pk_writer_reset(lisp);
    pk_writer_string(lisp, pk_str("expected "));
    pk_writer_string(lisp, pk_ident_type(expected1));
    pk_writer_string(lisp, pk_str(", "));
    pk_writer_string(lisp, pk_ident_type(expected2));
    pk_writer_string(lisp, pk_str(", "));
    pk_writer_string(lisp, pk_ident_type(expected3));
    pk_writer_string(lisp, pk_str(", "));
    pk_writer_string(lisp, pk_ident_type(expected4));
    pk_writer_string(lisp, pk_str(", got "));
    pk_writer_atom(lisp, found);
    pk_error_writer(lisp);
}

void pk_error_oom(Pocket lisp) {
    pk_error_atom(lisp, (PKAtom *)lisp->cache.empty_string);
}

void pk_throw(Pocket lisp, int symbol, int value) {
    PKAtomSymbol *a = pk_atom_assert_symbol(lisp, pk_stack_get(lisp, symbol));
    PKAtom *b = pk_stack_get(lisp, value);
    pk_jmp_trigger(lisp, a, b);
}
