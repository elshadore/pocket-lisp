#include "./src/pocket.h"
#include "./src/pocket_all.c"

PKAtom run(void) {
    if (!pk_init()) {
        return pk_error();
    }

    PKAtom a = pk_string(pk_str("(+ 1 2 3)"));
    PKAtom b = pk_read(a, PK_READ_EXPRESSION);
    
    pk_print(b);
    
    return pk_true();
}

int main(void) {
    if (pk_is_error(run())) {
        return 1;
    }
    return 0;
}
