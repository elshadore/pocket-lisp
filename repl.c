#include "./src/pocket.h"
#include "./src/pocket_all.c"

#define REPL_CODE \
"(setq repl/running true)" \
"(defun repl/quit ()" \
"     (setq repl/running nil))" \
"(while repl/running " \
"     (puts \"LISP>> \") " \
"     (print (eval (read (fs/read-user-input)))))" 

PKAtom cargs(int argc, char **argv) {
    PKAtom acc = pk_nil();
    for (int i = 0; i < argc; ++i) {
        PKAtom a = pk_stringc(argv[i]);
        pk_propagate(a);
        PKAtom b = pk_cons(a, acc);
        pk_propagate(b);
        acc = b;
    }
    return pk_reverse(acc);
}
    
PKAtom run(int argc, char **argv) {
    if (!pk_init()) {
        return pk_error();
    }

    PKAtom args = cargs(argc, argv);
    pk_propagate(args);

    PKAtom symbol = pk_symbol(pk_str("argv"));
    pk_propagate(symbol);

    pk_propagate(pk_set(PK_ENV_VAR, symbol, args));
    
    PKAtom a = pk_string(pk_str(REPL_CODE));
    pk_propagate(a);
    
    PKAtom b = pk_read(a, PK_MODE_EXPRLIST);
    pk_propagate(b);
    
    PKAtom c = pk_eval(b, PK_MODE_EXPRLIST);
    pk_propagate(c);

    pk_print(c);
    
    return pk_true();
}

int main(int argc, char **argv) {
    if (pk_is_error(run(argc, argv))) {
        return 1;
    }
    return 0;
}
