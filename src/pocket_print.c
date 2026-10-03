#include "pocket_internals.h"

PKAtom pk_print(PKAtom atom) {
    PKWriter w = PK_WRITER_EMPTY;
    PKAtom result = pk_writer_atom(&w, atom);
    if (!pk_is_error(atom)) {
        fprintf(stderr, "%.*s\n", (int)w.count, w.c);
    }
    pk_writer_deinit(&w);
    return result;
}

PKAtom pk_format(PKAtom atom) {
    PKWriter w = PK_WRITER_EMPTY;
    pk_propagate(pk_writer_atom(&w, atom));
    PKAtom result = pk_string(pk_writer_slice(&w));
    pk_writer_deinit(&w);
    return result;
}
