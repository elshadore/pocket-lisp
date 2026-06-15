#define STB_LEAKCHECK_IMPLEMENTATION
#include "./lib/stb_leakcheck.h"

#include "./src/pocket.h"

void *repl_alloc(void *user_env, void *ptr, size_t old_size, size_t new_size) {
    (void)user_env;
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

void repl_print(void *user_env, char *c, size_t length) {
    (void)user_env;
    printf("%.*s", (int)length, c);
}

void example(void *user_closure, Pocket lisp) {
    (void)user_closure;
    pk_push_int(lisp, 67);
    pk_stack_dump(lisp);
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
    pk_fastcall(NULL, lisp, example, 1);
    pk_stack_dump(lisp);
    pk_deinit(lisp);
    stb_leakcheck_dumpmem();
    return EXIT_SUCCESS;
}
