#include "./src/pocket.h"
#include "./src/pocket_all.c"

#define REPL_CODE \
"(set 'repl/running true)" \
"(fset 'repl/quit (lambda ()" \
      "(set 'repl/running nil)))" \
"(while repl/running" \
     "(puts \"LISP>> \")" \
     "(print (eval (read (fs/read-user-input)))))"

PKAtom run(void) {
    if (!pk_init()) {
        return pk_error();
    }

    PKAtom a = pk_string(pk_str(REPL_CODE));
    pk_propagate(a);
    
    PKAtom b = pk_read(a, PK_MODE_EXPRLIST);
    pk_propagate(b);
    
    PKAtom e = pk_compile(PK_CALL_FUN, b, PK_MODE_EXPRLIST);
    pk_propagate(e);

    PKAtom f = pk_funcall0(PK_CALL_FUN, e);
    pk_propagate(f);

    pk_print(f);
    
    return pk_true();
}

int main(void) {
    if (pk_is_error(run())) {
        return 1;
    }
    return 0;
}
