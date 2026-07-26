#include "pocket_internals.h"

#define pk_reader_is_finished(r_) ((r_)->curr >= (r_)->length)

#define pk_rat(r_) (r_)->c

PK_RES pk_reader_error(PKReader *r, const char *error_message) {
    fprintf(stderr, "READER-ERROR:\n  curr: [%zu / %zu]\n  bol = %zu\n  line = %zu\n  char = %c\n  error: %s\n", r->curr, r->length, r->bol + 1, r->line + 1, r->c, error_message);
    return pk_error(r->lisp);
}

pk_bool pk_reader_inc(PKReader *r) {
    r->curr += 1;
    if (pk_reader_is_finished(r)) {
        r->c = '\0';
        r->curr = r->length;
        return PK_FALSE;
    }

    r->c = r->string[r->curr];
    
    if (pk_rat(r) == '\n') {
        r->line += 1;
        r->bol = 0;
    } else {
        r->bol += 1;
    }
    
    return PK_TRUE;
}

PK_RES pk_reader_ince(PKReader *r) {
    if (!pk_reader_inc(r)) {
        return pk_reader_error(r, "reader is finished, expected more characters in string");
    } else {
        return PK_OK;
    }
}

pk_bool pk_reader_peek(PKReader *r, char *output) {
    size_t peek = r->curr + 1;
    if (peek >= r->length) {
        *output = '\0';
        return PK_FALSE;
    } else {
        *output = r->string[peek];
        return PK_TRUE;
    }
}

pk_bool pk_reader_trim_whitespace(PKReader *r) {
    pk_bool whitespace = PK_FALSE;
    
    for (;;) {
        if (pk_rat(r) == PK_COMMENT_CHAR) {
            while (pk_reader_inc(r) && pk_rat(r) != '\n');
        } else if ((!pk_reader_is_finished(r)) && pk_char_is_whitespace(pk_rat(r))) {
            while (pk_reader_inc(r) && pk_char_is_whitespace(pk_rat(r)));
            whitespace = PK_TRUE;
        } else {
            break;
        }
    }
    
    return whitespace;
}

pk_bool pk_reader_is_number_prefix(PKReader *r) {
    char c = pk_rat(r);
    if (pk_char_is_digit(c)) return PK_TRUE;
    if (c != '+' && c != '-') return PK_FALSE;
    if (!pk_reader_peek(r, &c)) return PK_FALSE;
    if (pk_char_is_digit(c)) return PK_TRUE;
    return PK_FALSE;
}

PKReader pk_reader_init(Pocket lisp, char *string, size_t length) {
    PKReader r;
    r.lisp = lisp;
    r.string = string;
    r.length = length;
    r.curr = 0;
    r.line = 0;
    r.bol = 0;
    r.c = string[0];
    return r;
}

PK_RES pk_read_atom_string(PKReader *r, PKAtomString **string) {
    PKWriter w = pk_writer_init(r->lisp);
    PK_RES result = PK_YIELD;
    
    if (pk_rat(r) != '\"') {
        return pk_reader_error(r, "string does not start with \", character");
    }

    while (pk_reader_inc(r)) {
        if (pk_rat(r) == '\"') {
            (void)pk_reader_inc(r);
            pk_defer(pk_atom_stringn(r->lisp, w.c, w.count, string));
            result = PK_OK;
            goto DEFER;
        } else if (pk_rat(r) == '\\') {
            pk_defer(pk_reader_ince(r));
            switch (pk_rat(r)) {
                case 'a': {
                    pk_defer(pk_writer_char(&w, '\a'));
                    break;
                }
                case 'b': {
                    pk_defer(pk_writer_char(&w, '\b'));
                    break;
                }
                case 'e': {
                    pk_defer(pk_writer_char(&w, '\x1B'));
                    break;
                }
                case 'f': {
                    pk_defer(pk_writer_char(&w, '\f'));
                    break;
                }
                case 'n': {
                    pk_defer(pk_writer_char(&w, '\n'));
                    break;
                }
                case 'r': {
                    pk_defer(pk_writer_char(&w, '\r'));
                    break;
                }
                case 't': {
                    pk_defer(pk_writer_char(&w, '\t'));
                    break;
                }
                case 'v': {
                    pk_defer(pk_writer_char(&w, '\v'));
                    break;
                }
                case 'x': {
                    char a = '\0';
                    char b = '\0';
                    pk_u8 byte = 0;
                    
                    pk_defer(pk_reader_ince(r));
                    a = pk_rat(r);
                    pk_defer(pk_reader_ince(r));
                    b = pk_rat(r);
                    
                    if (!pk_char_is_hex(a)) {
                        pk_defer(pk_reader_error(r, "hex escape invalid"));
                    }
                    if (!pk_char_is_hex(b)) {
                        pk_defer(pk_reader_error(r, "hex escape invalid"));
                    }
                    byte = (pk_u8)((pk_char_to_hex(a) << 4) | pk_char_to_hex(b));
                    pk_defer(pk_writer_char(&w, (char)byte));
                    break;
                }
                case '\\': {
                    pk_defer(pk_writer_char(&w, '\\'));
                    break;
                }
                case '\'': {
                    pk_defer(pk_writer_char(&w, '\''));
                    break;
                }
                case '\"': {
                    pk_defer(pk_writer_char(&w, '\"'));
                    break;
                }
                default: {
                    pk_defer(pk_reader_error(r, "invalid string escape code"));
                    break;
                }
            }
        } else {
            pk_defer(pk_writer_char(&w, pk_rat(r)));
        }
    }
    
    pk_defer(pk_reader_error(r, "string unterminated"));
    
    DEFER:
    pk_writer_deinit(&w);
    return result;
}

