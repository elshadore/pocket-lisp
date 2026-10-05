#include "user_internals.h"

ULAtom ul_str_dupe(ULStr input, ULStr *output) {
    if (input.length == 0) {
        *output = UL_STR_EMPTY;
        return ul_true();
    }
    char *buffer = ul_malloc(input.length * sizeof(char));
    if (buffer == NULL) {
        return ul_error_oom();
    }
    
    (void)memcpy(buffer, input.c, input.length);
    *output = (ULStr) {
        .c = buffer,
        .length = input.length,
    };

    return ul_true();
}

bool ul_str_eq(ULStr a, ULStr b) {
    if (a.length != b.length) return false;
    return (bool)(memcmp(a.c, b.c, a.length) == 0);
}

void ul_str_reverse_mut(char *c, size_t length) {
    if (length <= 1) return;
    
    for (size_t i = 0, j = length - 1; i < j; i++, j--) {
        char temp = c[i];
        c[i] = c[j];
        c[j] = temp;
    }
}

ul_u8 ul_char_to_digit(char c) {
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

char ul_char_from_digit(ul_u8 integer) {
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

bool ul_char_is_digit(char c) {
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

bool ul_char_is_whitespace(char c) {
    switch (c) {
        case '\0': return true;
        case ' ': return true;
        case '\n': return true;
        case '\f': return true;
        case '\r': return true;
        case '\t': return true;
        case '\v': return true;
        default: return false;
    }
}

bool ul_char_is_alphabet(char c) {
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= 'a' && c <= 'z') return true;
    return false;
}

bool ul_char_is_symbol(char c) {
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
            if (ul_char_is_alphabet(c)) return true;
            if (ul_char_is_digit(c)) return true;
            return false;
        }
    }
}

char ul_char_from_hex(ul_u8 byte) {
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

bool ul_char_is_hex(char c) {
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
        case 'A': return true;
        case 'B': return true;
        case 'C': return true;
        case 'D': return true;
        case 'E': return true;
        case 'F': return true;
        case 'a': return true;
        case 'b': return true;
        case 'c': return true;
        case 'd': return true;
        case 'e': return true;
        case 'f': return true;
        default: return false;
    }
}

ul_u8 ul_char_to_hex(char c) {
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

size_t ul_hash_djb2(const char *c, size_t length) {
    size_t hash = 5381;
    size_t i = 0;
    for (i = 0; i < length; ++i) {
        hash = ((hash << 5) + hash) + (size_t)c[i];
    }
    return hash;
}
