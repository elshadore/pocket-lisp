#include "ulisp_internals.h"

void ul_writer_deinit(ULWriter *w) {
    ul_free(w->c);
    *w = UL_WRITER_EMPTY;
}

ULAtom ul_writer_expand(ULWriter *w, size_t needed) {
    size_t new_count = w->count + needed;
    if (new_count > w->capacity) {
        size_t new_capacity = ul_grow_capacity(w->capacity, 128);
        while (new_capacity < new_count) {
            new_capacity *= 2;
        }
        char *new_c = ul_realloc(w->c, new_capacity * sizeof(char));
        if (new_c == NULL) {
            return ul_error_oom();
        }
        w->c = new_c;
        w->capacity = new_capacity;
    }
    return ul_true();
}

ULAtom ul_writer_char(ULWriter *w, char c) {
    ul_propagate(ul_writer_expand(w, 1));
    w->c[w->count++] = c;
    return ul_true();
}

ULAtom ul_writer_string(ULWriter *w, ULStr string) {
    ul_propagate(ul_writer_expand(w, string.length));
    memcpy(w->c + w->count, string.c, string.length);
    w->count += string.length;
    return ul_true();
}

ULAtom ul_writer_string_escaped(ULWriter *w, ULStr string) {
    for (size_t i = 0; i < string.length; ++i) {
        char c = string.c[i];
        switch (c) {
            case '"': {
                ul_propagate(ul_writer_string(w, ul_str("\\\"")));
                break;
            }
            case '\\': {
                ul_propagate(ul_writer_string(w, ul_str("\\\\")));
                break;
            }
            case '\n': {
                ul_propagate(ul_writer_string(w, ul_str("\\n")));
                break;
            }
            case '\t': {
                ul_propagate(ul_writer_string(w, ul_str("\\t")));
                break;
            }
            case '\r': {
                ul_propagate(ul_writer_string(w, ul_str("\\r")));
                break;
            }
            default: {
                ul_propagate(ul_writer_char(w, c));
                break;
            }
        }
    }
    return ul_true();
}

UL_PRINTF_FMT(2, 3) ULAtom ul_writer_fmt(ULWriter *w, const char *fmt, ...) {
    va_list args;
    
    va_start(args, fmt);
    size_t length = (size_t)vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    ul_propagate(ul_writer_expand(w, length + 1));
    
    va_start(args, fmt);
    vsnprintf(w->c + w->count, length + 1, fmt, args);
    va_end(args);

    w->count += length;
    
    return ul_true();
}

ULAtom ul_writer_int(ULWriter *w, int integer) {
    unsigned int dec = 0;
    size_t save = 0;
    
    if (integer < 0) {
        dec = (unsigned int)-integer;
        ul_propagate(ul_writer_char(w, '-'));
    } else {
        dec = (unsigned int)integer;
    }
    
    save = w->count;
    
    do {
        int digit = dec % 10;
        ul_propagate(ul_writer_char(w, ul_char_from_digit((ul_u8)digit)));
        
    } while ((dec /= 10) > 0);
    
    ul_str_reverse_mut(w->c + save, w->count - save);
    
    return ul_true();
}

ULAtom ul_writer_float(ULWriter *w, float floater) {
    // (void)floater;
    // ul_propagate(ul_writer_string(w, ul_str("[TODO: writing floats]")));
    // return ul_true();
    ul_propagate(ul_writer_fmt(w, "%f", floater));
    return ul_true();
}

ULAtom ul_writer_address(ULWriter *w, size_t address) {
    int i = 0;
    
    ul_propagate(ul_writer_string(w, ul_str("0x")));
    
    for (i = (int)(sizeof(size_t) * 2) - 1; i >= 0; i--) {
        ul_u8 nibble = (ul_u8)((address >> (i * 4)) & 0xf);
        ul_propagate(ul_writer_char(w, ul_char_from_hex(nibble)));
    }
    
    return ul_true();
}

ULAtom ul_writer_newline(ULWriter *w) {
    ul_propagate(ul_writer_char(w, '\n'));
    return ul_true();
}

ULStr ul_writer_slice(ULWriter *w) {
    return (ULStr) {
        .c = w->c,
        .length = w->count
    };
}

ULAtom ul_writer_atom(ULWriter *w, ULAtom atom) {
    switch (atom->header.type) {
        case UL_ATOMTY_NIL: {
            ul_propagate(ul_writer_string(w, ul_str("nil")));
            break;
        }
        case UL_ATOMTY_TRUE: {
            ul_propagate(ul_writer_string(w, ul_str("true")));
            break;
        }
        case UL_ATOMTY_NUMBER: {
            if (atom->number.num_type == UL_NUMTY_INT) {
                ul_propagate(ul_writer_fmt(w, "%d", atom->number.as.i));
            } else {
                ul_propagate(ul_writer_fmt(w, "%f", atom->number.as.f));
            }
            break;
        }
        case UL_ATOMTY_STRING: {
            ul_propagate(ul_writer_char(w, '"'));
            ul_propagate(ul_writer_string_escaped(w, atom->string.str));
            ul_propagate(ul_writer_char(w, '"'));
            break;
        }
        case UL_ATOMTY_SYMBOL: {
            ul_propagate(ul_writer_string(w, atom->symbol.name->string.str));
            break;
        }
        default: {
            break;
        }
    }
    return ul_true();
}
