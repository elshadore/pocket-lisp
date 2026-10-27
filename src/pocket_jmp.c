#include "pocket_internals.h"

bool pk_jmp_is_frame_match(PKFrame *frame, PKAtomSymbol *symbol) {
    if (frame->mode == PK_FRAME_MODE_PROTECT) {
        return true;
    }
    if ((frame->mode == PK_FRAME_MODE_CATCH) && (frame->catch == symbol)) {
        return true;
    }
    return false;
}

void pk_jmp_leave(Pocket lisp) {
    lisp->current_frame.jmp = NULL;
}

void pk_jmp_enter(Pocket lisp, PKJmp *jmp) {
    lisp->current_frame.jmp = jmp;
}

void pk_jmp_trigger(Pocket lisp, PKAtomSymbol *symbol, PKAtom *value) {
    lisp->throw_symbol = (PKAtom *)symbol;
    lisp->throw_value = value;

    for (size_t i = 0; i < lisp->frames.count; ++i) {
        size_t index = lisp->frames.count - i - 1;
        if (pk_jmp_is_frame_match(&lisp->frames.e[index], symbol)) {
            pk_frame_unwindn(lisp, i + 1);
            PKJmp *jmp = lisp->current_frame.jmp;
            lisp->current_frame.jmp = NULL;
            pk_stack_head_set_unchecked(lisp, lisp->throw_value);
            longjmp(jmp->buf, 1);
            return;
        }
    }

    if (symbol == lisp->cache.error) {
        fprintf(stderr, "FATAL ERROR: unprotected stack");
        abort();
    } else {
        PKWriter w = PK_WRITER_EMPTY;
        pk_writer_string(lisp, &w, pk_str("failed to catch throw: "));
        pk_writer_atom(lisp, &w, (PKAtom *)symbol);
        pk_error_writer(lisp, &w);
    }
}

void pk_atom_catch(Pocket lisp, PKAtomSymbol *symbol) {
    if (lisp->current_frame.mode == PK_FRAME_MODE_CATCH) {
        return;
    }
    lisp->current_frame.mode = PK_FRAME_MODE_CATCH;
    lisp->current_frame.catch = symbol;
}

void pk_protect(Pocket lisp) {
    lisp->current_frame.mode = PK_FRAME_MODE_PROTECT;
}

void pk_catch(Pocket lisp, int symbol) {
    pk_atom_catch(lisp, pk_atom_assert_symbol(lisp, pk_stack_get(lisp, symbol)));
}

