#include "./src/user_lisp.h"
#include "./src/user_lisp_all.c"

ULAtom run(void) {
    if (!ul_init()) {
        return ul_error();
    }
    
    ULAtom a = ul_int(35);
    ul_propagate(a);
    
    ULAtom b = ul_int(34);
    ul_propagate(b);
    
    ULAtom c = ul_add(a, b);
    ul_propagate(c);
    
    ul_print(c);
    
    return ul_true();
}

int main(void) {
    if (ul_is_error(run())) {
        return 1;
    }
    return 0;
}
