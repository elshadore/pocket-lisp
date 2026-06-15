#include "pocket_internals.h"

PKString pk_string_dupe(Pocket lisp, PKString string) {
    char *copy = pk_malloc(lisp, string.length);
    memcpy(copy, string.c, string.length);
    return (PKString){ .c = copy, .length = string.length };
}

void pk_string_free(Pocket lisp, PKString string) {
    pk_free(lisp, string.c, string.length);
}

PKString pk_string_from_cstr(char *cstr) {
    return (PKString){ .c = cstr, .length = strlen(cstr) };
}

bool pk_string_eq(PKString a, PKString b) {
    if (a.length != b.length) return false;
    return memcmp(a.c, b.c, a.length) == 0;
}

void pk_print(Pocket lisp, char *c, size_t length) {
    (lisp->print)(lisp->user_env, c, length);
}

void pk_stack_dump(Pocket lisp) {
    int top = pk_get_top(lisp);
    PKWriter w = pk_writer_init(lisp);
    pk_writer_printf(&w, "*~STACK-DUMP~*\n");
    for (int i = top; i > 0; i--) {
        int rel = pk_sp_relative(lisp, i);
        pk_writer_printf(&w, "[%d/%d] => ", rel, i);
        pk_writer_atom(&w, pk_stack_get(lisp, i));
        pk_writer_newline(&w);
    }
    pk_writer_print(&w);
    pk_writer_deinit(&w);
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
