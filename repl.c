#include "./src/pocket.h"

void *repl_alloc(void *user_closure, void *ptr, size_t old_size, size_t new_size) {
    (void)user_closure;
    if (old_size == 0) {
        if (new_size == 0) {
            return NULL;
        } else {
            return malloc(new_size);
        }
    } else {
        if (new_size == 0) {
            free(ptr);
            return NULL;
        } else {
            return realloc(ptr, new_size);
        }
    }
}

int main(void) {
    printf("henlo word!\n");
    Pocket lisp = pk_init(NULL, repl_alloc);
    if (lisp == NULL) {
        return EXIT_FAILURE;
    }
    pk_deinit(lisp);
    return EXIT_SUCCESS;
}
