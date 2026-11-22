#include "pocket_internals.h"

#ifdef PK_CFLAG_DEBUG
void pk_debug_stackdump_stack(Pocket lisp, PKAtom *ident, size_t index, size_t start, size_t finish) {
    pk_writer_fmt(lisp, "[%zu] => ", index);
    pk_writer_atom(lisp, ident);
    pk_writer_newline(lisp);

    size_t length = finish - start;
    for (size_t i = 0; i < length; ++i) {
        size_t index = length - i - 1;
        size_t abs = start + index;
        PKAtom *atom = lisp->stack.e[abs];
        pk_writer_fmt(lisp, "    [%zu][%d/%d] => ", finish - i, -((int)i + 1), ((int)i + 1));
        pk_writer_atom(lisp, atom);
        pk_writer_newline(lisp);
    }
}

void pk_debug_stackdump(Pocket lisp, const char *tag) {
    pk_writer_reset(lisp);

    pk_writer_cstr(lisp, "*~DEBUG-DUMP-STACK~*");
    if (tag != NULL) {
        pk_writer_cstr(lisp, " (tag = ");
        pk_writer_cstr(lisp, tag);
        pk_writer_cstr(lisp, ")");
    }

    pk_writer_newline(lisp);

    PKFrame *frame = &lisp->current_frame;
    pk_debug_stackdump_stack(lisp, frame->function, 0, frame->stack_offset, lisp->stack.count);

    for (size_t i = 0; i < lisp->frames.count; ++i) {
        size_t index = lisp->frames.count - i - 1;
        PKFrame *frame = &lisp->frames.e[index];
        pk_debug_stackdump_stack(lisp, frame->function, i + 1, frame->stack_offset, frame->stack_length);
    }
    
    pk_writer_string(lisp, pk_str("--DEBUG-DUMP-STACK-END--"));
    
    pk_internal_print(lisp, pk_writer_str(lisp));

    pk_writer_reset(lisp);
}

void pk_debug_envdump(Pocket lisp, const char *tag) {
    pk_writer_reset(lisp);
    
    pk_writer_string(lisp, pk_str("*~DEBUG-DUMP-ENV~*"));
    if (tag != NULL) {
        pk_writer_string(lisp, pk_str(" (tag = "));
        pk_writer_cstr(lisp, tag);
        pk_writer_string(lisp, pk_str(")"));
    }

    pk_writer_newline(lisp);
    
    pk_writer_string(lisp, pk_str("ENV:"));
    pk_writer_newline(lisp);
    size_t acc = 0;
    for (PKPool *iter = lisp->pool; iter != NULL; iter = iter->next) {
        for (size_t i = 0; i < PK_POOL_MAX; ++i) {
            PKAtom *symbol = &iter->e[i];
            if (!pk_atom_is_symbol(symbol)) {
                continue;
            }
            pk_writer_string(lisp, pk_str("    ["));
            pk_writer_fmt(lisp, "%zu", acc);
            pk_writer_string(lisp, pk_str("] => "));
            pk_writer_atom(lisp, symbol);
            pk_writer_newline(lisp);
            pk_writer_string(lisp, pk_str("    "));
            pk_writer_atom(lisp, symbol->symbol.var);
            pk_writer_newline(lisp);
            pk_writer_string(lisp, pk_str("    "));
            pk_writer_atom(lisp, symbol->symbol.fun);
            pk_writer_newline(lisp);
            acc += 1;
        }
    }
    
    pk_writer_string(lisp, pk_str("--DEBUG-DUMP-ENV-END--"));
    
    pk_internal_print(lisp, pk_writer_str(lisp));

    pk_writer_reset(lisp);
}

void pk_debug_leakcheck(Pocket lisp, const char *tag) {
    pk_writer_reset(lisp);
    
    pk_writer_string(lisp, pk_str("*~DEBUG-DUMP-LEAKED-MEMORY~*"));
    if (tag != NULL) {
        pk_writer_string(lisp, pk_str(" (tag = "));
        pk_writer_cstr(lisp, tag);
        pk_writer_string(lisp, pk_str(")"));
    }

    pk_writer_newline(lisp);
    size_t count = 0;

    for (PKPool *iter = lisp->pool; iter != NULL; iter = iter->next) {
        for (size_t i = 0; i < PK_POOL_MAX; ++i) {
            PKAtom *atom = &iter->e[i];
            if ((atom->header.type == PK_ATOMTY_FREE) || pk_atom_is_immortal(atom)) {
                continue;
            }
            PKStr ident = pk_ident_atomty(atom->header.type);
            pk_writer_fmt(lisp, "[%zu] => <%p> [%.*s] ", count, (void *)atom, (int)ident.length, ident.c);
            switch (atom->header.type) {
                case PK_ATOMTY_STRING: {
                    pk_writer_string(lisp, atom->string.str);
                    break;
                }
                case PK_ATOMTY_KEYWORD: {
                    pk_writer_string(lisp, atom->keyword.id);
                    break;
                }
                case PK_ATOMTY_SYMBOL: {
                    PKAtomString *name = atom->symbol.name;
                    if (name->header.type == PK_ATOMTY_STRING) {
                        pk_writer_string(lisp, name->str);
                    }
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
                case PK_ATOMTY_CONS: {
                    PKStr carty = pk_ident_atomty(atom->cons.car->header.type);
                    PKStr cdrty = pk_ident_atomty(atom->cons.cdr->header.type);
                    pk_writer_fmt(lisp, "[car] => %.*s, [cdr] => %.*s", (int)carty.length, carty.c, (int)cdrty.length, cdrty.c);
                    break;
                }
                default: break;
            }
            pk_writer_fmt(lisp, "\n");
            count += 1;
        }
    }

    pk_writer_fmt(lisp, "LEAKED: %zu\n", count);
    
    pk_writer_string(lisp, pk_str("--DEBUG-DUMP-LEAKED-MEMORY-END--"));
    
    pk_internal_print(lisp, pk_writer_str(lisp));

    pk_writer_reset(lisp);
}
#endif // PK_CFLAG_DEBUG
