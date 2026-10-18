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

void pk_strsub_ince(PKStrSub *sub) {
    if (!pk_strsub_inc(sub)) {
        pk_errors(sub->cmp->lisp, pk_str("string sub out of bounds"));
    }
}

void pk_strsub_mark(PKStrSub *sub) {
    sub->mark = sub->curr;
}

void pk_compile_strsub_value(PKStrSub *sub, PKStr string) {
    Pocket lisp = sub->cmp->lisp;

    PKAtom *read = pk_read_atom_from_str(lisp, string, PK_MODE_EXPRESSION);
    pk_compile_value(sub->cmp, read);
}

PKStr pk_strsub_slice(PKStrSub *sub) {
    return (PKStr) {
        .c = sub->string.c + sub->mark,
        .length = sub->curr - sub->mark,
    };
}

void pk_compile_strsub_(PKStrSub *sub) {
    Pocket lisp = sub->cmp->lisp;

    size_t count = 0;
    pk_strsub_mark(sub);

    do {
        if (pk_strsub_at(sub) == '\\') {
            pk_strsub_ince(sub);
            if (pk_strsub_at(sub) != '{') {
                pk_errors(lisp, pk_str("bad escape"));
            }
            pk_writer_char(lisp, &sub->w, '{');
            pk_strsub_ince(sub);
        } else if (pk_strsub_at(sub) == '{') {
            if (sub->w.count > 0) {
                pk_cmp_load(sub->cmp, (PKAtom *)pk_atom_string(lisp, pk_writer_slice(&sub->w)));
                count += 1;
            }

            pk_writer_reset(&sub->w);
            pk_strsub_ince(sub);

            pk_strsub_mark(sub);
            while (pk_strsub_at(sub) != '}') {
                pk_strsub_ince(sub);
            }
            {
                PKStr slice = pk_strsub_slice(sub);
                if (slice.length == 0) {
                    pk_errors(lisp, pk_str("empty sub"));
                }
                pk_compile_strsub_value(sub, slice);
                pk_strsub_mark(sub);
            }

            count += 1;
        } else {
            pk_writer_char(lisp, &sub->w, pk_strsub_at(sub));
        }
    } while (pk_strsub_inc(sub));

    if (sub->w.count > 0) {
        pk_cmp_load(sub->cmp, (PKAtom *)pk_atom_string(lisp, pk_writer_slice(&sub->w)));
        count += 1;
    }

    if (count > 1) {
        pk_cmp_push_byte(sub->cmp, PK_OP_STRCAT);
        pk_cmp_push_any(sub->cmp, count);
    }
}

void pk_compile_strsub(PKCompiler *c, PKStr string) {
    Pocket lisp = c->lisp;

    if (string.length == 0) {
        pk_cmp_load(c, (PKAtom *)lisp->cache.empty_string);
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

    pk_compile_strsub_(&sub);

    pk_writer_deinit(lisp, &sub.w);
}
