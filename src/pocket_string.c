#include "pocket_internals.h"

PKAtomString *pk_atom_string_nomemcpy(Pocket lisp, PKStr string) {
    if (string.length == 0) {
        return &PK_ATOM_EMPTY_STRING;
    }

    PKAtomString *result = (PKAtomString *)pk_atom_alloc(lisp);
    
    result->header.type = PK_ATOMTY_STRING;
    result->str = string;
    
    return result;
}

PKAtomString *pk_atom_string(Pocket lisp, PKStr string) {
    if (string.length == 0) {
        return &PK_ATOM_EMPTY_STRING;
    }

    if (string.length == 1) {
        pk_u8 c = (pk_u8)string.c[0];
        if (c < PK_ATOM_STRING_CACHE_LENGTH) {
            return &PK_ATOM_STRING_CACHE[c];
        }
    }

    PKStr dupe = pk_str_dupe(lisp, string);
    
    return pk_atom_string_nomemcpy(lisp, dupe);
}

PKAtomString *pk_atom_string_char(Pocket lisp, char c) {
    PKStr str = (PKStr) {
        .c = &c,
        .length = 1,
    };
    return pk_atom_string(lisp, str);
}

PKAtomSymbol *pk_atom_string_to_symbol(Pocket lisp, PKAtomString *string) {
    return pk_atom_symbol(lisp, string->str);
}

PKAtomKeyword *pk_atom_string_to_keyword(Pocket lisp, PKAtomString *string) {
    return pk_atom_keyword(lisp, string->str);
}

PKAtomBlob *pk_atom_string_to_bytes(Pocket lisp, PKAtomString *string) {
    return pk_atom_blob(lisp, (PKBlob) {
        .e = (pk_u8 *)string->str.c,
        .length = string->str.length,
    });
}

PKAtomString *pk_atom_string_alloc(Pocket lisp, size_t length) {
    if (length == 0) {
        return &PK_ATOM_EMPTY_STRING;
    }

    PKStr buffer = pk_str_alloc(lisp, length);
    
    return pk_atom_string_nomemcpy(lisp, buffer);
}

PKAtomString *pk_atom_stringc(Pocket lisp, char *cstring) {
    PKStr string = pk_str_from_cstr(cstring);
    return pk_atom_string(lisp, string);
}

PKAtomString *pk_atom_string_cat(Pocket lisp, PKAtoms atoms) {
    pk_writer_reset(lisp);
    
    for (size_t i = 0; i < atoms.length; ++i) {
        PKAtomString *string = pk_atom_assert_string(lisp, atoms.e[i]);
        pk_writer_string(lisp, string->str);
    }
    
    PKAtomString *result = pk_atom_string(lisp, pk_writer_str(lisp));
    pk_writer_reset(lisp);
    return result;
}

PKAtomString *pk_atom_string_reverse(Pocket lisp, PKStr input) {
    PKStr dupe = pk_str_dupe(lisp, input);
    pk_str_reverse_mut(dupe);
    return pk_atom_string_nomemcpy(lisp, dupe);
}

PKAtom *pk_atom_string_split(Pocket lisp, PKAtomString *string, PKAtomString *pattern) {
    PKStr source = string->str;
    PKStr sep = pattern->str;
    if (sep.length == 0) {
        return (PKAtom *)pk_atom_list1(lisp, (PKAtom *)string);
    }

    PKAccum acc = pk_accum_new(lisp);
    size_t start = 0;
    size_t i = 0;

    while (i + sep.length <= source.length) {
        PKStr window = {
            .c = source.c + i,
            .length = sep.length,
        };
        if (pk_str_eq(window, sep)) {
            if (i > start) {
                pk_accum_push(&acc, (PKAtom *)pk_atom_string(lisp, (PKStr) {
                    .c = source.c + start,
                    .length = i - start,
                }));
            }
            i += sep.length;
            start = i;
        } else {
            ++i;
        }
    }

    if (start < source.length) {
        pk_accum_push(&acc, (PKAtom *)pk_atom_string(lisp, (PKStr) {
            .c = source.c + start,
            .length = source.length - start,
        }));
    }

    return acc.head;
}

PKAtomString *pk_atom_string_el(Pocket lisp, PKAtomString *string, size_t index) {
    if (index >= string->str.length) {
        pk_errorf(lisp, "index: %zu, longer than the string length: %zu", index, string->str.length);
        return &PK_ATOM_EMPTY_STRING;
    }
    PKStr str = (PKStr) {
        .c = &string->str.c[index],
        .length = 1,
    };
    return pk_atom_string(lisp, str);
}

size_t pk_atom_string_length(Pocket lisp, PKAtomString *string) {
    (void)lisp;
    return string->str.length;
}

bool pk_atom_is_character(PKAtom *atom) {
    if (pk_atom_is_string(atom)) {
        return atom->string.str.length == 1;
    } else {
        return false;
    }
}

bool pk_atom_is_string(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_STRING;
}

bool pk_atom_string_eq(Pocket lisp, PKAtomString *lhs, PKAtomString *rhs) {
    (void)lisp;
    return pk_str_eq(lhs->str, rhs->str);
}

PKAtomString *pk_atom_assert_string(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_string(atom)) {
        pk_error_type(lisp, atom, PK_TYPE_STRING);
    }
    return (PKAtomString *)atom;
}

