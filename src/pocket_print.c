#include "pocket_internals.h"

void pk_atom_print(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_string(atom)) {
        PKAtomString *string = (PKAtomString *)atom;
        fprintf(stderr, "%.*s\n", (int)string->str.length, string->str.c);
    } else {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_atom(lisp, &w, atom);
        fprintf(stderr, "%.*s\n", (int)w.count, w.c);
        pk_writer_deinit(lisp, &w);
    }
}

void pk_atom_write(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_string(atom)) {
        PKAtomString *string = (PKAtomString *)atom;
        fprintf(stderr, "%.*s", (int)string->str.length, string->str.c);
    } else {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_atom(lisp, &w, atom);
        fprintf(stderr, "%.*s", (int)w.count, w.c);
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
