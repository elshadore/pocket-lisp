#include "pocket_internals.h"

#define pk_reader_is_finished(r_) ((r_)->curr >= (r_)->length)

pk_bool pk_reader_at(PKReader *r, char *output) {
    if (pk_reader_is_finished(r)) {
        r->curr = r->length;
        *output = '\0';
        return PK_FALSE;
    } else {
        *output = r->c[r->curr];
        return PK_TRUE;
    }
}

PKRes pk_reader_ate(PKReader *r, char *output) {
    if (!pk_reader_at(r, output)) {
        return pk_error(r->lisp);
    } else {
        return PK_Ok;
    }
}

pk_bool pk_reader_inc(PKReader *r, char *output) {
    r->curr += 1;
    return pk_reader_at(r, output);
}

PKRes pk_reader_ince(PKReader *r, char *output) {
    if (!pk_reader_inc(r, output)) {
        return pk_error(r->lisp);
    } else {
        return PK_Ok;
    }
}

pk_bool pk_reader_peek(PKReader *r, char *output) {
    size_t peek = r->curr + 1;
    if (peek >= r->length) {
        *output = '\0';
        return PK_FALSE;
    } else {
        *output = r->c[peek];
        return PK_TRUE;
    }
}

pk_bool pk_reader_trim_whitespace(PKReader *r) {
    pk_bool whitespace = PK_FALSE;
    char c = '\0';
    if (!pk_reader_at(r, &c)) {
        return PK_FALSE;
    }
    for (;;) {
        if (c == PK_COMMENT_CHAR) {
            while (pk_reader_inc(r, &c) && c != '\n');
        } else if ((!pk_reader_is_finished(r)) && pk_char_is_whitespace(c)) {
            while (pk_reader_inc(r, &c) && pk_char_is_whitespace(c));
            whitespace = PK_TRUE;
        } else {
            break;
        }
    }
    return whitespace;
}

pk_bool pk_reader_is_number_prefix(PKReader *r) {
    char c = '\0';
    if (pk_char_is_digit(c)) return PK_TRUE;
    if (c != '+' && c != '-') return PK_FALSE;
    if (!pk_reader_peek(r, &c)) return PK_FALSE;
    if (pk_char_is_digit(c)) return PK_TRUE;
    return PK_FALSE;
}

PKRes pk_read_atom_string(PKReader *r, PKAtomString **string) {
    PKWriter w = pk_writer_init(r->lisp);
    PKRes result = PK_Yield;
    char c = '\0';
    
    pk_try(pk_reader_ate(r, &c));
    
    if (c != '\"') {
        return pk_error(r->lisp);
    }

    while (pk_reader_inc(r, &c)) {
        if (c == '\"') {
            (void)pk_reader_inc(r, &c);
            pk_defer(pk_atom_string(r->lisp, w.c, w.count, string));
            result = PK_Ok;
            goto DEFER;
        } else if (c == '\\') {
            pk_defer(pk_reader_ince(r, &c));
            switch (c) {
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
                    
                    pk_defer(pk_reader_ince(r, &a));
                    pk_defer(pk_reader_ince(r, &b));
                    
                    if (!pk_char_is_hex(a)) {
                        pk_defer(pk_error(r->lisp));
                    }
                    if (!pk_char_is_hex(b)) {
                        pk_defer(pk_error(r->lisp));
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
                    pk_defer(pk_error(r->lisp));
                    break;
                }
            }
        } else {
            pk_defer(pk_writer_char(&w, c));
        }
    }
    
    pk_defer(pk_error(r->lisp));
    
    DEFER:
    pk_writer_deinit(&w);
    return result;
}

PKRes pk_read_atom_number(PKReader *r, PKAtomNumber **number) {
    char c = '\0';
    int acc = 0;
    pk_bool negetive = PK_FALSE;

    pk_try(pk_reader_ate(r, &c));
    
    if (c == '+' || c == '-') {
        if (c == '-') {
            negetive = PK_TRUE;
        }
        pk_try(pk_reader_ince(r, &c));
    }
    
    if (!pk_char_is_digit(c)) {
        return pk_error(r->lisp);
    }
    
    acc = pk_char_to_digit(c);
    
    while (pk_reader_inc(r, &c) && pk_char_is_digit(c)) {
        int a = pk_char_to_digit(c);
        acc *= 10;
        acc += a;
    }
    if (negetive) {
        acc = -acc;
    }
    
    pk_try(pk_atom_int(r->lisp, acc, number));
    
    return PK_Ok;
}

PKRes pk_read_atom_symbol(PKReader *r, PKAtomSymbol **symbol) {
    char c = '\0';
    size_t save = r->curr;

    pk_try(pk_reader_ate(r, &c));
    
    if (!pk_char_is_symbol(c)) {
        return pk_error(r->lisp);
    }

    do {
        if (!pk_reader_inc(r, &c)) break;
    } while(pk_char_is_symbol(c));

    pk_try(pk_atom_symbol_interned(r->lisp, r->c + save, r->curr - save, symbol));
    
    return PK_Ok;
}

