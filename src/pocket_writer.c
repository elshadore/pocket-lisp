#include "pocket_internals.h"

void pk_writer_deinit(PKWriter *w) {
    pk_free(w->c);
    *w = PK_WRITER_EMPTY;
}

PKAtom pk_writer_expand(PKWriter *w, size_t needed) {
    size_t new_count = w->count + needed;
    if (new_count > w->capacity) {
        size_t new_capacity = pk_grow_capacity(w->capacity, 128);
        while (new_capacity < new_count) {
            new_capacity *= 2;
        }
        char *new_c = pk_realloc(w->c, new_capacity * sizeof(char));
        if (new_c == NULL) {
            return pk_error_oom();
        }
        w->c = new_c;
        w->capacity = new_capacity;
    }
    return pk_true();
}

PKAtom pk_writer_char(PKWriter *w, char c) {
    pk_propagate(pk_writer_expand(w, 1));
    w->c[w->count++] = c;
    return pk_true();
}

PKAtom pk_writer_string(PKWriter *w, PKStr string) {
    pk_propagate(pk_writer_expand(w, string.length));
    memcpy(w->c + w->count, string.c, string.length);
    w->count += string.length;
    return pk_true();
}

PKAtom pk_writer_string_escaped(PKWriter *w, PKStr string) {
    for (size_t i = 0; i < string.length; ++i) {
        char c = string.c[i];
        switch (c) {
            case '"': {
                pk_propagate(pk_writer_string(w, pk_str("\\\"")));
                break;
            }
            case '\\': {
                pk_propagate(pk_writer_string(w, pk_str("\\\\")));
                break;
            }
            case '\n': {
                pk_propagate(pk_writer_string(w, pk_str("\\n")));
                break;
            }
            case '\t': {
                pk_propagate(pk_writer_string(w, pk_str("\\t")));
                break;
            }
            case '\r': {
                pk_propagate(pk_writer_string(w, pk_str("\\r")));
                break;
            }
            default: {
                pk_propagate(pk_writer_char(w, c));
                break;
            }
        }
    }
    return pk_true();
}

PK_PRINTF_FMT(2, 3) PKAtom pk_writer_fmt(PKWriter *w, const char *fmt, ...) {
    va_list args;
    
    va_start(args, fmt);
    size_t length = (size_t)vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    pk_propagate(pk_writer_expand(w, length + 1));
    
    va_start(args, fmt);
    vsnprintf(w->c + w->count, length + 1, fmt, args);
    va_end(args);

    w->count += length;
    
    return pk_true();
}

PKAtom pk_writer_int(PKWriter *w, int integer) {
    unsigned int dec = 0;
    size_t save = 0;
    
    if (integer < 0) {
        dec = (unsigned int)-integer;
        pk_propagate(pk_writer_char(w, '-'));
    } else {
        dec = (unsigned int)integer;
    }
    
    save = w->count;
    
    do {
        int digit = dec % 10;
        pk_propagate(pk_writer_char(w, pk_char_from_digit((pk_u8)digit)));
        
    } while ((dec /= 10) > 0);
    
    pk_str_reverse_mut(w->c + save, w->count - save);
    
    return pk_true();
}

PKAtom pk_writer_float(PKWriter *w, float floater) {
    // (void)floater;
    // pk_propagate(pk_writer_string(w, pk_str("[TODO: writing floats]")));
    // return pk_true();
    pk_propagate(pk_writer_fmt(w, "%f", floater));
    return pk_true();
}

PKAtom pk_writer_address(PKWriter *w, size_t address) {
    int i = 0;
    
    pk_propagate(pk_writer_string(w, pk_str("0x")));
    
    for (i = (int)(sizeof(size_t) * 2) - 1; i >= 0; i--) {
        pk_u8 nibble = (pk_u8)((address >> (i * 4)) & 0xf);
        pk_propagate(pk_writer_char(w, pk_char_from_hex(nibble)));
    }
    
    return pk_true();
}

PKAtom pk_writer_newline(PKWriter *w) {
    pk_propagate(pk_writer_char(w, '\n'));
    return pk_true();
}

PKStr pk_writer_slice(PKWriter *w) {
    return (PKStr) {
        .c = w->c,
        .length = w->count
    };
}

PKAtom pk_writer_atom(PKWriter *w, PKAtom atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_NIL: {
            pk_propagate(pk_writer_string(w, pk_str("nil")));
            break;
        }
        case PK_ATOMTY_TRUE: {
            pk_propagate(pk_writer_string(w, pk_str("true")));
            break;
        }
        case PK_ATOMTY_NUMBER: {
            if (atom->number.num_type == PK_NUMTY_INT) {
                pk_propagate(pk_writer_fmt(w, "%d", atom->number.as.i));
            } else {
                pk_propagate(pk_writer_fmt(w, "%f", atom->number.as.f));
            }
            break;
        }
        case PK_ATOMTY_STRING: {
            pk_propagate(pk_writer_char(w, '"'));
            pk_propagate(pk_writer_string_escaped(w, atom->string.str));
            pk_propagate(pk_writer_char(w, '"'));
            break;
        }
        case PK_ATOMTY_SYMBOL: {
            pk_propagate(pk_writer_string(w, atom->symbol.name->string.str));
            break;
        }
        default: {
            break;
        }
    }
    return pk_true();
}
