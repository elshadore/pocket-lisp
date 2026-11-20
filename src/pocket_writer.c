#include "pocket_internals.h"

void pk_writer_expand(Pocket lisp, size_t needed) {
    size_t new_count = lisp->writer.count + needed;
    if (new_count > lisp->writer.capacity) {
        size_t new_capacity = pk_grow_capacity(lisp->writer.capacity, 128);
        while (new_capacity < new_count) {
            new_capacity *= 2;
        }
        char *new_c = pk_realloc(lisp, lisp->writer.c, new_capacity * sizeof(char));
        lisp->writer.c = new_c;
        lisp->writer.capacity = new_capacity;
    }
}
    
void pk_writer_reset(Pocket lisp) {
    lisp->writer.count = 0;
}

void pk_writer_destroy(Pocket lisp) {
    pk_free(lisp, lisp->writer.c);
    lisp->writer = PK_WRITER_EMPTY;
}

void pk_writer_char(Pocket lisp, char c) {
    pk_writer_expand(lisp, 1);
    lisp->writer.c[lisp->writer.count++] = c;
}

void pk_writer_string(Pocket lisp, PKStr string) {
    if (string.length == 0) {
        return;
    }
    pk_writer_expand(lisp, string.length);
    memcpy(lisp->writer.c + lisp->writer.count, string.c, string.length);
    lisp->writer.count += string.length;
}

void pk_writer_cstr(Pocket lisp, const char *cstr) {
    size_t length = strlen(cstr);
    if (length == 0) {
        return;
    }
    pk_writer_expand(lisp, length);
    memcpy(lisp->writer.c + lisp->writer.count, cstr, length);
    lisp->writer.count += length;
}

void pk_writer_string_escaped(Pocket lisp, PKStr string) {
    for (size_t i = 0; i < string.length; ++i) {
        char c = string.c[i];
        switch (c) {
            case '"': {
                pk_writer_string(lisp, pk_str("\\\""));
                break;
            }
            case '\\': {
                pk_writer_string(lisp, pk_str("\\\\"));
                break;
            }
            case '\n': {
                pk_writer_string(lisp, pk_str("\\n"));
                break;
            }
            case '\t': {
                pk_writer_string(lisp, pk_str("\\t"));
                break;
            }
            case '\r': {
                pk_writer_string(lisp, pk_str("\\r"));
                break;
            }
            default: {
                pk_writer_char(lisp, c);
                break;
            }
        }
    }
}

void pk_writer_vfmt(Pocket lisp, const char *fmt, va_list args) {
    va_list measure;

    va_copy(measure, args);
    size_t length = (size_t)vsnprintf(NULL, 0, fmt, measure);
    va_end(measure);

    pk_writer_expand(lisp, length + 1);

    (void)vsnprintf(lisp->writer.c + lisp->writer.count, length + 1, fmt, args);

    lisp->writer.count += length;
}

PK_PRINTF_FMT(2, 3) void pk_writer_fmt(Pocket lisp, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    pk_writer_vfmt(lisp, fmt, args);
    va_end(args);
}

void pk_writer_int(Pocket lisp, int integer) {
    unsigned int dec = 0;
    size_t save = 0;
    
    if (integer < 0) {
        dec = (unsigned int)-integer;
        pk_writer_char(lisp, '-');
    } else {
        dec = (unsigned int)integer;
    }
    
    save = lisp->writer.count;
    
    do {
        int digit = dec % 10;
        pk_writer_char(lisp, pk_char_from_digit((pk_u8)digit));
        
    } while ((dec /= 10) > 0);

    PKStr str = (PKStr){
        .c = lisp->writer.c + save,
        .length = lisp->writer.count - save
    };
    pk_str_reverse_mut(str);
}

void pk_writer_address(Pocket lisp, size_t address) {
    int i = 0;
    pk_writer_string(lisp, pk_str("0x"));
    
    for (i = (int)(sizeof(size_t) * 2) - 1; i >= 0; i--) {
        pk_u8 nibble = (pk_u8)((address >> (i * 4)) & 0xf);
        pk_writer_char(lisp, pk_char_from_hex(nibble));
    }
}

