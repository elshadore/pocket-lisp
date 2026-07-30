#include "pocket_internals.h"

PK_TYPE pk_atom_typeof(PKAtomTy ty) {
    switch (ty) {
        case PKAtomTy_Nil: return PK_TYPE_NIL;
        case PKAtomTy_Symbol: return PK_TYPE_SYMBOL;
        case PKAtomTy_Cons: return PK_TYPE_CONS;
        case PKAtomTy_String: return PK_TYPE_STRING;
        case PKAtomTy_Number: return PK_TYPE_NUMBER;
        default: return PK_TYPE_UNKNOWN;
    }
}

const char *pk_ident_opcode(pk_u8 op) {
    switch (op) {
        case PK_OP_ILLEGAL: return "ILLEGAL";
        case PK_OP_NOP: return "NOP";
        case PK_OP_RET: return "RET";
        case PK_OP_LOAD: return "LOAD";
        case PK_OP_LOAD_NIL: return "LOAD_NIL";
        case PK_OP_CALL: return "CALL";
        case PK_OP_BLOCK_BEGIN: return "BLOCK_BEGIN";
        case PK_OP_BLOCK_END: return "BLOCK_END";
        case PK_OP_BLOCK_CLEAR: return "BLOCK_CLEAR";
        case PK_OP_JMP_IF_NIL: return "JMP_IF_NIL";
        case PK_OP_JMP: return "JMP";
        case PK_OP_JMP_BACK: return "JMP_BACK";
        case PK_OP_LET_VAR: return "LET_VAR";
        case PK_OP_LET_FUN: return "LET_FUN";
        case PK_OP_LOOKUP_VAR: return "LOOKUP_VAR";
        case PK_OP_LOOKUP_FUN: return "LOOKUP_FUN";
        case PK_OP_MAKE_LIST: return "MAKE_LIST";
        case PK_OP_MAKE_LIST_PACKED: return "MAKE_LIST_PACKED";
        case PK_OP_MERGE_LISTS: return "MERGE_LISTS";
        case PK_OP_STRCAT: return "STRCAT";
        default: return "UNKNOWN";
    }
}

const char *pk_ident_atomty(PKAtomTy ty) {
    switch (ty) {
        case PKAtomTy_Free: return "FREE";
        case PKAtomTy_Nil: return "NIL";
        case PKAtomTy_Symbol: return "SYMBOL";
        case PKAtomTy_Cons: return "CONS";
        case PKAtomTy_String: return "STRING";
        case PKAtomTy_Number: return "NUMBER";
        case PKAtomTy_CFunc: return "CFUNC";
        case PKAtomTy_LFunc: return "LFUNC";
        case PKAtomTy_LMacro: return "LMACRO";
        case PKAtomTy_Keyword: return "KEYWORD";
    }
}

const char *pk_ident_arity(pk_u8 arity) {
    switch (arity) {
        case PK_ARITY_NORMAL: return "NORMAL";
        case PK_ARITY_OPTIONAL: return "OPTIONAL";
        case PK_ARITY_VARIADIC: return "VARIADIC";
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
        case PK_OP_LET_VAR: return PK_OPCODE_TY_LIT;
        case PK_OP_LET_FUN: return PK_OPCODE_TY_LIT;
        case PK_OP_MAKE_LIST: return PK_OPCODE_TY_LIT;
        case PK_OP_MAKE_LIST_PACKED: return PK_OPCODE_TY_LIT;
        case PK_OP_MERGE_LISTS: return PK_OPCODE_TY_LIT;
        case PK_OP_STRCAT: return PK_OPCODE_TY_LIT;
        
        default: return PK_OPCODE_TY_NORMAL;
    }
}
