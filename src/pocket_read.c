#include "pocket_internals.h"

#define PK_COMMENT_CHAR '#'

#define pk_reader_is_finished(r_) ((r_)->curr >= (r_)->str.length)

#define pk_rat(r_) (r_)->c

PKAtom pk_reader_error_(PKReader *r, PKWriter *w, PKStr error_message) {
    pk_propagate(pk_writer_fmt(
        w,
        "READER-ERROR:\n  curr: [%zu / %zu]\n  bol = %zu\n  line = %zu\n  char = %c\n  error:",
        r->curr,
        r->str.length,
        r->bol + 1,
        r->line + 1,
        r->c
    ));

    pk_propagate(pk_writer_string(w, error_message));
    pk_propagate(pk_writer_newline(w));

    return pk_errors(pk_writer_slice(w));
}

PKAtom pk_reader_error(PKReader *r, PKStr error_message) {
    PKWriter w = PK_WRITER_EMPTY;
    PKAtom result = pk_reader_error_(r, &w, error_message);
    pk_writer_deinit(&w);
    return result;
}

bool pk_reader_inc(PKReader *r) {
    r->curr += 1;
    if (pk_reader_is_finished(r)) {
        r->c = '\0';
        r->curr = r->str.length;
        return false;
    }

    r->c = r->str.c[r->curr];
    
    if (pk_rat(r) == '\n') {
        r->line += 1;
        r->bol = 0;
    } else {
        r->bol += 1;
    }
    
    return true;
}

PKAtom pk_reader_ince(PKReader *r) {
    if (!pk_reader_inc(r)) {
        return pk_reader_error(r, pk_str("reader is finished, expected more characters in string"));
    } else {
        return pk_true();
    }
}

bool pk_reader_peek(PKReader *r, char *output) {
    size_t peek = r->curr + 1;
    if (peek >= r->str.length) {
        *output = '\0';
        return false;
    } else {
        *output = r->str.c[peek];
        return true;
    }
}

bool pk_reader_trim_whitespace(PKReader *r) {
    bool whitespace = false;
    
    for (;;) {
        if (pk_rat(r) == PK_COMMENT_CHAR) {
            while (pk_reader_inc(r) && pk_rat(r) != '\n');
        } else if ((!pk_reader_is_finished(r)) && pk_char_is_whitespace(pk_rat(r))) {
            while (pk_reader_inc(r) && pk_char_is_whitespace(pk_rat(r)));
            whitespace = true;
        } else {
            break;
        }
    }
    
    return whitespace;
}

bool pk_reader_is_number_prefix(PKReader *r) {
    char c = pk_rat(r);
    if (pk_char_is_digit(c)) return true;
    if (c != '+' && c != '-') return false;
    if (!pk_reader_peek(r, &c)) return false;
    if (pk_char_is_digit(c)) return true;
    return false;
}

PKReader pk_reader_init(PKStr string) {
    return (PKReader) {
        .str = string,
        .curr = 0,
        .line = 0,
        .bol = 0,
        .c = string.c[0],
    };
}

PKAtom pk_read_keyword(PKReader *r) {
    size_t save = 0;
    
    if (pk_rat(r) != ':') {
        return pk_reader_error(r, pk_str("keyword does not start with ':', character"));
    }
    
    pk_propagate(pk_reader_ince(r));
    
    if (pk_rat(r) == '|') {
        PKWriter w = PK_WRITER_EMPTY;
        PKAtom result = pk_read_barred(r, &w, PK_SYMTY_KEYWORD);
        pk_writer_deinit(&w);
        return result;
    }
    
    save = r->curr;
    
    if (!pk_char_is_symbol(pk_rat(r))) {
        return pk_reader_error(r, pk_str("expected valid symbol character at start of keyword"));
    }

    do {
        if (!pk_reader_inc(r)) break;
    } while(pk_char_is_symbol(pk_rat(r)));

    PKStr slice = (PKStr) {
        .c = r->str.c + save,
        .length = r->curr - save,
    };
    
    return pk_keyword(slice);
}