void pk_writer_newline(Pocket lisp) {
    pk_writer_char(lisp, '\n');
}

PKStr pk_writer_slice(Pocket lisp) {
    return (PKStr) {
        .c = lisp->writer.c,
        .length = lisp->writer.count
    };
}

void pk_writer_simple_object(Pocket lisp, void *obj, PKStr name) {
    pk_writer_string(lisp, pk_str("&<"));
    pk_writer_string(lisp, name);
    pk_writer_string(lisp, pk_str("::"));
    pk_writer_address(lisp, (size_t)obj);
    pk_writer_char(lisp, '>');
}

void pk_writer_simple_function(Pocket lisp, void *obj, PKStr type, PKArity arity) {
    pk_writer_string(lisp, pk_str("&<"));
    pk_writer_string(lisp, type);
    pk_writer_string(lisp, pk_str(":"));
    pk_writer_fmt(lisp, "%u", arity.args);
    pk_writer_string(lisp, pk_str(":"));
    pk_writer_string(lisp, pk_ident_arity(arity.type));
    pk_writer_string(lisp, pk_str("::"));
    pk_writer_address(lisp, (size_t)obj);
    pk_writer_char(lisp, '>');
}

void pk_writer_symbol(Pocket lisp, PKStr ident) {
    if (!pk_str_needs_bars(ident)) {
        pk_writer_string(lisp, ident);
        return;
    }
    
    pk_writer_char(lisp, '|');
    for (size_t i = 0; i < ident.length; ++i) {
        char c = ident.c[i];
        if (c == '|') {
            pk_writer_string(lisp, pk_str("\\|"));
        } else if (c == '\\') {
            pk_writer_string(lisp, pk_str("\\\\"));
        } else if (c == '\n') {
            pk_writer_string(lisp, pk_str("\\n"));
        } else {
            pk_writer_char(lisp, c);
        }
    }
    pk_writer_char(lisp, '|');
}

void pk_writer_atom_recursive(Pocket lisp, PKAtom *atom);

