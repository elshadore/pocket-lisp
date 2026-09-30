#include "./src/pocket.h"
#include "./src/pocket_all.c"

int main(void) {
    if (!pk_init()) return 1;
    
    PKAtom a = pk_int(35);
    if (pk_is_error(a)) return 1;
    
    PKAtom b = pk_int(34);
    if (pk_is_error(b)) return 1;
    
    PKAtom c = pk_add(a, b);
    if (pk_is_error(c)) return 1;
    
    pk_print(c);
    return 0;
}
