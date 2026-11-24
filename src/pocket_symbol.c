#include "pocket_internals.h"

PKAtomSymbol *pk_atom_gensym(Pocket lisp) {
    for (;;) {
        pk_writer_reset(lisp);
        pk_writer_fmt(lisp, "GENSYM::%zu", lisp->gensym_counter);
        lisp->gensym_counter += 1;
        PKAtom *get = pk_hashmap_get_str(lisp, &lisp->symbolizer, pk_writer_str(lisp));
        if (pk_atom_is_undefined(get)) {
            PKAtomSymbol *result = pk_atom_symbol(lisp, pk_writer_str(lisp));
            pk_writer_reset(lisp);
            return result;
        }
    }
}

PKAtomSymbol *pk_atom_symbol(Pocket lisp, PKStr string) {
    PKAtom *interned = pk_hashmap_get_str(lisp, &lisp->symbolizer, string);
    if (!pk_atom_is_undefined(interned)) {
        return (PKAtomSymbol *)interned;
    }
    
    PKAtomSymbol *result = (PKAtomSymbol *)pk_atom_alloc(lisp);

    PKAtomString *name = pk_atom_string(lisp, string);

    result->header.type = PK_ATOMTY_SYMBOL;
    result->var = &PK_ATOM_UNDEFINED;
    result->fun = &PK_ATOM_UNDEFINED;
    result->name = name;
    result->props = PK_HASHMAP_EMPTY;

    pk_hashmap_insert(lisp, &lisp->symbolizer, (PKAtom *)name, (PKAtom *)result, PK_HASHMAP_INSERT_NORMAL);

    pk_atom_set_immortal((PKAtom *)result);
    pk_atom_set_immortal((PKAtom *)name);
    
    return result;
}

PKAtomString *pk_atom_symbol_to_string(Pocket lisp, PKAtomSymbol *symbol) {
    (void)lisp;
    return symbol->name;
}

PKAtomKeyword *pk_atom_symbol_to_keyword(Pocket lisp, PKAtomSymbol *symbol) {
    return pk_atom_keyword(lisp, symbol->name->str);
}

PKAtom *pk_atom_symbol_put(Pocket lisp, PKAtomSymbol *symbol, PKAtom *key, PKAtom *value) {
    return pk_atom_property_put(lisp, (PKAtom *)symbol, &symbol->props, key, value);
}

PKAtom *pk_atom_symbol_remove(Pocket lisp, PKAtomSymbol *symbol, PKAtom *key) {
    return pk_atom_property_remove(lisp, &symbol->props, key);
}

PKAtom *pk_atom_symbol_get(Pocket lisp, PKAtomSymbol *symbol, PKAtom *key) {
    return pk_atom_property_get(lisp, &symbol->props, key);
}

bool pk_atom_symbol_is_property(Pocket lisp, PKAtomSymbol *symbol, PKAtom *key) {
    return pk_atom_property_is_property(lisp, &symbol->props, key);
}

PKAtom *pk_atom_symbol_to_plist(Pocket lisp, PKAtomSymbol *symbol) {
    return pk_atom_property_to_plist(lisp, &symbol->props);
}

PKAtomSymbol *pk_atom_symbolc(Pocket lisp, char *csymbol) {
    PKStr symbol = pk_str_from_cstr(csymbol);
    return pk_atom_symbol(lisp, symbol);
}

PKAtomString *pk_atom_keyword_to_string(Pocket lisp, PKAtomKeyword *keyword) {
    (void)lisp;
    return keyword->name;
}

PKAtomSymbol *pk_atom_keyword_to_symbol(Pocket lisp, PKAtomKeyword *keyword) {
    return pk_atom_symbol(lisp, keyword->name->str);
}

PKAtomKeyword *pk_atom_keyword(Pocket lisp, PKStr keyword) {
    PKAtom *interned = pk_hashmap_get_str(lisp, &lisp->keywordizer, keyword);
    if (!pk_atom_is_undefined(interned)) {
        return (PKAtomKeyword *)interned;
    }

    PKAtomKeyword *result = (PKAtomKeyword *)pk_atom_alloc(lisp);

    PKAtomString *name = pk_atom_string(lisp, keyword);

    result->header.type = PK_ATOMTY_KEYWORD;
    result->name = name;

    pk_hashmap_insert(lisp, &lisp->keywordizer, (PKAtom *)name, (PKAtom *)result, PK_HASHMAP_INSERT_NORMAL);

    pk_atom_set_immortal((PKAtom *)result);
    pk_atom_set_immortal((PKAtom *)name);

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
    return pk_str_eq(keyword->name->str, comp);
}

PKAtomSymbol *pk_atom_assert_symbol(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_symbol(atom)) {
        pk_error_type(lisp, atom, PK_TYPE_SYMBOL);
    }
    return (PKAtomSymbol *)atom;
}

PKAtomKeyword *pk_atom_assert_keyword(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_keyword(atom)) {
        pk_error_type(lisp, atom, PK_TYPE_KEYWORD);
    }
    return (PKAtomKeyword *)atom;
}
