#include "pocket_internals.h"

#define PK_COMMENT_CHAR '#'

#define pk_parser_is_finished(r_) ((r_)->curr >= (r_)->str.length)

#define pk_pat(r_) (r_)->c

void pk_parser_error(Pocket lisp, PKParser *r, PKStr error_message) {
    pk_writer_reset(lisp);
    pk_writer_fmt(
        lisp,
        "parser error:\n  curr: [%zu / %zu]\n  bol = %zu\n  line = %zu\n  char = %c\n  error:",
        r->curr,
        r->str.length,
        r->bol + 1,
        r->line + 1,
        r->c
    );

    pk_writer_string(lisp, error_message);
    pk_writer_newline(lisp);

    pk_error_writer(lisp);
}

bool pk_parser_inc(PKParser *r) {
    r->curr += 1;
    if (pk_parser_is_finished(r)) {
        r->c = '\0';
        r->curr = r->str.length;
        return false;
    }

    r->c = r->str.c[r->curr];
    
    if (pk_pat(r) == '\n') {
        r->line += 1;
        r->bol = 0;
    } else {
        r->bol += 1;
    }
    
    return true;
}

void pk_parser_ince(Pocket lisp, PKParser *r) {
    if (!pk_parser_inc(r)) {
        return pk_parser_error(lisp, r, pk_str("parser is finished, expected more characters in string"));
    }
}

bool pk_parser_peek(PKParser *r, char *output) {
    size_t peek = r->curr + 1;
    if (peek >= r->str.length) {
        *output = '\0';
        return false;
    } else {
        *output = r->str.c[peek];
        return true;
    }
}

bool pk_parser_trim_whitespace(PKParser *r) {
    bool whitespace = false;
    
    for (;;) {
        if (pk_pat(r) == PK_COMMENT_CHAR) {
            while (pk_parser_inc(r) && pk_pat(r) != '\n');
        } else if ((!pk_parser_is_finished(r)) && pk_char_is_whitespace(pk_pat(r))) {
            while (pk_parser_inc(r) && pk_char_is_whitespace(pk_pat(r)));
            whitespace = true;
        } else {
            break;
        }
    }
    
    return whitespace;
}

bool pk_parser_is_number_prefix(PKParser *r) {
    char c = pk_pat(r);
    if (pk_char_is_digit(c)) return true;
    if (c != '+' && c != '-') return false;
    if (!pk_parser_peek(r, &c)) return false;
    if (pk_char_is_digit(c)) return true;
    return false;
}

PKParser pk_parser_init(PKStr string) {
    return (PKParser) {
        .str = string,
        .curr = 0,
        .line = 0,
        .bol = 0,
        .c = string.c[0],
    };
}

PKAtom *pk_parse_atom_keyword(Pocket lisp, PKParser *r) {
    size_t save = 0;
    
    if (pk_pat(r) != ':') {
        pk_parser_error(lisp, r, pk_str("keyword does not start with ':', character"));
        return &PK_ATOM_NIL;
    }
    
    pk_parser_ince(lisp, r);
    
    if (pk_pat(r) == '|') {
        return pk_parse_atom_barred(lisp, r, PK_SYMTY_KEYWORD);
    }
    
    save = r->curr;
    
    if (!pk_char_is_symbol(pk_pat(r))) {
        pk_parser_error(lisp, r, pk_str("expected valid symbol character at start of keyword"));
        return &PK_ATOM_NIL;
    }

    do {
        if (!pk_parser_inc(r)) break;
    } while(pk_char_is_symbol(pk_pat(r)));

    PKStr slice = (PKStr) {
        .c = r->str.c + save,
        .length = r->curr - save,
    };
    
    return (PKAtom *)pk_atom_keyword(lisp, slice);
}

