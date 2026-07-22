#include "./src/pocket.h"
#include <string.h>
#include <stdio.h>

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

static char repl_buffer[4096];

PKRes repl_read_user_input(void *user_closure, Pocket lisp) {
    size_t len = 0;
    
    (void)user_closure;
    
    if (fgets(repl_buffer, sizeof(repl_buffer), stdin) == NULL) {
        pk_try(pk_push_string(lisp, ""));
        return PK_Ok;
    }

    len = strlen(repl_buffer);
    if (len > 0 && repl_buffer[len-1] == '\n') {
        repl_buffer[len-1] = '\0';
        len--;
    }

    pk_try(pk_push_stringn(lisp, repl_buffer, len));
    return PK_Ok;
}

PKRes repl_cmd_args(Pocket lisp, int argc, char **argv) {
    int i = 0;
    if (argc == 0) {
        pk_try(pk_push_nil(lisp));
    } else {
        for (i = 0; i < argc; ++i) {
            pk_try(pk_push_string(lisp, argv[i]));
        }
        pk_try(pk_list(lisp, -argc, -1));
    }
    pk_try(pk_push_symbol(lisp, "argv"));
    pk_try(pk_set(lisp, -1, -2));
    pk_try(pk_popn(lisp, argc + 2));
    return PK_Ok;
}


PKRes repl(Pocket lisp, int argc, char **argv) {
    pk_try(repl_cmd_args(lisp, argc, argv));
    pk_try(pk_push_cfunc(lisp, NULL, repl_read_user_input, 0, PKArity_Normal));
    pk_try(pk_push_symbol(lisp, "read-user-input"));
    pk_try(pk_fset(lisp, -1, -2));
    pk_try(pk_popn(lisp, 2));
    pk_try(pk_push_string(lisp, "./repl.pk"));
    pk_try(pk_slurp(lisp, -1));
    pk_try(pk_read(lisp, -1, PK_READ_LISTED));
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
    return EXIT_SUCCESS;
}
