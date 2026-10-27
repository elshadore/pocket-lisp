#include "pocket_internals.h"

void pk_writer_deinit(Pocket lisp, PKWriter *w) {
    pk_free(lisp, w->c);
    *w = PK_WRITER_EMPTY;
}

void pk_writer_expand(Pocket lisp, PKWriter *w, size_t needed) {
    size_t new_count = w->count + needed;
    if (new_count > w->capacity) {
        size_t new_capacity = pk_grow_capacity(w->capacity, 128);
        while (new_capacity < new_count) {
            new_capacity *= 2;
        }
        char *new_c = pk_realloc(lisp, w->c, new_capacity * sizeof(char));
        w->c = new_c;
        w->capacity = new_capacity;
    }
}

void pk_writer_char(Pocket lisp, PKWriter *w, char c) {
    pk_writer_expand(lisp, w, 1);
    w->c[w->count++] = c;
}

void pk_writer_cstr(Pocket lisp, PKWriter *w, const char *cstr) {
    size_t length = strlen(cstr);
    if (length == 0) {
        return;
    }
    pk_writer_expand(lisp, w, length);
    memcpy(w->c + w->count, cstr, length);
    w->count += length;
}

void pk_writer_string(Pocket lisp, PKWriter *w, PKStr string) {
    pk_writer_expand(lisp, w, string.length);
    memcpy(w->c + w->count, string.c, string.length);
    w->count += string.length;
}

void pk_writer_string_escaped(Pocket lisp, PKWriter *w, PKStr string) {
    for (size_t i = 0; i < string.length; ++i) {
        char c = string.c[i];
        switch (c) {
            case '"': {
                pk_writer_string(lisp, w, pk_str("\\\""));
                break;
            }
            case '\\': {
                pk_writer_string(lisp, w, pk_str("\\\\"));
                break;
            }
            case '\n': {
                pk_writer_string(lisp, w, pk_str("\\n"));
                break;
            }
            case '\t': {
                pk_writer_string(lisp, w, pk_str("\\t"));
                break;
            }
            case '\r': {
                pk_writer_string(lisp, w, pk_str("\\r"));
                break;
            }
            default: {
                pk_writer_char(lisp, w, c);
                break;
            }
        }
    }
}

void pk_writer_vfmt(Pocket lisp, PKWriter *w, const char *fmt, va_list args) {
    va_list measure;

    va_copy(measure, args);
    size_t length = (size_t)vsnprintf(NULL, 0, fmt, measure);
    va_end(measure);

    pk_writer_expand(lisp, w, length + 1);

    (void)vsnprintf(w->c + w->count, length + 1, fmt, args);

    w->count += length;
}

PK_PRINTF_FMT(3, 4) void pk_writer_fmt(Pocket lisp, PKWriter *w, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    pk_writer_vfmt(lisp, w, fmt, args);
    va_end(args);
}

void pk_writer_int(Pocket lisp, PKWriter *w, int integer) {
    unsigned int dec = 0;
    size_t save = 0;
    
    if (integer < 0) {
        dec = (unsigned int)-integer;
        pk_writer_char(lisp, w, '-');
    } else {
        dec = (unsigned int)integer;
    }
    
    save = w->count;
    
    do {
        int digit = dec % 10;
        pk_writer_char(lisp, w, pk_char_from_digit((pk_u8)digit));
        
    } while ((dec /= 10) > 0);

    PKStr str = (PKStr){
        .c = w->c + save,
        .length = w->count - save
    };
    pk_str_reverse_mut(str);
}

void pk_writer_address(Pocket lisp, PKWriter *w, size_t address) {
    int i = 0;
    pk_writer_string(lisp, w, pk_str("0x"));
    
    for (i = (int)(sizeof(size_t) * 2) - 1; i >= 0; i--) {
        pk_u8 nibble = (pk_u8)((address >> (i * 4)) & 0xf);
        pk_writer_char(lisp, w, pk_char_from_hex(nibble));
    }
}

void pk_writer_newline(Pocket lisp, PKWriter *w) {
    pk_writer_char(lisp, w, '\n');
}

