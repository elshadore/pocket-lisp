#include "./src/pocket.h"
#include "./src/pocket_all.c"

#define REPL_CODE \
"(setq repl/running true)" \
"(defun repl/quit ()" \
"     (setq repl/running nil))" \
"(while repl/running " \
"     (puts \"LISP>> \") " \
"     (print (eval (read (fs/read-user-input)))))" 

PKAtom run(void) {
    if (!pk_init()) {
        return pk_error();
    }

    PKAtom a = pk_string(pk_str(REPL_CODE));
    pk_propagate(a);
    
    PKAtom b = pk_read(a, PK_MODE_EXPRLIST);
    pk_propagate(b);
    
    PKAtom c = pk_eval(b, PK_MODE_EXPRLIST);
    pk_propagate(c);

    pk_print(c);
    
    return pk_true();
}

int main(void) {
    if (pk_is_error(run())) {
        return 1;
    }
    return 0;
}
