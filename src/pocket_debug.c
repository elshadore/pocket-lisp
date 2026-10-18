#include "pocket_internals.h"

void pk_debug_dump(Pocket lisp, const char *tag, PK_DUMP opt) {
    PKWriter w = PK_WRITER_EMPTY;

    pk_writer_cstr(lisp, &w, "*~DEBUG-DUMP~*");
    if (tag != NULL) {
        pk_writer_cstr(lisp, &w, " (tag = ");
        pk_writer_cstr(lisp, &w, tag);
        pk_writer_cstr(lisp, &w, ")");
    }

    pk_writer_newline(lisp, &w);

    if ((opt & PK_DUMP_STACK) != 0) {
        pk_writer_cstr(lisp, &w, "STACK: ");
        pk_writer_fmt(lisp, &w, "%zu", lisp->current_frame.stack_offset);
        pk_writer_newline(lisp, &w);

        for (size_t i = 0; i < lisp->stack.count; ++i) {
            size_t index = lisp->stack.count - i - 1;
            pk_writer_cstr(lisp, &w, "    [");
            pk_writer_fmt(lisp, &w, "%zu", index);
            pk_writer_cstr(lisp, &w, "] => ");
            pk_writer_atom(lisp, &w, lisp->stack.e[index]);
            pk_writer_newline(lisp, &w);
        }
    }

    if ((opt & PK_DUMP_ENV) != 0) {
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
    }

    PKStr slice = pk_writer_slice(&w);
    printf("%.*s\n", (int)slice.length, slice.c);
    fflush(stdout);

    pk_writer_deinit(lisp, &w);
}

void pk_debug_leakcheck(Pocket lisp) {
    fprintf(stderr, "debug: atom leakcheck begin\n");
    size_t count = 0;

    for (PKPool *iter = lisp->pool; iter != NULL; iter = iter->next) {
        for (size_t i = 0; i < PK_POOL_MAX; ++i) {
            PKAtom *atom = &iter->e[i];
            if (atom->header.type == PK_ATOMTY_FREE) {
                continue;
            }
            PKStr ident = pk_ident_atomty(atom->header.type);
            fprintf(stderr, "[%zu] => <%p> [%.*s] ", count, (void *)atom, (int)ident.length, ident.c);
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
                    PKAtomString *name = atom->symbol.name;
                    if (name->header.type == PK_ATOMTY_STRING) {
                        fprintf(stderr, "%.*s", (int)name->str.length, name->str.c);
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