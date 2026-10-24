#include "pocket_internals.h"

void pk_jmp_enter(Pocket lisp, PKJmp *jmp) {
    jmp->a_link_to_the_past = lisp->current_jmp;
    jmp->frames_count = lisp->frames.count;
    jmp->catch = lisp->current_frame.catch;
    jmp->mode = lisp->current_frame.mode;
    lisp->current_jmp = jmp;
}

PK_RES pk_jmp_leave(Pocket lisp, PKJmp *jmp, PK_RES res) {
    lisp->current_jmp = jmp->a_link_to_the_past;

    if (res == PK_OK) {
        lisp->yielded = false;
        lisp->throw_symbol = lisp->cache.nil;
        lisp->throw_value = lisp->cache.nil;
        return PK_OK;
    }

    size_t target = lisp->yielded ? lisp->yield_frames_count : jmp->frames_count;

    while (lisp->frames.count > target) {
        pk_frame_unwind(lisp);
    }

    size_t base = lisp->current_frame.stack_offset;
    lisp->stack.count = base;
    pk_stack_push(lisp, lisp->yielded ? lisp->throw_value : lisp->cache.nil);

    if (lisp->yielded && lisp->current_jmp == NULL) {
        lisp->yielded = false;
        lisp->throw_symbol = lisp->cache.nil;
        lisp->throw_value = lisp->cache.nil;
    }

    pk_gc_collect(lisp);

    return PK_YIELDED;
}

PK_RES pk_jmp_catch(Pocket lisp, PKJmp *jmp) {
    PKAtom *value = lisp->throw_value;

    lisp->current_jmp = jmp->a_link_to_the_past;

    while (lisp->frames.count > jmp->frames_count) {
        pk_frame_unwind(lisp);
    }

    lisp->stack.count = lisp->current_frame.stack_offset;
    pk_stack_push(lisp, value);

    lisp->yielded = false;
    lisp->throw_symbol = lisp->cache.nil;
    lisp->throw_value = lisp->cache.nil;

    pk_gc_collect(lisp);

    return PK_OK;
}

PK_RES pk_jmp_handle(Pocket lisp, PKJmp *jmp) {
    if (jmp->mode == PK_FRAME_MODE_PROTECT) {
        return pk_jmp_catch(lisp, jmp);
    }

    if (jmp->mode == PK_FRAME_MODE_CATCH) {
        if ((PKAtom *)jmp->catch == lisp->throw_symbol) {
            return pk_jmp_catch(lisp, jmp);
        }
    }

    lisp->current_jmp = jmp->a_link_to_the_past;

    pk_propagate(lisp);
    return PK_YIELDED;
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
