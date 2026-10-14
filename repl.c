#include "./src/pocket.h"
#include "./src/pocket_all.c"

#define REPL_CODE \
"(defvar repl/running true)\n" \
"(defun repl/quit ()\n" \
"     (setq repl/running nil))\n" \
"(defun read-eval-print (string)\n" \
"     (print (eval (read string))))\n" \
"(when (= (cadr args) \"-e\")\n" \
"    (setq repl/running nil)\n" \
"    (read-eval-print (caddr args)))\n" \
"(while repl/running\n" \
"     (puts \"LISP>> \")\n" \
"     (read-eval-print (fs/read-user-input)))\n"

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
        return pk_errors(pk_str("init failed"));
    }

    PKAtom args = cargs(argc, argv);
    pk_propagate(args);

    PKAtom symbol = pk_symbol(pk_str("args"));
    pk_propagate(symbol);

    pk_propagate(pk_set(PK_ENV_VAR, symbol, args));
    
    PKAtom a = pk_string(pk_str(REPL_CODE));
    pk_propagate(a);
    
    PKAtom b = pk_read(a, PK_MODE_EXPRLIST);
    pk_propagate(b);
    
    PKAtom c = pk_eval(b, PK_MODE_EXPRLIST);
    pk_propagate(c);

    return c;
}

int main(int argc, char **argv) {
    PKAtom result = run(argc, argv);
    if (pk_is_throwing(result)) {
        PKAtom message = pk_throw_contents(result);
        (void)pk_print(message);
        return 1;
    } else {
        (void)pk_print(result);
        return 0;
    }
}
