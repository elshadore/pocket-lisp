#include "pocket_internals.h"

typedef struct PKAtomGensymData_ {
    PKWriter w;
} PKAtomGensymData;

void pk_atom_gensym_body(void *user_closure, Pocket lisp) {
    PKAtomGensymData *data = (PKAtomGensymData *)user_closure;
    for (;;) {
        pk_writer_fmt(lisp, &data->w, "GENSYM::%zu", lisp->gensym_counter);
        lisp->gensym_counter += 1;
        PKAtom *get = pk_hashmap_get_str(lisp, &lisp->symbolizer, pk_writer_slice(&data->w));
        if (pk_atom_is_undefined(get)) {
            pk_stack_push(lisp, (PKAtom *)pk_atom_symbol(lisp, pk_writer_slice(&data->w)));
            pk_writer_deinit(lisp, &data->w);
            return;
        }
        pk_writer_reset(&data->w);
    }
}

PKAtomSymbol *pk_atom_gensym(Pocket lisp) {
    PKAtomGensymData data = (PKAtomGensymData) {
        .w = PK_WRITER_EMPTY,
    };

    PK_RES result = pk_protect(lisp, &data, pk_atom_gensym_body, 0);

    pk_writer_deinit(lisp, &data.w);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }

    PKAtom *out = pk_stack_head_pop(lisp);
    return (PKAtomSymbol *)out;
}

PKAtomSymbol *pk_atom_symbol(Pocket lisp, PKStr string) {
    PKAtom *interned = pk_hashmap_get_str(lisp, &lisp->symbolizer, string);
    if (!pk_atom_is_undefined(interned)) {
        return (PKAtomSymbol *)interned;
    }
    
    PKAtomSymbol *result = (PKAtomSymbol *)pk_atom_alloc(lisp);

    PKAtomString *name = pk_atom_string(lisp, string);

    result->header.type = PK_ATOMTY_SYMBOL;
    result->var = lisp->cache.undefined;
    result->fun = lisp->cache.undefined;
    result->name = name;
    result->type_type = PK_TYPETY_NOTYPE;
    result->slots = PK_SLOTMAP_EMPTY;
    result->methods = PK_HASHMAP_EMPTY;
    result->owned_memory = 0;

    pk_hashmap_insert(lisp, &lisp->symbolizer, (PKAtom *)name, (PKAtom *)result, PK_HASHMAP_INSERT_NORMAL);
    
    return result;
}

PKAtomSymbol *pk_atom_symbolc(Pocket lisp, char *csymbol) {
    PKStr symbol = pk_str_from_cstr(csymbol);
    return pk_atom_symbol(lisp, symbol);
}

PKAtomKeyword *pk_atom_keyword(Pocket lisp, PKStr keyword) {
    PKAtomKeyword *result = (PKAtomKeyword *)pk_atom_alloc(lisp);

    PKStr dupe = pk_str_dupe(lisp, keyword);

    result->header.type = PK_ATOMTY_KEYWORD;
    result->id = dupe;

    return result;
}

PKAtomKeyword *pk_atom_keywordc(Pocket lisp, char *ckeyword) {
    PKStr keyword = pk_str_from_cstr(ckeyword);
    return pk_atom_keyword(lisp, keyword);
}

bool pk_atom_is_symbol(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_SYMBOL;
}

PKAtom *pk_atom_list_symbols(Pocket lisp) {
    PKAccum acc = pk_accum_new(lisp);

    for (PKPool *iter = lisp->pool; iter != NULL; iter = iter->next) {
        for (size_t i = 0; i < PK_POOL_MAX; ++i) {
            PKAtom *atom = &iter->e[i];
            if (pk_atom_is_symbol(atom)) {
                pk_accum_push(&acc, atom);
            }
        }
    }

    return acc.head;
}

bool pk_atom_is_keyword(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_KEYWORD;
}

bool pk_atom_symbol_cmp(PKAtomSymbol *symbol, PKStr comp) {
    return pk_str_eq(symbol->name->str, comp);
}

bool pk_atom_keyword_cmp(PKAtomKeyword *keyword, PKStr comp) {
    return pk_str_eq(keyword->id, comp);
}

PKAtomSymbol *pk_atom_assert_symbol(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_symbol(atom)) {
        return (PKAtomSymbol *)atom;
    } else {
        pk_error_type(lisp, atom, lisp->cache.type_symbol);
        return NULL;
    }
}

PKAtomKeyword *pk_atom_assert_keyword(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_keyword(atom)) {
        return (PKAtomKeyword *)atom;
    } else {
        pk_error_type(lisp, atom, lisp->cache.type_keyword);
        return NULL;
    }
}
