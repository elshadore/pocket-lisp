#include "pocket_internals.h"

PKAtom *pk_env_get(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol) {
    switch (env) {
        case PK_ENV_VAR: {
            return symbol->var;
        }
        case PK_ENV_FUN: {
            return symbol->fun;
        }
        case PK_ENV_PROP: {
            return symbol->prop;
        }
        default: {
            return lisp->cache.undefined;
        }
    }
}

PKAtom *pk_env_set(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol, PKAtom *value) {
    PKAtom *temp = lisp->cache.undefined;

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
        case PK_ENV_PROP: {
            temp = symbol->prop;
            symbol->prop = value;
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
            
            PKAtom *value = pk_env_get(lisp, env, (PKAtomSymbol *)symbol);
            if (pk_atom_is_undefined(value)) {
                continue;
            }
            
            PKAtom *pair = (PKAtom *)pk_atom_cons(lisp, symbol, value);
            pk_accum_push(&acc, pair);
        }
    }
    
    return acc.head;
}