PKStr pk_writer_slice(PKWriter *w) {
    return (PKStr) {
        .c = w->c,
        .length = w->count
    };
}

void pk_writer_reset(PKWriter *w) {
    w->count = 0;
}

void pk_writer_simple_object(Pocket lisp, PKWriter *w, void *obj, PKStr name) {
    pk_writer_string(lisp, w, pk_str("&<"));
    pk_writer_string(lisp, w, name);
    pk_writer_string(lisp, w, pk_str("::"));
    pk_writer_address(lisp, w, (size_t)obj);
    pk_writer_char(lisp, w, '>');
}

void pk_writer_simple_function(Pocket lisp, PKWriter *w, void *obj, PKStr type, PKArity arity) {
    pk_writer_string(lisp, w, pk_str("&<"));
    pk_writer_string(lisp, w, type);
    pk_writer_string(lisp, w, pk_str("::"));
    pk_writer_fmt(lisp, w, "%u", arity.args);
    pk_writer_string(lisp, w, pk_str(":"));
    pk_writer_string(lisp, w, pk_ident_arity(arity.type));
    pk_writer_string(lisp, w, pk_str("::"));
    pk_writer_address(lisp, w, (size_t)obj);
    pk_writer_char(lisp, w, '>');
}

void pk_writer_symbol_name(Pocket lisp, PKWriter *w, PKStr name) {
    if (!pk_str_needs_bars(name)) {
        pk_writer_string(lisp, w, name);
        return;
    }
    
    pk_writer_char(lisp, w, '|');
    for (size_t i = 0; i < name.length; ++i) {
        char c = name.c[i];
        if (c == '|') {
            pk_writer_string(lisp, w, pk_str("\\|"));
        } else if (c == '\\') {
            pk_writer_string(lisp, w, pk_str("\\\\"));
        } else if (c == '\n') {
            pk_writer_string(lisp, w, pk_str("\\n"));
        } else {
            pk_writer_char(lisp, w, c);
        }
    }
    pk_writer_char(lisp, w, '|');
}

void pk_writer_atom_char(Pocket lisp, PKWriter *w, char c) {
    pk_writer_string(lisp, w, pk_str("^\\"));
    PKStr name = PK_STR_EMPTY;
    if (pk_char_to_cname(c, &name)) {
        pk_writer_string(lisp, w, name);
    } else {
        pk_writer_char(lisp, w, c);
    }
}

void pk_writer_atom_blob(Pocket lisp, PKWriter *w, PKBlob blob) {
    pk_writer_char(lisp, w, '[');
    if (blob.length > 0) {
        for (size_t i = 0; i < (blob.length - 1); ++i) {
            pk_writer_fmt(lisp, w, "%u ", blob.e[i]);
        }
        pk_writer_fmt(lisp, w, "%u", blob.e[blob.length - 1]);
    }
    pk_writer_char(lisp, w, ']');
}

void pk_writer_atom_recursive(Pocket lisp, PKWriter *w, PKHashmap *clones, PKAtom *atom);

