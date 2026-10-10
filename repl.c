#include "./src/pocket.h"
#include "./src/pocket_all.c"

PKAtom run(void) {
    if (!pk_init()) {
        return pk_error();
    }

    PKAtom a = pk_string(pk_str("(print \"henlo word!\")"));
    pk_propagate(a);
    
    PKAtom b = pk_read(a, PK_READ_EXPRESSION);
    pk_propagate(b);
    
    pk_print(b);
    
    PKAtom e = pk_compile(b);
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
