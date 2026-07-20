#include "pocket_internals.h"

PKRes pk_process_quote(Pocket lisp, PKAtom *atom) {
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Quote_End, (PKFrameData){.table = lisp->table}));
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Quote, (PKFrameData){.atom = (PKAtom *)atom}));
    lisp->table = PK_HASHTABLE_EMPTY;
    return PK_Ok;
}

PKRes pk_interp_quote(Pocket lisp) {
    PKAtom *atom = lisp->current_frame.as.atom;
    switch (atom->tag.ty) {
        case PKAtomTy_Cons: {
            PKAtomCons *cons = (PKAtomCons *)atom;
            lisp->current_frame.as.atom = cons->cdr;
            pk_frame_push(lisp, 0, PKEvalMode_Quote, (PKFrameData){.atom = cons->car});
            return PK_Ok;
        }
        default: {
            PKAtoms slice = pk_stack_slice(lisp);
            
            PKAtom *result = NULL;
            if (slice.length == 0) {
                result = atom;
            } else {
                if (pk_atom_is_nil(atom)) {
                    pk_try(pk_slice_list(lisp, slice, &result));
                } else {
                    pk_try(pk_push(lisp, atom));
                    pk_try(pk_slice_list_tailed(lisp, pk_stack_slice(lisp), &result));
                }
            }
            pk_try(pk_ret_this(lisp, result));
            return PK_Ok;
        }
    }
}

PKRes pk_interp_quote_end(Pocket lisp) {
    pk_hashtable_deinit(lisp, &lisp->table);
    lisp->table = lisp->current_frame.as.table;
    pk_try(pk_ret_top(lisp));
    return PK_Ok;
}
