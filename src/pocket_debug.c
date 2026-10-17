#include "pocket_internals.h"

PKAtom pk_debug_dump_(PKWriter *w, const char *tag, PK_DUMP opt) {
    pk_propagate(pk_writer_string(w, pk_str("*~DEBUG-DUMP~*")));
    if (tag != NULL) {
        pk_propagate(pk_writer_string(w, pk_str(" (tag = ")));
        pk_propagate(pk_writer_cstr(w, tag));
        pk_propagate(pk_writer_string(w, pk_str(")")));
    }
    
    pk_propagate(pk_writer_newline(w));

    if ((opt & PK_DUMP_STACK) != 0) {
        pk_propagate(pk_writer_string(w, pk_str("STACK: ")));
        pk_propagate(pk_writer_fmt(w, "%zu", PK_CORE.current_frame.stack_offset));
        pk_propagate(pk_writer_newline(w));

        for (size_t i = 0; i < PK_CORE.stack.count; ++i) {
            size_t index = PK_CORE.stack.count - i - 1;
            pk_propagate(pk_writer_string(w, pk_str("    [")));
            pk_propagate(pk_writer_fmt(w, "%zu", index));
            pk_propagate(pk_writer_string(w, pk_str("] => ")));
            PKAtom atom = PK_CORE.stack.e[index];
            pk_propagate(pk_writer_atom(w, atom));
            pk_propagate(pk_writer_newline(w));
        }
    }
    if ((opt & PK_DUMP_ENV) != 0){
        pk_propagate(pk_writer_string(w, pk_str("ENV:")));
        pk_propagate(pk_writer_newline(w));
        size_t acc = 0;
        for (PKPool *iter = PK_CORE.pool; iter != NULL; iter = iter->next) {
            for (size_t i = 0; i < PK_POOL_MAX; ++i) {
                PKAtom symbol = &iter->e[i];
                if (pk_is_symbol(symbol)) {
                    pk_propagate(pk_writer_string(w, pk_str("    [")));
                    pk_propagate(pk_writer_fmt(w, "%zu", acc));
                    pk_propagate(pk_writer_string(w, pk_str("] => ")));
                    pk_propagate(pk_writer_atom(w, symbol));
                    pk_propagate(pk_writer_newline(w));
                    pk_propagate(pk_writer_string(w, pk_str("    ")));
                    pk_propagate(pk_writer_atom(w, symbol->symbol.var));
                    pk_propagate(pk_writer_newline(w));
                    pk_propagate(pk_writer_string(w, pk_str("    ")));
                    pk_propagate(pk_writer_atom(w, symbol->symbol.fun));
                    pk_propagate(pk_writer_newline(w));
                    pk_propagate(pk_writer_string(w, pk_str("    ")));
                    pk_propagate(pk_writer_atom(w, symbol->symbol.prop));
                    pk_propagate(pk_writer_newline(w));
                    acc += 1;
                }
            }
        }
    }

    pk_print_str(pk_writer_slice(w));
    
    return pk_true();
}
    
PKAtom pk_debug_dump(const char *tag, PK_DUMP opt) {
    PKWriter w = PK_WRITER_EMPTY;
    PKAtom result = pk_debug_dump_(&w, tag, opt);
    pk_writer_deinit(&w);
    return result;
}

void pk_debug_leakcheck(void) {
    fprintf(stderr, "debug: atom leakcheck begin\n");
    size_t count = 0;
    
    for (PKPool *iter = PK_CORE.pool; iter != NULL; iter = iter->next) {
        for (size_t i = 0; i < PK_POOL_MAX; ++i) {
            PKAtom atom = &iter->e[i];
            if (atom->header.type == PK_ATOMTY_FREE) {
                continue;
            }
            PKStr ident = pk_ident_atomty(atom->header.type);
            fprintf(stderr, "[%zu] => <%p> [%.*s] ", count, atom, (int)ident.length, ident.c);
            switch (atom->header.type) {
                case PK_ATOMTY_STRING: {
                    fprintf(stderr, "%.*s", (int)atom->string.str.length, atom->string.str.c);
                    break;
                }
                case PK_ATOMTY_KEYWORD: {
                    fprintf(stderr, "%.*s", (int)atom->keyword.id.length, atom->keyword.id.c);
                    break;
                }
                case PK_ATOMTY_SYMBOL: {
                    PKAtom name = atom->symbol.name;
                    if (name->header.type == PK_ATOMTY_STRING) {
                        fprintf(stderr, "%.*s", (int)name->string.str.length, name->string.str.c);
                    }
                    break;
                }
                case PK_ATOMTY_NUMBER: {
                    if (atom->number.num_type == PK_NUMTY_INT) {
                        fprintf(stderr, "%d", atom->number.as.i);
                    } else {
                        fprintf(stderr, "%f", atom->number.as.f);
                    }
                    break;
                }
                case PK_ATOMTY_CONS: {
                    fprintf(stderr, "car ty=%u cdr ty=%u", atom->cons.car->header.type, atom->cons.cdr->header.type);
                    break;
                }
                default: break;
            }
            fprintf(stderr, "\n");
            count += 1;
        }
    }
    
    fprintf(stderr, "debug: atom leakcheck count: %zu\n", count);
}
