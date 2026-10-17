#include "./src/pocket.h"
#include "./src/pocket_all.c"

#include "repl.h"

PKAtom cargs(int argc, char **argv) {
    PKAccum acc = pk_accum_new();
    for (int i = 0; i < argc; ++i) {
        PKAtom a = pk_stringc(argv[i]);
        pk_propagate(a);
        pk_propagate(pk_accum_push(&acc, a));
        pk_ref_dec(a);
    }
    return acc.head;
}
    
PKAtom run(int argc, char **argv) {
    if (!pk_init()) {
        return pk_errors(pk_str("init failed"));
    }

    PKAtom args = cargs(argc, argv);
    pk_propagate(args);

    PKAtom symbol = pk_symbol(pk_str("args"));
    pk_propagate(symbol);

    pk_propagate(pk_set(PK_ENV_VAR, symbol, args));
    pk_ref_dec(symbol);
    pk_ref_dec(args);
    
    PKAtom a = pk_string(pk_str(REPL_CODE));
    pk_propagate(a);
    
    PKAtom b = pk_read(a, PK_MODE_EXPRLIST);
    pk_propagate(b);
    
    PKAtom c = pk_eval(b, PK_MODE_EXPRLIST);
    pk_ref_dec(a);
    pk_ref_dec(b);
    pk_propagate(c);

    return c;
}

int main(int argc, char **argv) {
    PKAtom result = run(argc, argv);
    (void)result;
    pk_deinit();
    return 0;
}
