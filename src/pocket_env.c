#include "pocket_internals.h"

void pk_env_grow(Pocket lisp, PKEnv *env) {
    size_t new_capacity = pk_grow_capacity(env->capacity, PK_ENV_INIT_CAPACITY);
    PKEnvSlot **new_e = pk_malloc(lisp, new_capacity * sizeof(PKEnvSlot *));
    for (size_t i = 0; i < new_capacity; i++) {
        new_e[i] = NULL;
    }

    for (size_t i = 0; i < env->capacity; i++) {
        PKEnvSlot *entry = env->e[i];
        while (entry) {
            PKEnvSlot *next = entry->chain;
            size_t bucket = pk_hash_pointer(entry->key) % new_capacity;
            entry->chain = new_e[bucket];
            new_e[bucket] = entry;
            entry = next;
        }
    }

    if (env->e) {
        pk_free(lisp, env->e, env->capacity * sizeof(PKEnvSlot *));
    }
    env->e = new_e;
    env->capacity = new_capacity;
}

PKAtom *pk_env_put(Pocket lisp, PKEnv *env, PKAtomSymbol *key, PKAtom *value) {
    if (env->capacity == 0) {
        pk_env_grow(lisp, env);
    }

    size_t bucket = pk_hash_pointer(key) % env->capacity;

    for (PKEnvSlot *slot = env->e[bucket]; slot; slot = slot->chain) {
        if (slot->key == key) {
            PKAtom *result = slot->value;
            slot->value = value;
            return result;
        }
    }

    PKEnvSlot *entry = pk_malloc(lisp, sizeof(PKEnvSlot));
    *entry = (PKEnvSlot) {
        .key = key,
        .value = value,
        .chain = env->e[bucket],
    };
    env->e[bucket] = entry;
    env->count++;
    return NULL;
}

PKAtom *pk_env_query(Pocket lisp, PKEnv *env, PKAtomSymbol *key) {
    (void)lisp;
    if (env->capacity == 0) {
        return NULL;
    }

    size_t bucket = pk_hash_pointer(key) % env->capacity;

    for (PKEnvSlot *entry = env->e[bucket]; entry; entry = entry->chain) {
        if (entry->key == key) {
            return entry->value;
        }
    }

    return NULL;
}

PKAtom *pk_env_remove(Pocket lisp, PKEnv *env, PKAtomSymbol *key) {
    if (env->capacity == 0) {
        return NULL;
    }

    size_t bucket = pk_hash_pointer(key) % env->capacity;

    PKEnvSlot *prev = NULL;
    for (PKEnvSlot *slot = env->e[bucket]; slot; prev = slot, slot = slot->chain) {
        if (slot->key == key) {
            PKAtom *result = slot->value;
            if (prev) {
                prev->chain = slot->chain;
            } else {
                env->e[bucket] = slot->chain;
            }
            pk_free(lisp, slot, sizeof(PKEnvSlot));
            env->count--;
            return result;
        }
    }
    return NULL;
}

void pk_env_deinit(Pocket lisp, PKEnv *env) {
    for (size_t i = 0; i < env->capacity; i++) {
        PKEnvSlot *entry = env->e[i];
        while (entry) {
            PKEnvSlot *next = entry->chain;
            pk_free(lisp, entry, sizeof(PKEnvSlot));
            entry = next;
        }
    }
    if (env->e) {
        pk_free(lisp, env->e, env->capacity * sizeof(PKEnvSlot *));
    }
    *env = (PKEnv){0};
}

void pk_env_set(Pocket lisp, PKAtomSymbol *sym, PKAtom *value) {
    pk_env_put(lisp, &lisp->vars, sym, value);
}

PKAtom *pk_env_get(Pocket lisp, PKAtomSymbol *sym) {
    PKAtom *value = pk_env_query(lisp, &lisp->vars, sym);
    if (value == NULL) pk_error(lisp);
    return value;
}

void pk_env_unbind(Pocket lisp, PKAtomSymbol *sym) {
    pk_env_remove(lisp, &lisp->vars, sym);
}

void pk_env_fset(Pocket lisp, PKAtomSymbol *sym, PKAtom *value) {
    pk_env_put(lisp, &lisp->funs, sym, value);
}

PKAtom *pk_env_fget(Pocket lisp, PKAtomSymbol *sym) {
    PKAtom *value = pk_env_query(lisp, &lisp->funs, sym);
    if (value == NULL) pk_error(lisp);
    return value;
}

void pk_env_funbind(Pocket lisp, PKAtomSymbol *sym) {
    pk_env_remove(lisp, &lisp->funs, sym);
}
