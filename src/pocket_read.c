#include "pocket_internals.h"

#define PK_COMMENT_CHAR '#'

#define pk_reader_is_finished(r_) ((r_)->curr >= (r_)->str.length)

#define pk_rat(r_) (r_)->c

void pk_reader_error_(Pocket lisp, PKReader *r, PKWriter *w, PKStr error_message) {
    pk_writer_fmt(
        lisp,
        w,
        "reader error:\n  curr: [%zu / %zu]\n  bol = %zu\n  line = %zu\n  char = %c\n  error:",
        r->curr,
        r->str.length,
        r->bol + 1,
        r->line + 1,
        r->c
    );

    pk_writer_string(lisp, w, error_message);
    pk_writer_newline(lisp, w);
    pk_errors(lisp, pk_writer_slice(w));
}

void pk_reader_error(Pocket lisp, PKReader *r, PKStr error_message) {
    PKWriter w = PK_WRITER_EMPTY;
    pk_reader_error_(lisp, r, &w, error_message);
    pk_writer_deinit(lisp, &w);
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

void pk_reader_ince(Pocket lisp, PKReader *r) {
    if (!pk_reader_inc(r)) {
        return pk_reader_error(lisp, r, pk_str("reader is finished, expected more characters in string"));
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

PKAtom *pk_read_atom_keyword(Pocket lisp, PKReader *r) {
    size_t save = 0;
    
    if (pk_rat(r) != ':') {
        pk_reader_error(lisp, r, pk_str("keyword does not start with ':', character"));
        return lisp->cache.nil;
    }
    
    pk_reader_ince(lisp, r);
    
    if (pk_rat(r) == '|') {
        PKWriter w = PK_WRITER_EMPTY;
        PKAtom *result = pk_read_atom_barred(lisp, r, &w, PK_SYMTY_KEYWORD);
        pk_writer_deinit(lisp, &w);
        return result;
    }
    
    save = r->curr;
    
    if (!pk_char_is_symbol(pk_rat(r))) {
        pk_reader_error(lisp, r, pk_str("expected valid symbol character at start of keyword"));
        return lisp->cache.nil;
    }

    do {
        if (!pk_reader_inc(r)) break;
    } while(pk_char_is_symbol(pk_rat(r)));

    PKStr slice = (PKStr) {
        .c = r->str.c + save,
        .length = r->curr - save,
    };
    
    return (PKAtom *)pk_atom_keyword(lisp, slice);
}

void pk_read_atom_write_escape(Pocket lisp, PKReader *r, PKWriter *w) {
    switch (pk_rat(r)) {
        case 'a': {
            pk_writer_char(lisp, w, '\a');
            break;
        }
        case 'b': {
            pk_writer_char(lisp, w, '\b');
            break;
        }
        case 'e': {
            pk_writer_char(lisp, w, '\x1B');
            break;
        }
        case 'f': {
            pk_writer_char(lisp, w, '\f');
            break;
        }
        case 'n': {
            pk_writer_char(lisp, w, '\n');
            break;
        }
        case 'r': {
            pk_writer_char(lisp, w, '\r');
            break;
        }
        case 't': {
            pk_writer_char(lisp, w, '\t');
            break;
        }
        case 'v': {
            pk_writer_char(lisp, w, '\v');
            break;
        }
        case 'x': {
            char a = '\0';
            char b = '\0';
            pk_u8 byte = 0;

            pk_reader_ince(lisp, r);
            a = pk_rat(r);
            pk_reader_ince(lisp, r);
            b = pk_rat(r);

            if (!pk_char_is_hex(a)) {
                pk_reader_error(lisp, r, pk_str("hex escape invalid"));
            }
            if (!pk_char_is_hex(b)) {
                pk_reader_error(lisp, r, pk_str("hex escape invalid"));
            }
            byte = (pk_u8)((pk_char_to_hex(a) << 4) | pk_char_to_hex(b));
            pk_writer_char(lisp, w, (char)byte);
            break;
        }
        case '|': {
            pk_writer_char(lisp, w, '|');
            break;
        }
        case '\\': {
            pk_writer_char(lisp, w, '\\');
            break;
        }
        case '\'': {
            pk_writer_char(lisp, w, '\'');
            break;
        }
        case '\"': {
            pk_writer_char(lisp, w, '\"');
            break;
        }
        default: {
            pk_reader_error(lisp, r, pk_str("invalid escape code"));
            break;
        }
    }
}

PKAtom *pk_read_atom_char(Pocket lisp, PKReader *r) {
    if (pk_rat(r) != '^') {
        pk_reader_error(lisp, r, pk_str("character literal does not start with ^, character"));
        return lisp->cache.nil;
    }

    pk_reader_ince(lisp, r);

    if (pk_rat(r) != '\\') {
        pk_reader_error(lisp, r, pk_str("expected \\ after ^, character"));
        return lisp->cache.nil;
    }

    pk_reader_ince(lisp, r);

    char c = pk_rat(r);

    // TODO: pk_char_is_graphical
    if (pk_char_is_alphabet(c) || c == '-') {
        size_t save = r->curr;
        while (pk_reader_inc(r) && (pk_char_is_alphabet(pk_rat(r)) || pk_char_is_digit(pk_rat(r)) || pk_rat(r) == '-'));

        PKStr name = (PKStr) {
            .c = (char *)r->str.c + save,
            .length = r->curr - save,
        };

        char mapped = '\0';
        if (pk_char_from_cname(name, &mapped)) {
            return (PKAtom *)pk_atom_char(lisp, mapped);
        }

        if (name.length == 1) {
            return (PKAtom *)pk_atom_char(lisp, c);
        }

        pk_reader_error(lisp, r, pk_str("unknown character name"));
        return lisp->cache.nil;
    }

    if (c == '\0') {
        pk_reader_error(lisp, r, pk_str("reader is finished, expected character after \\"));
        return lisp->cache.nil;
    }

    pk_reader_ince(lisp, r);
    return (PKAtom *)pk_atom_char(lisp, c);
}

PKAtom *pk_read_atom_string_(Pocket lisp, PKReader *r, PKWriter *w) {
    if (pk_rat(r) != '\"') {
         pk_reader_error(lisp, r, pk_str("string does not start with \", character"));
         return lisp->cache.nil;
    }

    while (pk_reader_inc(r)) {
        if (pk_rat(r) == '\"') {
            
            (void)pk_reader_inc(r);
            return (PKAtom *)pk_atom_string(lisp, pk_writer_slice(w));
            
        } else if (pk_rat(r) == '\\') {
            pk_reader_ince(lisp, r);
            pk_read_atom_write_escape(lisp, r, w);
        } else {
            pk_writer_char(lisp, w, pk_rat(r));
        }
    }
    
    pk_reader_error(lisp, r, pk_str("string unterminated"));
    return lisp->cache.nil;
}
    
PKAtom *pk_read_atom_string(Pocket lisp, PKReader *r) {
    PKWriter w = PK_WRITER_EMPTY;
    
    PKAtom *result = pk_read_atom_string_(lisp, r, &w);
    pk_writer_deinit(lisp, &w);
    
    return result;
}

pk_u8 pk_read_byte(Pocket lisp, PKReader *r) {
    char c = pk_rat(r);
    if (!pk_char_is_digit(c)) {
        pk_reader_error(lisp, r, pk_str("byte pattern does not start with a digit"));
        return 0;
    }
    
    size_t acc = pk_char_to_digit(c);
    
    while (pk_reader_inc(r)) {
        char c = pk_rat(r);
        if (!pk_char_is_digit(c)) {
            break;
        }
        acc *= 10;
        acc += pk_char_to_digit(c);
        if (acc >= UINT8_MAX) {
            pk_reader_error(lisp, r, pk_str("blob byte member larger than 255"));
            return 0;
        }
    }
    
    return (pk_u8)acc;
}
    
PKAtom *pk_read_atom_blob_(Pocket lisp, PKReader *r, PKBytes *b) {
    if (pk_rat(r) != '[') {
         pk_reader_error(lisp, r, pk_str("blob does not start with [, character"));
         return lisp->cache.nil;
    }
    
    pk_reader_ince(lisp, r);
    
    bool first = false;
    bool whitespace = pk_reader_trim_whitespace(r);
    for (;;) {
        if (pk_rat(r) == ']') {
            return (PKAtom *)pk_atom_blob(lisp, pk_bytes_slice(b));
        }
        if (!whitespace && !first) {
            pk_reader_error(lisp, r, pk_str("expected whitespace seperation between blob bytes"));
            return lisp->cache.nil;
        }
        pk_bytes_push(lisp, b, pk_read_byte(lisp, r), 8);
        pk_reader_ince(lisp, r);
        whitespace = pk_reader_trim_whitespace(r);
        first = false;
    }
}
    
PKAtom *pk_read_atom_blob(Pocket lisp, PKReader *r) {
    PKBytes b = PK_BYTES_EMPTY;
    
    PKAtom *result = pk_read_atom_blob_(lisp, r, &b);
    pk_bytes_deinit(lisp, &b);
    
    return result;
}

PKAtom *pk_read_atom_number(Pocket lisp, PKReader *r) {
    bool negetive = false;

    if (pk_rat(r) == '+' || pk_rat(r) == '-') {
        if (pk_rat(r) == '-') {
            negetive = true;
        }
        pk_reader_ince(lisp, r);
    }
    
    if (!pk_char_is_digit(pk_rat(r))) {
        pk_reader_error(lisp, r, pk_str("expected digit character at start of number"));
        return lisp->cache.nil;
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
            pk_reader_error(lisp, r, pk_str("expected digit character after decimal point in number"));
            return lisp->cache.nil;
        }

        pk_reader_ince(lisp, r);

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

        return (PKAtom *)pk_atom_float(lisp, (float)dbl);
    }

    if (negetive) {
        acc = -acc;
    }

    return (PKAtom *)pk_atom_int(lisp, acc);
}

PKAtom *pk_read_atom_barred(Pocket lisp, PKReader *r, PKWriter *w, PK_SYMTY type) {
    pk_reader_ince(lisp, r);
    
    while ((!pk_reader_is_finished(r)) && (pk_rat(r) != '|')) {
        if (pk_rat(r) == '\\') {
            pk_reader_ince(lisp, r);
            pk_read_atom_write_escape(lisp, r, w);
        } else {
            pk_writer_char(lisp, w, pk_rat(r));
        }
        (void)pk_reader_inc(r);
    }
    
    if (pk_reader_is_finished(r)) {
        if (type == PK_SYMTY_SYMBOL) {
            pk_reader_error(lisp, r, pk_str("unterminated symbol, expected closing |"));
            return lisp->cache.nil;
        } else {
            pk_reader_error(lisp, r, pk_str("unterminated keyword, expected closing |"));
            return lisp->cache.nil;
        }
    }
    
    PKStr slice = pk_writer_slice(w);
    if (slice.length == 0) {
        if (type == PK_SYMTY_SYMBOL) {
            pk_reader_error(lisp, r, pk_str("empty symbol between | characters"));
            return lisp->cache.nil;
        } else {
            pk_reader_error(lisp, r, pk_str("empty keyword between | characters"));
            return lisp->cache.nil;
        }
    }
    
    (void)pk_reader_inc(r);
    if (type == PK_SYMTY_SYMBOL) {
        return (PKAtom *)pk_atom_symbol(lisp, slice);
    } else {
        return (PKAtom *)pk_atom_keyword(lisp, slice);
    }
}

PKAtom *pk_read_atom_symbol(Pocket lisp, PKReader *r) {
    size_t save = r->curr;

    if (pk_rat(r) == '|') {
        PKWriter w = PK_WRITER_EMPTY;
        PKAtom *result = pk_read_atom_barred(lisp, r, &w, PK_SYMTY_SYMBOL);
        pk_writer_deinit(lisp, &w);
        return result;
    }
    
    if (!pk_char_is_symbol(pk_rat(r))) {
        pk_reader_error(lisp, r, pk_str("expected valid symbol character at start of symbol"));
        return lisp->cache.nil;
    }

    do {
        if (!pk_reader_inc(r)) break;
    } while(pk_char_is_symbol(pk_rat(r)));

    PKStr slice = (PKStr) {
        .c = r->str.c + save,
        .length = r->curr - save,
    };
    
    if (pk_str_eq(slice, pk_str("nil"))) {
        return lisp->cache.nil;
    }
    if (pk_str_eq(slice, pk_str("true"))) {
        return lisp->cache.atom_true;
    }
    
    return (PKAtom *)pk_atom_symbol(lisp, slice);
}

PKAtom *pk_read_atom_cons(Pocket lisp, PKReader *r) {
    if (pk_rat(r) != '(') {
        pk_reader_error(lisp, r, pk_str("expected list to begin with (, character"));
        return lisp->cache.nil;
    }

    pk_reader_ince(lisp, r);

    PKAccum acc = pk_accum_new(lisp);
    bool first = true;
    bool separated = false;
    bool whitespace = pk_reader_trim_whitespace(r);

    while (!pk_reader_is_finished(r)) {
        if (pk_rat(r) == ')') {
            (void)pk_reader_inc(r);
            if (first) {
                return lisp->cache.nil;
            } else {
                return acc.head;
            }
        } else if (!first && !whitespace && !separated) {
            pk_reader_error(lisp, r, pk_str("list elements must be seperated with whitespace"));
            return lisp->cache.nil;
        } else if (pk_rat(r) == '.') {
            if (first) {
                pk_reader_error(lisp, r, pk_str("cannot create a dotted cons without elements before the dot"));
                return lisp->cache.nil;
            }
            pk_reader_ince(lisp, r);
            (void)pk_reader_trim_whitespace(r);
            
            PKAtom *packed_cons = pk_read_atom(lisp, r);
            (void)pk_reader_trim_whitespace(r);
            if (pk_rat(r) != ')') {
                pk_reader_error(lisp, r, pk_str("cannot create a dotted cons with multiple elements after the dot"));
                return lisp->cache.nil;
            }
            
            (void)pk_reader_inc(r);
            acc.tail->cons.cdr = packed_cons;
            return acc.head;
        } else {
            PKAtom *atom = pk_read_atom(lisp, r);
            pk_accum_push(&acc, atom);
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

    pk_reader_error(lisp, r, pk_str("unterminated list"));
    return lisp->cache.nil;
}

PKAtom *pk_read_atom_simple_macro(Pocket lisp, PKReader *r, PKAtomSymbol *macro) {
    pk_reader_ince(lisp, r);

    PKAtom *contents = pk_read_atom(lisp, r);
    PKAtom *result = (PKAtom *)pk_atom_list2(lisp, (PKAtom *)macro, contents);
    
    return result;
}

PKAtom *pk_read_atom_unquote_macro(Pocket lisp, PKReader *r) {
    PKAtom *macro = lisp->cache.nil;
    
    pk_reader_ince(lisp, r);
    if (pk_rat(r) == '@') {
        macro = (PKAtom *)lisp->cache.unquote_splice;
        pk_reader_ince(lisp, r);
    } else {
        macro = (PKAtom *)lisp->cache.unquote;
    }
    
    PKAtom *contents = pk_read_atom(lisp, r);
    PKAtom *result = (PKAtom *)pk_atom_list2(lisp, macro, contents);
    
    return result;
}

PKAtom *pk_read_atom(Pocket lisp, PKReader *r) {
    if (pk_reader_is_finished(r)) {
         pk_reader_error(lisp, r, pk_str("reader is already finished in atom reading"));
         return lisp->cache.nil;
    }
    
    switch (pk_rat(r)) {
        case '(': {
            return pk_read_atom_cons(lisp, r);
        }
        case ')': {
            pk_reader_error(lisp, r, pk_str("encountered end of list ), character without start of list (, character"));
            return lisp->cache.nil;
        }
        case '[': {
            return pk_read_atom_blob(lisp, r);
        }
        case ']': {
            pk_reader_error(lisp, r, pk_str("encountered end of blob ], character without start of list [, character"));
            return lisp->cache.nil;
        }
        case '\'': {
            return pk_read_atom_simple_macro(lisp, r, lisp->cache.quote);
        }
        case '`': {
            return pk_read_atom_simple_macro(lisp, r, lisp->cache.quasiquote);
        }
        case '$': {
            return pk_read_atom_simple_macro(lisp, r, lisp->cache.string_substitute);
        }
        case ',': {
            return pk_read_atom_unquote_macro(lisp, r);
        }
        case '\"': {
            return pk_read_atom_string(lisp, r);
        }
        case ':': {
            return pk_read_atom_keyword(lisp, r);
        }
        case '^': {
            return pk_read_atom_char(lisp, r);
        }
        case '&': {
            pk_reader_error(lisp, r, pk_str("cannot read printed lisp object notation"));
            return lisp->cache.nil;
        }
        default: {
            if (pk_reader_is_number_prefix(r)) {
                return pk_read_atom_number(lisp, r);
            } else {
                return pk_read_atom_symbol(lisp, r);
            }
        }
    }
}

PKAtom *pk_read_atom_from_str(Pocket lisp, PKStr string, PK_MODE mode) {
    if (string.length == 0) {
        return lisp->cache.nil;
    }

    PKReader r = pk_reader_init(string);
    (void)pk_reader_trim_whitespace(&r);
    if (pk_reader_is_finished(&r)) {
        return lisp->cache.nil;
    }

    PKAtom *first = pk_read_atom(lisp, &r);
    (void)pk_reader_trim_whitespace(&r);

    if (mode == PK_MODE_EXPRESSION) {
        if (!pk_reader_is_finished(&r)) {
            pk_reader_error(lisp, &r, pk_str("trailing character"));
            return lisp->cache.nil;
        }
        return first;
    }

    PKAtom *cons = (PKAtom *)pk_atom_list1(lisp, first);
    PKAccum acc = pk_accum_new_with(lisp, cons);

    while (!pk_reader_is_finished(&r)) {
        PKAtom *atom = pk_read_atom(lisp, &r);
        pk_accum_push(&acc, atom);
        (void)pk_reader_trim_whitespace(&r);
    }

    return acc.head;
}
    
PKAtom *pk_read_atom_from_string(Pocket lisp, PKAtomString *string, PK_MODE mode) {
    return pk_read_atom_from_str(lisp, string->str, mode);
}