void pk_writer_write_atom(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_UNDEFINED: {
            pk_writer_string(lisp, pk_str("&<Undefined>"));
            break;
        }
        case PK_ATOMTY_NIL: {
            pk_writer_string(lisp, pk_str("nil"));
            break;
        }
        case PK_ATOMTY_TRUE: {
            pk_writer_string(lisp, pk_str("true"));
            break;
        }
        case PK_ATOMTY_NUMBER: {
            if (atom->number.num_type == PK_NUMTY_INT) {
                pk_writer_fmt(lisp, "%d", atom->number.as.i);
            } else {
                pk_writer_fmt(lisp, "%f", atom->number.as.f);
            }
            break;
        }
        case PK_ATOMTY_STRING: {
            pk_writer_char(lisp, '"');
            pk_writer_string_escaped(lisp, atom->string.str);
            pk_writer_char(lisp, '"');
            break;
        }
        case PK_ATOMTY_SYMBOL: {
            pk_writer_symbol(lisp, atom->symbol.name->str);
            break;
        }
        case PK_ATOMTY_KEYWORD: {
            pk_writer_char(lisp, ':');
            pk_writer_symbol(lisp, atom->keyword.id);
            break;
        }
        case PK_ATOMTY_CONS: {
            pk_writer_char(lisp, '(');
            (void)pk_hashmap_insert(lisp, &lisp->cloner, atom, atom, PK_HASHMAP_INSERT_NORMAL);

            pk_writer_atom_recursive(lisp, atom->cons.car);

            PKAtom *iter = atom->cons.cdr;
            bool cyclic = false;
            while (pk_atom_is_cons(iter)) {
                PKAtom *seen = pk_hashmap_get(lisp, &lisp->cloner, iter);
                if (!pk_atom_is_undefined(seen)) {
                    pk_writer_char(lisp, ' ');
                    cyclic = true;
                    break;
                }
                (void)pk_hashmap_insert(lisp, &lisp->cloner, iter, iter, PK_HASHMAP_INSERT_NORMAL);
                pk_writer_char(lisp, ' ');
                pk_writer_atom_recursive(lisp, iter->cons.car);
                iter = iter->cons.cdr;
            }
            if (cyclic) {
                pk_writer_simple_object(lisp, iter, pk_str("Cons"));
            } else if (!pk_atom_is_nil(iter)) {
                pk_writer_string(lisp, pk_str(" . "));
                pk_writer_atom_recursive(lisp, iter);
            }

            pk_writer_char(lisp, ')');
            break;
        }
        case PK_ATOMTY_CFUN: {
            pk_writer_simple_function(lisp, atom, pk_str("Function::c"), atom->cfun.arity);
            break;
        }
        case PK_ATOMTY_LFUN: {
            pk_writer_simple_function(lisp, atom, pk_str("Function::lisp"), atom->lfun.arity);
            break;
        }
        case PK_ATOMTY_CMACRO: {
            pk_writer_simple_function(lisp, atom, pk_str("Macro::c"), atom->cmacro.arity);
            break;
        }
        case PK_ATOMTY_LMACRO: {
            pk_writer_simple_function(lisp, atom, pk_str("Macro::lisp"), atom->lmacro.arity);
            break;
        }
        case PK_ATOMTY_ARRAY: {
            PKArray *array = &atom->array.atoms;
            pk_writer_string(lisp, pk_str("&<Array::("));
            (void)pk_hashmap_insert(lisp, &lisp->cloner, atom, atom, PK_HASHMAP_INSERT_NORMAL);
            for (size_t i = 0; i < array->count; ++i) {
                if (i != 0) {
                    pk_writer_char(lisp, ' ');
                }
                pk_writer_atom_recursive(lisp, array->e[i]);
            }
            pk_writer_string(lisp, pk_str(")>"));
            break;
        }
        case PK_ATOMTY_OBJECT: {
            PKAtomSymbol *type = atom->object.type;
            PKHashmap *map = &atom->object.props;
            pk_writer_string(lisp, pk_str("&<"));
            pk_writer_symbol(lisp, type->name->str);
            pk_writer_string(lisp, pk_str("::"));
            pk_writer_char(lisp, '(');
            (void)pk_hashmap_insert(lisp, &lisp->cloner, atom, atom, PK_HASHMAP_INSERT_NORMAL);
            bool first = true;
            for (size_t i = 0; i < map->capacity; ++i) {
                PKHashmapSlot *slot = &map->e[i];
                if (slot->state == PK_HASHMAP_SLOT_OCCUPIED) {
                    if (!first) {
                        pk_writer_char(lisp, ' ');
                    }
                    pk_writer_atom_recursive(lisp, slot->key);
                    pk_writer_char(lisp, ' ');
                    pk_writer_atom_recursive(lisp, slot->value);
                    first = false;
                }
            }
            pk_writer_string(lisp, pk_str(")>"));
            break;
        }
        case PK_ATOMTY_DESTROYED: {
            pk_writer_simple_object(lisp, atom, pk_str("Destroyed"));
            break;
        }
        default: {
            pk_writer_simple_object(lisp, atom, pk_str("Unknown"));
            break;
        }
    }
}

void pk_writer_atom_recursive(Pocket lisp, PKAtom *atom) {
    if (pk_atom_kind(atom) == PK_ATOMKIND_VALUE) {
        pk_writer_write_atom(lisp, atom);
        return;
    }

    PKAtom *get = pk_hashmap_get(lisp, &lisp->cloner, atom);
    if (pk_atom_is_undefined(get)) {
        (void)pk_hashmap_insert(lisp, &lisp->cloner, atom, atom, PK_HASHMAP_INSERT_NORMAL);
        pk_writer_write_atom(lisp, atom);
    } else {
        pk_writer_simple_object(lisp, atom, pk_ident_atomty(atom->header.type));
    }
}

void pk_writer_atom(Pocket lisp, PKAtom *atom) {
    pk_hashmap_deinit(lisp, &lisp->cloner);
    pk_writer_write_atom(lisp, atom);
    pk_hashmap_deinit(lisp, &lisp->cloner);
}
