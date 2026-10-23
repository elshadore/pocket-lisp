#include "pocket_internals.h"

PKAtomSymbol *pk_atom_gensym(Pocket lisp) {
    // TODO: better implementation of this.
    // This is a very hacky implementation.
    PKWriter w = PK_WRITER_EMPTY;
    pk_writer_cstr(lisp, &w, "__gensym_");
    pk_writer_fmt(lisp, &w, "%zu", lisp->gensym_counter);
    lisp->gensym_counter += 1;
    PKAtomSymbol *result = pk_atom_symbol(lisp, pk_writer_slice(&w));
    pk_writer_deinit(lisp, &w);
    return result;
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
    result->prop = lisp->cache.undefined;
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
        pk_error_type(lisp, PK_TYPE_SYMBOL, atom);
        return NULL;
    }
}

PKAtomKeyword *pk_atom_assert_keyword(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_keyword(atom)) {
        return (PKAtomKeyword *)atom;
    } else {
        pk_error_type(lisp, PK_TYPE_KEYWORD, atom);
        return NULL;
    }
}
