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
        case PK_OP_RET: return "RET";
        case PK_OP_LOAD: return "LOAD";
        case PK_OP_CALL: return "CALL";
        case PK_OP_BLOCK_BEGIN: return "BLOCK_BEGIN";
        case PK_OP_BLOCK_END: return "BLOCK_END";
        case PK_OP_JMP_IF_NIL: return "JMP_IF_NIL";
        case PK_OP_JMP: return "JMP";
        case PK_OP_JMP_BACK: return "JMP_BACK";
        case PK_OP_LET: return "LET";
        case PK_OP_LOOKUP_VAR: return "LOOKUP_VAR";
        case PK_OP_LOOKUP_FUN: return "LOOKUP_FUN";
        default: return "UNKNOWN";
    }
}

PK_OPCODE_TY pk_opcode_ty(pk_u8 op) {
    switch (op) {
    
        case PK_OP_LOAD: return PK_OPCODE_TY_LOAD;
        case PK_OP_LOOKUP_VAR: return PK_OPCODE_TY_LOAD;
        case PK_OP_LOOKUP_FUN: return PK_OPCODE_TY_LOAD;
        
        case PK_OP_CALL: return PK_OPCODE_TY_LIT;
        case PK_OP_JMP_IF_NIL: return PK_OPCODE_TY_LIT;
        case PK_OP_JMP: return PK_OPCODE_TY_LIT;
        case PK_OP_JMP_BACK: return PK_OPCODE_TY_LIT;
        case PK_OP_LET: return PK_OPCODE_TY_LIT;
        
        default: return PK_OPCODE_TY_NORMAL;
    }
}
