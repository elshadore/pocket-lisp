#include "pocket_internals.h"

PKRes pk_error_impl(Pocket lisp, const char *file, int line) {
    (void)lisp;
    fprintf(stderr, "ERROR: %s:%d\n", file, line);
    /* fprintf(stderr, "[READER] %zu => %c\n%.*s\n", lisp->read.curr, lisp->c, (int)lisp->read.src.length, lisp->read.src.c); */
    (void)pk_dump_stack(lisp, "error");
    return PK_Yield;
}
