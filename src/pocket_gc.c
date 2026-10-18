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
            pk_gc_mark_atoms(lisp, atom->lobject.slots);
            break;
        }
        default: break;
    }
}

void pk_gc_collect(Pocket lisp) {
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
