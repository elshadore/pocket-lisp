#include "pocket_internals.h"

void pk_writer_deinit(Pocket lisp, PKWriter *w) {
    pk_free(lisp, w->c);
    *w = PK_WRITER_EMPTY;
}

void pk_writer_expand(Pocket lisp, PKWriter *w, size_t needed) {
    size_t new_count = w->count + needed;
    if (new_count > w->capacity) {
        size_t new_capacity = pk_grow_capacity(w->capacity, 128);
        while (new_capacity < new_count) {
            new_capacity *= 2;
        }
        char *new_c = pk_realloc(lisp, w->c, new_capacity * sizeof(char));
        w->c = new_c;
        w->capacity = new_capacity;
    }
}

void pk_writer_char(Pocket lisp, PKWriter *w, char c) {
    pk_writer_expand(lisp, w, 1);
    w->c[w->count++] = c;
}

void pk_writer_cstr(Pocket lisp, PKWriter *w, const char *cstr) {
    size_t length = strlen(cstr);
    if (length == 0) {
        return;
    }
    pk_writer_expand(lisp, w, length);
    memcpy(w->c + w->count, cstr, length);
    w->count += length;
}

void pk_writer_string(Pocket lisp, PKWriter *w, PKStr string) {
    pk_writer_expand(lisp, w, string.length);
    memcpy(w->c + w->count, string.c, string.length);
    w->count += string.length;
}

void pk_writer_string_escaped(Pocket lisp, PKWriter *w, PKStr string) {
    for (size_t i = 0; i < string.length; ++i) {
        char c = string.c[i];
        switch (c) {
            case '"': {
                pk_writer_string(lisp, w, pk_str("\\\""));
                break;
            }
            case '\\': {
                pk_writer_string(lisp, w, pk_str("\\\\"));
                break;
            }
            case '\n': {
                pk_writer_string(lisp, w, pk_str("\\n"));
                break;
            }
            case '\t': {
                pk_writer_string(lisp, w, pk_str("\\t"));
                break;
            }
            case '\r': {
                pk_writer_string(lisp, w, pk_str("\\r"));
                break;
            }
            default: {
                pk_writer_char(lisp, w, c);
                break;
            }
        }
    }
}

PK_PRINTF_FMT(3, 4) void pk_writer_fmt(Pocket lisp, PKWriter *w, const char *fmt, ...) {
    va_list args;
    
    va_start(args, fmt);
    size_t length = (size_t)vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    pk_writer_expand(lisp, w, length + 1);
    
    va_start(args, fmt);
    vsnprintf(w->c + w->count, length + 1, fmt, args);
    va_end(args);

    w->count += length;
}

void pk_writer_int(Pocket lisp, PKWriter *w, int integer) {
    unsigned int dec = 0;
    size_t save = 0;
    
    if (integer < 0) {
        dec = (unsigned int)-integer;
        pk_writer_char(lisp, w, '-');
    } else {
        dec = (unsigned int)integer;
    }
    
    save = w->count;
    
    do {
        int digit = dec % 10;
        pk_writer_char(lisp, w, pk_char_from_digit((pk_u8)digit));
        
    } while ((dec /= 10) > 0);

    PKStr str = (PKStr){
        .c = w->c + save,
        .length = w->count - save
    };
    pk_str_reverse_mut(str);
}

void pk_writer_address(Pocket lisp, PKWriter *w, size_t address) {
    int i = 0;
    pk_writer_string(lisp, w, pk_str("0x"));
    
    for (i = (int)(sizeof(size_t) * 2) - 1; i >= 0; i--) {
        pk_u8 nibble = (pk_u8)((address >> (i * 4)) & 0xf);
        pk_writer_char(lisp, w, pk_char_from_hex(nibble));
    }
}

void pk_writer_newline(Pocket lisp, PKWriter *w) {
    pk_writer_char(lisp, w, '\n');
}

PKStr pk_writer_slice(PKWriter *w) {
    return (PKStr) {
        .c = w->c,
        .length = w->count
    };
}

void pk_writer_reset(PKWriter *w) {
    w->count = 0;
}

void pk_writer_simple_object(Pocket lisp, PKWriter *w, void *obj, PKStr name) {
    pk_writer_string(lisp, w, pk_str("&<"));
    pk_writer_string(lisp, w, name);
    pk_writer_string(lisp, w, pk_str("::"));
    pk_writer_address(lisp, w, (size_t)obj);
    pk_writer_char(lisp, w, '>');
}

