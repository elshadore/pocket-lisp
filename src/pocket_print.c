#include "pocket_internals.h"

void pk_internal_print(Pocket lisp, PKStr print) {
    (void)lisp;
    fprintf(stderr, "%.*s", (int)print.length, print.c);
    fflush(stderr);
}

void pk_internal_println(Pocket lisp, PKStr print) {
    (void)lisp;
    fprintf(stderr, "%.*s\n", (int)print.length, print.c);
    fflush(stderr);
}

void pk_atom_print(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_string(atom)) {
        PKAtomString *string = (PKAtomString *)atom;
        pk_internal_println(lisp, string->str);
    } else {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_atom(lisp, &w, atom);
        pk_internal_println(lisp, pk_writer_slice(&w));
        pk_writer_deinit(lisp, &w);
    }
}

void pk_atom_write(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_string(atom)) {
        PKAtomString *string = (PKAtomString *)atom;
        pk_internal_print(lisp, string->str);
    } else {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_atom(lisp, &w, atom);
        pk_internal_print(lisp, pk_writer_slice(&w));
        pk_writer_deinit(lisp, &w);
    }
}

PKAtomString *pk_atom_format(Pocket lisp, PKAtom *atom) {
    PKWriter w = PK_WRITER_EMPTY;
    pk_writer_atom(lisp, &w, atom);
    PKAtomString *result = pk_atom_string(lisp, pk_writer_slice(&w));
    pk_writer_deinit(lisp, &w);
    return result;
}
