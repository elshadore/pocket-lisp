#include "pocket_internals.h"

void pk_atom_mark(PKAtom *atom) {
    atom->header.flags |= PK_ATOMFLAG_MARKED;
}

void pk_atom_unmark(PKAtom *atom) {
    atom->header.flags &= ~PK_ATOMFLAG_MARKED;
}

bool pk_atom_is_marked(PKAtom *atom) {
    return (atom->header.flags & PK_ATOMFLAG_MARKED) != 0;
}

void pk_gc_mark_cache(Pocket lisp) {
    PKCache *c = &lisp->cache;
    
    pk_gc_mark(lisp, (PKAtom *)c->nil);
    pk_gc_mark(lisp, (PKAtom *)c->atom_true);
    pk_gc_mark(lisp, (PKAtom *)c->undefined);
    pk_gc_mark(lisp, (PKAtom *)c->empty_string);
    pk_gc_mark(lisp, (PKAtom *)c->out_of_memory);

    pk_gc_mark(lisp, (PKAtom *)c->lambda);
    pk_gc_mark(lisp, (PKAtom *)c->macro);
    pk_gc_mark(lisp, (PKAtom *)c->progn);
    pk_gc_mark(lisp, (PKAtom *)c->sym_if);
    pk_gc_mark(lisp, (PKAtom *)c->sym_while);
    pk_gc_mark(lisp, (PKAtom *)c->sym_and);
    pk_gc_mark(lisp, (PKAtom *)c->sym_or);
    pk_gc_mark(lisp, (PKAtom *)c->error);
    pk_gc_mark(lisp, (PKAtom *)c->throw);
    pk_gc_mark(lisp, (PKAtom *)c->var);
    pk_gc_mark(lisp, (PKAtom *)c->fvar);
    pk_gc_mark(lisp, (PKAtom *)c->setq);
    pk_gc_mark(lisp, (PKAtom *)c->fsetq);
    pk_gc_mark(lisp, (PKAtom *)c->psetq);
    pk_gc_mark(lisp, (PKAtom *)c->let);
    pk_gc_mark(lisp, (PKAtom *)c->let_star);
    pk_gc_mark(lisp, (PKAtom *)c->protect);
    pk_gc_mark(lisp, (PKAtom *)c->flet);
    pk_gc_mark(lisp, (PKAtom *)c->flet_star);
    pk_gc_mark(lisp, (PKAtom *)c->block);
    pk_gc_mark(lisp, (PKAtom *)c->apply);
    pk_gc_mark(lisp, (PKAtom *)c->macro_apply);
    pk_gc_mark(lisp, (PKAtom *)c->quote);
    pk_gc_mark(lisp, (PKAtom *)c->quasiquote);
    pk_gc_mark(lisp, (PKAtom *)c->unquote);
    pk_gc_mark(lisp, (PKAtom *)c->unquote_splice);
    pk_gc_mark(lisp, (PKAtom *)c->string_substitute);

    pk_gc_mark(lisp, lisp->throw_symbol);
    pk_gc_mark(lisp, lisp->throw_value);
}

void pk_gc_mark_hashmap(Pocket lisp, PKHashmap *map) {
    for (size_t i = 0; i < map->capacity; ++i) {
        PKHashmapSlot *slot = &map->e[i];
        if (slot->state == PK_HASHMAP_SLOT_OCCUPIED) {
            pk_gc_mark(lisp, slot->key);
            pk_gc_mark(lisp, slot->value);
        }
    }
}

void pk_gc_mark_slotmap(Pocket lisp, PKSlotmap *map) {
    for (size_t i = 0; i < map->capacity; ++i) {
        PKSlotmapSlot *slot = &map->e[i];
        if (slot->state == PK_SLOTMAP_SLOT_OCCUPIED) {
            pk_gc_mark(lisp, slot->symbol);
        }
    }
}

void pk_gc_mark_atoms(Pocket lisp, PKAtoms atoms) {
    for (size_t i = 0; i < atoms.length; ++i) {
        pk_gc_mark(lisp, atoms.e[i]);
    }
}

void pk_gc_mark(Pocket lisp, PKAtom *atom) {
    (void)lisp;
    
    if (pk_atom_is_marked(atom)) {
        return;
    }

    pk_atom_mark(atom);
    
    switch (atom->header.type) {
        case PK_ATOMTY_CONS: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            pk_gc_mark(lisp, cons->car);
            pk_gc_mark(lisp, cons->cdr);
            break;
        }
        case PK_ATOMTY_SYMBOL: {
            PKAtomSymbol *symbol = (PKAtomSymbol *)atom;
            pk_gc_mark(lisp, (PKAtom *)symbol->name);
            pk_gc_mark(lisp, (PKAtom *)symbol->var);
            pk_gc_mark(lisp, (PKAtom *)symbol->fun);
            pk_gc_mark(lisp, (PKAtom *)symbol->prop);
            pk_gc_mark_slotmap(lisp, &symbol->slots);
            pk_gc_mark_hashmap(lisp, &symbol->methods);
            break;
        }
        case PK_ATOMTY_LFUN: {
            pk_gc_mark_atoms(lisp, atom->lfun.atoms);
            break;
        }
        case PK_ATOMTY_LMACRO: {
            pk_gc_mark_atoms(lisp, atom->lmacro.atoms);
            break;
        }
        case PK_ATOMTY_LOBJECT: {
            pk_gc_mark(lisp, atom->lobject.type);
            pk_gc_mark_atoms(lisp, atom->lobject.slots);
            break;
        }
        case PK_ATOMTY_COBJECT: {
            pk_gc_mark(lisp, atom->cobject.type);
            break;
        }
        default: break;
    }
}

void pk_gc_collect(Pocket lisp) {
    pk_gc_mark_cache(lisp);
    pk_gc_collect_no_cache(lisp);
}

void pk_gc_collect_no_cache(Pocket lisp) {
    // Mark
    for (size_t i = 0; i < lisp->stack.count; ++i) {
        pk_gc_mark(lisp, (PKAtom *)lisp->stack.e[i]);
    }

    for (size_t i = 0; i < lisp->lets.count; ++i) {
        PKLet let = lisp->lets.e[i];
        pk_gc_mark(lisp, (PKAtom *)let.symbol);
        pk_gc_mark(lisp, (PKAtom *)let.restore);
    }

    pk_gc_mark_hashmap(lisp, &lisp->symbolizer);

    // Sweep
    for (PKPool *pool = lisp->pool; pool; pool = pool->next) {
        for (size_t i = 0; i < PK_POOL_MAX; ++i) {
            PKAtom *atom = &pool->e[i];
            if (atom->header.type != PK_ATOMTY_FREE) {
                if (!pk_atom_is_marked(atom)) {
                    pk_atom_free(lisp, atom);
                }
            }
            pk_atom_unmark(atom);
        }
    }
}