void pk_parse_atom_write_escape(Pocket lisp, PKParser *r) {
    switch (pk_pat(r)) {
        case 'a': {
            pk_writer_char(lisp, '\a');
            break;
        }
        case 'b': {
            pk_writer_char(lisp, '\b');
            break;
        }
        case 'e': {
            pk_writer_char(lisp, '\x1B');
            break;
        }
        case 'f': {
            pk_writer_char(lisp, '\f');
            break;
        }
        case 'n': {
            pk_writer_char(lisp, '\n');
            break;
        }
        case 'r': {
            pk_writer_char(lisp, '\r');
            break;
        }
        case 't': {
            pk_writer_char(lisp, '\t');
            break;
        }
        case 'v': {
            pk_writer_char(lisp, '\v');
            break;
        }
        case 'x': {
            char a = '\0';
            char b = '\0';
            pk_u8 byte = 0;

            pk_parser_ince(lisp, r);
            a = pk_pat(r);
            pk_parser_ince(lisp, r);
            b = pk_pat(r);

            if (!pk_char_is_hex(a)) {
                pk_parser_error(lisp, r, pk_str("hex escape invalid"));
            }
            if (!pk_char_is_hex(b)) {
                pk_parser_error(lisp, r, pk_str("hex escape invalid"));
            }
            byte = (pk_u8)((pk_char_to_hex(a) << 4) | pk_char_to_hex(b));
            pk_writer_char(lisp, (char)byte);
            break;
        }
        case '|': {
            pk_writer_char(lisp, '|');
            break;
        }
        case '\\': {
            pk_writer_char(lisp, '\\');
            break;
        }
        case '\'': {
            pk_writer_char(lisp, '\'');
            break;
        }
        case '\"': {
            pk_writer_char(lisp, '\"');
            break;
        }
        default: {
            pk_parser_error(lisp, r, pk_str("invalid escape code"));
            break;
        }
    }
}

PKAtom *pk_parse_atom_string(Pocket lisp, PKParser *r) {
    pk_writer_reset(lisp);
    
    if (pk_pat(r) != '\"') {
         pk_parser_error(lisp, r, pk_str("string does not start with \", character"));
         return &PK_ATOM_NIL;
    }

    while (pk_parser_inc(r)) {
        if (pk_pat(r) == '\"') {
            
            (void)pk_parser_inc(r);
            PKAtom *result = (PKAtom *)pk_atom_string(lisp, pk_writer_str(lisp));
            pk_writer_reset(lisp);
            return result;
            
        } else if (pk_pat(r) == '\\') {
            pk_parser_ince(lisp, r);
            pk_parse_atom_write_escape(lisp, r);
        } else {
            pk_writer_char(lisp, pk_pat(r));
        }
    }
    
    pk_parser_error(lisp, r, pk_str("string unterminated"));
    
    return (PKAtom *)&PK_ATOM_EMPTY_STRING;
}

