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
        // Each argument is consed onto the front of the list, so walk the argv
        // backwards to keep them in order.
        pk_push_nil(lisp);
        for (int i = repl->argc - 1; i >= 0; --i) {
            pk_push_stringc(lisp, repl->argv[i]);
            pk_stack_set(lisp, -2, (PKAtom *)pk_atom_cons(lisp, pk_stack_get(lisp, -1), pk_stack_get(lisp, -2)));
            pk_pop(lisp);
        }
    }
    
    pk_push_symbol(lisp, pk_str("args"));
    pk_set(lisp, PK_ENV_VAR, -1, -2);
    pk_pop3(lisp);

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
    pk_pcall(lisp, PK_CALL_FUN, 0);
    pk_deinit(lisp);
    
    return 0;
}
