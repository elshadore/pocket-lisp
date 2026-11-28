#include "pocket_internals.h"

void pk_cmp_let(Pocket lisp, PKCompiler *c, PKAtom *symbol) {
    pk_cmp_load(lisp, c, symbol);
    pk_cmp_push_byte(lisp, c, PK_OP_LET);
    pk_cmp_push_byte(lisp, c, 1);
}

void pk_cmp_let_stack(Pocket lisp, PKCompiler *c, PKAtom *symbol, size_t stack_offset) {
    pk_cmp_push_byte(lisp, c, PK_OP_DIG);
    pk_cmp_push_any(lisp, c, stack_offset);
    pk_cmp_let(lisp, c, symbol);
}

void pk_compile_destructure_list_destructure(Pocket lisp, PKCompiler *c, PKAtom *list) {
    PKAtom *iter = list;
    while (pk_atom_is_cons(iter)) {
        PKAtom *atom = iter->cons.car;
        pk_cmp_push_byte(lisp, c, PK_OP_LIST_POP);
        switch (atom->header.type) {
            case PK_ATOMTY_SYMBOL: {
                break;
            }
            case PK_ATOMTY_CONS: {
                pk_compile_destructure_list_destructure(lisp, c, atom);
                break;
            }
            default: {
                pk_error_type2(lisp, atom, PK_TYPE_SYMBOL, PK_TYPE_CONS);
                break;
            }
        }
        iter = iter->cons.cdr;
    }
    pk_cmp_push_byte(lisp, c, PK_OP_POP);
}

size_t pk_compile_destructure_list_symbol_loading(Pocket lisp, PKCompiler *c, PKAtom *list) {
    size_t acc = 0;
    PKAtom *iter = list;
    while (pk_atom_is_cons(iter)) {
        PKAtom *atom = iter->cons.car;
        switch (atom->header.type) {
            case PK_ATOMTY_SYMBOL: {
                pk_cmp_load(lisp, c, atom);
                acc += 1;
                break;
            }
            case PK_ATOMTY_CONS: {
                pk_compile_destructure_list_symbol_loading(lisp, c, atom);
                break;
            }
            default: break;
        }
        iter = iter->cons.cdr;
    }
    return acc;
}

size_t pk_compile_destructure_list(Pocket lisp, PKCompiler *c, PKAtom *list) {
    pk_compile_destructure_list_destructure(lisp, c, list);
    return pk_compile_destructure_list_symbol_loading(lisp, c, list);
}

PKDSB pk_compile_destructure(Pocket lisp, PKCompiler *c, PKAtom *args) {
    pk_u8 arity = PK_INT_ARITY_NORMAL;
    
    size_t nargs = 0;
    size_t option = 0;
    size_t index = 0;
    
    PKAtom *iter = args;
    while (pk_atom_is_cons(iter)) {
        PKAtom *atom = iter->cons.car;
        switch (atom->header.type) {
            case PK_ATOMTY_SYMBOL: {
                pk_cmp_let_stack(lisp, c, atom, index);
                nargs += 1;
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
                        pk_cmp_let_stack(lisp, c, (PKAtom *)symbol, index);
                        nargs += 1;
                        option += 1;
                        iter = iter->cons.cdr;
                    }
                    goto DONE;
                } else if (pk_atom_keyword_cmp(keyword, pk_str("rest"))) {
                    arity = PK_INT_ARITY_VARIADIC;
                    PKAtomCons *rest = pk_atom_assert_cons(lisp, iter->cons.cdr);
                    PKAtomSymbol *symbol = pk_atom_assert_symbol(lisp, rest->car);
                    pk_cmp_let_stack(lisp, c, (PKAtom *)symbol, index);
                    iter = rest->cdr;
                    goto DONE;
                } else {
                    PKStr key = keyword->name->str;
                    pk_errorf(lisp, "unknown lambda keyword directive: %.*s", (int)key.length, key.c);
                }
                break;
            }
            case PK_ATOMTY_CONS: {
                nargs += 1;
                break;
            }
            default: {
                pk_error_type3(lisp, atom, PK_TYPE_SYMBOL, PK_TYPE_KEYWORD, PK_TYPE_CONS);
                break;
            }
        }
        index += 1;
        iter = iter->cons.cdr;
    }
    DONE:
    
    return (PKDSB) {
        .args = pk_byte_cast(lisp, nargs),
        .arity = arity,
        .option = pk_byte_cast(lisp, option),
    };
}
