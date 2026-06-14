#define STB_LEAKCHECK_IMPLEMENTATION
#include "./lib/stb_leakcheck.h"

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

void repl_print(void *user_closure, char *c, size_t length) {
    (void)user_closure;
    printf("%.*s", (int)length, c);
}

int main(void) {
    printf("henlo word!\n");
    Pocket lisp = pk_init(NULL, repl_alloc, repl_print);
    if (lisp == NULL) {
        return EXIT_FAILURE;
    }
    pk_push_int(lisp, 69);
    pk_push_int(lisp, 420);
    pk_add(lisp, -1, -2);
    pk_read_cstr(lisp, "(+ 1 2 3 4)");
    pk_stack_dump(lisp);
    pk_deinit(lisp);
    stb_leakcheck_dumpmem();
    return EXIT_SUCCESS;
}
