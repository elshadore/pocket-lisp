#include "pocket_internals.h"

void pk_puts_str(PKStr string) {
    fprintf(stderr, "%.*s", (int)string.length, string.c);
}

void pk_print_str(PKStr string) {
    fprintf(stderr, "%.*s\n", (int)string.length, string.c);
}

PKAtom pk_print(PKAtom atom) {
    if (pk_is_string(atom)) {
        pk_print_str(atom->string.str);
        return pk_true();
    } else {
        PKWriter w = PK_WRITER_EMPTY;
        PKAtom result = pk_writer_atom(&w, atom);
        if (!pk_is_error(atom)) {
            pk_print_str(pk_writer_slice(&w));
        }
        pk_writer_deinit(&w);
        return result;
    }
}

PKAtom pk_puts(PKAtom atom) {
    if (pk_is_string(atom)) {
        pk_puts_str(atom->string.str);
        return pk_true();
    } else {
        PKWriter w = PK_WRITER_EMPTY;
        PKAtom result = pk_writer_atom(&w, atom);
        if (!pk_is_error(atom)) {
            pk_puts_str(pk_writer_slice(&w));
        }
        pk_writer_deinit(&w);
        return result;
    }
}

PKAtom pk_format(PKAtom atom) {
    PKWriter w = PK_WRITER_EMPTY;
    pk_propagate(pk_writer_atom(&w, atom));
    PKAtom result = pk_string(pk_writer_slice(&w));
    pk_writer_deinit(&w);
    return result;
}
