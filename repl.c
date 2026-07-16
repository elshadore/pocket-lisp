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

PKRes repl_cmd_args(Pocket lisp, int argc, char **argv) {
    if (argc == 0) {
        pk_try(pk_push_nil(lisp));
    } else {
        for (int i = 0; i < argc; ++i) {
            pk_try(pk_push_cstr(lisp, argv[i]));
        }
        pk_try(pk_list(lisp, -argc, -1));
    }
    pk_try(pk_push_symbol(lisp, pkstr("argv")));
    pk_try(pk_set(lisp, -1, -2));
    pk_try(pk_popn(lisp, argc + 2));
    return PK_Ok;
}

PKRes repl(Pocket lisp, int argc, char **argv) {
    pk_try(repl_cmd_args(lisp, argc, argv));
    pk_try(pk_push_cfunc(lisp, NULL, repl_read_user_input, 0, PKArity_Normal));
    pk_try(pk_push_symbol(lisp, pkstr("read-user-input")));
    pk_try(pk_fset(lisp, -1, -2));
    pk_try(pk_popn(lisp, 2));
    pk_try(pk_read_string(lisp, pkstr(REPL_SRC)));
    pk_try(pk_evlist(lisp, -1));
    return PK_Ok;
}

int main(int argc, char **argv) {
    Pocket lisp = pk_init(NULL, repl_alloc, repl_print);
    if (lisp == NULL) {
        return EXIT_FAILURE;
    }
    if (repl(lisp, argc, argv) == PK_Yield) {
        fprintf(stderr, "REPL exited with error\n");
    }
    pk_deinit(lisp);
    stb_leakcheck_dumpmem();
    return EXIT_SUCCESS;
}
