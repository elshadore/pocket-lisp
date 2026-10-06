#include "./src/pocket.h"
#include "./src/pocket_all.c"

PKAtom function(void *user_closure, PKAtom a, PKAtom b) {
    (void)user_closure;
    PKAtom c = pk_add(a, b);
    PKAtom d = pk_add(c, pk_int(1000));
    return d;
}

PKAtom run(void) {
    if (!pk_init()) {
        return pk_error();
    }

    PKAtom a = pk_string(pk_str("(+ 1 2 3)"));
    PKAtom b = pk_read(a, PK_READ_EXPRESSION);
    
    pk_print(b);
    
    PKAtom c = pk_cfun2(NULL, function, PK_ARITY_NORMAL);

    PKAtom d = pk_funcall2(c, pk_int(34), pk_int(35));

    pk_print(d);
    
    return pk_true();
}

int main(void) {
    if (pk_is_error(run())) {
        return 1;
    }
    return 0;
}
