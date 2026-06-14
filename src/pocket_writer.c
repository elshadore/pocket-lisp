#include "pocket_internals.h"

void pk_writer_cons_loop(PKWriter *w, PKAtom *atom);

PKWriter pk_writer_init(Pocket lisp) {
    return (PKWriter) {
        .lisp = lisp,
        .c = NULL,
        .count = 0,
        .capacity = 0,
    };
}

void pk_writer_deinit(PKWriter *w) {
    if (w->c != NULL) {
        pk_free(w->lisp, w->c, w->capacity);
    }
    *w = pk_writer_init(w->lisp);
}

void pk_writer_grow(PKWriter *w, size_t needed) {
    size_t new_count = w->count + needed;
    if (new_count > w->capacity) {
        size_t new_capacity = pk_grow_capacity(w->capacity, PK_WRITER_INIT_CAPACITY);
        while (new_capacity < new_count) {
            new_capacity *= 2;
        }
        w->c = pk_realloc(w->lisp, w->c, w->capacity, new_capacity);
        w->capacity = new_capacity;
    }
}

void pk_writer_char(PKWriter *w, char c) {
    pk_writer_grow(w, 1);
    w->c[w->count++] = c;
}

void pk_writer_cstr(PKWriter *w, char *cstr) {
    pk_writer_string(w, pk_string_from_cstr(cstr));
}

void pk_writer_string(PKWriter *w, PKString s) {
    if (s.length == 0) {
        return;
    }
    pk_writer_grow(w, s.length);
    memcpy(w->c + w->count, s.c, s.length);
    w->count += s.length;
}

void pk_writer_int(PKWriter *w, int v) {
    char buf[32];
    int n = snprintf(buf, sizeof(buf), "%d", v);
    pk_writer_string(w, pk_string_new(buf, (size_t)n));
}

void pk_writer_float(PKWriter *w, float v) {
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "%g", v);
    pk_writer_string(w, pk_string_new(buf, (size_t)n));
}

PKString pk_writer_get(PKWriter *w) {
    return (PKString){ .c = w->c, .length = w->count };
}

void pk_writer_reset(PKWriter *w) {
    w->count = 0;
}


void pk_writer_string_escaped(PKWriter *w, PKString s) {
    for (size_t i = 0; i < s.length; ++i) {
        char c = s.c[i];
        switch (c) {
            case '"': {
                pk_writer_string(w, pkstr("\\\""));
                break;
            }
            case '\\': {
                pk_writer_string(w, pkstr("\\\\"));
                break;
            }
            case '\n': {
                pk_writer_string(w, pkstr("\\n"));
                break;
            }
            case '\t': {
                pk_writer_string(w, pkstr("\\t"));
                break;
            }
            case '\r': {
                pk_writer_string(w, pkstr("\\r"));
                break;
            }
            default: {
                pk_writer_char(w, c);
                break;
            }
        }
    }
}

void pk_writer_atom(PKWriter *w, PKAtom *atom) {
    switch (atom->tag.ty) {
        case PKAtomTy_Nil: {
            pk_writer_cstr(w, "nil");
            break;
        }
        case PKAtomTy_Number: {
            if (atom->number.ty == PKNumber_Int) {
                pk_writer_int(w, atom->number.as.i);
            } else {
                pk_writer_float(w, atom->number.as.f);
            }
            break;
        }
        case PKAtomTy_Symbol: {
            pk_writer_string(w, atom->symbol.id);
            break;
        }
        case PKAtomTy_String: {
            pk_writer_char(w, '"');
            pk_writer_string_escaped(w, atom->string.lit);
            pk_writer_char(w, '"');
            break;
        }
        case PKAtomTy_Cons: {
            pk_writer_char(w, '(');
            pk_writer_atom(w, atom->cons.car);
            pk_writer_cons_loop(w, atom);
            pk_writer_char(w, ')');
            break;
        }
        default: {
            pk_writer_cstr(w, "#<unknown>");
            break;
        }
    }
}

void pk_writer_cons_loop(PKWriter *w, PKAtom *atom) {
    PKAtom *rest = atom->cons.cdr;
    while (rest != NULL && rest->tag.ty == PKAtomTy_Cons) {
        pk_writer_cstr(w, " ");
        pk_writer_atom(w, rest->cons.car);
        rest = rest->cons.cdr;
    }
    if (rest != NULL && rest->tag.ty != PKAtomTy_Nil) {
        pk_writer_cstr(w, " . ");
        pk_writer_atom(w, rest);
    }
}
