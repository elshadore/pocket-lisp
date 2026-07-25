#include "pocket_internals.h"

PKSymTable *pk_environment(Pocket lisp, PKEnvTy ty) {
    switch (ty) {
        case PKEnvTy_Var: return &lisp->vars;
        case PKEnvTy_Fun: return &lisp->funs;
        default: return NULL;
    }
}

PK_RES pk_env_set(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value, PKAtom **output) {
    pk_try(pk_symtable_put(lisp, pk_environment(lisp, ty), sym, value, output));
    return PK_OK;
}

PK_RES pk_env_get(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom **output) {
    pk_try(pk_symtable_get(lisp, pk_environment(lisp, ty), sym, output));
    if (*output == NULL) return pk_error(lisp);
    return PK_OK;
}

PK_RES pk_env_unbind(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom **output) {
    pk_try(pk_symtable_rem(lisp, pk_environment(lisp, ty), sym, output));
    return PK_OK;
}

PK_RES pk_set(Pocket lisp, int symbol, int value_sp) {
    PKAtom *sym_atom = NULL;
    PKAtomSymbol *sym = NULL;
    PKAtom *value = NULL;
    PKAtom *_ignored = NULL;
    
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    pk_try(pk_stack_get(lisp, value_sp, &value));
    pk_try(pk_env_set(lisp, PKEnvTy_Var, sym, value, &_ignored));
    return PK_OK;
}

PK_RES pk_get(Pocket lisp, int symbol) {
    PKAtom *sym_atom = NULL;
    PKAtomSymbol *sym = NULL;
    PKAtom *value = NULL;
    
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    pk_try(pk_env_get(lisp, PKEnvTy_Var, sym, &value));
    pk_try(pk_push(lisp, value));
    return PK_OK;
}

PK_RES pk_unbind(Pocket lisp, int symbol) {
    PKAtom *sym_atom = NULL;
    PKAtomSymbol *sym = NULL;
    PKAtom *_ignored = NULL;
    
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    pk_try(pk_env_unbind(lisp, PKEnvTy_Var, sym, &_ignored));
    return PK_OK;
}

PK_RES pk_fset(Pocket lisp, int symbol, int value) {
    PKAtom *sym_atom = NULL;
    PKAtomSymbol *sym = NULL;
    PKAtom *val = NULL;
    PKAtom *_ignored = NULL;
    
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    pk_try(pk_stack_get(lisp, value, &val));
    pk_try(pk_env_set(lisp, PKEnvTy_Fun, sym, val, &_ignored));
    return PK_OK;
}

PK_RES pk_fget(Pocket lisp, int symbol) {
    PKAtom *sym_atom = NULL;
    PKAtomSymbol *sym = NULL;
    PKAtom *value = NULL;
    
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    pk_try(pk_env_get(lisp, PKEnvTy_Fun, sym, &value));
    pk_try(pk_push(lisp, value));
    return PK_OK;
}

PK_RES pk_funbind(Pocket lisp, int symbol) {
    PKAtom *sym_atom = NULL;
    PKAtomSymbol *sym = NULL;
    PKAtom *_ignored = NULL;
    
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    pk_try(pk_env_unbind(lisp, PKEnvTy_Fun, sym, &_ignored));
    return PK_OK;
}

PK_RES pk_let(Pocket lisp, int symbol, int value) {
    PKAtom *sym_atom = NULL;
    PKAtomSymbol *sym = NULL;
    PKAtom *val = NULL;
    
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    pk_try(pk_stack_get(lisp, value, &val));
    pk_try(pk_let_push(lisp, PKEnvTy_Var, sym, val));
    return PK_OK;
}

PK_RES pk_flet(Pocket lisp, int symbol, int value) {
    PKAtom *sym_atom = NULL;
    PKAtomSymbol *sym = NULL;
    PKAtom *val = NULL;
    
    pk_try(pk_stack_get(lisp, symbol, &sym_atom));
    pk_try(pk_atom_cast_symbol(lisp, sym_atom, &sym));
    pk_try(pk_stack_get(lisp, value, &val));
    pk_try(pk_let_push(lisp, PKEnvTy_Fun, sym, val));
    return PK_OK;
}