PKAtom *pk_parse_atom_number(Pocket lisp, PKParser *r) {
    bool negetive = false;

    if (pk_pat(r) == '+' || pk_pat(r) == '-') {
        if (pk_pat(r) == '-') {
            negetive = true;
        }
        pk_parser_ince(lisp, r);
    }
    
    if (!pk_char_is_digit(pk_pat(r))) {
        pk_parser_error(lisp, r, pk_str("expected digit character at start of number"));
        return &PK_ATOM_NIL;
    }
    
    int acc = pk_char_to_digit(pk_pat(r));
    
    while (pk_parser_inc(r) && pk_char_is_digit(pk_pat(r))) {
        int a = pk_char_to_digit(pk_pat(r));
        acc *= 10;
        acc += a;
    }

    if (pk_pat(r) == '.') {
        char peek = '\0';
        if (!pk_parser_peek(r, &peek) || !pk_char_is_digit(peek)) {
            pk_parser_error(lisp, r, pk_str("expected digit character after decimal point in number"));
            return &PK_ATOM_NIL;
        }

        pk_parser_ince(lisp, r);

        double dbl = acc;
        double divisor = 1;

        do {
            dbl = (dbl * 10) + pk_char_to_digit(pk_pat(r));
            divisor *= 10;
        } while (pk_parser_inc(r) && pk_char_is_digit(pk_pat(r)));

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

PKAtom *pk_parse_atom_barred(Pocket lisp, PKParser *r, PK_SYMTY type) {
    pk_parser_ince(lisp, r);

    pk_writer_reset(lisp);
    
    while ((!pk_parser_is_finished(r)) && (pk_pat(r) != '|')) {
        if (pk_pat(r) == '\\') {
            pk_parser_ince(lisp, r);
            pk_parse_atom_write_escape(lisp, r);
        } else {
            pk_writer_char(lisp, pk_pat(r));
        }
        (void)pk_parser_inc(r);
    }
    
    if (pk_parser_is_finished(r)) {
        if (type == PK_SYMTY_SYMBOL) {
            pk_parser_error(lisp, r, pk_str("unterminated symbol, expected closing |"));
            return &PK_ATOM_NIL;
        } else {
            pk_parser_error(lisp, r, pk_str("unterminated keyword, expected closing |"));
            return &PK_ATOM_NIL;
        }
    }
    
    PKStr slice = pk_writer_str(lisp);
    if (slice.length == 0) {
        if (type == PK_SYMTY_SYMBOL) {
            pk_parser_error(lisp, r, pk_str("empty symbol between | characters"));
            return &PK_ATOM_NIL;
        } else {
            pk_parser_error(lisp, r, pk_str("empty keyword between | characters"));
            return &PK_ATOM_NIL;
        }
    }
    
    (void)pk_parser_inc(r);
    PKAtom *result = &PK_ATOM_NIL;
    if (type == PK_SYMTY_SYMBOL) {
        result = (PKAtom *)pk_atom_symbol(lisp, slice);
    } else {
        result = (PKAtom *)pk_atom_keyword(lisp, slice);
    }
    pk_writer_reset(lisp);
    return result;
}

PKAtom *pk_parse_atom_symbol(Pocket lisp, PKParser *r) {
    size_t save = r->curr;

    if (pk_pat(r) == '|') {
        return pk_parse_atom_barred(lisp, r, PK_SYMTY_SYMBOL);
    }
    
    if (!pk_char_is_symbol(pk_pat(r))) {
        pk_parser_error(lisp, r, pk_str("expected valid symbol character at start of symbol"));
        return &PK_ATOM_NIL;
    }

    do {
        if (!pk_parser_inc(r)) break;
    } while(pk_char_is_symbol(pk_pat(r)));

    PKStr slice = (PKStr) {
        .c = r->str.c + save,
        .length = r->curr - save,
    };
    
    if (pk_str_eq(slice, pk_str("nil"))) {
        return &PK_ATOM_NIL;
    }
    if (pk_str_eq(slice, pk_str("true"))) {
        return &PK_ATOM_TRUE;
    }
    
    return (PKAtom *)pk_atom_symbol(lisp, slice);
}

PKAtom *pk_parse_atom_cons(Pocket lisp, PKParser *r) {
    if (pk_pat(r) != '(') {
        pk_parser_error(lisp, r, pk_str("expected list to begin with (, character"));
        return &PK_ATOM_NIL;
    }

    pk_parser_ince(lisp, r);

    PKAccum acc = pk_accum_new(lisp);
    
    bool first = true;
    bool separated = false;
    bool whitespace = pk_parser_trim_whitespace(r);

    while (!pk_parser_is_finished(r)) {
        if (pk_pat(r) == ')') {
            (void)pk_parser_inc(r);
            if (first) {
                return &PK_ATOM_NIL;
            } else {
                return acc.head;
            }
        } else if (!first && !whitespace && !separated) {
            pk_parser_error(lisp, r, pk_str("list elements must be seperated with whitespace"));
        } else if (pk_pat(r) == '.') {
            if (first) {
                pk_parser_error(lisp, r, pk_str("cannot create a dotted cons without elements before the dot"));
            }
            pk_parser_ince(lisp, r);
            (void)pk_parser_trim_whitespace(r);

            PKAtom *packed_cons = pk_parse_atom(lisp, r);
            acc.tail->cons.cdr = packed_cons;

            (void)pk_parser_trim_whitespace(r);
            if (pk_pat(r) != ')') {
                pk_parser_error(lisp, r, pk_str("cannot create a dotted cons with multiple elements after the dot"));
                return &PK_ATOM_NIL;
            }

            (void)pk_parser_inc(r);
            return &PK_ATOM_NIL;
        } else {
            PKAtom *atom = pk_parse_atom(lisp, r);
            pk_accum_push(&acc, atom);
            first = false;
            char last = r->str.c[r->curr - 1];
            if (last == ')' || last == '"') {
                separated = true;
            } else {
                separated = false;
            }
            whitespace = pk_parser_trim_whitespace(r);
        }
    }

    pk_parser_error(lisp, r, pk_str("unterminated list"));
    return &PK_ATOM_NIL;
}

PKAtom *pk_parse_atom_blob(Pocket lisp, PKParser *r) {
    if (pk_pat(r) != '[') {
        pk_parser_error(lisp, r, pk_str("blob does not start with [, character"));
        return &PK_ATOM_NIL;
    }

    pk_parser_ince(lisp, r);
    pk_writer_reset(lisp);

    bool first = true;
    bool whitespace = pk_parser_trim_whitespace(r);

    while (!pk_parser_is_finished(r)) {
        if (pk_pat(r) == ']') {
            (void)pk_parser_inc(r);
            PKAtomBlob *result = pk_atom_blob(lisp, pk_writer_slice(lisp));
            pk_writer_reset(lisp);
            return (PKAtom *)result;
        } else if (!first && !whitespace) {
            pk_parser_error(lisp, r, pk_str("blob elements must be seperated with whitespace"));
        } else if (!pk_parser_is_number_prefix(r)) {
            pk_parser_error(lisp, r, pk_str("expected a number in blob"));
        } else {
            PKAtomNumber *number = pk_atom_assert_number(lisp, pk_parse_atom_number(lisp, r));
            if (number->num_type != PK_NUMTY_INT) {
                pk_parser_error(lisp, r, pk_str("blob elements must be integers"));
            }
            if (number->as.i < 0 || number->as.i > UINT8_MAX) {
                pk_parser_error(lisp, r, pk_str("blob element is outside the byte range 0..255"));
            }
            pk_writer_byte(lisp, (pk_u8)number->as.i);
            first = false;
            whitespace = pk_parser_trim_whitespace(r);
        }
    }

    pk_parser_error(lisp, r, pk_str("unterminated blob"));
    return &PK_ATOM_NIL;
}

PKAtom *pk_parse_atom_simple_macro(Pocket lisp, PKParser *r, PKAtomSymbol *macro) {
    pk_parser_ince(lisp, r);

    PKAtom *contents = pk_parse_atom(lisp, r);
    PKAtom *result = (PKAtom *)pk_atom_list2(lisp, (PKAtom *)macro, contents);
    
    return result;
}

PKAtom *pk_parse_atom_unquote_macro(Pocket lisp, PKParser *r) {
    PKAtom *macro = &PK_ATOM_NIL;
    
    pk_parser_ince(lisp, r);
    if (pk_pat(r) == '@') {
        macro = (PKAtom *)lisp->cache.unquote_splice;
        pk_parser_ince(lisp, r);
    } else {
        macro = (PKAtom *)lisp->cache.unquote;
    }
    
    PKAtom *contents = pk_parse_atom(lisp, r);
    PKAtom *result = (PKAtom *)pk_atom_list2(lisp, macro, contents);
    
    return result;
}

PKAtom *pk_parse_atom(Pocket lisp, PKParser *r) {
    if (pk_parser_is_finished(r)) {
         pk_parser_error(lisp, r, pk_str("parser is already finished in atom reading"));
         return &PK_ATOM_NIL;
    }
    
    switch (pk_pat(r)) {
        case '(': {
            return pk_parse_atom_cons(lisp, r);
        }
        case ')': {
            pk_parser_error(lisp, r, pk_str("encountered end of list ), character without start of list (, character"));
            return &PK_ATOM_NIL;
        }
        case '[': {
            return pk_parse_atom_blob(lisp, r);
        }
        case ']': {
            pk_parser_error(lisp, r, pk_str("encountered end of blob ], character without start of blob [, character"));
            return &PK_ATOM_NIL;
        }
        case '\'': {
            return pk_parse_atom_simple_macro(lisp, r, lisp->cache.quote);
        }
        case '`': {
            return pk_parse_atom_simple_macro(lisp, r, lisp->cache.quasiquote);
        }
        case '$': {
            return pk_parse_atom_simple_macro(lisp, r, lisp->cache.string_substitute);
        }
        case ',': {
            return pk_parse_atom_unquote_macro(lisp, r);
        }
        case '\"': {
            return pk_parse_atom_string(lisp, r);
        }
        case ':': {
            return pk_parse_atom_keyword(lisp, r);
        }
        case '&': {
            pk_parser_error(lisp, r, pk_str("cannot parse printed lisp object notation"));
            return &PK_ATOM_NIL;
        }
        default: {
            if (pk_parser_is_number_prefix(r)) {
                return pk_parse_atom_number(lisp, r);
            } else {
                return pk_parse_atom_symbol(lisp, r);
            }
        }
    }
}

PKAtom *pk_parse_atom_from_str(Pocket lisp, PKStr string, PK_MODE mode) {
    if (string.length == 0) {
        return &PK_ATOM_NIL;
    }

    PKParser r = pk_parser_init(string);
    (void)pk_parser_trim_whitespace(&r);
    if (pk_parser_is_finished(&r)) {
        return &PK_ATOM_NIL;
    }

    PKAtom *first = pk_parse_atom(lisp, &r);
    (void)pk_parser_trim_whitespace(&r);

    if (mode == PK_MODE_EXPRESSION) {
        if (!pk_parser_is_finished(&r)) {
            pk_parser_error(lisp, &r, pk_str("trailing character"));
            return &PK_ATOM_NIL;
        }
        return first;
    }

    pk_stack_push(lisp, first);
    size_t count = 1;

    while (!pk_parser_is_finished(&r)) {
        PKAtom *atom = pk_parse_atom(lisp, &r);
        pk_stack_push(lisp, atom);
        count += 1;
        (void)pk_parser_trim_whitespace(&r);
    }

    pk_stack_op_list(lisp, count);

    return pk_stack_head_pop(lisp);
}
    
PKAtom *pk_parse_atom_from_string(Pocket lisp, PKAtomString *string, PK_MODE mode) {
    return pk_parse_atom_from_str(lisp, string->str, mode);
}
