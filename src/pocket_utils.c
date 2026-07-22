#include "pocket_internals.h"

PKRes pk_string_dupe(Pocket lisp, const char *c, size_t length, char **output) {
    char *copy = NULL;
    
    if (length == 0) {
        *output = NULL;
        return PK_Ok;
    }
    
    pk_try(pk_malloc(lisp, length, (void **)&copy));
    (void)memcpy(copy, c, length);
    *output = copy;

    return PK_Ok;
}

pk_bool pk_string_eq(const char *a, size_t a_length, const char *b, size_t b_length) {
    if (a_length != b_length) return PK_FALSE;
    return (pk_bool)memcmp(a, b, a_length) == 0;
}

void pk_string_reverse(char *c, size_t length) {
    size_t i = 0;
    size_t j = 0;
    
    if (length <= 1) return;
    j = length - 1;
    
    for (; i < j; i++, j--) {
        char temp = c[i];
        c[i] = c[j];
        c[j] = temp;
    }
}

PKRes pk_print(Pocket lisp, char *c, size_t length) {
    (lisp->print)(lisp->user_env, c, length);
    return PK_Ok;
}

PKRes pk_puts(Pocket lisp, char *c, size_t length) {
    (void)lisp;
    fprintf(stderr, "%.*s", (int)length, c);
    return PK_Ok;
}

pk_u8 pk_char_to_digit(char c) {
    switch (c) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        default: return 0;
    }
}

char pk_char_from_digit(pk_u8 integer) {
    switch (integer) {
        case 0: return '0';
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        default: return '\0';
    }
}

pk_bool pk_char_is_digit(char c) {
    switch (c) {
        case '0': return PK_TRUE;
        case '1': return PK_TRUE;
        case '2': return PK_TRUE;
        case '3': return PK_TRUE;
        case '4': return PK_TRUE;
        case '5': return PK_TRUE;
        case '6': return PK_TRUE;
        case '7': return PK_TRUE;
        case '8': return PK_TRUE;
        case '9': return PK_TRUE;
        default: return PK_FALSE;
    }
}

pk_bool pk_char_is_whitespace(char c) {
    switch (c) {
        case '\0': return PK_TRUE;
        case ' ': return PK_TRUE;
        case '\n': return PK_TRUE;
        case '\f': return PK_TRUE;
        case '\r': return PK_TRUE;
        case '\t': return PK_TRUE;
        case '\v': return PK_TRUE;
        default: return PK_FALSE;
    }
}

pk_bool pk_char_is_alphabet(char c) {
    if (c >= 'A' && c <= 'Z') return PK_TRUE;
    if (c >= 'a' && c <= 'z') return PK_TRUE;
    return PK_FALSE;
}

pk_bool pk_char_is_symbol(char c) {
    switch (c) {
        case '+': return PK_TRUE;
        case '-': return PK_TRUE;
        case '/': return PK_TRUE;
        case '*': return PK_TRUE;
        case '!': return PK_TRUE;
        case '?': return PK_TRUE;
        case '>': return PK_TRUE;
        case '<': return PK_TRUE;
        case '=': return PK_TRUE;
        case '_': return PK_TRUE;
        case '%': return PK_TRUE;
        default: {
            if (pk_char_is_alphabet(c)) return PK_TRUE;
            if (pk_char_is_digit(c)) return PK_TRUE;
            return PK_FALSE;
        }
    }
}

char pk_char_from_hex(pk_u8 byte) {
    switch (byte) {
        case 0: return '0';
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        case 10: return 'A';
        case 11: return 'B';
        case 12: return 'C';
        case 13: return 'D';
        case 14: return 'E';
        default: return 'F';
    }
}

pk_bool pk_char_is_hex(char c) {
    switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f': return PK_TRUE;
        default: return PK_FALSE;
    }
}

pk_u8 pk_char_to_hex(char c) {
    switch (c) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'A': return 10;
        case 'B': return 11;
        case 'C': return 12;
        case 'D': return 13;
        case 'E': return 14;
        case 'F': return 15;
        case 'a': return 10;
        case 'b': return 11;
        case 'c': return 12;
        case 'd': return 13;
        case 'e': return 14;
        case 'f': return 15;
        default: return 0;
    }
}

