#include "pocket_internals.h"

PKString pk_string_dupe(Pocket lisp, PKString string) {
    char *copy = pk_malloc(lisp, string.length);
    memcpy(copy, string.c, string.length);
    return (PKString){ .c = copy, .length = string.length };
}

void pk_string_free(Pocket lisp, PKString string) {
    pk_free(lisp, string.c, string.length);
}

PKString pk_string_from_cstr(char *cstr) {
    return (PKString){ .c = cstr, .length = strlen(cstr) };
}

bool pk_string_eq(PKString a, PKString b) {
    if (a.length != b.length) return false;
    return memcmp(a.c, b.c, a.length) == 0;
}

void pk_stack_dump(Pocket lisp) {
    int top = pk_get_top(lisp);
    PKWriter w = pk_writer_init(lisp);
    pk_writer_printf(&w, "*~STACK-DUMP~*\n");
    for (int i = top; i > 0; i--) {
        int rel = pk_sp_relative(lisp, i);
        pk_writer_printf(&w, "[%d/%d] => ", rel, i);
        pk_writer_atom(&w, pk_stack_get(lisp, i));
        pk_writer_newline(&w);
    }
    pk_writer_print(&w);
    pk_writer_deinit(&w);
}
