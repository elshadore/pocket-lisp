#include "pocket_internals.h"

bool pk_reader_is_finished(PKReader *r) {
    return r->curr >= r->src.length;
}

bool pk_reader_try_inc(PKReader *r) {
    r->curr += 1;
    if (pk_reader_is_finished(r)) {
        r->curr = r->src.length;
        r->c = '\0';
        return false;
    } else {
        r->c = r->src.c[r->curr];
        // printf("inc => %c\n", r->c); 
        return true;
    }
}

char pk_reader_inc(PKReader *r) {
    if (!pk_reader_try_inc(r)) {
        pk_error(r->lisp);
    }
    return r->c;
}

char pk_reader_peek(PKReader *r) {
    size_t peek = r->curr + 1;
    if (peek >= r->src.length) {
        pk_error(r->lisp);
    }
    return r->src.c[peek];
}

#define PK_COMMENT_CHAR ';'

bool pk_reader_trim_whitespace(PKReader *r) {
    bool whitespace = false;
    for (;;) {
        if (r->c == PK_COMMENT_CHAR) {
            while (pk_reader_try_inc(r) && r->c != '\n');
        } else if (!pk_reader_is_finished(r) && pk_char_is_whitespace(r->c)) {
            while (pk_reader_try_inc(r) && pk_char_is_whitespace(r->c));
            whitespace = true;
        } else {
            break;
        }
    }
    return whitespace;
}

PKAtomString *pk_read_atom_string(PKReader *r) {
    if (r->c != '\"') {
        pk_error(r->lisp);
    }

    PKWriter w = pk_writer_init(r->lisp);

    while (pk_reader_try_inc(r)) {
        if (r->c == '\"') {
            PKString string = pk_string_new(w.c, w.count);
            (void)pk_reader_try_inc(r);
            return pk_atom_string(r->lisp, string);
        } else if (r->c == '\\') {
            if (!pk_reader_try_inc(r)) {
                pk_error(r->lisp);
            }
            switch (r->c) {
                case 'a': {
                    pk_writer_char(&w, '\a');
                    break;
                }
                case 'b': {
                    pk_writer_char(&w, '\b');
                    break;
                }
                case 'e': {
                    pk_writer_char(&w, '\e');
                    break;
                }
                case 'f': {
                    pk_writer_char(&w, '\f');
                    break;
                }
                case 'n': {
                    pk_writer_char(&w, '\n');
                    break;
                }
                case 'r': {
                    pk_writer_char(&w, '\r');
                    break;
                }
                case 't': {
                    pk_writer_char(&w, '\t');
                    break;
                }
                case 'v': {
                    pk_writer_char(&w, '\v');
                    break;
                }
                case 'x': {
                    uint8_t a = pk_reader_inc(r);
                    uint8_t b = pk_reader_inc(r);
                    if (!pk_char_is_hex(a)) {
                        pk_error(r->lisp);
                    }
                    if (!pk_char_is_hex(b)) {
                        pk_error(r->lisp);
                    }
                    uint8_t c = (a << 4) & b;
                    pk_writer_char(&w, (char)c);
                    break;
                }
                case '\\': {
                    pk_writer_char(&w, '\\');
                    break;
                }
                case '\'': {
                    pk_writer_char(&w, '\'');
                    break;
                }
                case '\"': {
                    pk_writer_char(&w, '\"');
                    break;
                }
                default: {
                    pk_error(r->lisp);
                    break;
                }
            }
        } else {
            pk_writer_char(&w, r->c);
        }
    }
    pk_error(r->lisp);
    // unreachable
    return NULL;
}

PKAtomNumber *pk_read_atom_number(PKReader *r) {
    bool negetive = false;
    if (r->c == '+' || r->c == '-') {
        if (r->c == '-') {
            negetive = true;
        }
        (void)pk_reader_inc(r);
    }
    int acc = pk_char_to_digit(r->c);
    while (pk_reader_try_inc(r) && pk_char_is_digit(r->c)) {
        int a = pk_char_to_digit(r->c);
        acc *= 10;
        acc += a;
    }
    if (negetive) {
        acc = -acc;
    }
    return pk_atom_int(r->lisp, acc);
}

PKAtomSymbol *pk_read_atom_symbol(PKReader *r) {
    size_t curr = r->curr;
    if (!pk_char_is_symbol(r->c)) {
        pk_error(r->lisp);
    }
    
    do {
        if (!pk_reader_try_inc(r)) break;
    } while(pk_char_is_symbol(r->c));
    
    return pk_atom_symbol_interned(r->lisp, pk_string_new(r->src.c + curr, r->curr - curr));
}

