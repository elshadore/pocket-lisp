#include "pocket_internals.h"

void pk_init_cache(Pocket lisp) {
    PKCache *c = &lisp->cache;
    c->lambda = pk_atom_symbol(lisp, pk_str("lambda"));
    c->macro = pk_atom_symbol(lisp, pk_str("macro"));
    c->defvar = pk_atom_symbol(lisp, pk_str("defvar"));
    c->defun = pk_atom_symbol(lisp, pk_str("defun"));
    c->defmacro = pk_atom_symbol(lisp, pk_str("defmacro"));
    c->progn = pk_atom_symbol(lisp, pk_str("progn"));
    c->sym_if = pk_atom_symbol(lisp, pk_str("if"));
    c->sym_while = pk_atom_symbol(lisp, pk_str("while"));
    c->sym_and = pk_atom_symbol(lisp, pk_str("and"));
    c->sym_or = pk_atom_symbol(lisp, pk_str("or"));
    c->error = pk_atom_symbol(lisp, pk_str("error"));
    c->throw = pk_atom_symbol(lisp, pk_str("throw"));
    c->var = pk_atom_symbol(lisp, pk_str("var"));
    c->fvar = pk_atom_symbol(lisp, pk_str("fvar"));
    c->setq = pk_atom_symbol(lisp, pk_str("setq"));
    c->fsetq = pk_atom_symbol(lisp, pk_str("fsetq"));
    c->let = pk_atom_symbol(lisp, pk_str("let"));
    c->let_star = pk_atom_symbol(lisp, pk_str("let*"));
    c->protect = pk_atom_symbol(lisp, pk_str("protect"));
    c->flet = pk_atom_symbol(lisp, pk_str("flet"));
    c->flet_star = pk_atom_symbol(lisp, pk_str("flet*"));
    c->block = pk_atom_symbol(lisp, pk_str("block"));
    c->apply = pk_atom_symbol(lisp, pk_str("apply"));
    c->macro_apply = pk_atom_symbol(lisp, pk_str("macro-apply"));
    c->quote = pk_atom_symbol(lisp, pk_str("quote"));
    c->quasiquote = pk_atom_symbol(lisp, pk_str("quasiquote"));
    c->unquote = pk_atom_symbol(lisp, pk_str("unquote"));
    c->unquote_splice = pk_atom_symbol(lisp, pk_str("unquote-splice"));
    c->string_substitute = pk_atom_symbol(lisp, pk_str("string-substitute"));
}

Pocket pk_init(void *user_env) {
    (void)user_env;

    Pocket lisp = (Pocket)malloc(sizeof(PocketLispMachine));
    if (lisp == NULL) {
        return NULL;
    }

    *lisp = POCKET_LISP_MACHINE_EMPTY;

    pk_init_cache(lisp);

    PKJmp jmp;
    pk_jmp_enter(lisp, &jmp);

    if (setjmp(jmp.buf) != 0) {
        pk_jmp_leave(lisp);
        pk_deinit(lisp);
        return NULL;
    }

    pk_std_load(lisp);

    pk_jmp_leave(lisp);

    // When creating the LISP we have gc disabled, we then enable it after init and do a gc collect.
    lisp->gc_enabled = true;
    pk_gc_collect(lisp);

    return lisp;
}

void pk_deinit(Pocket lisp) {
    if (lisp == NULL) {
        return;
    }

    pk_writer_destroy(lisp);
    
    pk_array_deinit(lisp, &lisp->stack);
    pk_free(lisp, lisp->frames.e);
    lisp->frames = PK_FRAMES_EMPTY;
    pk_free(lisp, lisp->lets.e);
    lisp->lets = PK_LETS_EMPTY;
    pk_hashmap_deinit(lisp, &lisp->symbolizer);

    pk_gc_collect(lisp);

    #ifdef PK_CFLAG_DEBUG
    pk_debug_leakcheck(lisp, "pk_deinit()");
    #endif

    {
        PKPool *iter = lisp->pool;
        while (iter != NULL) {
            PKPool *pool = iter;
            iter = iter->next;
            pk_free(lisp, pool);
        }
    }

    *lisp = POCKET_LISP_MACHINE_EMPTY;

    pk_free(lisp, lisp);
}