PKAtomString *pk_atom_assert_string_nil_is_empty(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_nil(atom)) {
        return &PK_ATOM_EMPTY_STRING;
    } else {
        return pk_atom_assert_string(lisp, atom);
    }
}

bool pk_atom_string_cmp(PKAtomString *string, PKStr comp) {
    return pk_str_eq(string->str, comp);
}

bool pk_atom_string_all_chars(PKAtomString *string, bool (*predicate)(char)) {
    for (size_t i = 0; i < string->str.length; ++i) {
        if (!predicate(string->str.c[i])) {
            return false;
        }
    }
    return true;
}

bool pk_char_is_alphanumeric(char c) {
    return pk_char_is_alphabet(c) || pk_char_is_digit(c);
}

bool pk_char_is_graphic(char c) {
    pk_u8 byte = (pk_u8)c;
    return (byte >= 0x21) && (byte <= 0x7e);
}

PKAtomString *pk_atom_string_make(Pocket lisp, size_t count, PKAtomString *fill) {
    pk_writer_reset(lisp);

    for (size_t i = 0; i < count; ++i) {
        pk_writer_string(lisp, fill->str);
    }

    PKAtomString *result = pk_atom_string(lisp, pk_writer_str(lisp));
    pk_writer_reset(lisp);
    return result;
}

bool pk_atom_string_is_numeric(PKAtomString *string) {
    return pk_atom_string_all_chars(string, pk_char_is_digit);
}

bool pk_atom_string_is_alphabetic(PKAtomString *string) {
    return pk_atom_string_all_chars(string, pk_char_is_alphabet);
}

bool pk_atom_string_is_whitespace(PKAtomString *string) {
    return pk_atom_string_all_chars(string, pk_char_is_whitespace);
}

bool pk_atom_string_is_alphanumeric(PKAtomString *string) {
    return pk_atom_string_all_chars(string, pk_char_is_alphanumeric);
}

bool pk_atom_string_is_graphic(PKAtomString *string) {
    return pk_atom_string_all_chars(string, pk_char_is_graphic);
}

bool pk_atom_string_is_upcase(PKAtomString *string) {
    for (size_t i = 0; i < string->str.length; ++i) {
        char c = string->str.c[i];
        if (c >= 'a' && c <= 'z') {
            return false;
        }
    }
    return true;
}

bool pk_atom_string_is_downcase(PKAtomString *string) {
    for (size_t i = 0; i < string->str.length; ++i) {
        char c = string->str.c[i];
        if (c >= 'A' && c <= 'Z') {
            return false;
        }
    }
    return true;
}

PKAtomString *pk_atom_string_upcase(Pocket lisp, PKAtomString *string) {
    PKAtomString *result = pk_atom_string_alloc(lisp, string->str.length);

    for (size_t i = 0; i < string->str.length; ++i) {
        char c = string->str.c[i];
        if (c >= 'a' && c <= 'z') {
            result->str.c[i] = (char)(c - ('a' - 'A'));
        } else {
            result->str.c[i] = c;
        }
    }

    return result;
}

PKAtomString *pk_atom_string_downcase(Pocket lisp, PKAtomString *string) {
    PKAtomString *result = pk_atom_string_alloc(lisp, string->str.length);

    for (size_t i = 0; i < string->str.length; ++i) {
        char c = string->str.c[i];
        if (c >= 'A' && c <= 'Z') {
            result->str.c[i] = (char)(c + ('a' - 'A'));
        } else {
            result->str.c[i] = c;
        }
    }

    return result;
}

PKAtomString *pk_atom_string_map(Pocket lisp, PKAtomString *string, PKAtom *function) {
    pk_frame_push(lisp, &PK_ATOM_NIL, 0);

    for (size_t i = 0; i < string->str.length; ++i) {
        PKAtomString *character = pk_atom_string_char(lisp, string->str.c[i]);
        pk_stack_push(lisp, (PKAtom *)character);
        pk_stack_push(lisp, function);
        pk_funcall(lisp, PK_CALL_FUN, 1);
        PKAtom *result = pk_stack_head_pop(lisp);
        PKAtomString *mapped = pk_generic_to_string(lisp, result);
        pk_stack_push(lisp, (PKAtom *)pk_atom_string(lisp, mapped->str));
    }

    PKAtomString *result = pk_atom_string_cat(lisp, pk_stack_slice(lisp));
    pk_frame_unwind(lisp);
    return result;
}

PKAtomString *pk_atom_string_filter(Pocket lisp, PKAtomString *string, PKAtom *function) {
    pk_frame_push(lisp, &PK_ATOM_NIL, 0);

    for (size_t i = 0; i < string->str.length; ++i) {
        PKAtomString *character = pk_atom_string_char(lisp, string->str.c[i]);
        pk_stack_push(lisp, (PKAtom *)character);
        pk_stack_push(lisp, function);
        pk_funcall(lisp, PK_CALL_FUN, 1);
        PKAtom *result = pk_stack_head_pop(lisp);
        if (pk_atom_is_true(result)) {
            pk_stack_push(lisp, (PKAtom *)character);
        }
    }

    PKAtomString *result = pk_atom_string_cat(lisp, pk_stack_slice(lisp));
    pk_frame_unwind(lisp);
    return result;
}
