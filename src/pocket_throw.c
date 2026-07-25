#include "pocket_internals.h"

PK_RES pk_error_impl(Pocket lisp, const char *file, int line) {
    fprintf(stderr, "ERROR: %s:%d\n\n", file, line);
    (void)pk_dump_trace(lisp, "error");
    (void)pk_dump_env(lisp, "error");
    return PK_YIELD;
}
