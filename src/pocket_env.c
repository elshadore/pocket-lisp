#include "pocket_internals.h"

PKSymTable *pk_environment(Pocket lisp, PKEnvTy ty) {
    switch (ty) {
        case PKEnvTy_Var: return &lisp->vars;
        case PKEnvTy_Fun: return &lisp->funs;
    }
}

PKAtom *pk_env_set(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value) {
    return pk_symtable_put(lisp, pk_environment(lisp, ty), sym, value);
}

PKAtom *pk_env_get(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym) {
    PKAtom *value = pk_symtable_get(lisp, pk_environment(lisp, ty), sym);
    if (value == NULL) pk_error(lisp);
    return value;
}

PKAtom *pk_env_unbind(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym) {
    return pk_symtable_rem(lisp, pk_environment(lisp, ty), sym);
}

void pk_set(Pocket lisp, int symbol, int value_sp) {
    PKAtomSymbol *sym = pk_atom_cast_symbol(lisp, pk_stack_get(lisp, symbol));
    PKAtom *value = pk_stack_get(lisp, value_sp);
    pk_env_set(lisp, PKEnvTy_Var, sym, value);
}

void pk_get(Pocket lisp, int symbol) {
    PKAtomSymbol *sym = pk_atom_cast_symbol(lisp, pk_stack_get(lisp, symbol));
    pk_push(lisp, pk_env_get(lisp, PKEnvTy_Var, sym));
}

void pk_unbind(Pocket lisp, int symbol) {
    PKAtomSymbol *sym = pk_atom_cast_symbol(lisp, pk_stack_get(lisp, symbol));
    pk_env_unbind(lisp, PKEnvTy_Var, sym);
}

void pk_fset(Pocket lisp, int symbol, int value) {
    PKAtomSymbol *sym = pk_atom_cast_symbol(lisp, pk_stack_get(lisp, symbol));
    pk_env_set(lisp, PKEnvTy_Fun, sym, pk_stack_get(lisp, value));
}

void pk_fget(Pocket lisp, int symbol) {
    PKAtomSymbol *sym = pk_atom_cast_symbol(lisp, pk_stack_get(lisp, symbol));
    pk_push(lisp, pk_env_get(lisp, PKEnvTy_Fun, sym));
}

void pk_funbind(Pocket lisp, int symbol) {
    PKAtomSymbol *sym = pk_atom_cast_symbol(lisp, pk_stack_get(lisp, symbol));
    pk_env_unbind(lisp, PKEnvTy_Fun, sym);
}

void pk_let(Pocket lisp, int symbol, int value) {
    PKAtomSymbol *sym = pk_atom_cast_symbol(lisp, pk_stack_get(lisp, symbol));
    pk_let_push(lisp, PKEnvTy_Var, sym, pk_stack_get(lisp, value));
}

void pk_flet(Pocket lisp, int symbol, int value) {
    PKAtomSymbol *sym = pk_atom_cast_symbol(lisp, pk_stack_get(lisp, symbol));
    pk_let_push(lisp, PKEnvTy_Fun, sym, pk_stack_get(lisp, value));
}
