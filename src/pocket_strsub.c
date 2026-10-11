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

PKAtom pk_strsub_ince(PKStrSub *sub) {
    if (!pk_strsub_inc(sub)) {
        return pk_error();
    } else {
        return pk_true();
    }
}

void pk_strsub_mark(PKStrSub *sub) {
    sub->mark = sub->curr;
}

PKAtom pk_compile_strsub_value(PKStrSub *sub, PKStr string) {
    PKAtom read = pk_read_str(string, PK_MODE_EXPRESSION);
    pk_propagate(read);
    pk_propagate(pk_compile_value(sub->cmp, read));
    return pk_true();
}

PKStr pk_strsub_slice(PKStrSub *sub) {
    return (PKStr) {
        .c = sub->string.c + sub->mark,
        .length = sub->curr - sub->mark,
    };
}

PKAtom pk_compile_strsub_(PKStrSub *sub) {
    size_t count = 0;
    pk_strsub_mark(sub);
    
    do {
        if (pk_strsub_at(sub) == '\\') {
            pk_propagate(pk_strsub_ince(sub));
            if (pk_strsub_at(sub) != '{') {
                return pk_error();
            }
            pk_propagate(pk_writer_char(&sub->w, '{'));
            pk_propagate(pk_strsub_ince(sub));
        } else if (pk_strsub_at(sub) == '{') {
            if (sub->w.count > 0) {
                PKAtom before = pk_string(pk_writer_slice(&sub->w));
                pk_propagate(before);
                pk_propagate(pk_cmp_load(sub->cmp, before));
                count += 1;
            }
                
            pk_writer_reset(&sub->w);
            pk_propagate(pk_strsub_ince(sub));
            
            pk_strsub_mark(sub);
            while (pk_strsub_at(sub) != '}') {
                pk_propagate(pk_strsub_ince(sub));
            }
            {
                PKStr slice = pk_strsub_slice(sub);
                if (slice.length == 0) {
                    return pk_error();
                }
                pk_propagate(pk_compile_strsub_value(sub, slice));
                pk_strsub_mark(sub);
            }

            count += 1;
        } else {
            pk_propagate(pk_writer_char(&sub->w, pk_strsub_at(sub)));
        }
    } while (pk_strsub_inc(sub));
    
    if (sub->w.count > 0) {
        PKAtom before = pk_string(pk_writer_slice(&sub->w));
        pk_propagate(before);
        pk_propagate(pk_cmp_load(sub->cmp, before));
        count += 1;
    }

    if (count > 1) {
        pk_propagate(pk_cmp_push_byte(sub->cmp, PK_OP_STRCAT));
        pk_propagate(pk_cmp_push_any(sub->cmp, count));
    }
    
    return pk_true();
}

PKAtom pk_compile_strsub(PKCompiler *c, PKStr string) {
    if (string.length == 0) {
        pk_propagate(pk_cmp_push_atom(c, pk_string_empty()));
        return pk_true();
    }
    
    PKStrSub sub = (PKStrSub) {
        .c = string.c[0],
        .string = string,
        .curr = 0,
        .cmp = c,
        .w = PK_WRITER_EMPTY,
    };

    PKAtom result = pk_compile_strsub_(&sub);
    
    pk_writer_deinit(&sub.w);

    return result;
}
