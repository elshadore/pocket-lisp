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
        case PKEvalMode_Quote_End: return pkstr("Qoute_End");
        case PKEvalMode_If: return pkstr("If");
        case PKEvalMode_While: return pkstr("While");
        case PKEvalMode_While_2: return pkstr("While_2");
        case PKEvalMode_Let_Eval: return pkstr("Let_Eval");
        case PKEvalMode_Let_Bind: return pkstr("Let_Bind");
        case PKEvalMode_Read_Mode: return pkstr("Read_Mode");
        case PKEvalMode_Read_Atom: return pkstr("Read_Atom");
        case PKEvalMode_Read_Append: return pkstr("Read_Append");
        case PKEvalMode_Read_Cons: return pkstr("Read_Cons");
        case PKEvalMode_Read_Cons_2: return pkstr("Read_Cons_2");
        case PKEvalMode_Read_Cons_3: return pkstr("Read_Cons_3");
        case PKEvalMode_Read_All: return pkstr("Read_All");
        case PKEvalMode_Read_All_2: return pkstr("Read_All_2");
    }
}

PKEvalFrameTy pk_evalmode_framety(PKEvalMode mode) {
    switch (mode) {
        case PKEvalMode_Read_Mode: return PKEvalFrameTy_None;
        case PKEvalMode_Read_Cons: return PKEvalFrameTy_None;
        case PKEvalMode_Read_All: return PKEvalFrameTy_None;
        case PKEvalMode_Read_Atom: return PKEvalFrameTy_None;
        case PKEvalMode_Root: return PKEvalFrameTy_None;
        case PKEvalMode_User: return PKEvalFrameTy_None;
        case PKEvalMode_Ret: return PKEvalFrameTy_None;
        case PKEvalMode_If: return PKEvalFrameTy_Tuple;
        case PKEvalMode_While: return PKEvalFrameTy_Tuple;
        case PKEvalMode_While_2: return PKEvalFrameTy_Tuple;
        default: return PKEvalFrameTy_Atom;
    }
}

bool pk_evalmode_readty(PKEvalMode mode) {
    switch (mode) {
        case PKEvalMode_Read_Atom:
        case PKEvalMode_Read_Mode:
        case PKEvalMode_Read_Append:
        case PKEvalMode_Read_Cons:
        case PKEvalMode_Read_Cons_2:
        case PKEvalMode_Read_Cons_3:
        case PKEvalMode_Read_All:
        case PKEvalMode_Read_All_2: return true;
        default: return false;
    }
}

PKType pk_atom_typeof(PKAtomTy ty) {
    switch (ty) {
        case PKAtomTy_Nil: return PKType_Nil;
        case PKAtomTy_Cons: return PKType_Cons;
        case PKAtomTy_String: return PKType_String;
        case PKAtomTy_Symbol: return PKType_Symbol;
        case PKAtomTy_Number: return PKType_Number;
        default: return PKType_Unknown;
    }
}
