#include "pocket_internals.h"

PKAccum pk_accum_new(void) {
    return (PKAccum) {
        .head = pk_nil(),
        .tail = pk_nil(),
    };
}

PKAtom pk_accum_push(PKAccum *acc, PKAtom atom) {
    PKAtom cons = pk_cons(atom, pk_nil());

    pk_propagate(cons);
    
    if (pk_is_nil(acc->head)) {
        acc->head = cons;
        acc->tail = cons;
    } else {
        acc->tail->cons.cdr = cons;
        acc->tail = cons;
    }

    return cons;
}

PKAtom pk_str_dupe(PKStr input, PKStr *output) {
    if (input.length == 0) {
        *output = PK_STR_EMPTY;
        return pk_true();
    }
    char *buffer = pk_malloc(input.length * sizeof(char));
    if (buffer == NULL) {
        return pk_error_oom();
    }
    
    (void)memcpy(buffer, input.c, input.length);
    *output = (PKStr) {
        .c = buffer,
        .length = input.length,
    };

    return pk_true();
}

bool pk_str_eq(PKStr a, PKStr b) {
    if (a.length != b.length) return false;
    return (bool)(memcmp(a.c, b.c, a.length) == 0);
}

void pk_str_reverse_mut(char *c, size_t length) {
    if (length <= 1) return;
    
    for (size_t i = 0, j = length - 1; i < j; i++, j--) {
        char temp = c[i];
        c[i] = c[j];
        c[j] = temp;
    }
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

bool pk_char_is_hex(char c) {
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
