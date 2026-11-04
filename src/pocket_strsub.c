#include "pocket_internals.h"

typedef struct PKStrSub_ {
    PKCompiler *cmp;
    PKWriter w;
    PKStr string;
    size_t curr;
    size_t mark;
    char c;
} PKStrSub;

#define pk_strsub_at(sub_) ((sub_)->c)

bool pk_strsub_inc(PKStrSub *sub) {
    sub->curr += 1;
    if (sub->curr >= sub->string.length) {
        sub->curr = sub->string.length;
        return false;
    }
    sub->c = sub->string.c[sub->curr];
    return true;
}

void pk_strsub_ince(Pocket lisp, PKStrSub *sub) {
    if (!pk_strsub_inc(sub)) {
        pk_errors(lisp, pk_str("string sub out of bounds"));
    }
}

void pk_strsub_mark(PKStrSub *sub) {
    sub->mark = sub->curr;
}

void pk_compile_strsub_value(Pocket lisp, PKStrSub *sub, PKStr string) {
    PKAtom *parsed = pk_parse_atom_from_str(lisp, string, PK_MODE_EXPRESSION);
    pk_compile_value(lisp, sub->cmp, parsed);
}

PKStr pk_strsub_slice(PKStrSub *sub) {
    return (PKStr) {
        .c = sub->string.c + sub->mark,
        .length = sub->curr - sub->mark,
    };
}

void pk_compile_strsub_(Pocket lisp, PKStrSub *sub) {
    size_t count = 0;
    pk_strsub_mark(sub);

    do {
        if (pk_strsub_at(sub) == '\\') {
            pk_strsub_ince(lisp, sub);
            if (pk_strsub_at(sub) != '{') {
                pk_errors(lisp, pk_str("bad escape"));
            }
            pk_writer_char(lisp, &sub->w, '{');
            pk_strsub_ince(lisp, sub);
        } else if (pk_strsub_at(sub) == '{') {
            if (sub->w.count > 0) {
                pk_cmp_load(lisp, sub->cmp, (PKAtom *)pk_atom_string(lisp, pk_writer_slice(&sub->w)));
                count += 1;
            }

            pk_writer_reset(&sub->w);
            pk_strsub_ince(lisp, sub);

            pk_strsub_mark(sub);
            while (pk_strsub_at(sub) != '}') {
                pk_strsub_ince(lisp, sub);
            }
            {
                PKStr slice = pk_strsub_slice(sub);
                if (slice.length == 0) {
                    pk_errors(lisp, pk_str("empty sub"));
                }
                pk_compile_strsub_value(lisp, sub, slice);
                pk_strsub_mark(sub);
            }

            count += 1;
        } else {
            pk_writer_char(lisp, &sub->w, pk_strsub_at(sub));
        }
    } while (pk_strsub_inc(sub));

    if (sub->w.count > 0) {
        pk_cmp_load(lisp, sub->cmp, (PKAtom *)pk_atom_string(lisp, pk_writer_slice(&sub->w)));
        count += 1;
    }

    if (count > 1) {
        pk_cmp_push_byte(lisp, sub->cmp, PK_OP_STRCAT);
        pk_cmp_push_any(lisp, sub->cmp, count);
    }
}

void pk_compile_strsub(Pocket lisp, PKCompiler *c, PKStr string) {
    if (string.length == 0) {
        pk_cmp_load(lisp, c, (PKAtom *)lisp->cache.empty_string);
        return;
    }

    PKStrSub sub = (PKStrSub) {
        .c = string.c[0],
        .string = string,
        .curr = 0,
        .mark = 0,
        .cmp = c,
        .w = PK_WRITER_EMPTY,
    };

    pk_compile_strsub_(lisp, &sub);

    pk_writer_deinit(lisp, &sub.w);
}
