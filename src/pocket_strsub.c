#include "pocket_internals.h"

typedef struct PKStrSub_ {
    Pocket lisp;
    PKCompiler *cmp;
    PKWriter w;
    char *string;
    size_t length;
    size_t curr;
    size_t mark;
    char c;
} PKStrSub;

#define pk_strsub_at(sub_) ((sub_)->c)

pk_bool pk_strsub_inc(PKStrSub *sub) {
    sub->curr += 1;
    if (sub->curr >= sub->length) {
        sub->curr = sub->length;
        return PK_FALSE;
    }
    sub->c = sub->string[sub->curr];
    return PK_TRUE;
}

PK_RES pk_strsub_ince(PKStrSub *sub) {
    if (!pk_strsub_inc(sub)) {
        return pk_error(sub->lisp);
    } else {
        return PK_OK;
    }
}

void pk_strsub_mark(PKStrSub *sub) {
    sub->mark = sub->curr;
}

PK_RES pk_compile_strsub_value(PKStrSub *sub, char *string, size_t length) {
    PKAtom *read = NULL;
    pk_try(pk_read_string(sub->lisp, string, length, PK_READ_EXPRESSION, &read));
    pk_try(pk_compile_value(sub->cmp, read));
    return PK_OK;
}

char *pk_strsub_slice(PKStrSub *sub, size_t *length) {
    *length = sub->curr - sub->mark;
    return sub->string + sub->mark;
}

PK_RES pk_compile_strsub_1(PKStrSub *sub) {
    size_t count = 0;
    pk_strsub_mark(sub);
    
    do {
        if (pk_strsub_at(sub) == '\\') {
            pk_try(pk_strsub_ince(sub));
            if (pk_strsub_at(sub) != '{') {
                return pk_error(sub->lisp);
            }
            pk_try(pk_writer_char(&sub->w, '{'));
            pk_try(pk_strsub_ince(sub));
        } else if (pk_strsub_at(sub) == '{') {
            if (sub->w.count > 0) {
                PKAtomString *before = NULL;
                pk_try(pk_writer_to_string(&sub->w, &before));
                pk_try(pk_cmp_load(sub->cmp, (PKAtom *)before));
                count += 1;
            }
                
            pk_writer_reset(&sub->w);
            pk_try(pk_strsub_ince(sub));
            
            pk_strsub_mark(sub);
            while (pk_strsub_at(sub) != '}') {
                pk_try(pk_strsub_ince(sub));
            }
            {
                size_t length = 0;
                char *string = pk_strsub_slice(sub, &length);
                if (length == 0) {
                    return pk_error(sub->lisp);
                }
                pk_try(pk_compile_strsub_value(sub, string, length));
                pk_strsub_mark(sub);
            }

            count += 1;
        } else {
            pk_try(pk_writer_char(&sub->w, pk_strsub_at(sub)));
        }
    } while (pk_strsub_inc(sub));
    
    if (sub->w.count > 0) {
        PKAtomString *before = NULL;
        pk_try(pk_writer_to_string(&sub->w, &before));
        pk_try(pk_cmp_load(sub->cmp, (PKAtom *)before));
        count += 1;
    }

    if (count > 1) {
        pk_try(pk_cmp_push_byte(sub->cmp, PK_OP_STRCAT));
        pk_try(pk_cmp_push_any(sub->cmp, count));
    }
    return PK_OK;
}

PK_RES pk_compile_strsub(PKCompiler *c, char *string, size_t length) {
    PKStrSub sub;
    PK_RES result = PK_YIELD;
    sub.lisp = c->lisp;
    sub.c = string[0];
    sub.string = string;
    sub.curr = 0;
    sub.cmp = c;
    sub.w = pk_writer_init(c->lisp);
    sub.length = length;

    result = pk_compile_strsub_1(&sub);
    
    pk_writer_deinit(&sub.w);

    return result;
}