size_t pk_hash_djb2(const char *c, size_t length) {
    size_t hash = 5381;
    size_t i = 0;
    for (i = 0; i < length; ++i) {
        hash = ((hash << 5) + hash) + (size_t)c[i];
    }
    return hash;
}

size_t pk_hash_pointer(void *ptr) {
    size_t p = (size_t)ptr;
    return (size_t)(p >> 3);
}

PKRes pk_dump_stack(Pocket lisp, const char *tag) {
    int top = pk_get_top(lisp);
    int i = 0;
    PKWriter w = pk_writer_init(lisp);
    PKRes result = PK_Yield;
    
    pk_defer(pk_writer_string(&w, "*~STACK-DUMP~*"));
    if (tag != NULL) {
        pk_defer(pk_writer_string(&w, " (tag = "));
        pk_defer(pk_writer_string(&w, tag));
        pk_defer(pk_writer_string(&w, ")"));
    }
    pk_defer(pk_writer_newline(&w));
    
    for (i = top; i > 0; i--) {
        int rel = pk_sp_relative(lisp, i);
        PKAtom *a = NULL;
        pk_defer(pk_stack_get(lisp, i, &a));
        
        pk_defer(pk_writer_string(&w, "    ["));
        pk_defer(pk_writer_int(&w, rel));
        pk_defer(pk_writer_string(&w, "/"));
        pk_defer(pk_writer_int(&w, i));
        pk_defer(pk_writer_string(&w, "] => "));
        
        pk_defer(pk_writer_atom(&w, a));
        pk_defer(pk_writer_newline(&w));
    }
    pk_defer(pk_writer_print(&w));
    result = PK_Ok;
    DEFER:
    pk_writer_deinit(&w);
    return result;
}