PKRes pk_read_atom_cons(PKReader *r, PKAtom **output) {
    char c = '\0';
    PKAtomCons *head = NULL;
    PKAtomCons *acc = head;
    pk_bool first = PK_TRUE;
    pk_bool whitespace = PK_FALSE;
    
    pk_try(pk_reader_ate(r, &c));
    
    if (c != '(') {
        return pk_error(r->lisp);
    }
    

    pk_try(pk_reader_ince(r, &c));


    whitespace = pk_reader_trim_whitespace(r);

    while (!pk_reader_is_finished(r)) {
        if (c == ')') {
            (void)pk_reader_inc(r, &c);
            if (first) {
                *output = pk_atom_nil(r->lisp);
            } else {
                *output = (PKAtom *)head;
            }
            return PK_Ok;
        } else if (!first && !whitespace) {
            return pk_error(r->lisp);
        } else if (c == '.') {
            PKAtom *cdr = NULL;
            if (first) {
                return pk_error(r->lisp);
            }
            pk_try(pk_reader_ince(r, &c));
            (void)pk_reader_trim_whitespace(r);
            pk_try(pk_read_atom(r, &cdr));
            (void)pk_reader_trim_whitespace(r);
            pk_try(pk_reader_ince(r, &c));
            if (c != ')') {
                return pk_error(r->lisp);
            }
            acc->cdr = cdr;
            *output = (PKAtom *)head;
            return PK_Ok;
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

    return pk_error(r->lisp);
}

PKRes pk_read_atom_simple_macro(PKReader *r, PKAtomSymbol *macro, PKAtomCons **output) {
    char c = '\0';
    PKAtom *a = NULL;
    
    pk_try(pk_reader_ince(r, &c));
    pk_try(pk_read_atom(r, &a));
    pk_try(pk_atom_list2(r->lisp, (PKAtom *)macro, a, output));
    
    return PK_Ok;
}

PKRes pk_read_atom_unquote_macro(PKReader *r, PKAtomCons **output) {
    PKAtomSymbol *macro = NULL;
    PKAtom *a = NULL;
    char c = '\0';
    
    pk_try(pk_reader_ince(r, &c));
    if (c == '@') {
        macro = r->lisp->cache.unquote_splice;
        pk_try(pk_reader_ince(r, &c));
    } else {
        macro = r->lisp->cache.unquote;
    }
    
    pk_try(pk_read_atom(r, &a));
    pk_try(pk_atom_list2(r->lisp, (PKAtom *)macro, a, output));
    
    return PK_Ok;
}

PKRes pk_read_atom(PKReader *r, PKAtom **atom) {
    char c = '\0';

    pk_try((pk_reader_ate(r, &c)));
    
    switch (c) {
        case '(': {
            pk_try(pk_read_atom_cons(r, atom));
            return PK_Ok;
        }
        case ')': {
            return pk_error(r->lisp);
        }
        case '\'': {
            pk_try(pk_read_atom_simple_macro(r, r->lisp->cache.quote, (PKAtomCons **)atom));
            return PK_Ok;
        }
        case '`': {
            pk_try(pk_read_atom_simple_macro(r, r->lisp->cache.quasiquote, (PKAtomCons **)atom));
            return PK_Ok;
        }
        case '$': {
            pk_try(pk_read_atom_simple_macro(r, r->lisp->cache.string_substitute, (PKAtomCons **)atom));
            return PK_Ok;
        }
        case ',': {
            pk_try(pk_read_atom_unquote_macro(r, (PKAtomCons **)atom));
            return PK_Ok;
        }
        case '\"': {
            pk_try(pk_read_atom_string(r, (PKAtomString **)atom));
            return PK_Ok;
        }
        default: {
            if (pk_reader_is_number_prefix(r)) {
                pk_try(pk_read_atom_number(r, (PKAtomNumber **)atom));
            } else {
                pk_try(pk_read_atom_symbol(r, (PKAtomSymbol **)atom));
            }
            return PK_Ok;
        }
    }
}

PKRes pk_read_string(Pocket lisp, char *c, size_t length, PK_READ mode, PKAtom **output) {
    PKReader r;
    PKAtom *first = NULL;
    PKAtomCons *head = NULL;
    PKAtomCons *tail = NULL;

    r.c = c;
    r.curr = 0;
    r.length = length;
    r.lisp = lisp;

    (void)pk_reader_trim_whitespace(&r);
    if (pk_reader_is_finished(&r)) {
        *output = pk_atom_nil(lisp);
        return PK_Ok;
    }

    pk_try(pk_read_atom(&r, &first));

    if (mode == PK_READ_EXPRESSION) {
        (void)pk_reader_trim_whitespace(&r);
        if (!pk_reader_is_finished(&r)) {
            return pk_error(lisp);
        }
        *output = first;
        return PK_Ok;
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
    return PK_Ok;
}
