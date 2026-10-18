#include "pocket_internals.h"

void pk_debug_stackdump(Pocket lisp, const char *tag) {
    PKWriter w = PK_WRITER_EMPTY;

    pk_writer_cstr(lisp, &w, "*~DEBUG-DUMP-STACK~*");
    if (tag != NULL) {
        pk_writer_cstr(lisp, &w, " (tag = ");
        pk_writer_cstr(lisp, &w, tag);
        pk_writer_cstr(lisp, &w, ")");
    }

    pk_writer_newline(lisp, &w);

    pk_writer_cstr(lisp, &w, "STACK: ");
    pk_writer_fmt(lisp, &w, "%zu", lisp->current_frame.stack_offset);
    pk_writer_newline(lisp, &w);

    for (size_t i = 0; i < lisp->stack.count; ++i) {
        size_t index = lisp->stack.count - i - 1;
        pk_writer_cstr(lisp, &w, "    [");
        pk_writer_fmt(lisp, &w, "%d/%d", -((int)i + 1), ((int)i + 1));
        pk_writer_cstr(lisp, &w, "] => ");
        pk_writer_atom(lisp, &w, lisp->stack.e[index]);
        pk_writer_newline(lisp, &w);
    }
    
    pk_writer_cstr(lisp, &w, "--DEBUG-DUMP-STACK-END--");
    
    pk_internal_println(lisp, pk_writer_slice(&w));

    pk_writer_deinit(lisp, &w);
}

void pk_debug_envdump(Pocket lisp, const char *tag) {
    PKWriter w = PK_WRITER_EMPTY;
    
    pk_writer_cstr(lisp, &w, "*~DEBUG-DUMP-ENV~*");
    if (tag != NULL) {
        pk_writer_cstr(lisp, &w, " (tag = ");
        pk_writer_cstr(lisp, &w, tag);
        pk_writer_cstr(lisp, &w, ")");
    }

    pk_writer_newline(lisp, &w);
    
    pk_writer_cstr(lisp, &w, "ENV:");
    pk_writer_newline(lisp, &w);
    size_t acc = 0;
    for (PKPool *iter = lisp->pool; iter != NULL; iter = iter->next) {
        for (size_t i = 0; i < PK_POOL_MAX; ++i) {
            PKAtom *symbol = &iter->e[i];
            if (!pk_atom_is_symbol(symbol)) {
                continue;
            }
            pk_writer_cstr(lisp, &w, "    [");
            pk_writer_fmt(lisp, &w, "%zu", acc);
            pk_writer_cstr(lisp, &w, "] => ");
            pk_writer_atom(lisp, &w, symbol);
            pk_writer_newline(lisp, &w);
            pk_writer_cstr(lisp, &w, "    ");
            pk_writer_atom(lisp, &w, symbol->symbol.var);
            pk_writer_newline(lisp, &w);
            pk_writer_cstr(lisp, &w, "    ");
            pk_writer_atom(lisp, &w, symbol->symbol.fun);
            pk_writer_newline(lisp, &w);
            pk_writer_cstr(lisp, &w, "    ");
            pk_writer_atom(lisp, &w, symbol->symbol.prop);
            pk_writer_newline(lisp, &w);
            acc += 1;
        }
    }
    
    pk_writer_cstr(lisp, &w, "--DEBUG-DUMP-ENV-END--");
    
    pk_internal_println(lisp, pk_writer_slice(&w));

    pk_writer_deinit(lisp, &w);
}

void pk_debug_leakcheck(Pocket lisp, const char *tag) {
    PKWriter w = PK_WRITER_EMPTY;
    
    pk_writer_cstr(lisp, &w, "*~DEBUG-DUMP-LEAKED-MEMORY~*");
    if (tag != NULL) {
        pk_writer_cstr(lisp, &w, " (tag = ");
        pk_writer_cstr(lisp, &w, tag);
        pk_writer_cstr(lisp, &w, ")");
    }

    pk_writer_newline(lisp, &w);
    size_t count = 0;

    for (PKPool *iter = lisp->pool; iter != NULL; iter = iter->next) {
        for (size_t i = 0; i < PK_POOL_MAX; ++i) {
            PKAtom *atom = &iter->e[i];
            if (atom->header.type == PK_ATOMTY_FREE) {
                continue;
            }
            PKStr ident = pk_ident_atomty(atom->header.type);
            pk_writer_fmt(lisp, &w, "[%zu] => <%p> [%.*s] ", count, (void *)atom, (int)ident.length, ident.c);
            switch (atom->header.type) {
                case PK_ATOMTY_STRING: {
                    pk_writer_string(lisp, &w, atom->string.str);
                    break;
                }
                case PK_ATOMTY_KEYWORD: {
                    pk_writer_string(lisp, &w, atom->keyword.id);
                    break;
                }
                case PK_ATOMTY_SYMBOL: {
                    PKAtomString *name = atom->symbol.name;
                    if (name->header.type == PK_ATOMTY_STRING) {
                        pk_writer_string(lisp, &w, name->str);
                    }
                    break;
                }
                case PK_ATOMTY_NUMBER: {
                    if (atom->number.num_type == PK_NUMTY_INT) {
                        pk_writer_fmt(lisp, &w, "%d", atom->number.as.i);
                    } else {
                        pk_writer_fmt(lisp, &w, "%f", atom->number.as.f);
                    }
                    break;
                }
                case PK_ATOMTY_CONS: {
                    pk_writer_fmt(lisp, &w, "car ty=%u cdr ty=%u", atom->cons.car->header.type, atom->cons.cdr->header.type);
                    break;
                }
                default: break;
            }
            pk_writer_fmt(lisp, &w, "\n");
            count += 1;
        }
    }

    pk_writer_fmt(lisp, &w, "LEAKED: %zu\n", count);
    
    pk_writer_cstr(lisp, &w, "--DEBUG-DUMP-LEAKED-MEMORY-END--");
    
    pk_internal_println(lisp, pk_writer_slice(&w));

    pk_writer_deinit(lisp, &w);
}
