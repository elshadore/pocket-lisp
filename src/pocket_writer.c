#include "pocket_internals.h"

PK_RES pk_writer_cons_loop(PKWriter *w, PKHashTable *ht, PKAtomCons *cons);

PKWriter pk_writer_init(Pocket lisp) {
    PKWriter w;
    w.lisp = lisp;
    w.c = NULL;
    w.count = 0;
    w.capacity = 0;
    return w;
}

PK_RES pk_writer_deinit(PKWriter *w) {
    if (w->c != NULL) {
        pk_free(w->lisp, w->c, w->capacity);
    }
    *w = pk_writer_init(w->lisp);
    return PK_OK;
}

PK_RES pk_writer_expand(PKWriter *w, size_t needed) {
    size_t new_count = w->count + needed;
    if (new_count > w->capacity) {
        size_t new_capacity = pk_grow_capacity(w->capacity, PK_WRITER_INIT_CAPACITY);
        while (new_capacity < new_count) {
            new_capacity *= 2;
        }
        pk_try(pk_realloc(w->lisp, w->c, w->capacity, new_capacity, (void **)&w->c));
        w->capacity = new_capacity;
    }
    return PK_OK;
}

PK_RES pk_writer_char(PKWriter *w, char c) {
    pk_try(pk_writer_expand(w, 1));
    w->c[w->count++] = c;
    return PK_OK;
}

PK_RES pk_writer_string(PKWriter *w, const char *string) {
    size_t length = 0;
    
    length = strlen(string);
    
    return pk_writer_stringn(w, string, length);
}

PK_RES pk_writer_stringn(PKWriter *w, const char *string, size_t length) {
    pk_try(pk_writer_expand(w, length));
    memcpy(w->c + w->count, string, length);
    w->count += length;
    return PK_OK;
}

PK_RES pk_writer_stringn_escaped(PKWriter *w, const char *string, size_t length) {
    size_t i = 0;
    
    for (i = 0; i < length; ++i) {
        char c = string[i];
        switch (c) {
            case '"': {
                pk_try(pk_writer_string(w, "\\\""));
                break;
            }
            case '\\': {
                pk_try(pk_writer_string(w, "\\\\"));
                break;
            }
            case '\n': {
                pk_try(pk_writer_string(w, "\\n"));
                break;
            }
            case '\t': {
                pk_try(pk_writer_string(w, "\\t"));
                break;
            }
            case '\r': {
                pk_try(pk_writer_string(w, "\\r"));
                break;
            }
            default: {
                pk_try(pk_writer_char(w, c));
                break;
            }
        }
    }
    return PK_OK;
}
    
PK_RES pk_writer_string_escaped(PKWriter *w, const char *string) {
    size_t length = 0;
    length = strlen(string);

    return pk_writer_stringn_escaped(w, string, length);
}

PK_RES pk_writer_int(PKWriter *w, int integer) {
    unsigned int dec = 0;
    size_t save = 0;
    
    if (integer < 0) {
        dec = (unsigned int)-integer;
        pk_try(pk_writer_char(w, '-'));
    } else {
        dec = (unsigned int)integer;
    }
    
    save = w->count;
    
    do {
        int digit = dec % 10;
        pk_try(pk_writer_char(w, pk_char_from_digit((pk_u8)digit)));
        
    } while ((dec /= 10) > 0);
    
    pk_string_reverse(w->c + save, w->count - save);
    
    return PK_OK;
}

PK_RES pk_writer_float(PKWriter *w, float floater) {
    (void)floater;
    pk_try(pk_writer_string(w, "[TODO: writing floats]"));
    return PK_OK;
}

PK_RES pk_writer_address(PKWriter *w, size_t address) {
    int i = 0;
    
    pk_try(pk_writer_string(w, "0x"));
    
    for (i = (int)(sizeof(size_t) * 2) - 1; i >= 0; i--) {
        pk_u8 nibble = (pk_u8)((address >> (i * 4)) & 0xf);
        pk_try(pk_writer_char(w, pk_char_from_hex(nibble)));
    }
    
    return PK_OK;
}

PK_RES pk_writer_newline(PKWriter *w) {
    pk_try(pk_writer_char(w, '\n'));
    return PK_OK;
}

PK_RES pk_writer_get(PKWriter *w, char **out_c, size_t *out_length) {
    *out_c = w->c;
    *out_length = w->count;
    return PK_OK;
}

PK_RES pk_writer_reset(PKWriter *w) {
    w->count = 0;
    return PK_OK;
}

void pk_writer_print(PKWriter *w) {
    pk_print(w->lisp, w->c, w->count);
}

void pk_writer_puts(PKWriter *w) {
    pk_puts(w->lisp, w->c, w->count);
}