PKAtom pk_read_write_escape(PKReader *r, PKWriter *w) {
    switch (pk_rat(r)) {
        case 'a': {
            pk_propagate(pk_writer_char(w, '\a'));
            break;
        }
        case 'b': {
            pk_propagate(pk_writer_char(w, '\b'));
            break;
        }
        case 'e': {
            pk_propagate(pk_writer_char(w, '\x1B'));
            break;
        }
        case 'f': {
            pk_propagate(pk_writer_char(w, '\f'));
            break;
        }
        case 'n': {
            pk_propagate(pk_writer_char(w, '\n'));
            break;
        }
        case 'r': {
            pk_propagate(pk_writer_char(w, '\r'));
            break;
        }
        case 't': {
            pk_propagate(pk_writer_char(w, '\t'));
            break;
        }
        case 'v': {
            pk_propagate(pk_writer_char(w, '\v'));
            break;
        }
        case 'x': {
            char a = '\0';
            char b = '\0';
            pk_u8 byte = 0;

            pk_propagate(pk_reader_ince(r));
            a = pk_rat(r);
            pk_propagate(pk_reader_ince(r));
            b = pk_rat(r);

            if (!pk_char_is_hex(a)) {
                pk_propagate(pk_reader_error(r, pk_str("hex escape invalid")));
            }
            if (!pk_char_is_hex(b)) {
                pk_propagate(pk_reader_error(r, pk_str("hex escape invalid")));
            }
            byte = (pk_u8)((pk_char_to_hex(a) << 4) | pk_char_to_hex(b));
            pk_propagate(pk_writer_char(w, (char)byte));
            break;
        }
        case '|': {
            pk_propagate(pk_writer_char(w, '|'));
            break;
        }
        case '\\': {
            pk_propagate(pk_writer_char(w, '\\'));
            break;
        }
        case '\'': {
            pk_propagate(pk_writer_char(w, '\''));
            break;
        }
        case '\"': {
            pk_propagate(pk_writer_char(w, '\"'));
            break;
        }
        default: {
            return pk_reader_error(r, pk_str("invalid escape code"));
        }
    }
    return pk_true();
}

PKAtom pk_read_string_(PKReader *r, PKWriter *w) {
    if (pk_rat(r) != '\"') {
        return pk_reader_error(r, pk_str("string does not start with \", character"));
    }

    while (pk_reader_inc(r)) {
        if (pk_rat(r) == '\"') {
            
            (void)pk_reader_inc(r);
            return pk_string(pk_writer_slice(w));
            
        } else if (pk_rat(r) == '\\') {
            pk_propagate(pk_reader_ince(r));
            pk_propagate(pk_read_write_escape(r, w));
        } else {
            pk_propagate(pk_writer_char(w, pk_rat(r)));
        }
    }
    
    return pk_reader_error(r, pk_str("string unterminated"));
}
    
PKAtom pk_read_string(PKReader *r) {
    PKWriter w = PK_WRITER_EMPTY;
    
    PKAtom result = pk_read_string_(r, &w);
    pk_writer_deinit(&w);
    
    return result;
}

PKAtom pk_read_number(PKReader *r) {
    bool negetive = false;

    if (pk_rat(r) == '+' || pk_rat(r) == '-') {
        if (pk_rat(r) == '-') {
            negetive = true;
        }
        pk_propagate(pk_reader_ince(r));
    }
    
    if (!pk_char_is_digit(pk_rat(r))) {
        return pk_reader_error(r, pk_str("expected digit character at start of number"));
    }
    
    int acc = pk_char_to_digit(pk_rat(r));
    
    while (pk_reader_inc(r) && pk_char_is_digit(pk_rat(r))) {
        int a = pk_char_to_digit(pk_rat(r));
        acc *= 10;
        acc += a;
    }

    if (pk_rat(r) == '.') {
        char peek = '\0';
        if (!pk_reader_peek(r, &peek) || !pk_char_is_digit(peek)) {
            return pk_reader_error(r, pk_str("expected digit character after decimal point in number"));
        }

        pk_propagate(pk_reader_ince(r));

        double dbl = acc;
        double divisor = 1;

        do {
            dbl = (dbl * 10) + pk_char_to_digit(pk_rat(r));
            divisor *= 10;
        } while (pk_reader_inc(r) && pk_char_is_digit(pk_rat(r)));

        dbl /= divisor;

        if (negetive) {
            dbl = -dbl;
        }

        return pk_float((float)dbl);
    }

    if (negetive) {
        acc = -acc;
    }

    return pk_int(acc);
}

