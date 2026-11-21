#include "pocket_internals.h"

PKAtom *pk_env_fetch(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol) {
    (void)lisp;

    switch (env) {
        case PK_ENV_VAR: {
            return symbol->var;
        }
        case PK_ENV_FUN: {
            return symbol->fun;
        }
        default: {
            return &PK_ATOM_UNDEFINED;
        }
    }
}

PKAtom *pk_env_fetch_loose(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol) {
    PKAtom *result = pk_env_fetch(lisp, env, symbol);
    if (pk_atom_is_undefined(result)) {
        return &PK_ATOM_NIL;
    } else {
        return result;
    }
}
    
PKAtom *pk_env_fetch_strict(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol) {
    PKAtom *result = pk_env_fetch(lisp, env, symbol);
    if (pk_atom_is_undefined(result)) {
        PKStr name = symbol->name->str;
        pk_errorf(lisp, "symbol: %.*s, is undefined\n", (int)name.length, name.c);
        return &PK_ATOM_NIL;
    }
    return result;
}
    
PKAtom *pk_env_set(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol, PKAtom *value) {
    (void)lisp;

    PKAtom *temp = &PK_ATOM_UNDEFINED;

    switch (env) {
        case PK_ENV_VAR: {
            temp = symbol->var;
            symbol->var = value;
            break;
        }
        case PK_ENV_FUN: {
            temp = symbol->fun;
            symbol->fun = value;
            break;
        }
        default: break;
    }

    return temp;
}

PKAtom *pk_env_let(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol, PKAtom *value) {
    if (lisp->lets.count >= lisp->lets.capacity) {
        size_t new_capacity = pk_grow_capacity(lisp->lets.capacity, 32);
        PKLet *new_e = pk_realloc(lisp, lisp->lets.e, new_capacity * sizeof(PKLet));
        lisp->lets.e = new_e;
        lisp->lets.capacity = new_capacity;
    }

    PKAtom *restore = pk_env_set(lisp, env, symbol, value);

    lisp->lets.e[lisp->lets.count++] = (PKLet) {
        .env = env,
        .symbol = symbol,
        .restore = restore,
    };

    return restore;
}

void pk_env_let_unwind(Pocket lisp, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        size_t index = lisp->lets.count - i - 1;
        PKLet unwind = lisp->lets.e[index];
        pk_env_set(lisp, unwind.env, unwind.symbol, unwind.restore);
    }
    lisp->lets.count -= n;
}

PKAtom *pk_env_alist(Pocket lisp, PK_ENV env) {
    PKAccum acc = pk_accum_new(lisp);
    
    for (PKPool *iter = lisp->pool; iter != NULL; iter = iter->next) {
        for (size_t i = 0; i < PK_POOL_MAX; ++i) {
            PKAtom *symbol = &iter->e[i];
            if (!pk_atom_is_symbol(symbol)) {
                continue;
            }
            
            PKAtom *value = pk_env_fetch(lisp, env, (PKAtomSymbol *)symbol);
            if (pk_atom_is_undefined(value)) {
                continue;
            }
            
            PKAtom *pair = (PKAtom *)pk_atom_cons(lisp, symbol, value);
            pk_accum_push(&acc, pair);
        }
    }
    
    return acc.head;
}