void pk_writer_atom_(Pocket lisp, PKWriter *w, PKHashmap *clones, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_UNDEFINED: {
            pk_writer_string(lisp, w, pk_str("&<Undefined>"));
            break;
        }
        case PK_ATOMTY_NIL: {
            pk_writer_string(lisp, w, pk_str("nil"));
            break;
        }
        case PK_ATOMTY_TRUE: {
            pk_writer_string(lisp, w, pk_str("true"));
            break;
        }
        case PK_ATOMTY_NUMBER: {
            if (atom->number.num_type == PK_NUMTY_INT) {
                pk_writer_fmt(lisp, w, "%d", atom->number.as.i);
            } else {
                pk_writer_fmt(lisp, w, "%f", atom->number.as.f);
            }
            break;
        }
        case PK_ATOMTY_STRING: {
            pk_writer_char(lisp, w, '"');
            pk_writer_string_escaped(lisp, w, atom->string.str);
            pk_writer_char(lisp, w, '"');
            break;
        }
        case PK_ATOMTY_SYMBOL: {
            pk_writer_symbol_name(lisp, w, atom->symbol.name->str);
            break;
        }
        case PK_ATOMTY_CHARACTER: {
            pk_writer_atom_char(lisp, w, atom->character.c);
            break;
        }
        case PK_ATOMTY_KEYWORD: {
            pk_writer_char(lisp, w, ':');
            pk_writer_symbol_name(lisp, w, atom->keyword.id);
            break;
        }
        case PK_ATOMTY_CONS: {
            pk_writer_char(lisp, w, '(');
            (void)pk_hashmap_insert(lisp, clones, atom, atom, PK_HASHMAP_INSERT_NORMAL);

            pk_writer_atom_recursive(lisp, w, clones, atom->cons.car);

            PKAtom *iter = atom->cons.cdr;
            bool cyclic = false;
            while (pk_atom_is_cons(iter)) {
                PKAtom *seen = pk_hashmap_get(lisp, clones, iter);
                if (!pk_atom_is_undefined(seen)) {
                    pk_writer_char(lisp, w, ' ');
                    cyclic = true;
                    break;
                }
                (void)pk_hashmap_insert(lisp, clones, iter, iter, PK_HASHMAP_INSERT_NORMAL);
                pk_writer_char(lisp, w, ' ');
                pk_writer_atom_recursive(lisp, w, clones, iter->cons.car);
                iter = iter->cons.cdr;
            }
            if (cyclic) {
                pk_writer_simple_object(lisp, w, iter, pk_str("Cons"));
            } else if (!pk_atom_is_nil(iter)) {
                pk_writer_string(lisp, w, pk_str(" . "));
                pk_writer_atom_recursive(lisp, w, clones, iter);
            }

            pk_writer_char(lisp, w, ')');
            break;
        }
        case PK_ATOMTY_CFUN: {
            pk_writer_simple_function(lisp, w, atom, pk_str("Function"), atom->cfun.arity);
            break;
        }
        case PK_ATOMTY_LFUN: {
            pk_writer_simple_function(lisp, w, atom, pk_str("Function"), atom->lfun.arity);
            break;
        }
        case PK_ATOMTY_CMACRO: {
            pk_writer_simple_function(lisp, w, atom, pk_str("Macro"), atom->cmacro.arity);
            break;
        }
        case PK_ATOMTY_LMACRO: {
            pk_writer_simple_function(lisp, w, atom, pk_str("Macro"), atom->lmacro.arity);
            break;
        }
        case PK_ATOMTY_BLOB: {
            pk_writer_atom_blob(lisp, w, atom->blob.bytes);
            break;
        }
        case PK_ATOMTY_ARRAY: {
            PKArray *array = &atom->array.atoms;
            pk_writer_string(lisp, w, pk_str("&<"));
            pk_writer_string(lisp, w, pk_str("Array::"));
            pk_writer_char(lisp, w, '(');
            (void)pk_hashmap_insert(lisp, clones, atom, atom, PK_HASHMAP_INSERT_NORMAL);
            for (size_t i = 0; i < array->count; ++i) {
                if (i != 0) {
                    pk_writer_char(lisp, w, ' ');
                }
                pk_writer_atom_recursive(lisp, w, clones, array->e[i]);
            }
            pk_writer_string(lisp, w, pk_str(")>"));
            break;
        }
        case PK_ATOMTY_STRING_ARRAY: {
            pk_writer_string(lisp, w, pk_str("&<String-Array::"));
            pk_writer_string_escaped(lisp, w, pk_writer_slice(&atom->string_array.string));
            pk_writer_string(lisp, w, pk_str(">"));
            break;
        }
        case PK_ATOMTY_BYTE_ARRAY: {
            pk_writer_string(lisp, w, pk_str("&<Byte-Array::"));
            pk_writer_atom_blob(lisp, w, pk_bytes_slice(&atom->byte_array.bytes));
            pk_writer_string(lisp, w, pk_str(">"));
            break;
        }
        case PK_ATOMTY_TABLE: {
            PKHashmap *map = &atom->table.map;
            pk_writer_string(lisp, w, pk_str("&<"));
            pk_writer_string(lisp, w, pk_str("Table::"));
            pk_writer_char(lisp, w, '(');
            (void)pk_hashmap_insert(lisp, clones, atom, atom, PK_HASHMAP_INSERT_NORMAL);
            bool first = true;
            for (size_t i = 0; i < map->capacity; ++i) {
                PKHashmapSlot *slot = &map->e[i];
                if (slot->state == PK_HASHMAP_SLOT_OCCUPIED) {
                    if (!first) {
                        pk_writer_char(lisp, w, ' ');
                    }
                    first = false;
                    pk_writer_char(lisp, w, '(');
                    pk_writer_atom_recursive(lisp, w, clones, slot->key);
                    pk_writer_string(lisp, w, pk_str(" . "));
                    pk_writer_atom_recursive(lisp, w, clones, slot->value);
                    pk_writer_char(lisp, w, ')');
                }
            }
            pk_writer_string(lisp, w, pk_str(")>"));
            break;
        }
        case PK_ATOMTY_COBJECT: {
            pk_writer_simple_object(lisp, w, atom, atom->cobject.type->name->str);
            break;
        }
        case PK_ATOMTY_LOBJECT: {
            PKAtomSymbol *type = atom->lobject.type;
            PKAtoms slots = atom->lobject.slots;
            pk_writer_string(lisp, w, pk_str("&<"));
            // TODO: this produces potentially unreadable symbols. Not barred symbols |foo bar|.
            // However we will keep this for now until we actually start using symbol types.
            // Same can be said for cobject.
            pk_writer_string(lisp, w, type->name->str);
            pk_writer_string(lisp, w, pk_str("::"));
            pk_writer_char(lisp, w, '(');
            (void)pk_hashmap_insert(lisp, clones, atom, atom, PK_HASHMAP_INSERT_NORMAL);
            size_t printed = 0;
            for (size_t index = 0; index < slots.length; ++index) {
                PKAtom *symbol = NULL;
                for (size_t i = 0; i < type->slots.capacity; ++i) {
                    PKSlotmapSlot *slot = &type->slots.e[i];
                    if ((slot->state == PK_SLOTMAP_SLOT_OCCUPIED) && (slot->index == index)) {
                        symbol = slot->symbol;
                        break;
                    }
                }
                if (symbol == NULL) {
                    continue;
                }
                if (printed != 0) {
                    pk_writer_char(lisp, w, ' ');
                }
                printed += 1;
                pk_writer_char(lisp, w, '(');
                pk_writer_atom_recursive(lisp, w, clones, (PKAtom *)symbol);
                pk_writer_string(lisp, w, pk_str(" . "));
                pk_writer_atom_recursive(lisp, w, clones, slots.e[index]);
                pk_writer_char(lisp, w, ')');
            }
            pk_writer_string(lisp, w, pk_str(")>"));
            break;
        }
        case PK_ATOMTY_STREAM: {
            pk_writer_simple_object(lisp, w, atom, pk_str("Stream"));
            break;
        }
        default: {
            pk_writer_simple_object(lisp, w, atom, pk_str("Unknown"));
            break;
        }
    }
}

void pk_writer_atom_recursive(Pocket lisp, PKWriter *w, PKHashmap *clones, PKAtom *atom) {
    if (pk_atom_kind(atom) == PK_ATOMKIND_VALUE) {
        pk_writer_atom_(lisp, w, clones, atom);
        return;
    }

    PKAtom *get = pk_hashmap_get(lisp, clones, atom);
    if (pk_atom_is_undefined(get)) {
        (void)pk_hashmap_insert(lisp, clones, atom, atom, PK_HASHMAP_INSERT_NORMAL);
        pk_writer_atom_(lisp, w, clones, atom);
    } else {
        pk_writer_simple_object(lisp, w, atom, pk_ident_atomty(atom->header.type));
    }
}
    
void pk_writer_atom(Pocket lisp, PKWriter *w, PKAtom *atom) {
    PKHashmap clones = PK_HASHMAP_EMPTY;
    pk_writer_atom_(lisp, w, &clones, atom);
    pk_hashmap_deinit(lisp, &clones);
}
