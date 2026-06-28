#define STB_LEAKCHECK_IMPLEMENTATION
#include "./lib/stb_leakcheck.h"

#include "./src/pocket.h"
#include "repl.h"
#include "example.h"

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

PKRes repl_read_user_input(void *user_closure, Pocket lisp) {
    (void)user_closure;
    char buf[4096];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        pk_try(pk_push_string(lisp, pkstr("")));
        return PK_Ok;
    }

    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') {
        buf[len-1] = '\0';
        len--;
    }

    pk_try(pk_push_nstr(lisp, buf, len));
    return PK_Ok;
}

PKRes repl(Pocket lisp) {
    pk_try(pk_push_cfunc(lisp, NULL, repl_read_user_input, 0, PKArity_Normal));
    pk_try(pk_push_symbol(lisp, pkstr("read-user-input")));
    pk_try(pk_fset(lisp, -1, -2));
    pk_try(pk_popn(lisp, 2));
    pk_try(pk_read_string(lisp, pkstr(REPL_SRC)));
    pk_try(pk_evlist(lisp, -1));
    return PK_Ok;
}

PKRes testicle(Pocket lisp) {
    pk_try(pk_read_string(lisp, pkstr(EXAMPLE_SRC)));
    pk_try(pk_evlist(lisp, -1));
    return PK_Ok;
}

int main(void) {
    Pocket lisp = pk_init(NULL, repl_alloc, repl_print);
    if (lisp == NULL) {
        return EXIT_FAILURE;
    }
    if (testicle(lisp) == PK_Yield) {
        fprintf(stderr, "REPL exited with error\n");
    }
    pk_deinit(lisp);
    stb_leakcheck_dumpmem();
    return EXIT_SUCCESS;
}