PK_RES pk_read_atom_number(PKReader *r, PKAtomNumber **number) {
    int acc = 0;
    pk_bool negetive = PK_FALSE;

    if (pk_rat(r) == '+' || pk_rat(r) == '-') {
        if (pk_rat(r) == '-') {
            negetive = PK_TRUE;
        }
        pk_try(pk_reader_ince(r));
    }
    
    if (!pk_char_is_digit(pk_rat(r))) {
        return pk_reader_error(r, "expected digit character at start of number");
    }
    
    acc = pk_char_to_digit(pk_rat(r));
    
    while (pk_reader_inc(r) && pk_char_is_digit(pk_rat(r))) {
        int a = pk_char_to_digit(pk_rat(r));
        acc *= 10;
        acc += a;
    }
    if (negetive) {
        acc = -acc;
    }
    
    pk_try(pk_atom_int(r->lisp, acc, number));
    
    return PK_OK;
}

PK_RES pk_read_atom_symbol(PKReader *r, PKAtomSymbol **symbol) {
    size_t save = r->curr;

    if (!pk_char_is_symbol(pk_rat(r))) {
        return pk_reader_error(r, "expected valid symbol character at start of symbol");
    }

    do {
        if (!pk_reader_inc(r)) break;
    } while(pk_char_is_symbol(pk_rat(r)));

    pk_try(pk_atom_symboln_interned(r->lisp, r->string + save, r->curr - save, symbol));
    
    return PK_OK;
}

PK_RES pk_read_atom_cons(PKReader *r, PKAtom **output) {
    PKAtomCons *head = NULL;
    PKAtomCons *acc = head;
    pk_bool first = PK_TRUE;
    pk_bool whitespace = PK_FALSE;
    
    if (pk_rat(r) != '(') {
        return pk_reader_error(r, "expected list to begin with (, character");
    }

    pk_try(pk_reader_ince(r));

    whitespace = pk_reader_trim_whitespace(r);

    while (!pk_reader_is_finished(r)) {
        if (pk_rat(r) == ')') {
            (void)pk_reader_inc(r);
            if (first) {
                *output = pk_atom_nil(r->lisp);
            } else {
                *output = (PKAtom *)head;
            }
            return PK_OK;
        } else if (!first && !whitespace) {
            return pk_reader_error(r, "list elements must be seperated with whitespace");
        } else if (pk_rat(r) == '.') {
            PKAtom *cdr = NULL;
            if (first) {
                return pk_reader_error(r, "cannot create a dotted cons without elements before the dot");
            }
            pk_try(pk_reader_ince(r));
            (void)pk_reader_trim_whitespace(r);
            pk_try(pk_read_atom(r, &cdr));
            (void)pk_reader_trim_whitespace(r);
            if (pk_rat(r) != ')') {
                return pk_reader_error(r, "cannot create a dotted cons with multiple elements after the dot");
            }
            (void)pk_reader_inc(r);
            acc->cdr = cdr;
            *output = (PKAtom *)head;
            return PK_OK;
        } else {
            PKAtom *car = NULL;
            PKAtomCons *cons = NULL;
        
            pk_try(pk_read_atom(r, &car));
            pk_try(pk_atom_cons(r->lisp, car, pk_atom_nil(r->lisp), &cons));

            if (first) {
                head = cons;
            } else {
                acc->cdr = (PKAtom *)cons;
            }
            acc = cons;
            first = PK_FALSE;
            whitespace = pk_reader_trim_whitespace(r);
        }
    }

    return pk_reader_error(r, "unterminated list");
}

