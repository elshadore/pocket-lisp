#include "user_internals.h"

ULAtom ul_print(ULAtom atom) {
    ULWriter w = UL_WRITER_EMPTY;
    ULAtom result = ul_writer_atom(&w, atom);
    if (!ul_is_error(atom)) {
        fprintf(stderr, "%.*s\n", (int)w.count, w.c);
    }
    ul_writer_deinit(&w);
    return result;
}

ULAtom ul_format(ULAtom atom) {
    ULWriter w = UL_WRITER_EMPTY;
    ul_propagate(ul_writer_atom(&w, atom));
    ULAtom result = ul_string(ul_writer_slice(&w));
    ul_writer_deinit(&w);
    return result;
}