void pk_writer_simple_function(Pocket lisp, PKWriter *w, void *obj, PKStr type, pk_u8 args, pk_u8 arity) {
    pk_writer_string(lisp, w, pk_str("&<"));
    pk_writer_string(lisp, w, type);
    pk_writer_string(lisp, w, pk_str("::"));
    pk_writer_fmt(lisp, w, "%u", args);
    pk_writer_string(lisp, w, pk_str(":"));
    pk_writer_string(lisp, w, pk_ident_arity(arity));
    pk_writer_string(lisp, w, pk_str("::"));
    pk_writer_address(lisp, w, (size_t)obj);
    pk_writer_char(lisp, w, '>');
}

void pk_writer_symbol_name(Pocket lisp, PKWriter *w, PKStr name) {
    if (!pk_str_needs_bars(name)) {
        pk_writer_string(lisp, w, name);
        return;
    }
    
    pk_writer_char(lisp, w, '|');
    for (size_t i = 0; i < name.length; ++i) {
        char c = name.c[i];
        if (c == '|') {
            pk_writer_string(lisp, w, pk_str("\\|"));
        } else if (c == '\\') {
            pk_writer_string(lisp, w, pk_str("\\\\"));
        } else if (c == '\n') {
            pk_writer_string(lisp, w, pk_str("\\n"));
        } else {
            pk_writer_char(lisp, w, c);
        }
    }
    pk_writer_char(lisp, w, '|');
}

void pk_writer_atom(Pocket lisp, PKWriter *w, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_UNDEFINED: {
            pk_writer_string(lisp, w, pk_str("&<UNDEFINED>"));
            break;
        }
        case PK_ATOMTY_NIL: {
            pk_writer_string(lisp, w, pk_str("nil"));
            break;
        }
        case PK_ATOMTY_TRUE: {
            pk_writer_string(lisp, w, pk_str("true"));
            break;
        }
        case PK_ATOMTY_NUMBER: {
            if (atom->number.num_type == PK_NUMTY_INT) {
                pk_writer_fmt(lisp, w, "%d", atom->number.as.i);
            } else {
                pk_writer_fmt(lisp, w, "%f", atom->number.as.f);
            }
            break;
        }
        case PK_ATOMTY_STRING: {
            pk_writer_char(lisp, w, '"');
            pk_writer_string_escaped(lisp, w, atom->string.str);
            pk_writer_char(lisp, w, '"');
            break;
        }
        case PK_ATOMTY_SYMBOL: {
            pk_writer_symbol_name(lisp, w, atom->symbol.name->string.str);
            break;
        }
        case PK_ATOMTY_KEYWORD: {
            pk_writer_char(lisp, w, ':');
            pk_writer_symbol_name(lisp, w, atom->keyword.id);
            break;
        }
        case PK_ATOMTY_CONS: {
            pk_writer_char(lisp, w, '(');
            
            pk_writer_atom(lisp, w, atom->cons.car);
            
            PKAtom *iter = atom->cons.cdr;
            while (pk_atom_is_cons(iter)) {
                pk_writer_char(lisp, w, ' ');
                pk_writer_atom(lisp, w, iter->cons.car);
                iter = iter->cons.cdr;
            }
            if (!pk_atom_is_nil(iter)) {
                pk_writer_string(lisp, w, pk_str(" . "));
                pk_writer_atom(lisp, w, iter);
            }
            
            pk_writer_char(lisp, w, ')');
            break;
        }
        case PK_ATOMTY_CFUN: {
            pk_writer_simple_function(lisp, w, atom, pk_str("CFUN"), atom->cfun.args, atom->cfun.arity);
            break;
        }
        case PK_ATOMTY_LFUN: {
            pk_writer_simple_function(lisp, w, atom, pk_str("LFUN"), atom->lfun.args, atom->lfun.arity);
            break;
        }
        case PK_ATOMTY_CMACRO: {
            pk_writer_simple_function(lisp, w, atom, pk_str("CMACRO"), atom->cmacro.args, atom->cmacro.arity);
            break;
        }
        case PK_ATOMTY_LMACRO: {
            pk_writer_simple_function(lisp, w, atom, pk_str("LMACRO"), atom->lmacro.args, atom->lmacro.arity);
            break;
        }
        default: {
            pk_writer_simple_object(lisp, w, atom, pk_str("UNKNOWN"));
            break;
        }
    }
}
