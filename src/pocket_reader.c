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
        return true;
    }
}

PKRes pk_reader_inc(PKReader *r) {
    if (!pk_reader_try_inc(r)) {
        return pk_error(r->lisp);
    }
    return PK_Ok;
}

PKRes pk_reader_peek(PKReader *r, char *output) {
    size_t peek = r->curr + 1;
    if (peek >= r->src.length) {
        return pk_error(r->lisp);
    }
    *output = r->src.c[peek];
    return PK_Ok;
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

PKRes pk_read_atom_string(PKReader *r, PKAtomString **output) {
    if (r->c != '\"') {
        return pk_error(r->lisp);
    }

    PKWriter w = pk_writer_init(r->lisp);
    PKRes result = PK_Yield;
    
    while (pk_reader_try_inc(r)) {
        if (r->c == '\"') {
            PKString string = pk_string_new(w.c, w.count);
            (void)pk_reader_try_inc(r);
            PKAtomString *atom = NULL;
            pk_defer(pk_atom_string(r->lisp, string, &atom));
            *output = atom;
            result = PK_Ok;
            goto DEFER;
        } else if (r->c == '\\') {
            pk_defer(pk_reader_inc(r));
            switch (r->c) {
                case 'a': { pk_defer(pk_writer_char(&w, '\a')); break; }
                case 'b': { pk_defer(pk_writer_char(&w, '\b')); break; }
                case 'e': { pk_defer(pk_writer_char(&w, '\e')); break; }
                case 'f': { pk_defer(pk_writer_char(&w, '\f')); break; }
                case 'n': { pk_defer(pk_writer_char(&w, '\n')); break; }
                case 'r': { pk_defer(pk_writer_char(&w, '\r')); break; }
                case 't': { pk_defer(pk_writer_char(&w, '\t')); break; }
                case 'v': { pk_defer(pk_writer_char(&w, '\v')); break; }
                case 'x': {
                    pk_defer(pk_reader_inc(r));
                    char a = r->c;
                    pk_defer(pk_reader_inc(r));
                    char b = r->c;
                    if (!pk_char_is_hex(a)) return pk_error(r->lisp);
                    if (!pk_char_is_hex(b)) return pk_error(r->lisp);
                    uint8_t byte = (uint8_t)((pk_char_to_hex(a) << 4) | pk_char_to_hex(b));
                    pk_defer(pk_writer_char(&w, (char)byte));
                    break;
                }
                case '\\': { pk_defer(pk_writer_char(&w, '\\')); break; }
                case '\'': { pk_defer(pk_writer_char(&w, '\'')); break; }
                case '\"': { pk_defer(pk_writer_char(&w, '\"')); break; }
                default: {
                    pk_error(r->lisp);
                    goto DEFER;
                    break;
                }
            }
        } else {
            pk_try(pk_writer_char(&w, r->c));
        }
    }
    pk_error(r->lisp);
    DEFER:
    pk_writer_deinit(&w);
    return result;
}

PKRes pk_read_atom_number(PKReader *r, PKAtomNumber **output) {
    bool negetive = false;
    if (r->c == '+' || r->c == '-') {
        if (r->c == '-') {
            negetive = true;
        }
        pk_try(pk_reader_inc(r));
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
    pk_try(pk_atom_int(r->lisp, acc, output));
    return PK_Ok;
}

PKRes pk_read_atom_symbol(PKReader *r, PKAtomSymbol **output) {
    size_t curr = r->curr;
    if (!pk_char_is_symbol(r->c)) {
        return pk_error(r->lisp);
    }

    do {
        if (!pk_reader_try_inc(r)) break;
    } while(pk_char_is_symbol(r->c));

    pk_try(pk_atom_symbol_interned(r->lisp, pk_string_new(r->src.c + curr, r->curr - curr), output));
    return PK_Ok;
}

PKRes pk_read_atom_cons(PKReader *r, PKAtom **output) {
    if (r->c != '(') {
        return pk_error(r->lisp);
    }

    pk_try(pk_reader_inc(r));

    PKAtomCons *head = NULL;
    PKAtomCons *acc = head;

    bool first = true;
    bool whitespace = pk_reader_trim_whitespace(r);

    while (!pk_reader_is_finished(r)) {
        if (r->c == ')') {
            (void)pk_reader_try_inc(r);
            if (first) {
                *output = pk_atom_nil(r->lisp);
            } else {
                *output = (PKAtom *)head;
            }
            return PK_Ok;
        }
        if (!first && !whitespace) {
            return pk_error(r->lisp);
        }
        if (r->c == '.') {
            if (first) {
                return pk_error(r->lisp);
            }
            pk_try(pk_reader_inc(r));
            (void)pk_reader_trim_whitespace(r);
            PKAtom *cdr;
            pk_try(pk_read_atom(r, &cdr));
            (void)pk_reader_trim_whitespace(r);
            pk_try(pk_reader_inc(r));
            if (r->c != ')') {
                return pk_error(r->lisp);
            }
            acc->cdr = cdr;
            *output = (PKAtom *)head;
            return PK_Ok;
        }

        PKAtom *car;
        pk_try(pk_read_atom(r, &car));
        PKAtomCons *cons;
        pk_try(pk_atom_cons(r->lisp, car, pk_atom_nil(r->lisp), &cons));

        if (first) {
            head = cons;
        } else {
            acc->cdr = (PKAtom *)cons;
        }
        acc = cons;
        first = false;
        whitespace = pk_reader_trim_whitespace(r);
    }

    return pk_error(r->lisp);
}

bool pk_reader_is_number_prefix(PKReader *r) {
    PKReader p = *r;
    if (pk_char_is_digit(p.c)) return true;
    if (p.c != '+' && p.c != '-') return false;
    if (!pk_reader_try_inc(&p)) return false;
    if (pk_char_is_digit(p.c)) return true;
    return false;
}

PKRes pk_read_simple_macro(PKReader *r, PKAtomSymbol *macro, PKAtom **output) {
    pk_try(pk_reader_inc(r));
    PKAtom *value;
    pk_try(pk_read_atom(r, &value));
    PKAtomCons *b;
    pk_try(pk_atom_cons(r->lisp, value, pk_atom_nil(r->lisp), &b));
    PKAtomCons *a;
    pk_try(pk_atom_cons(r->lisp, (PKAtom *)macro, (PKAtom *)b, &a));
    *output = (PKAtom *)a;
    return PK_Ok;
}

PKRes pk_read_unquote_macro(PKReader *r, PKAtom **output) {
    pk_try(pk_reader_inc(r));
    PKAtomSymbol *macro = NULL;
    if (r->c == '@') {
        macro = r->lisp->cache.unquote_splice;
        pk_try(pk_reader_inc(r));
    } else {
        macro = r->lisp->cache.unquote;
    }
    PKAtom *value;
    pk_try(pk_read_atom(r, &value));
    PKAtomCons *b;
    pk_try(pk_atom_cons(r->lisp, value, pk_atom_nil(r->lisp), &b));
    PKAtomCons *a;
    pk_try(pk_atom_cons(r->lisp, (PKAtom *)macro, (PKAtom *)b, &a));
    *output = (PKAtom *)a;
    return PK_Ok;
}

PKRes pk_read_atom(PKReader *r, PKAtom **output) {
    if (pk_reader_is_finished(r)) {
        return pk_error(r->lisp);
    }
    switch (r->c) {
        case '(': {
            pk_try(pk_read_atom_cons(r, output));
            return PK_Ok;
        }
        case ')': {
            return pk_error(r->lisp);
        }
        case '\'': {
            pk_try(pk_read_simple_macro(r, r->lisp->cache.quote, output));
            return PK_Ok;
        }
        case '`': {
            pk_try(pk_read_simple_macro(r, r->lisp->cache.quasiquote, output));
            return PK_Ok;
        }
        case '$': {
            pk_try(pk_read_simple_macro(r, r->lisp->cache.string_substitute, output));
            return PK_Ok;
        }
        case ',': {
            pk_try(pk_read_unquote_macro(r, output));
            return PK_Ok;
        }
        case '\"': {
            PKAtomString *s;
            pk_try(pk_read_atom_string(r, &s));
            *output = (PKAtom *)s;
            return PK_Ok;
        }
        default: {
            if (pk_reader_is_number_prefix(r)) {
                PKAtomNumber *n;
                pk_try(pk_read_atom_number(r, &n));
                *output = (PKAtom *)n;
            } else {
                PKAtomSymbol *s;
                pk_try(pk_read_atom_symbol(r, &s));
                *output = (PKAtom *)s;
            }
            return PK_Ok;
        }
    }
}

PKRes pk_read_from_string(Pocket lisp, PKString string, PKAtom **output) {
    if (string.length == 0) {
        *output = pk_atom_nil(lisp);
        return PK_Ok;
    }

    PKReader r = (PKReader) {
        .lisp = lisp,
        .src = string,
        .curr = 0,
        .c = string.c[0],
    };
    (void)pk_reader_trim_whitespace(&r);

    PKAtom *atom;
    pk_try(pk_read_atom(&r, &atom));
    PKAtomCons *head;
    pk_try(pk_atom_cons_car(lisp, atom, &head));
    PKAtomCons *acc = head;

    while (!pk_reader_is_finished(&r)) {
        (void)pk_reader_trim_whitespace(&r);
        PKAtom *next_atom;
        pk_try(pk_read_atom(&r, &next_atom));
        PKAtomCons *cons;
        pk_try(pk_atom_cons_car(lisp, next_atom, &cons));
        acc->cdr = (PKAtom *)cons;
        acc = cons;
    }

    *output = (PKAtom *)head;
    return PK_Ok;
}
