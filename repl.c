#include "./src/user.h"
#include "./src/user_all.c"

int main(void) {
    if (!ul_init()) return 1;
    
    ULAtom a = ul_int(35);
    if (ul_is_error(a)) return 1;
    
    ULAtom b = ul_int(34);
    if (ul_is_error(b)) return 1;
    
    ULAtom c = ul_add(a, b);
    if (ul_is_error(c)) return 1;
    
    ul_print(c);
    return 0;
}
