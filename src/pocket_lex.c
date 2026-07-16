#include "pocket_internals.h"

PKRes pk_lex_set(Pocket lisp, PKSet set) {
    pk_try(pk_frame_push(lisp, 0, PKEvalMode_Lex_Set, (PKFrameData){.set = lisp->set}));
    lisp->set = set;
    return PK_Ok;
}

PKRes pk_interp_lex_set(Pocket lisp) {
    pk_set_deinit(lisp, &lisp->set);
    lisp->set = lisp->current_frame.as.set;
    pk_try(pk_ret_top(lisp));
    return PK_Ok;
}
