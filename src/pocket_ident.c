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

const char *pk_ident_opcode(pk_u8 op) {
    switch (op) {
        case PK_OP_ILLEGAL: return "ILLEGAL";
        case PK_OP_NOP: return "NOP";
        case PK_OP_LOAD: return "LOAD";
        case PK_OP_CALL: return "CALL";
        case PK_OP_BLOCK: return "BLOCK";
        case PK_OP_RET: return "RET";
        default: return "UNKNOWN";
    }
}

PK_OPCODE_TY pk_opcode_ty(pk_u8 op) {
    switch (op) {
        case PK_OP_LOAD: return PK_OPCODE_TY_LOAD;
        case PK_OP_CALL: return PK_OPCODE_TY_LIT;
        default: return PK_OPCODE_TY_NORMAL;
    }
}
