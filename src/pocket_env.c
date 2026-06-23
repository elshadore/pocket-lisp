#include "pocket_internals.h"

PKSymTable *pk_environment(Pocket lisp, PKEnvTy ty) {
    switch (ty) {
        case PKEnvTy_Var: return &lisp->vars;
        case PKEnvTy_Fun: return &lisp->funs;
    }
    return NULL;
}

PKRes pk_env_set(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value, PKAtom **output) {
    pk_try(pk_symtable_put(lisp, pk_environment(lisp, ty), sym, value, output));
    return PK_Ok;
}

PKRes pk_env_get(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom **output) {
    pk_try(pk_symtable_get(lisp, pk_environment(lisp, ty), sym, output));
    if (*output == NULL) return pk_error(lisp);
    return PK_Ok;
}

PKRes pk_env_unbind(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom **output) {
    pk_try(pk_symtable_rem(lisp, pk_environment(lisp, ty), sym, output));
    return PK_Ok;
}

PKRes pk_set(Pocket lisp, int symbol, int value_sp) {
    PKAtom *sym_atom;
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    PKAtomSymbol *sym;
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    PKAtom *value;
    pk_try(pk_stack_get(lisp, value_sp, &value));
    PKAtom *_ignored;
    pk_try(pk_env_set(lisp, PKEnvTy_Var, sym, value, &_ignored));
    return PK_Ok;
}

PKRes pk_get(Pocket lisp, int symbol) {
    PKAtom *sym_atom;
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    PKAtomSymbol *sym;
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    PKAtom *value;
    pk_try(pk_env_get(lisp, PKEnvTy_Var, sym, &value));
    pk_try(pk_push(lisp, value));
    return PK_Ok;
}

PKRes pk_unbind(Pocket lisp, int symbol) {
    PKAtom *sym_atom;
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    PKAtomSymbol *sym;
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    PKAtom *_ignored;
    pk_try(pk_env_unbind(lisp, PKEnvTy_Var, sym, &_ignored));
    return PK_Ok;
}

PKRes pk_fset(Pocket lisp, int symbol, int value) {
    PKAtom *sym_atom;
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    PKAtomSymbol *sym;
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    PKAtom *val;
    pk_try(pk_stack_get(lisp, value, &val));
    PKAtom *_ignored;
    pk_try(pk_env_set(lisp, PKEnvTy_Fun, sym, val, &_ignored));
    return PK_Ok;
}

PKRes pk_fget(Pocket lisp, int symbol) {
    PKAtom *sym_atom;
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    PKAtomSymbol *sym;
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    PKAtom *value;
    pk_try(pk_env_get(lisp, PKEnvTy_Fun, sym, &value));
    pk_try(pk_push(lisp, value));
    return PK_Ok;
}

PKRes pk_funbind(Pocket lisp, int symbol) {
    PKAtom *sym_atom;
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    PKAtomSymbol *sym;
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    PKAtom *_ignored;
    pk_try(pk_env_unbind(lisp, PKEnvTy_Fun, sym, &_ignored));
    return PK_Ok;
}

PKRes pk_let(Pocket lisp, int symbol, int value) {
    PKAtom *sym_atom;
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    PKAtomSymbol *sym;
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    PKAtom *val;
    pk_try(pk_stack_get(lisp, value, &val));
    pk_try(pk_let_push(lisp, PKEnvTy_Var, sym, val));
    return PK_Ok;
}

PKRes pk_flet(Pocket lisp, int symbol, int value) {
    PKAtom *sym_atom;
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    PKAtomSymbol *sym;
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    PKAtom *val;
    pk_try(pk_stack_get(lisp, value, &val));
    pk_try(pk_let_push(lisp, PKEnvTy_Fun, sym, val));
    return PK_Ok;
}
