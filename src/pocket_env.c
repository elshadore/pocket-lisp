#include "pocket_internals.h"

PKAtom pk_set(PK_ENV env, PKAtom symbol, PKAtom value) {
    if (!pk_is_symbol(symbol)) {
        return pk_error();
    }
    
    PKAtom temp = pk_nil();
    
    switch (env) {
        case PK_ENV_VAR: {
            temp = symbol->symbol.var;
            symbol->symbol.var = value;
            break;
        }
        case PK_ENV_FUN: {
            temp = symbol->symbol.fun;
            symbol->symbol.fun = value;
            break;
        }
        case PK_ENV_PROP: {
            temp = symbol->symbol.prop;
            symbol->symbol.prop = value;
            break;
        }
        default: break;
    }
    
    if (pk_is_undefined(temp)) {
        return pk_nil();
    } else {
        return temp;
    }
}

PKAtom pk_get(PK_ENV env, PKAtom symbol) {
    if (!pk_is_symbol(symbol)) {
        return pk_error();
    }
    
    PKAtom result = pk_nil();
    
    switch (env) {
        case PK_ENV_VAR: {
            result = symbol->symbol.var;
            break;
        }
        case PK_ENV_FUN: {
            result = symbol->symbol.fun;
            break;
        }
        case PK_ENV_PROP: {
            result = symbol->symbol.prop;
            break;
        }
        default: break;
    }
    
    if (pk_is_undefined(result)) {
        return pk_nil();
    } else {
        return result;
    }
}

bool pk_is_bound(PK_ENV env, PKAtom symbol) {
    if (!pk_is_symbol(symbol)) {
        return false;
    }

    PKAtom fetch = pk_nil();
    switch (env) {
        case PK_ENV_VAR: {
            fetch = symbol->symbol.var;
            break;
        }
        case PK_ENV_FUN: {
            fetch = symbol->symbol.fun;
            break;
        }
        case PK_ENV_PROP: {
            fetch = symbol->symbol.prop;
            break;
        }
        default: break;
    }
    
    if (pk_is_undefined(fetch)) {
        return false;
    } else {
        return true;
    }
}

PKAtom pk_unbind(PK_ENV env, PKAtom symbol) {
    if (!pk_is_symbol(symbol)) {
        return pk_error();
    }

    PKAtom result = pk_nil();

    switch (env) {
        case PK_ENV_VAR: {
            result = symbol->symbol.var;
            symbol->symbol.var = pk_undefined();
            break;
        }
        case PK_ENV_FUN: {
            result = symbol->symbol.fun;
            symbol->symbol.fun = pk_undefined();
            break;
        }
        case PK_ENV_PROP: {
            result = symbol->symbol.prop;
            symbol->symbol.prop = pk_undefined();
            break;
        }
        default: break;
    }

    if (pk_is_undefined(result)) {
        return pk_nil();
    } else {
        return result;
    }
}

PKAtom pk_let(PK_ENV env, PKAtom symbol, PKAtom value) {
    if (PK_CORE.lets.count >= PK_CORE.lets.capacity) {
        size_t new_capacity = pk_grow_capacity(PK_CORE.lets.capacity, 32);
        PKLet *new_e = pk_realloc(PK_CORE.lets.e, new_capacity * sizeof(PKLet));
        if (new_e == NULL) {
            return pk_error();
        }
        PK_CORE.lets.e = new_e;
        PK_CORE.lets.capacity = new_capacity;
    }

    bool was_bound = pk_is_bound(env, symbol);

    PKAtom restore = pk_set(env, symbol, value);
    pk_propagate(restore);

    if (!was_bound) {
        restore = pk_undefined();
    }

    PK_CORE.lets.e[PK_CORE.lets.count++] = (PKLet) {
        .env = env,
        .symbol = symbol,
        .restore = restore,
    };

    return restore;
}

void pk_let_unwind(size_t n) {
    for (size_t i = 0; i < n; ++i) {
        size_t index = PK_CORE.lets.count - i - 1;
        PKLet unwind = PK_CORE.lets.e[index];
        // The only error that can occur is a type error on the symbol input.
        // So it's okay to skip errors here.
        (void)pk_set(unwind.env, unwind.symbol, unwind.restore);
    }
    PK_CORE.lets.count -= n;
}
