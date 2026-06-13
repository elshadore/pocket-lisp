#include "pocket_internals.h"

Pocket pk_init(void *user_closure, PKAllocFn alloc) {
    Pocket lisp = (alloc)(user_closure, NULL, 0, sizeof(struct PocketLispMachine_));
    if (lisp == NULL) {
        return NULL;
    }
    *lisp = (struct PocketLispMachine_) {
        .user_closure = user_closure,
        .alloc = alloc,
        .stack = (PKStack){0},
        .free = NULL,
        .pool = NULL,
    };

    return lisp;
}

void pk_deinit(Pocket lisp) {
    if (lisp == NULL) {
        return;
    }
}
