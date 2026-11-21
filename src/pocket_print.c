#include "pocket_internals.h"

void pk_internal_print(Pocket lisp, PKStr print) {
    (void)lisp;
    fprintf(stderr, "%.*s\n", (int)print.length, print.c);
    fflush(stderr);
}

void pk_internal_puts(Pocket lisp, PKStr print) {
    (void)lisp;
    fprintf(stderr, "%.*s", (int)print.length, print.c);
    fflush(stderr);
}

void pk_atom_print(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_string(atom)) {
        PKAtomString *string = (PKAtomString *)atom;
        pk_internal_print(lisp, string->str);
        return;
    }

    pk_writer_reset(lisp);
    pk_writer_atom(lisp, atom);
    pk_internal_print(lisp, pk_writer_str(lisp));
    pk_writer_reset(lisp);
}

void pk_atom_puts(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_string(atom)) {
        PKAtomString *string = (PKAtomString *)atom;
        pk_internal_puts(lisp, string->str);
        return;
    }
    
    pk_writer_reset(lisp);
    pk_writer_atom(lisp, atom);
    pk_internal_puts(lisp, pk_writer_str(lisp));
    pk_writer_reset(lisp);
}

PKAtomString *pk_atom_format(Pocket lisp, PKAtom *atom) {
    pk_writer_reset(lisp);
    pk_writer_atom(lisp, atom);
    pk_internal_puts(lisp, pk_writer_str(lisp));
    PKAtomString *result = pk_atom_string(lisp, pk_writer_str(lisp));
    pk_writer_reset(lisp);
    return result;
}
