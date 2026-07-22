#include "pocket_internals.h"

PKType pk_atom_typeof(PKAtomTy ty) {
    switch (ty) {
        case PKAtomTy_Nil: return PKType_Nil;
        case PKAtomTy_Symbol: return PKType_Symbol;
        case PKAtomTy_Cons: return PKType_Cons;
        case PKAtomTy_String: return PKType_String;
        case PKAtomTy_Number: return PKType_Number;
        default: return PKType_Unknown;
    }
}
