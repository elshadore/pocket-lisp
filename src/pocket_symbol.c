#include "pocket_internals.h"

PKAtom pk_symbol(PKStr string) {
    PKAtom interned = pk_hashmap_get_str(&PK_CORE.symbolizer, string);
    if (!pk_is_undefined(interned)) {
        pk_propagate(pk_ref_inc(interned));
        return interned;
    }
    
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);

    PKAtom name = pk_string(string);
    pk_propagate(name);

    atom->header.type = PK_ATOMTY_SYMBOL;
    atom->symbol.var = pk_undefined();
    atom->symbol.fun = pk_undefined();
    atom->symbol.prop = pk_undefined();
    atom->symbol.name = name;
    atom->symbol.type_type = PK_TYPETY_NOTYPE;
    atom->symbol.slots = PK_SLOTMAP_EMPTY;
    atom->symbol.methods = PK_HASHMAP_EMPTY;
    atom->symbol.owned_memory = 0;

    pk_propagate(pk_hashmap_insert(&PK_CORE.symbolizer, name, atom));
    
    return atom;
}

PKAtom pk_symbolc(char *csymbol) {
    PKStr symbol = pk_str_from_cstr(csymbol);
    return pk_symbol(symbol);
}

PKAtom pk_keyword(PKStr keyword) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);

    PKStr dupe = PK_STR_EMPTY;
    pk_propagate(pk_str_dupe(keyword, &dupe));

    atom->header.type = PK_ATOMTY_KEYWORD;
    atom->keyword.id = dupe;

    return atom;
}
bool pk_symbol_qeq(PKAtom symbol, PKStr comp) {
    if (!pk_is_symbol(symbol)) {
        return false;
    }
    return pk_str_eq(symbol->symbol.name->string.str, comp);
}

bool pk_keyword_qeq(PKAtom keyword, PKStr comp) {
    if (!pk_is_keyword(keyword)) {
        return false;
    }
    return pk_str_eq(keyword->keyword.id, comp);
}

bool pk_is_symbol(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_SYMBOL;
}

bool pk_is_keyword(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_KEYWORD;
}
