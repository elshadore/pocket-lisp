#include "pocket_internals.h"

PKString pk_ident_evalmode(PKEvalMode mode) {
    switch (mode) {
        case PKEvalMode_Root: return pkstr("Root");
        case PKEvalMode_Eval: return pkstr("Eval");
        case PKEvalMode_Apply: return pkstr("Apply");
        case PKEvalMode_Evlist: return pkstr("Evlist");
        case PKEvalMode_Evlist_2: return pkstr("Evlist_2");
        case PKEvalMode_Evargs: return pkstr("Evargs");
        case PKEvalMode_Quote: return pkstr("Quote");
        case PKEvalMode_If: return pkstr("If");
    }
}
