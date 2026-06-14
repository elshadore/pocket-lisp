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