PK_RES pk_read_atom_simple_macro(PKReader *r, PKAtomSymbol *macro, PKAtomCons **output) {
    PKAtom *a = NULL;
    
    pk_try(pk_reader_ince(r));
    pk_try(pk_read_atom(r, &a));
    pk_try(pk_atom_list2(r->lisp, (PKAtom *)macro, a, output));
    
    return PK_OK;
}

PK_RES pk_read_atom_unquote_macro(PKReader *r, PKAtomCons **output) {
    PKAtomSymbol *macro = NULL;
    PKAtom *a = NULL;
    
    pk_try(pk_reader_ince(r));
    if (pk_rat(r) == '@') {
        macro = r->lisp->cache.unquote_splice;
        pk_try(pk_reader_ince(r));
    } else {
        macro = r->lisp->cache.unquote;
    }
    
    pk_try(pk_read_atom(r, &a));
    pk_try(pk_atom_list2(r->lisp, (PKAtom *)macro, a, output));
    
    return PK_OK;
}

PK_RES pk_read_atom(PKReader *r, PKAtom **atom) {
    if (pk_reader_is_finished(r)) {
        return pk_reader_error(r, "reader is already finished in atom reading");
    }
    
    switch (pk_rat(r)) {
        case '(': {
            pk_try(pk_read_atom_cons(r, atom));
            return PK_OK;
        }
        case ')': {
            return pk_reader_error(r, "encountered end of list ), character without start of list (, character");
        }
        case '\'': {
            pk_try(pk_read_atom_simple_macro(r, r->lisp->cache.quote, (PKAtomCons **)atom));
            return PK_OK;
        }
        case '`': {
            pk_try(pk_read_atom_simple_macro(r, r->lisp->cache.quasiquote, (PKAtomCons **)atom));
            return PK_OK;
        }
        case '$': {
            pk_try(pk_read_atom_simple_macro(r, r->lisp->cache.string_substitute, (PKAtomCons **)atom));
            return PK_OK;
        }
        case ',': {
            pk_try(pk_read_atom_unquote_macro(r, (PKAtomCons **)atom));
            return PK_OK;
        }
        case '\"': {
            pk_try(pk_read_atom_string(r, (PKAtomString **)atom));
            return PK_OK;
        }
        default: {
            if (pk_reader_is_number_prefix(r)) {
                pk_try(pk_read_atom_number(r, (PKAtomNumber **)atom));
            } else {
                pk_try(pk_read_atom_symbol(r, (PKAtomSymbol **)atom));
            }
            return PK_OK;
        }
    }
}

PK_RES pk_read_string(Pocket lisp, char *c, size_t length, PK_READ mode, PKAtom **output) {
    PKReader r;
    PKAtom *first = NULL;
    PKAtomCons *head = NULL;
    PKAtomCons *tail = NULL;

    if (length == 0) {
        *output = pk_atom_nil(lisp);
        return PK_OK;
    }

    r = pk_reader_init(lisp, c, length);

    (void)pk_reader_trim_whitespace(&r);
    if (pk_reader_is_finished(&r)) {
        *output = pk_atom_nil(lisp);
        return PK_OK;
    }

    pk_try(pk_read_atom(&r, &first));

    if (mode == PK_READ_EXPRESSION) {
        (void)pk_reader_trim_whitespace(&r);
        if (!pk_reader_is_finished(&r)) {
            return pk_error(lisp);
        }
        *output = first;
        return PK_OK;
    }

    pk_try(pk_atom_cons_car(lisp, first, &head));
    tail = head;

    (void)pk_reader_trim_whitespace(&r);
    while (!pk_reader_is_finished(&r)) {
        PKAtom *atom = NULL;
        PKAtomCons *cons = NULL;
        
        pk_try(pk_read_atom(&r, &atom));
        pk_try(pk_atom_cons_car(lisp, atom, &cons));
        
        tail->cdr = (PKAtom *)cons;
        tail = cons;
        
        (void)pk_reader_trim_whitespace(&r);
    }

    *output = (PKAtom *)head;
    return PK_OK;
}
