#define STB_LEAKCHECK_IMPLEMENTATION
#include "./lib/stb_leakcheck.h"

#include "./src/pocket.h"
#include "repl.h"

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
    printf("%.*s\n", (int)length, c);
}

void repl_read_user_input(void *user_closure, Pocket lisp) {
    (void)user_closure;
    char buf[4096];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        pk_push_string(lisp, pkstr(""));
        return;
    }
    
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') {
        buf[len-1] = '\0';
        len--;
    }
    
    pk_push_nstr(lisp, buf, len);
}

void repl(Pocket lisp) {
    pk_push_cfunc(lisp, NULL, repl_read_user_input, 1, PKArity_Normal);
    pk_push_symbol(lisp, pkstr("read-user-input"));
    pk_fset(lisp, -1, -2);
    pk_popn(lisp, 2);
    
    pk_read_string(lisp, pkstr(REPL_SRC));
    pk_evlist(lisp, -1);
}

int main(void) {
    // printf("henlo word!\n");
    Pocket lisp = pk_init(NULL, repl_alloc, repl_print);
    if (lisp == NULL) {
        return EXIT_FAILURE;
    }
    repl(lisp);
    pk_deinit(lisp);
    stb_leakcheck_dumpmem();
    return EXIT_SUCCESS;
}
