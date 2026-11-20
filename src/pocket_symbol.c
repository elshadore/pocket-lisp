#include "pocket_internals.h"

PKAtomSymbol *pk_atom_gensym(Pocket lisp) {
    for (;;) {
        pk_writer_reset(lisp);
        pk_writer_fmt(lisp, "GENSYM::%zu", lisp->gensym_counter);
        lisp->gensym_counter += 1;
        PKAtom *get = pk_hashmap_get_str(lisp, &lisp->symbolizer, pk_writer_slice(lisp));
        if (pk_atom_is_undefined(get)) {
            PKAtomSymbol *result = pk_atom_symbol(lisp, pk_writer_slice(lisp));
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
    result->var = lisp->cache.undefined;
    result->fun = lisp->cache.undefined;
    result->name = name;

    pk_hashmap_insert(lisp, &lisp->symbolizer, (PKAtom *)name, (PKAtom *)result, PK_HASHMAP_INSERT_NORMAL);
    
    return result;
}

PKAtomString *pk_atom_symbol_to_string(Pocket lisp, PKAtomSymbol *symbol) {
    (void)lisp;
    return symbol->name;
}

PKAtomKeyword *pk_atom_symbol_to_keyword(Pocket lisp, PKAtomSymbol *symbol) {
    return pk_atom_keyword(lisp, symbol->name->str);
}

PKAtomSymbol *pk_atom_symbolc(Pocket lisp, char *csymbol) {
    PKStr symbol = pk_str_from_cstr(csymbol);
    return pk_atom_symbol(lisp, symbol);
}

PKAtomString *pk_atom_keyword_to_string(Pocket lisp, PKAtomKeyword *keyword) {
    return pk_atom_string(lisp, keyword->id);
}

PKAtomSymbol *pk_atom_keyword_to_symbol(Pocket lisp, PKAtomKeyword *keyword) {
    return pk_atom_symbol(lisp, keyword->id);
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
