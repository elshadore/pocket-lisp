#include "pocket_internals.h"

#define pk_reader_at(lisp_) (lisp_)->c

pk_bool pk_reader_is_finished(Pocket lisp) {
    return lisp->read.curr >= lisp->read.src.length;
}

void pk_reader_set(Pocket lisp, PKReader read) {
    lisp->read = read;
    if (pk_reader_is_finished(lisp)) {
        lisp->c = '\0';
    } else {
        lisp->c = lisp->read.src.c[lisp->read.curr];
    }
}

PKRes pk_reader_push(Pocket lisp, PKReader read) {
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Read_Mode, (PKFrameData){.read = lisp->read}));
    pk_reader_set(lisp, read);
    return PK_Ok;
}

PKRes pk_reader_pop(Pocket lisp) {
    pk_reader_set(lisp, lisp->current_frame.as.read);
    pk_try(pk_ret_top(lisp));
    return PK_Ok;
}

pk_bool pk_reader_try_inc(Pocket lisp) {
    lisp->read.curr += 1;
    if (pk_reader_is_finished(lisp)) {
        lisp->read.curr = lisp->read.src.length;
        lisp->c = '\0';
        return PK_FALSE;
    } else {
        lisp->c = lisp->read.src.c[lisp->read.curr];
        return PK_TRUE;
    }
}

PKRes pk_reader_inc(Pocket lisp) {
    if (!pk_reader_try_inc(lisp)) {
        return pk_error(lisp);
    } else {
        return PK_Ok;
    }
}

pk_bool pk_reader_peek(Pocket lisp, char *output) {
    size_t peek = lisp->read.curr + 1;
    if (peek >= lisp->read.src.length) {
        return PK_FALSE;
    }
    *output = lisp->read.src.c[peek];
    return PK_TRUE;
}

pk_bool pk_reader_trim_whitespace(Pocket lisp) {
    pk_bool whitespace = PK_FALSE;
    for (;;) {
        if (pk_reader_at(lisp) == PK_COMMENT_CHAR) {
            while (pk_reader_try_inc(lisp) && pk_reader_at(lisp) != '\n');
        } else if (!pk_reader_is_finished(lisp) && pk_char_is_whitespace(pk_reader_at(lisp))) {
            while (pk_reader_try_inc(lisp) && pk_char_is_whitespace(pk_reader_at(lisp)));
            whitespace = PK_TRUE;
        } else {
            break;
        }
    }
    return whitespace;
}

pk_bool pk_reader_is_number_prefix(Pocket lisp) {
    char c = '\0';
    if (pk_char_is_digit(lisp->c)) return PK_TRUE;
    if (lisp->c != '+' && lisp->c != '-') return PK_FALSE;
    if (!pk_reader_peek(lisp, &c)) return PK_FALSE;
    if (pk_char_is_digit(c)) return PK_TRUE;
    return PK_FALSE;
}

PKRes pk_read_routine_string(Pocket lisp) {
    PKWriter w = pk_writer_init(lisp);
    PKRes result = PK_Yield;

    if (pk_reader_at(lisp) != '\"') {
        return pk_error(lisp);
    }

    while (pk_reader_try_inc(lisp)) {
        if (pk_reader_at(lisp) == '\"') {
            PKString string;
            PKAtomString *atom = NULL;
            
            string.c = w.c;
            string.length = w.count;
            (void)pk_reader_try_inc(lisp);
            pk_defer(pk_atom_string(lisp, string, &atom));
            pk_defer(pk_ret_this(lisp, (PKAtom *)atom));
            result = PK_Ok;
            goto DEFER;
        } else if (pk_reader_at(lisp) == '\\') {
            pk_defer(pk_reader_inc(lisp));
            switch (pk_reader_at(lisp)) {
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
                    
                    pk_defer(pk_reader_inc(lisp));
                    a = pk_reader_at(lisp);
                    pk_defer(pk_reader_inc(lisp));
                    b = pk_reader_at(lisp);
                    if (!pk_char_is_hex(a)) {
                        pk_defer(pk_error(lisp));
                    }
                    if (!pk_char_is_hex(b)) {
                        pk_defer(pk_error(lisp));
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
                    pk_defer(pk_error(lisp));
                    break;
                }
            }
        } else {
            pk_defer(pk_writer_char(&w, pk_reader_at(lisp)));
        }
    }
    
    pk_defer(pk_error(lisp));
    
    DEFER:
    pk_writer_deinit(&w);
    return result;
}

