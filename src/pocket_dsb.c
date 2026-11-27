#include "pocket_internals.h"

/* PKDSB pk_compile_destructure_list(Pocket lisp, PKAtom *list) { */
    
/* } */

PKDSB pk_compile_destructure(Pocket lisp, PKCompiler *c, PKAtom *args) {
    pk_u8 arity = PK_INT_ARITY_NORMAL;
    
    size_t nargs = 0;
    size_t lets = 0;
    size_t option = 0;
    
    PKAtom *iter = args;
    while (pk_atom_is_cons(iter)) {
        PKAtom *atom = iter->cons.car;
        switch (atom->header.type) {
            case PK_ATOMTY_SYMBOL: {
                pk_cmp_load(lisp, c, atom);
                nargs += 1;
                lets += 1;
                break;
            }
            case PK_ATOMTY_KEYWORD: {
                if (arity != PK_INT_ARITY_NORMAL) {
                    pk_errors(lisp, pk_str("can only have one keyword mode"));
                }
                PKAtomKeyword *keyword = (PKAtomKeyword *)atom;
                if (pk_atom_keyword_cmp(keyword, pk_str("opt")) || pk_atom_keyword_cmp(keyword, pk_str("optional"))) {
                    arity = PK_INT_ARITY_OPTIONAL;
                    iter = iter->cons.cdr;
                    while (pk_atom_is_cons(iter)) {
                        PKAtomSymbol *symbol = pk_atom_assert_symbol(lisp, iter->cons.car);
                        pk_cmp_load(lisp, c, (PKAtom *)symbol);
                        nargs += 1;
                        lets += 1;
                        option += 1;
                        iter = iter->cons.cdr;
                    }
                    goto DONE;
                } else if (pk_atom_keyword_cmp(keyword, pk_str("rest"))) {
                    arity = PK_INT_ARITY_VARIADIC;
                    PKAtomCons *rest = pk_atom_assert_cons(lisp, iter->cons.cdr);
                    PKAtomSymbol *symbol = pk_atom_assert_symbol(lisp, rest->car);
                    pk_cmp_load(lisp, c, (PKAtom *)symbol);
                    lets += 1;
                    iter = rest->cdr;
                    goto DONE;
                } else {
                    PKStr key = keyword->name->str;
                    pk_errorf(lisp, "unknown lambda keyword directive: %.*s", (int)key.length, key.c);
                }
                break;
            }
            case PK_ATOMTY_CONS: {
                break;
            }
            default: {
                pk_error_type3(lisp, atom, PK_TYPE_SYMBOL, PK_TYPE_KEYWORD, PK_TYPE_CONS);
                break;
            }
        }
        iter = iter->cons.cdr;
    }
    DONE:
    
    pk_cmp_push_byte(lisp, c, PK_OP_LET);
    pk_cmp_push_any(lisp, c, lets);

    return (PKDSB) {
        .args = pk_byte_cast(lisp, nargs),
        .lets = pk_byte_cast(lisp, lets),
        .arity = arity,
        .option = pk_byte_cast(lisp, option),
    };
}
