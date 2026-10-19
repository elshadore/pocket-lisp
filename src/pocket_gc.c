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
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.nil);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.atom_true);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.undefined);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.empty_string);

    pk_gc_mark(lisp, (PKAtom *)lisp->cache.lambda);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.macro);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.progn);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.sym_if);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.sym_while);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.sym_and);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.sym_or);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.error);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.throw);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.var);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.fvar);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.let);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.let_star);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.protect);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.unwind_protect);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.flet);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.flet_star);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.block);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.apply);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.macro_apply);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.quote);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.quasiquote);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.unquote);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.unquote_splice);
    pk_gc_mark(lisp, (PKAtom *)lisp->cache.string_substitute);
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