/*
PKRes pk_frame_dump(Pocket lisp, PKWriter *w, PKFrame *frame, size_t length, size_t index) {
    PKString id = pk_ident_evalmode(frame->mode);
    pk_try(pk_writer_printf(w, "FRAME: [%zu] [%.*s]", index, (int)id.length, id.c));
    
    switch (pk_evalmode_framety(frame->mode)) {
        case PKEvalFrameTy_None: {
            break;
        }
        case PKEvalFrameTy_Atom: {
            pk_try(pk_writer_string(w, pkstr(" => ")));
            pk_try(pk_writer_atom(w, frame->as.atom));
            break;
        }
        case PKEvalFrameTy_Tuple: {
            pk_try(pk_writer_string(w, pkstr(" => ")));
            pk_try(pk_writer_atom(w, frame->as.t.a));
            pk_try(pk_writer_string(w, pkstr(" | ")));
            pk_try(pk_writer_atom(w, frame->as.t.b));
            break;
        }
        case PKEvalFrameTy_CFn: {
            break;
        }
    }
    // if (pk_evalmode_readty(frame->mode)) {
    //     pk_try(pk_writer_string(w, pkstr(" => ")));
    //     pk_try(pk_writer_char(w, lisp->c));
    // }
    pk_try(pk_writer_newline(w));
    
    PKAtoms atoms = pk_frame_slice(lisp, frame, length);
    for (size_t i = 0; i < atoms.length; ++i) {
        size_t index = pk_index_inv(i, atoms.length);
        int a = (int)(index + 1);
        pk_try(pk_writer_printf(w, "    [%d / %d] => ", -a, a));
        pk_try(pk_writer_atom(w, atoms.e[index]));
        pk_try(pk_writer_newline(w));
    }
    return PK_Ok;
}

PKRes pk_trace_dump(Pocket lisp, const char *tag) {
    PKWriter w = pk_writer_init(lisp);
    PKRes result = PK_Yield;
    pk_defer(pk_writer_printf(&w, "*~TRACE-DUMP~* (tag = %s)\n", tag));
    if (lisp->c != '\0') {
        pk_defer(pk_writer_printf(&w, "READER: %zu => %c\n", lisp->read.curr, lisp->c));
    }
    size_t length = pk_frame_length(lisp);
    size_t dec = pk_stack_total(lisp) - length;
    pk_defer(pk_frame_dump(lisp, &w, &lisp->current_frame, length, lisp->frames.count));
    for (size_t i = 0; i < lisp->frames.count; ++i) {
        size_t index = pk_index_inv(i, lisp->frames.count);
        PKFrame frame = lisp->frames.e[index];
        size_t length = dec - frame.stack_offset;
        pk_defer(pk_frame_dump(lisp, &w, &frame, length, index));
        dec -= length;
    }
    pk_defer(pk_writer_print(&w));
    result = PK_Ok;
    DEFER:
    pk_writer_deinit(&w);
    return result;
}

PKRes pk_env_dump(Pocket lisp, const char *tag) {
    PKWriter w = pk_writer_init(lisp);
    PKRes result = PK_Yield;
    pk_defer(pk_writer_printf(&w, "*~ENVIRONMENT~* (tag = %s)\n", tag));
    pk_defer(pk_writer_printf(&w, "SECTION: VARS\n"));
    for (size_t i = 0; i < lisp->vars.capacity; i++) {
        for (PKSymTableSlot *slot = lisp->vars.e[i]; slot; slot = slot->chain) {
            pk_defer(pk_writer_string(&w, pkstr("    [")));
            pk_defer(pk_writer_atom(&w, (PKAtom *)slot->key));
            pk_defer(pk_writer_string(&w, pkstr("] => ")));
            pk_defer(pk_writer_atom(&w, slot->value));
            pk_defer(pk_writer_newline(&w));
        }
    }
    pk_defer(pk_writer_printf(&w, "SECTION: FUNS\n"));
    for (size_t i = 0; i < lisp->funs.capacity; i++) {
        for (PKSymTableSlot *slot = lisp->funs.e[i]; slot; slot = slot->chain) {
            pk_defer(pk_writer_string(&w, pkstr("    [")));
            pk_defer(pk_writer_atom(&w, (PKAtom *)slot->key));
            pk_defer(pk_writer_string(&w, pkstr("] => ")));
            pk_defer(pk_writer_atom(&w, slot->value));
            pk_defer(pk_writer_newline(&w));
        }
    }
    pk_defer(pk_writer_print(&w));
    result = PK_Ok;
    DEFER:
    pk_writer_deinit(&w);
    return result;
}
*/

PKRes pk_util_slurpn(Pocket lisp, const char *file_path, size_t length, char **out_c, size_t *out_length) {
    char *buffer = NULL;
    PKRes result = PK_Yield;
    
    pk_try(pk_malloc(lisp, length + 1, (void **)&buffer));
    (void)memcpy(buffer, file_path, length);
    buffer[length] = '\0';

    result = pk_util_slurp(lisp, buffer, out_c, out_length);

    pk_free(lisp, buffer, length + 1);

    return result;
}
    
PKRes pk_util_slurp(Pocket lisp, const char *file_path, char **out_c, size_t *out_length) {
    FILE *f = NULL;
    long length = 0;
    size_t read = 0;
    char *buf = NULL;

    f = fopen(file_path, "rb");
    
    if (f == NULL) {
        return pk_error(lisp);
    }
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    if (length < 0) {
        fclose(f);
        return pk_error(lisp);
    }
    rewind(f);
    if (!pk_malloc(lisp, (size_t)length, (void **)&buf)) {
        fclose(f);
        return PK_Yield;
    }
    read = fread(buf, 1, (size_t)length, f);
    fclose(f);
    
    *out_c = buf;
    *out_length = read;
    
    return PK_Ok;
}

size_t pk_next_pow2(size_t value) {
    size_t result = value;
    result--;
    result |= result >> 1;
    result |= result >> 2;
    result |= result >> 4;
    result |= result >> 8;
    result |= result >> 16;
    result |= result >> 32;
    return result + 1;
}
