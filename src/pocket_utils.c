#include "pocket_internals.h"

PKRes pk_string_dupe(Pocket lisp, PKString string, PKString *output) {
    if (string.length == 0) {
        *output = PK_STRING_EMPTY;
        return PK_Ok;
    }
    void *copy;
    pk_try(pk_malloc(lisp, string.length, &copy));
    memcpy(copy, string.c, string.length);
    *output = (PKString){ .c = copy, .length = string.length };
    return PK_Ok;
}

void pk_string_free(Pocket lisp, PKString string) {
    pk_free(lisp, string.c, string.length);
}

PKRes pk_string_from_cstr(char *cstr, PKString *output) {
    *output = (PKString){ .c = cstr, .length = strlen(cstr) };
    return PK_Ok;
}

bool pk_string_eq(PKString a, PKString b) {
    if (a.length != b.length) return false;
    return memcmp(a.c, b.c, a.length) == 0;
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

uint8_t pk_char_to_digit(char c) {
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

char pk_char_from_digit(uint8_t integer) {
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

bool pk_char_is_digit(char c) {
    switch (c) {
        case '0': return true;
        case '1': return true;
        case '2': return true;
        case '3': return true;
        case '4': return true;
        case '5': return true;
        case '6': return true;
        case '7': return true;
        case '8': return true;
        case '9': return true;
        default: return false;
    }
}

bool pk_char_is_whitespace(char c) {
    switch (c) {
        case ' ': return true;
        case '\n': return true;
        case '\f': return true;
        case '\r': return true;
        case '\t': return true;
        case '\v': return true;
        default: return false;
    }
}

bool pk_char_is_alphabet(char c) {
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= 'a' && c <= 'z') return true;
    return false;
}

bool pk_char_is_symbol(char c) {
    switch (c) {
        case '+': return true;
        case '-': return true;
        case '/': return true;
        case '*': return true;
        case '!': return true;
        case '?': return true;
        case '>': return true;
        case '<': return true;
        case '=': return true;
        case '_': return true;
        case '%': return true;
        default: {
            if (pk_char_is_alphabet(c)) return true;
            if (pk_char_is_digit(c)) return true;
            return false;
        }
    }
}

char pk_char_from_hex(uint8_t byte) {
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

bool pk_char_is_hex(char c) {
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
        case 'f': return true;
        default: return false;
    }
}

uint8_t pk_char_to_hex(char c) {
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

size_t pk_hash_djb2(char *c, size_t length) {
    size_t hash = 5381;
    for (size_t i = 0; i < length; ++i) {
        hash = ((hash << 5) + hash) + (size_t)c[i];
    }
    return hash;
}

size_t pk_hash_pointer(void *ptr) {
    uintptr_t p = (uintptr_t)ptr;
    return (size_t)(p >> 3);
}

PKRes pk_stack_dump(Pocket lisp, const char *tag) {
    int top = pk_get_top(lisp);
    PKWriter w = pk_writer_init(lisp);
    PKRes result = PK_Yield;
    pk_defer(pk_writer_printf(&w, "*~STACK-DUMP~* (tag = %s)\n", tag));
    for (int i = top; i > 0; i--) {
        int rel = pk_sp_relative(lisp, i);
        PKAtom *a;
        pk_defer(pk_stack_get(lisp, i, &a));
        pk_defer(pk_writer_printf(&w, "    [%d/%d] => ", rel, i));
        pk_defer(pk_writer_atom(&w, a));
        pk_defer(pk_writer_newline(&w));
    }
    pk_defer(pk_writer_print(&w));
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

PKRes pk_slurp(Pocket lisp, const char *file_path, PKString *output) {
    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        return pk_error(lisp);
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    if (length < 0) {
        fclose(f);
        return pk_error(lisp);
    }
    rewind(f);
    void *buf;
    pk_try(pk_malloc(lisp, (size_t)length, &buf));
    size_t read = fread(buf, 1, (size_t)length, f);
    fclose(f);
    *output = pk_string_new(buf, read);
    return PK_Ok;
}