PKAtom pk_read_barred(PKReader *r, PKWriter *w, PK_SYMTY type) {
    pk_propagate(pk_reader_ince(r));
    
    while ((!pk_reader_is_finished(r)) && (pk_rat(r) != '|')) {
        if (pk_rat(r) == '\\') {
            pk_propagate(pk_reader_ince(r));
            pk_propagate(pk_read_write_escape(r, w));
        } else {
            pk_propagate(pk_writer_char(w, pk_rat(r)));
        }
        (void)pk_reader_inc(r);
    }
    
    if (pk_reader_is_finished(r)) {
        if (type == PK_SYMTY_SYMBOL) {
            return pk_reader_error(r, pk_str("unterminated symbol, expected closing |"));
        } else {
            return pk_reader_error(r, pk_str("unterminated keyword, expected closing |"));
        }
    }
    
    PKStr slice = pk_writer_slice(w);
    if (slice.length == 0) {
        if (type == PK_SYMTY_SYMBOL) {
            return pk_reader_error(r, pk_str("empty symbol between | characters"));
        } else {
            return pk_reader_error(r, pk_str("empty keyword between | characters"));
        }
    }
    
    (void)pk_reader_inc(r);
    if (type == PK_SYMTY_SYMBOL) {
        return pk_symbol(slice);
    } else {
        return pk_keyword(slice);
    }
}

PKAtom pk_read_symbol(PKReader *r) {
    size_t save = r->curr;

    if (pk_rat(r) == '|') {
        PKWriter w = PK_WRITER_EMPTY;
        PKAtom result = pk_read_barred(r, &w, PK_SYMTY_SYMBOL);
        pk_writer_deinit(&w);
        return result;
    }
    
    if (!pk_char_is_symbol(pk_rat(r))) {
        return pk_reader_error(r, pk_str("expected valid symbol character at start of symbol"));
    }

    do {
        if (!pk_reader_inc(r)) break;
    } while(pk_char_is_symbol(pk_rat(r)));

    PKStr slice = (PKStr) {
        .c = r->str.c + save,
        .length = r->curr - save,
    };
    
    if (pk_str_eq(slice, pk_str("nil"))) {
        return pk_nil();
    }
    if (pk_str_eq(slice, pk_str("true"))) {
        return pk_true();
    }
    
    return pk_symbol(slice);
}

PKAtom pk_read_cons(PKReader *r) {
    if (pk_rat(r) != '(') {
        return pk_reader_error(r, pk_str("expected list to begin with (, character"));
    }

    pk_propagate(pk_reader_ince(r));

    PKAccum acc = pk_accum_new();
    bool first = true;
    bool separated = false;
    bool whitespace = pk_reader_trim_whitespace(r);

    while (!pk_reader_is_finished(r)) {
        if (pk_rat(r) == ')') {
            (void)pk_reader_inc(r);
            if (first) {
                return pk_nil();
            } else {
                return acc.head;
            }
        } else if (!first && !whitespace && !separated) {
            return pk_reader_error(r, pk_str("list elements must be seperated with whitespace"));
        } else if (pk_rat(r) == '.') {
            if (first) {
                return pk_reader_error(r, pk_str("cannot create a dotted cons without elements before the dot"));
            }
            pk_propagate(pk_reader_ince(r));
            (void)pk_reader_trim_whitespace(r);
            
            PKAtom packed_cons = pk_read_atom(r);
            pk_propagate(packed_cons);
            
            (void)pk_reader_trim_whitespace(r);
            if (pk_rat(r) != ')') {
                return pk_reader_error(r, pk_str("cannot create a dotted cons with multiple elements after the dot"));
            }
            
            (void)pk_reader_inc(r);
            pk_cons_link(acc.tail, packed_cons);
            return acc.head;
        } else {
            PKAtom atom = pk_read_atom(r);
            pk_propagate(atom);
            pk_accum_push(&acc, atom);
            pk_ref_dec(atom);
            first = false;
            char last = r->str.c[r->curr - 1];
            if (last == ')' || last == '"') {
                separated = true;
            } else {
                separated = false;
            }
            whitespace = pk_reader_trim_whitespace(r);
        }
    }

    return pk_reader_error(r, pk_str("unterminated list"));
}

