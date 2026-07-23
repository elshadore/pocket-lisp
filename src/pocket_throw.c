#include "pocket_internals.h"

PKRes pk_error_impl(Pocket lisp, const char *file, int line) {
    (void)lisp;
    fprintf(stderr, "ERROR: %s:%d\n", file, line);
    (void)pk_dump_env(lisp, "error");
    (void)pk_dump_stack(lisp, "error");
    return PK_Yield;
}