PK_RES pk_writer_atom_object(PKWriter *w, PKAtom *atom){
    const char *ident = pk_ident_atomty(atom->tag.ty);
    pk_try(pk_writer_string(w, "&<"));
    pk_try(pk_writer_string(w, ident));
    pk_try(pk_writer_string(w, "::"));
    pk_try(pk_writer_address(w, (size_t)atom));
    pk_try(pk_writer_char(w, '>'));
    return PK_OK;
}

PK_RES pk_writer_atom_function(PKWriter *w, void *mem, const char *type, PKFuncArity arity){
    pk_try(pk_writer_string(w, "&<"));
    pk_try(pk_writer_string(w, type));
    pk_try(pk_writer_string(w, "::"));
    pk_try(pk_writer_address(w, (size_t)mem));
    pk_try(pk_writer_string(w, "::"));
    pk_try(pk_writer_int(w, arity.args));
    pk_try(pk_writer_string(w, "::"));
    pk_try(pk_writer_string(w, pk_ident_arity(arity.mode)));
    pk_try(pk_writer_char(w, '>'));
    return PK_OK;
}
    
PK_RES pk_writer_atom_rec(PKWriter *w, PKHashTable *ht, PKAtom *atom) {
    switch (atom->tag.ty) {
        case PKAtomTy_Nil: {
            pk_try(pk_writer_string(w, "nil"));
            break;
        }
        case PKAtomTy_Number: {
            if (atom->number.ty == PKNumberTy_Int) {
                pk_try(pk_writer_int(w, atom->number.as.i));
            } else {
                pk_try(pk_writer_float(w, atom->number.as.f));
            }
            break;
        }
        case PKAtomTy_Symbol: {
            pk_try(pk_writer_stringn(w, atom->symbol.id->c, atom->symbol.id->length));
            break;
        }
        case PKAtomTy_Keyword: {
            pk_try(pk_writer_char(w, ':'));
            pk_try(pk_writer_stringn(w, atom->keyword.c, atom->keyword.length));
            break;
        }
        case PKAtomTy_String: {
            pk_try(pk_writer_char(w, '"'));
            pk_try(pk_writer_stringn_escaped(w, atom->string.c, atom->string.length));
            pk_try(pk_writer_char(w, '"'));
            break;
        }
        case PKAtomTy_Cons: {
            PKAtom *ignore = NULL;
            if (pk_hashtable_get(w->lisp, ht, atom, &ignore)) {
                pk_try(pk_writer_atom_object(w, atom));
            } else {
                pk_try(pk_hashtable_put(w->lisp, ht, atom, atom));
                pk_try(pk_writer_char(w, '('));
                pk_try(pk_writer_atom_rec(w, ht, atom->cons.car));
                pk_try(pk_writer_cons_loop(w, ht, (PKAtomCons *)atom));
                pk_try(pk_writer_char(w, ')'));
            }
            break;
        }
        case PKAtomTy_CFunc: {
            PKAtomCFunc *cfunc = (PKAtomCFunc *)atom;
            pk_try(pk_writer_atom_function(w, atom, "CFUNC", cfunc->arity));
            break;
        }
        case PKAtomTy_LFunc: {
            PKAtomLFunc *lfunc = (PKAtomLFunc *)atom;
            pk_try(pk_writer_atom_function(w, atom, "LFUNC", lfunc->arity));
            break;
        }
        default: {
            pk_try(pk_writer_atom_object(w, atom));
            break;
        }
    }
    return PK_OK;
}
    
PK_RES pk_writer_atom(PKWriter *w, PKAtom *atom) {
    PKHashTable ht = pk_hashtable_init();
    PK_RES result = pk_writer_atom_rec(w, &ht, atom);
    pk_hashtable_deinit(w->lisp, &ht);
    return result;
}

PK_RES pk_writer_cons_loop(PKWriter *w, PKHashTable *ht, PKAtomCons *cons) {
    PKAtom *iter = cons->cdr;
    
    while (iter->tag.ty == PKAtomTy_Cons) {
        PKAtom *ignore = NULL;
        pk_try(pk_writer_char(w, ' '));
        if (pk_hashtable_get(w->lisp, ht, iter, &ignore)) {
            pk_try(pk_writer_atom_object(w, iter));
            return PK_OK;
        } else {
            pk_try(pk_hashtable_put(w->lisp, ht, iter, iter));
            pk_try(pk_writer_atom_rec(w, ht, iter->cons.car));
            iter = iter->cons.cdr;
        }
    }
    
    if (iter->tag.ty != PKAtomTy_Nil) {
        pk_try(pk_writer_string(w, " . "));
        pk_try(pk_writer_atom_rec(w, ht, iter));
    }
    
    return PK_OK;
}

PK_RES pk_writer_to_string(PKWriter *w, PKAtomString **output) {
    return pk_atom_stringn(w->lisp, w->c, w->count, output);
}
