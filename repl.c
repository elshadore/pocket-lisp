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
    
    PKAtom e = pk_compile(b);
    
    pk_print(e);
    
    PKAtom c = pk_cfun2(NULL, PK_CALL_FUN, function, PK_ARITY_NORMAL);

    PKAtom d = pk_funcall2(PK_CALL_FUN, c, pk_int(34), pk_int(35));
    pk_propagate(pk_debug_dump("function", PK_DUMP_STACK));

    pk_print(d);

    pk_propagate(pk_debug_dump("env", PK_DUMP_ENV));
    return pk_true();
}

int main(void) {
    if (pk_is_error(run())) {
        return 1;
    }
    return 0;
}
