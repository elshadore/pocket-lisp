#include "pocket_internals.h"

void pk_gc_mark(PKAtom *atom) {
    if (atom->tag.marked) {
        return;
    }
    
    atom->tag.marked = true;
    switch (atom->tag.ty) {
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            pk_gc_mark(cons->car);
            pk_gc_mark(cons->cdr);
            break;
        }
        case PKAtomTy_Symbol: {
            PKAtomSymbol *symbol = (PKAtomSymbol *)atom;
            pk_gc_mark((PKAtom *)symbol->id);
            if (symbol->chain != NULL) {
                pk_gc_mark((PKAtom *)symbol->chain);
            }
            break;
        }
        default: break;
    }
}

void pk_gc_mark_symtable(PKSymTable *st) {
    for (size_t i = 0; i < st->capacity; ++i) {
        for (PKSymTableSlot *slot = st->e[i]; slot; slot = slot->chain) {
            pk_gc_mark((PKAtom *)slot->key);
            pk_gc_mark((PKAtom *)slot->value);
        }
    }
}

void pk_gc_collect(Pocket lisp) {
    // Mark
    
    pk_gc_mark((PKAtom *)lisp->cache.nil);
    pk_gc_mark((PKAtom *)lisp->cache.nilsym);
    pk_gc_mark((PKAtom *)lisp->cache.t);
    pk_gc_mark((PKAtom *)lisp->cache.lambda);
    pk_gc_mark((PKAtom *)lisp->cache.if_sym);
    pk_gc_mark((PKAtom *)lisp->cache.quote);
    pk_gc_mark((PKAtom *)lisp->cache.empty_string);
    
    for (size_t i = 0; i < lisp->intern.count; ++i) {
        PKAtomSymbol *symbol = lisp->intern.e[i];
        if (symbol == NULL) continue;
        pk_gc_mark((PKAtom *)symbol);
    }

    pk_gc_mark_symtable(&lisp->vars);
    pk_gc_mark_symtable(&lisp->funs);

    for (size_t i = 0; i < lisp->stack.count; ++i) {
        pk_gc_mark((PKAtom *)lisp->stack.e[i]);
    }

    for (size_t i = 0; i < lisp->lets.count; ++i) {
        PKLet let = lisp->lets.e[i];
        pk_gc_mark((PKAtom *)let.symbol);
        if (let.restore != NULL) {
            pk_gc_mark((PKAtom *)let.restore);
        }
    }

    // Sweep

    for (PKPool *pool = lisp->pool; pool; pool = pool->next) {
        for (size_t i = 0; i < PK_POOL_MAX; ++i) {
            PKAtom *atom = &pool->e[i];
            if (atom->tag.ty != PKAtomTy_Free) {
                if (!atom->tag.marked) {
                    pk_atom_free(lisp, atom);
                }
            }
            atom->tag.marked = false;
        }
    }
}
