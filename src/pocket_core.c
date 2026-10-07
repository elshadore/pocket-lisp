#include "pocket_internals.h"

PocketLispMachine PK_CORE = POCKET_LISP_MACHINE_EMPTY;

PKAtom pk_init2(void) {
    PK_CORE.cache.lambda = pk_symbol(pk_str("lambda"));
    pk_propagate(PK_CORE.cache.lambda);
    
    PK_CORE.cache.macro = pk_symbol(pk_str("macro"));
    pk_propagate(PK_CORE.cache.macro);
    
    PK_CORE.cache.progn = pk_symbol(pk_str("progn"));
    pk_propagate(PK_CORE.cache.progn);
    
    PK_CORE.cache.sym_if = pk_symbol(pk_str("if"));
    pk_propagate(PK_CORE.cache.sym_if);
    
    PK_CORE.cache.sym_while = pk_symbol(pk_str("while"));
    pk_propagate(PK_CORE.cache.sym_while);
    
    PK_CORE.cache.error = pk_symbol(pk_str("error"));
    pk_propagate(PK_CORE.cache.error);
    
    PK_CORE.cache.let = pk_symbol(pk_str("let"));
    pk_propagate(PK_CORE.cache.let);
    
    PK_CORE.cache.let_star = pk_symbol(pk_str("let*"));
    pk_propagate(PK_CORE.cache.let_star);
    
    PK_CORE.cache.flet = pk_symbol(pk_str("flet"));
    pk_propagate(PK_CORE.cache.flet);
    
    PK_CORE.cache.flet_star = pk_symbol(pk_str("flet*"));
    pk_propagate(PK_CORE.cache.flet_star);
    
    PK_CORE.cache.block = pk_symbol(pk_str("block"));
    pk_propagate(PK_CORE.cache.block);
    
    PK_CORE.cache.apply = pk_symbol(pk_str("apply"));
    pk_propagate(PK_CORE.cache.apply);
    
    PK_CORE.cache.macro_apply = pk_symbol(pk_str("macro-apply"));
    pk_propagate(PK_CORE.cache.macro_apply);
    
    PK_CORE.cache.quote = pk_symbol(pk_str("quote"));
    pk_propagate(PK_CORE.cache.quote);
    
    PK_CORE.cache.quasiquote = pk_symbol(pk_str("quasiquote"));
    pk_propagate(PK_CORE.cache.quasiquote);
    
    PK_CORE.cache.unquote = pk_symbol(pk_str("unquote"));
    pk_propagate(PK_CORE.cache.unquote);
    
    PK_CORE.cache.unquote_splice = pk_symbol(pk_str("unquote-splice"));
    pk_propagate(PK_CORE.cache.unquote_splice);
    
    PK_CORE.cache.string_substitute = pk_symbol(pk_str("string-substitute"));
    pk_propagate(PK_CORE.cache.string_substitute);

    return pk_true();
}

bool pk_init(void) {
    if (PK_CORE.initialized) {
        return true;
    }
    
    PK_CORE.initialized = true;
    
    if (pk_is_error(pk_init2())) {
        pk_deinit();
        return false;
    }
    return true;
}

void pk_deinit(void) {
    if (!PK_CORE.initialized) {
        return;
    }
    
    PK_CORE = POCKET_LISP_MACHINE_EMPTY;
}

PKAtom pk_frame_push(size_t arity) {
    if (PK_CORE.frames.count >= PK_CORE.frames.capacity) {
        size_t new_capacity = pk_grow_capacity(PK_CORE.frames.capacity, 16);
        PKFrame *new_e = pk_realloc(PK_CORE.frames.e, new_capacity * sizeof(PKFrame));
        if (new_e == NULL) {
            return pk_error_oom();
        }
        PK_CORE.frames.e = new_e;
        PK_CORE.frames.capacity = new_capacity;
    }
    
    size_t stack_offset = pk_frame_length() - arity;
    size_t lets_offset = PK_CORE.lets.count;
    
    PK_CORE.frames.e[PK_CORE.frames.count++] = PK_CORE.current_frame;

    PK_CORE.current_frame = (PKFrame) {
        .stack_offset = stack_offset,
        .let_offset = lets_offset,
    };

    return pk_true();
}

PKAtom pk_frame_pushc(PKAtoms atoms) {
    pk_propagate(pk_frame_push(0));
    for (size_t i = 0; i < atoms.length; ++i) {
        PKAtom atom = pk_stack_push(atoms.e[i]);
        if (pk_is_error(atom)) {
            pk_frame_unwind();
            return atom;
        }
    }
    return pk_true();
}
    
void pk_frame_unwind(void) {
    PKFrame unwind = PK_CORE.frames.e[PK_CORE.frames.count - 1];
    size_t diff = PK_CORE.current_frame.stack_offset = unwind.stack_offset;
    size_t stack_count = unwind.stack_offset + diff;
    size_t lets_unwind = PK_CORE.lets.count - PK_CORE.current_frame.let_offset;
    pk_let_unwind(lets_unwind);
    
    for (size_t i = 0; i < diff; ++i) {
        size_t index = PK_CORE.stack.count - i - 1;
        pk_ref_dec(PK_CORE.stack.e[index]);
    }
    
    PK_CORE.stack.count = stack_count;
    PK_CORE.frames.count -= 1;
    PK_CORE.current_frame = unwind;
}

PKAtom pk_stack_push(PKAtom atom) {
    pk_propagate(pk_array_push(&PK_CORE.stack, atom, 128));
    
    PKAtom result = pk_ref_inc(atom);
    if (pk_is_error(result)) {
        PK_CORE.stack.count -= 1;
    }
    return result;
}

