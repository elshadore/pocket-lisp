#include "pocket_internals.h"

#ifdef PK_CFLAG_DEBUG
void pk_debug_stackdump_stack(Pocket lisp, PKWriter *w, size_t index, size_t start, PKAtoms stack) {
    pk_writer_fmt(lisp, w, "[%zu] => ", index);
    pk_writer_newline(lisp, w);

    for (size_t i = 0; i < stack.length; ++i) {
        size_t index = stack.length - i - 1;
        pk_writer_fmt(lisp, w, "    [%zu][%d/%d] => ", start - i, -((int)i + 1), ((int)i + 1));
        pk_writer_atom(lisp, w, stack.e[index]);
        pk_writer_newline(lisp, w);
    }
}

void pk_debug_stackdump(Pocket lisp, const char *tag) {
    PKWriter w = PK_WRITER_EMPTY;

    pk_writer_cstr(lisp, &w, "*~DEBUG-DUMP-STACK~*");
    if (tag != NULL) {
        pk_writer_cstr(lisp, &w, " (tag = ");
        pk_writer_cstr(lisp, &w, tag);
        pk_writer_cstr(lisp, &w, ")");
    }

    pk_writer_newline(lisp, &w);

    pk_debug_stackdump_stack(lisp, &w, 0, lisp->stack.count, pk_stack_slice(lisp));

    for (size_t i = 0; i < lisp->frames.count; ++i) {
        size_t index = lisp->frames.count - i - 1;

        PKFrame *frame = &lisp->frames.e[index];

        if (frame->stack_offset > lisp->stack.count) {
            continue;
        }

        PKAtoms stack = (PKAtoms) {
            .e = lisp->stack.e + frame->stack_offset,
            .length = lisp->stack.count - frame->stack_offset,
        };

        pk_debug_stackdump_stack(lisp, &w, i + 1, lisp->stack.count, stack);
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
                    PKStr carty = pk_ident_atomty(atom->cons.car->header.type);
                    PKStr cdrty = pk_ident_atomty(atom->cons.cdr->header.type);
                    pk_writer_fmt(lisp, &w, "[car] => %.*s, [cdr] => %.*s", (int)carty.length, carty.c, (int)cdrty.length, cdrty.c);
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
#endif // PK_CFLAG_DEBUG
