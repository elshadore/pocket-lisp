#include "./src/pocket.h"
#include "./src/pocket_all.c"

PKAtom run(void) {
    if (!pk_init()) {
        return pk_error();
    }
    
    PKAtom a = pk_int(35);
    pk_propagate(a);
    
    PKAtom b = pk_int(34);
    pk_propagate(b);
    
    PKAtom c = pk_add(a, b);
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