PKAtom *pk_read_atom_cons(PKReader *r) {
    if (r->c != '(') {
        pk_error(r->lisp);
    }
    
    pk_reader_inc(r);
    
    PKAtomCons *head = NULL;
    PKAtomCons *acc = head;
    
    bool first = true;
    bool whitespace = pk_reader_trim_whitespace(r);
    
    while (!pk_reader_is_finished(r)) {
        if (r->c == ')') {
            (void)pk_reader_try_inc(r);
            if (first) {
                return pk_atom_nil(r->lisp);
            } else {
                return (PKAtom *)head;
            }
        }
        if (!first && !whitespace) {
            pk_error(r->lisp);
        }
        if (r->c == '.') {
            if (first) {
                pk_error(r->lisp);
            }
            if (!pk_reader_try_inc(r)) {
                pk_error(r->lisp);
            }
            (void)pk_reader_trim_whitespace(r);
            PKAtom *cdr = pk_read_atom(r);
            (void)pk_reader_trim_whitespace(r);
            if (!pk_reader_try_inc(r)) {
                pk_error(r->lisp);
            }
            if (r->c != ')') {
                pk_error(r->lisp);
            }
            acc->cdr = cdr;
            return (PKAtom *)head;
        }

        PKAtom *car = pk_read_atom(r);
        PKAtomCons *cons = pk_atom_cons(r->lisp, car, pk_atom_nil(r->lisp));

        if (first) {
            head = cons;
        } else {
            acc->cdr = (PKAtom *)cons;
  
        }
        acc = cons;
        first = false;
        whitespace = pk_reader_trim_whitespace(r);
    }
    
    pk_error(r->lisp);
    // unreachable
    return NULL;
}

bool pk_reader_is_number_prefix(PKReader *r) {
    PKReader p = *r;
    if (pk_char_is_digit(p.c)) return true;
    if (p.c != '+' && p.c != '-') return false;
    if (!pk_reader_try_inc(&p)) return false;
    if (pk_char_is_digit(p.c)) return true;
    return false;
}

PKAtom *pk_read_simple_macro(PKReader *r, PKAtomSymbol *macro) {
    if (!pk_reader_try_inc(r)) {
        pk_error(r->lisp);
    }
    PKAtom *value = pk_read_atom(r);
    PKAtomCons *b = pk_atom_cons(r->lisp, value, pk_atom_nil(r->lisp));
    PKAtomCons *a = pk_atom_cons(r->lisp, (PKAtom *)macro, (PKAtom *)b);
    return (PKAtom *)a;
}

PKAtom *pk_read_unquote_macro(PKReader *r) {
    if (!pk_reader_try_inc(r)) {
        pk_error(r->lisp);
    }
    PKAtomSymbol *macro = NULL;
    if (r->c == '@') {
        macro = r->lisp->cache.unquote_splice;
        (void)pk_reader_try_inc(r);
    } else {
        macro = r->lisp->cache.unquote;
    }
    PKAtom *value = pk_read_atom(r);
    PKAtomCons *b = pk_atom_cons(r->lisp, value, pk_atom_nil(r->lisp));
    PKAtomCons *a = pk_atom_cons(r->lisp, (PKAtom *)macro, (PKAtom *)b);
    return (PKAtom *)a;
}

PKAtom *pk_read_atom(PKReader *r) {
    if (pk_reader_is_finished(r)) {
        pk_error(r->lisp);
    }
    switch (r->c) {
        case '(': {
            return (PKAtom *)pk_read_atom_cons(r);
        }
        case ')': {
            pk_error(r->lisp);
            break;
        }
        case '\'': {
            return pk_read_simple_macro(r, r->lisp->cache.quote);
        }
        case '`': {
            return pk_read_simple_macro(r, r->lisp->cache.quasiquote);
        }
        case '$': {
            return pk_read_simple_macro(r, r->lisp->cache.string_substitute);
        }
        case ',': {
            return pk_read_unquote_macro(r);
        }
        case '\"': {
            return (PKAtom *)pk_read_atom_string(r);
        }
        default: {
            if (pk_reader_is_number_prefix(r)) {
                return (PKAtom *)pk_read_atom_number(r);
            } else {
                return (PKAtom *)pk_read_atom_symbol(r);
            }
        }
    }
    // unreadable
    return NULL;
}


PKAtom *pk_read_from_string(Pocket lisp, PKString string) {
    if (string.length == 0) {
        return pk_atom_nil(lisp);
    }
    
    PKReader r = (PKReader) {
        .lisp = lisp,
        .src = string,
        .curr = 0,
        .c = string.c[0],
    };
    (void)pk_reader_trim_whitespace(&r);
    
    PKAtom *atom = pk_read_atom(&r);
    PKAtomCons *head = pk_atom_cons_car(lisp, atom);
    PKAtomCons *acc = head;
    
    while (!pk_reader_is_finished(&r)) {
        (void)pk_reader_trim_whitespace(&r);
        PKAtom *atom = pk_read_atom(&r);
        PKAtomCons *cons = pk_atom_cons_car(lisp, atom);
        acc->cdr = (PKAtom *)cons;
        acc = cons;
    }
    
    return (PKAtom *)head;
}
