#include "pocket_internals.h"

PKStr pk_str_alloc(Pocket lisp, size_t length) {
    if (length == 0) {
        return PK_STR_EMPTY;
    }
    char *buffer = pk_malloc(lisp, length * sizeof(char));
    return (PKStr) {
        .c = buffer,
        .length = length,
    };
}

PKStr pk_str_dupe(Pocket lisp, PKStr input) {
    if (input.length == 0) {
        return PK_STR_EMPTY;
    }
    char *buffer = pk_malloc(lisp, input.length * sizeof(char));
    
    (void)memcpy(buffer, input.c, input.length);
    
    return (PKStr) {
        .c = buffer,
        .length = input.length,
    };
}

PKStr pk_str_from_cstr(char *cstr) {
    size_t length = strlen(cstr);
    return (PKStr) {
        .c = cstr,
        .length = length,
    };
}

bool pk_str_eq(PKStr a, PKStr b) {
    if (a.length != b.length) return false;
    if (a.length == 0) return true;
    return (bool)(memcmp(a.c, b.c, a.length) == 0);
}

void pk_str_reverse_mut(PKStr string) {
    if (string.length <= 1) return;
    
    for (size_t i = 0, j = string.length - 1; i < j; i++, j--) {
        char temp = string.c[i];
        string.c[i] = string.c[j];
        string.c[j] = temp;
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

bool pk_str_needs_bars(PKStr string) {
    if (string.length == 0) {
        return true;
    }
    for (size_t i = 0; i < string.length; ++i) {
        if (!pk_char_is_symbol(string.c[i])) {
            return true;
        }
    }
    return false;
}

typedef struct PKCharName_ {
    PKStr name;
    char c;
} PKCharName;

PKCharName PK_CHAR_NAMES[] = {
    { pk_str("null"),                0    },
    { pk_str("soh"),                 1    },
    { pk_str("stx"),                 2    },
    { pk_str("etx"),                 3    },
    { pk_str("eot"),                 4    },
    { pk_str("enq"),                 5    },
    { pk_str("ack"),                 6    },
    { pk_str("bell"),                7    },
    { pk_str("backspace"),           8    },
    { pk_str("tab"),                 9    },
    { pk_str("newline"),             10   },
    { pk_str("vertical-tab"),        11   },
    { pk_str("formfeed"),            12   },
    { pk_str("return"),              13   },
    { pk_str("shift-out"),           14   },
    { pk_str("shift-in"),            15   },
    { pk_str("data-link-escape"),    16   },
    { pk_str("device-control-1"),    17   },
    { pk_str("device-control-2"),    18   },
    { pk_str("device-control-3"),    19   },
    { pk_str("device-control-4"),    20   },
    { pk_str("negative-acknowledge"),21   },
    { pk_str("synchronous-idle"),    22   },
    { pk_str("end-of-transmission-block"), 23 },
    { pk_str("cancel"),              24   },
    { pk_str("end-of-medium"),       25   },
    { pk_str("substitute"),          26   },
    { pk_str("escape"),              27   },
    { pk_str("file-separator"),      28   },
    { pk_str("group-separator"),     29   },
    { pk_str("record-separator"),    30   },
    { pk_str("unit-separator"),      31   },
    { pk_str("space"),               32   },
    { pk_str("delete"),              127  },
};

bool pk_char_from_cname(PKStr input, char *output) {
    size_t length = pk_alen(PK_CHAR_NAMES);
    for (size_t i = 0; i < length; ++i) {
        PKCharName *entry = &PK_CHAR_NAMES[i];
        if (pk_str_eq(input, entry->name)) {
            *output = entry->c;
            return true;
        }
    }
    return false;
}

bool pk_char_to_cname(char c, PKStr *output) {
    size_t length = pk_alen(PK_CHAR_NAMES);
    for (size_t i = 0; i < length; ++i) {
        PKCharName *entry = &PK_CHAR_NAMES[i];
        if (entry->c == c) {
            *output = entry->name;
            return true;
        }
    }
    return false;
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

