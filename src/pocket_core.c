#include "pocket_internals.h"

void pk_init_cache(Pocket lisp) {
    PKCache *c = &lisp->cache;
    
    c->nil = pk_atom_new_nil(lisp);
    
    lisp->throw_symbol = (PKAtom *)c->nil;
    lisp->throw_value = (PKAtom *)c->nil;

    c->atom_true = pk_atom_new_true(lisp);
    c->undefined = pk_atom_new_undefined(lisp);
    c->empty_string = pk_atom_string_new_empty(lisp);
    c->out_of_memory = pk_atom_string(lisp, pk_str("out of memory"));

    c->lambda = pk_atom_symbol(lisp, pk_str("lambda"));
    c->macro = pk_atom_symbol(lisp, pk_str("macro"));
    c->progn = pk_atom_symbol(lisp, pk_str("progn"));
    c->sym_if = pk_atom_symbol(lisp, pk_str("if"));
    c->sym_while = pk_atom_symbol(lisp, pk_str("while"));
    c->sym_and = pk_atom_symbol(lisp, pk_str("and"));
    c->sym_or = pk_atom_symbol(lisp, pk_str("or"));
    c->error = pk_atom_symbol(lisp, pk_str("error"));
    c->throw = pk_atom_symbol(lisp, pk_str("throw"));
    c->var = pk_atom_symbol(lisp, pk_str("var"));
    c->fvar = pk_atom_symbol(lisp, pk_str("fvar"));
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

    pk_std_load(lisp);

    return lisp;
}

void pk_deinit(Pocket lisp) {
    if (lisp == NULL) {
        return;
    }

    pk_array_deinit(lisp, &lisp->stack);
    pk_free(lisp, lisp->frames.e);
    lisp->frames = PK_FRAMES_EMPTY;
    pk_free(lisp, lisp->lets.e);
    lisp->lets = PK_LETS_EMPTY;
    pk_hashmap_deinit(lisp, &lisp->symbolizer);

    pk_gc_collect_no_cache(lisp);

    #ifdef POCKET_CFLAG_DEBUG
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
