#include "pocket_internals.h"

PKAtomSymbol *pk_atom_symbol(Pocket lisp, PKStr string) {
    PKAtom *interned = pk_hashmap_get_str(&lisp->symbolizer, string);
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

    pk_hashmap_insert(&lisp->symbolizer, (PKAtom *)name, (PKAtom *)result);
    
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

