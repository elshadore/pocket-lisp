#include "pocket_internals.h"

PKAtomData PK_ATOM_STRING_EMPTY = (PKAtomData) {
    .string = (PKAtomString) {
        .header.type = PK_ATOMTY_STRING,
        .header.flags = PK_ATOMFLAG_IMMORTAL,
        .header.refcount = 0,
        .str = PK_STR_EMPTY,
    }
};

PKAtom pk_string_empty(void) {
    return &PK_ATOM_STRING_EMPTY;
}

PKAtom pk_string_nomemcpy(PKStr string) {
    if (string.length == 0) {
        return pk_string_empty();
    }

    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    
    atom->header.type = PK_ATOMTY_STRING;
    atom->string.str = string;
    
    return atom;
}

PKAtom pk_string(PKStr string) {
    if (string.length == 0) {
        return pk_string_empty();
    }

    PKStr dupe = PK_STR_EMPTY;
    pk_propagate(pk_str_dupe(string, &dupe));
    
    return pk_string_nomemcpy(dupe);
}

PKAtom pk_string_alloc(size_t length) {
    if (length == 0) {
        return pk_string_empty();
    }

    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);
    
    PKStr buffer = PK_STR_EMPTY;
    pk_propagate(pk_str_alloc(length, &buffer));
    
    atom->header.type = PK_ATOMTY_STRING;
    atom->string.str = buffer;
    
    return atom;
}

PKAtom pk_stringc(char *cstring) {
    PKStr string = pk_str_from_cstr(cstring);
    return pk_string(string);
}

PKAtom pk_string_cat_part(PKAtom atom, PKStr *part, PKAtom *owned) {
    if (pk_is_string(atom)) {
        *part = atom->string.str;
        *owned = pk_nil();
        return pk_true();
    }
    
    PKAtom result = pk_format(atom);
    pk_propagate(result);
    *part = result->string.str;
    *owned = result;
    return pk_true();
}

PKAtom pk_string_cat(PKAtoms strings) {
    size_t length = 0;
    for (size_t i = 0; i < strings.length; ++i) {
        PKStr part = PK_STR_EMPTY;
        PKAtom owned = pk_nil();
        PKAtom result = pk_string_cat_part(strings.e[i], &part, &owned);
        if (pk_is_throwing(result)) {
            return result;
        }
        pk_ref_dec(owned);
        length += part.length;
    }
    PKAtom atom = pk_string_alloc(length);
    
    pk_propagate(atom);

    {
        size_t offset = 0;
        for (size_t i = 0; i < strings.length; ++i) {
            PKStr part = PK_STR_EMPTY;
            PKAtom owned = pk_nil();
            PKAtom result = pk_string_cat_part(strings.e[i], &part, &owned);
            pk_propagate(result);
            if (part.length > 0) {
                memcpy(atom->string.str.c + offset, part.c, part.length);
                offset += part.length;
            }
            pk_ref_dec(owned);
        }
    }
    return atom;
}

bool pk_is_string(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_STRING;
}

PKAtom pk_typecheck_string(PKAtom atom) {
    if (pk_is_string(atom)) {
        return pk_true();
    } else {
        return pk_errors(pk_str("expected string"));
    }
}

PKStr pk_to_string(PKAtom atom) {
    if (!pk_is_string(atom)) {
        return PK_STR_EMPTY;
    } else {
        return atom->string.str;
    }
}

bool pk_string_qeq(PKAtom string, PKStr comp) {
    if (!pk_is_string(string)) {
        return false;
    }
    return pk_str_eq(string->string.str, comp);
}
