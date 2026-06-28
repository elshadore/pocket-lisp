#include "pocket_internals.h"

PKString pk_ident_evalmode(PKEvalMode mode) {
    switch (mode) {
        case PKEvalMode_Root: return pkstr("Root");
        case PKEvalMode_User: return pkstr("User");
        case PKEvalMode_Ret: return pkstr("Ret");
        case PKEvalMode_Eval: return pkstr("Eval");
        case PKEvalMode_Apply: return pkstr("Apply");
        case PKEvalMode_Evlist: return pkstr("Evlist");
        case PKEvalMode_Evlist_2: return pkstr("Evlist_2");
        case PKEvalMode_Evargs: return pkstr("Evargs");
        case PKEvalMode_Quote: return pkstr("Quote");
        case PKEvalMode_If: return pkstr("If");
        case PKEvalMode_While: return pkstr("While");
        case PKEvalMode_While_2: return pkstr("While_2");
    }
}

PKEvalFrameTy pk_evalmode_framety(PKEvalMode mode) {
    switch (mode) {
        case PKEvalMode_Root: return PKEvalFrameTy_None;
        case PKEvalMode_User: return PKEvalFrameTy_None;
        case PKEvalMode_Ret: return PKEvalFrameTy_None;
        case PKEvalMode_If: return PKEvalFrameTy_Tuple;
        case PKEvalMode_While: return PKEvalFrameTy_Tuple;
        case PKEvalMode_While_2: return PKEvalFrameTy_Tuple;
        default: return PKEvalFrameTy_Atom;
    }
}
