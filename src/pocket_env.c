#include "pocket_internals.h"

PKAtom pk_get_raw(PK_ENV env, PKAtom symbol) {
    if (!pk_is_symbol(symbol)) {
        return pk_errors(pk_str("expected symbol"));
    }

    switch (env) {
        case PK_ENV_VAR: {
            return symbol->symbol.var;
        }
        case PK_ENV_FUN: {
            return symbol->symbol.fun;
        }
        case PK_ENV_PROP: {
            return symbol->symbol.prop;
        }
        default: break;
    }

    return pk_errors(pk_str("bad env property"));
}

PKAtom pk_set_raw(PK_ENV env, PKAtom symbol, PKAtom value) {
    if (!pk_is_symbol(symbol)) {
        return pk_errors(pk_str("expected symbol"));
    }

    pk_propagate(pk_ref_inc(value));
    
    PKAtom temp = pk_undefined();

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
        default: return pk_errors(pk_str("bad env property"));
    }

    return temp;
}

PKAtom pk_set(PK_ENV env, PKAtom symbol, PKAtom value) {
    PKAtom temp = pk_set_raw(env, symbol, value);
    pk_propagate(temp);

    if (pk_is_undefined(temp)) {
        return pk_nil();
    } else {
        return temp;
    }
}

PKAtom pk_set_discard(PK_ENV env, PKAtom symbol, PKAtom value) {
    PKAtom temp = pk_set_raw(env, symbol, value);
    pk_propagate(temp);
    pk_ref_dec(temp);
    return pk_true();
}

PKAtom pk_get(PK_ENV env, PKAtom symbol) {
    PKAtom result = pk_get_raw(env, symbol);
    pk_propagate(result);

    if (pk_is_undefined(result)) {
        return pk_nil();
    } else {
        return result;
    }
}

bool pk_is_bound(PK_ENV env, PKAtom symbol) {
    PKAtom fetch = pk_get_raw(env, symbol);
    if (pk_is_throwing(fetch)) {
        return false;
    }

    return !pk_is_undefined(fetch);
}

PKAtom pk_unbind(PK_ENV env, PKAtom symbol) {
    PKAtom result = pk_set_raw(env, symbol, pk_undefined());
    pk_propagate(result);

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
            return pk_errors(pk_str("out of memory"));
        }
        PK_CORE.lets.e = new_e;
        PK_CORE.lets.capacity = new_capacity;
    }

    PKAtom restore = pk_set_raw(env, symbol, value);
    pk_propagate(restore);

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
        PKAtom displaced = pk_set(unwind.env, unwind.symbol, unwind.restore);
        pk_ref_dec(unwind.restore);
        if (!pk_is_throwing(displaced)) {
            pk_ref_dec(displaced);
        }
    }
    PK_CORE.lets.count -= n;
}