PKAtom pk_read_simple_macro(PKReader *r, PKAtom macro) {
    pk_propagate(pk_reader_ince(r));

    PKAtom contents = pk_read_atom(r);
    pk_propagate(contents);

    PKAtom result = pk_list2(macro, contents);
    pk_ref_dec(contents);
    return result;
}

PKAtom pk_read_unquote_macro(PKReader *r) {
    PKAtom macro = pk_nil();
    
    pk_propagate(pk_reader_ince(r));
    if (pk_rat(r) == '@') {
        macro = PK_CORE.cache.unquote_splice;
        pk_propagate(pk_reader_ince(r));
    } else {
        macro = PK_CORE.cache.unquote;
    }
    
    PKAtom contents = pk_read_atom(r);
    pk_propagate(contents);
    
    PKAtom result = pk_list2(macro, contents);
    pk_ref_dec(contents);
    return result;
}

PKAtom pk_read_atom(PKReader *r) {
    if (pk_reader_is_finished(r)) {
        return pk_reader_error(r, pk_str("reader is already finished in atom reading"));
    }
    
    switch (pk_rat(r)) {
        case '(': {
            return pk_read_cons(r);
        }
        case ')': {
            return pk_reader_error(r, pk_str("encountered end of list ), character without start of list (, character"));
        }
        case '\'': {
            return pk_read_simple_macro(r, PK_CORE.cache.quote);
        }
        case '`': {
            return pk_read_simple_macro(r, PK_CORE.cache.quasiquote);
        }
        case '$': {
            return pk_read_simple_macro(r, PK_CORE.cache.string_substitute);
        }
        case ',': {
            return pk_read_unquote_macro(r);
        }
        case '\"': {
            return pk_read_string(r);
        }
        case ':': {
            return pk_read_keyword(r);
        }
        default: {
            if (pk_reader_is_number_prefix(r)) {
                return pk_read_number(r);
            } else {
                return pk_read_symbol(r);
            }
        }
    }
}

PKAtom pk_read_str(PKStr string, PK_MODE mode) {
    if (string.length == 0) {
        return pk_nil();
    }

    PKReader r = pk_reader_init(string);
    (void)pk_reader_trim_whitespace(&r);
    if (pk_reader_is_finished(&r)) {
        return pk_nil();
    }

    PKAtom first = pk_read_atom(&r);
    pk_propagate(first);
    (void)pk_reader_trim_whitespace(&r);

    if (mode == PK_MODE_EXPRESSION) {
        if (!pk_reader_is_finished(&r)) {
            PKAtom error = pk_errors(pk_str("trailing characters"));
            pk_ref_dec(first);
            return error;
        }
        return first;
    }

    PKAtom cons = pk_cons(first, pk_nil());
    pk_propagate(cons);
    pk_ref_dec(first);
    PKAccum acc = (PKAccum){.head = cons, .tail = cons};

    while (!pk_reader_is_finished(&r)) {
        PKAtom atom = pk_read_atom(&r);
        pk_propagate(atom);
        pk_propagate(pk_accum_push(&acc, atom));
        pk_ref_dec(atom);
        (void)pk_reader_trim_whitespace(&r);
    }

    return acc.head;
}
    
PKAtom pk_read(PKAtom string, PK_MODE mode) {
    if (!pk_is_string(string)) {
        return pk_errors(pk_str("expected string"));
    }
    return pk_read_str(string->string.str, mode);
}
