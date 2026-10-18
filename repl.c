#define PK_CFLAG_DEBUG
#include "./src/pocket.h"
#include "./src/pocket_all.c"

#include "repl.h"

typedef struct Repl_ {
    int argc;
    char **argv;
} Repl;

void run_repl(void *user_closure, Pocket lisp) {
    Repl *repl = (Repl *)user_closure;

    {
        for (int i = 0; i < repl->argc; ++i) {
            pk_push_stringc(lisp, repl->argv[i]);
        }
        if (repl->argc == 0) {
            pk_push_nil(lisp);
        } else {
            pk_list(lisp, -repl->argc, -1);
        }
    }

    pk_push_symbol(lisp, pk_str("args"));
    pk_set(lisp, PK_ENV_VAR, -1, -2);
    pk_debug_stackdump(lisp, "stack");
    pk_popn(lisp, repl->argc + 3);

    pk_push_string(lisp, pk_str(REPL_CODE));
    pk_read(lisp, -1, PK_MODE_EXPRLIST);
    pk_eval(lisp, -1, PK_MODE_EXPRLIST);
}

int main(int argc, char **argv) {
    Pocket lisp = pk_init(NULL);
    if (lisp == NULL) {
        fprintf(stderr, "lisp failed to init");
        return 1;
    }
    
    Repl repl = (Repl) {
        .argc = argc,
        .argv = argv,
    };
    
    pk_push_cfun(lisp, &repl, run_repl, 0, PK_ARITY_NORMAL, 0);
    pk_fcall(lisp, PK_CALL_FUN, 0);
    pk_deinit(lisp);
    
    return 0;
}