PKRes pk_read_routine_number(Pocket lisp) {
    PKAtomNumber *atom = NULL;
    pk_bool negetive = PK_FALSE;
    int acc = 0;
    
    if (pk_reader_at(lisp) == '+' || pk_reader_at(lisp) == '-') {
        if (pk_reader_at(lisp) == '-') {
            negetive = PK_TRUE;
        }
        pk_try(pk_reader_inc(lisp));
    }
    
    acc = pk_char_to_digit(pk_reader_at(lisp));
    while (pk_reader_try_inc(lisp) && pk_char_is_digit(pk_reader_at(lisp))) {
        int a = pk_char_to_digit(pk_reader_at(lisp));
        acc *= 10;
        acc += a;
    }
    if (negetive) {
        acc = -acc;
    }
    
    pk_try(pk_atom_int(lisp, acc, &atom));
    pk_try(pk_ret_this(lisp, (PKAtom *)atom));
    
    return PK_Ok;
}

PKRes pk_read_routine_symbol(Pocket lisp) {
    PKAtomSymbol *atom = NULL;
    size_t curr = lisp->read.curr;
    PKString string;
    
    if (!pk_char_is_symbol(pk_reader_at(lisp))) {
        return pk_error(lisp);
    }

    do {
        if (!pk_reader_try_inc(lisp)) break;
    } while(pk_char_is_symbol(pk_reader_at(lisp)));

    string.c = lisp->read.src.c + curr;
    string.length = lisp->read.curr - curr;
    pk_try(pk_atom_symbol_interned(lisp, string, &atom));
    pk_try(pk_ret_this(lisp, (PKAtom *)atom));
    
    return PK_Ok;
}

PKRes pk_read_routine_cons(Pocket lisp) {
    if (pk_reader_at(lisp) != '(') {
        return pk_error(lisp);
    }

    pk_try(pk_reader_inc(lisp));

    (void)pk_reader_trim_whitespace(lisp);
    if (pk_reader_at(lisp) == ')') {
        (void)pk_reader_try_inc(lisp);
        pk_try(pk_ret_nil(lisp));
        return PK_Ok;
    } else if (pk_reader_at(lisp) == '.') {
        return pk_error(lisp);
    }
    lisp->current_frame.mode = PKEvalMode_Read_Cons;
            
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Read_Atom, (PKFrameData){0}));
    return PK_Ok;
}

PKRes pk_interp_cons(Pocket lisp) {
    PKAtom *atom = NULL;
    PKAtomCons *cons = NULL;
    
    pk_try(pk_stack_head(lisp, &atom));
    pk_frame_clear(lisp);
    
    pk_try(pk_atom_cons_car(lisp, atom, &cons));
    (void)pk_reader_trim_whitespace(lisp);
    if (pk_reader_at(lisp) == ')') {
        pk_try(pk_ret_this(lisp, (PKAtom *)cons));
        (void)pk_reader_try_inc(lisp);
        return PK_Ok;
    } else if (pk_reader_at(lisp) == '.') {
        pk_try(pk_reader_inc(lisp));
        (void)pk_reader_trim_whitespace(lisp);
        lisp->current_frame.mode = PKEvalMode_Read_Cons_3;
    } else {
        lisp->current_frame.mode = PKEvalMode_Read_Cons_2;
    }
    lisp->current_frame.as.tc.head = cons;
    lisp->current_frame.as.tc.tail = cons;
    
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Read_Atom, (PKFrameData){0}));
    return PK_Ok;
}

PKRes pk_interp_cons_2(Pocket lisp) {
    PKAtom *atom = NULL;
    PKAtomCons *cons = NULL;
    
    pk_try(pk_stack_head(lisp, &atom));
    pk_frame_clear(lisp);
    
    pk_try(pk_atom_cons_car(lisp, atom, &cons));
    lisp->current_frame.as.tc.tail->cdr = (PKAtom *)cons;
    lisp->current_frame.as.tc.tail = cons;
    
    (void)pk_reader_trim_whitespace(lisp);
    if (pk_reader_at(lisp) == ')') {
        (void)pk_reader_try_inc(lisp);
        pk_try(pk_ret_this(lisp, (PKAtom *)lisp->current_frame.as.tc.head));
        return PK_Ok;
    } else if (pk_reader_at(lisp) == '.') {
        pk_try(pk_reader_inc(lisp));
        (void)pk_reader_trim_whitespace(lisp);
        lisp->current_frame.mode = PKEvalMode_Read_Cons_3;
    } else {
        lisp->current_frame.mode = PKEvalMode_Read_Cons_2;
    }
    
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Read_Atom, (PKFrameData){0}));
    return PK_Ok;
}

PKRes pk_interp_cons_3(Pocket lisp) {
    PKAtom *atom = NULL;
    pk_try(pk_stack_head(lisp, &atom));
    lisp->current_frame.as.tc.tail->cdr = atom;
    (void)pk_reader_trim_whitespace(lisp);
    if (pk_reader_at(lisp) != ')') {
        return pk_error(lisp);
    }
    pk_try(pk_reader_inc(lisp));
    return pk_ret_this(lisp, (PKAtom *)lisp->current_frame.as.tc.head);
}

