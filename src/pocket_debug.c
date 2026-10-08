#include "pocket_internals.h"

PKAtom pk_debug_dump_(PKWriter *w, const char *tag) {
    pk_propagate(pk_writer_string(w, pk_str("*~DEBUG-DUMP~*")));
    if (tag != NULL) {
        pk_propagate(pk_writer_string(w, pk_str(" (tag = ")));
        pk_propagate(pk_writer_cstr(w, tag));
        pk_propagate(pk_writer_string(w, pk_str(")")));
    }
    
    pk_propagate(pk_writer_newline(w));

    for (size_t i = 0; i < PK_CORE.stack.count; ++i) {
        size_t index = PK_CORE.stack.count - i - 1;
        pk_propagate(pk_writer_string(w, pk_str("    [")));
        pk_propagate(pk_writer_fmt(w, "%zu", i));
        pk_propagate(pk_writer_string(w, pk_str("] => ")));
        PKAtom atom = PK_CORE.stack.e[index];
        pk_propagate(pk_writer_atom(w, atom));
        pk_propagate(pk_writer_newline(w));
    }

    {
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
    
PKAtom pk_debug_dump(const char *tag) {
    PKWriter w = PK_WRITER_EMPTY;
    PKAtom result = pk_debug_dump_(&w, tag);
    pk_writer_deinit(&w);
    return result;
}