PKRes pk_read_routine_simple_macro(Pocket lisp, PKAtomSymbol *macro) {
    pk_try(pk_reader_inc(lisp));
    lisp->current_frame.mode = PKEvalMode_Read_Append;
    lisp->current_frame.as.atom = (PKAtom *)macro;
    
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Read_Atom, (PKFrameData){0}));
    return PK_Ok;
}

PKRes pk_read_routine_unquote_macro(Pocket lisp) {
    PKAtomSymbol *macro = NULL;
    
    pk_try(pk_reader_inc(lisp));
    if (pk_reader_at(lisp) == '@') {
        macro = lisp->cache.unquote_splice;
        pk_try(pk_reader_inc(lisp));
    } else {
        macro = lisp->cache.unquote;
    }
    lisp->current_frame.mode = PKEvalMode_Read_Append;
    lisp->current_frame.as.atom = (PKAtom *)macro;
    
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Read_Atom, (PKFrameData){0}));
    return PK_Ok;
}

PKRes pk_interp_read_append(Pocket lisp) {
    PKAtom *atom = NULL;
    PKAtomCons *list = NULL;
    
    pk_try(pk_stack_head(lisp, &atom));
    pk_try(pk_atom_list2(lisp, lisp->current_frame.as.atom, atom, &list));
    pk_try(pk_ret_this(lisp, (PKAtom *)list));
    return PK_Ok;
}

PKRes pk_interp_read_atom(Pocket lisp) {
    if (pk_reader_is_finished(lisp)) {
        return pk_error(lisp);
    }
    switch (pk_reader_at(lisp)) {
        case '(': {
            pk_try(pk_read_routine_cons(lisp));
            return PK_Ok;
        }
        case ')': {
            return pk_error(lisp);
        }
        case '\'': {
            pk_try(pk_read_routine_simple_macro(lisp, lisp->cache.quote));
            return PK_Ok;
        }
        case '`': {
            pk_try(pk_read_routine_simple_macro(lisp, lisp->cache.quasiquote));
            return PK_Ok;
        }
        case '$': {
            pk_try(pk_read_routine_simple_macro(lisp, lisp->cache.string_substitute));
            return PK_Ok;
        }
        case ',': {
            pk_try(pk_read_routine_unquote_macro(lisp));
            return PK_Ok;
        }
        case '\"': {
            pk_try(pk_read_routine_string(lisp));
            return PK_Ok;
        }
        default: {
            if (pk_reader_is_number_prefix(lisp)) {
                pk_try(pk_read_routine_number(lisp));
            } else {
                pk_try(pk_read_routine_symbol(lisp));
            }
            return PK_Ok;
        }
    }
}

PKRes pk_interp_read_all(Pocket lisp) {
    PKAtom *atom = NULL;
    pk_try(pk_stack_head(lisp, &atom));
    
    PKAtomCons *cons = NULL;
    pk_try(pk_atom_cons_car(lisp, atom, &cons));
    (void)pk_reader_trim_whitespace(lisp);
    
    if (pk_reader_is_finished(lisp)) {
        pk_try(pk_ret_this(lisp, (PKAtom *)cons));
        return PK_Ok;
    } else {
        
        lisp->current_frame.mode = PKEvalMode_Read_All_2;
        lisp->current_frame.as.tc = (PKTCons) {
            .head = cons,
            .tail = cons
        };
        pk_try(pk_frame_push(lisp, 0, PKEvalMode_Read_Atom, (PKFrameData){0}));
        return PK_Ok;
    }
}

PKRes pk_interp_read_all_2(Pocket lisp) {
    PKAtom *atom = NULL;
    pk_try(pk_stack_head(lisp, &atom));
    pk_frame_clear(lisp);
    
    PKAtomCons *cons = NULL;
    pk_try(pk_atom_cons_car(lisp, atom, &cons));
    lisp->current_frame.as.tc.tail->cdr = (PKAtom *)cons;
    lisp->current_frame.as.tc.tail = cons;
    
    (void)pk_reader_trim_whitespace(lisp);
    if (pk_reader_is_finished(lisp)) {
        pk_try(pk_ret_this(lisp, (PKAtom *)lisp->current_frame.as.tc.head));
        return PK_Ok;
    } else {
        pk_try(pk_frame_push(lisp, 0, PKEvalMode_Read_Atom, (PKFrameData){0}));
        return PK_Ok;
    }
}

PKRes pk_read_string(Pocket lisp, char *c, size_t length, PK_READ mode) {
    
    return PK_Ok;
}
