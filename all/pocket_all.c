#ifndef POCKET_LISP_ALL_H
#define POCKET_LISP_ALL_H

#include "../src/pocket_array.c"
#include "../src/pocket_atom.c"
#include "../src/pocket_blob.c"
#include "../src/pocket_call.c"
#include "../src/pocket_compiler.c"
#include "../src/pocket_core.c"

#ifdef PK_CFLAG_DEBUG
    #include "../src/pocket_debug.c"
#endif // PK_CFLAG_DEBUG

#include "../src/pocket_destroy.c"
#include "../src/pocket_env.c"
#include "../src/pocket_eval.c"
#include "../src/pocket_frame.c"
#include "../src/pocket_fs.c"
#include "../src/pocket_function.c"
#include "../src/pocket_gc.c"
#include "../src/pocket_generic.c"
#include "../src/pocket_hashmap.c"
#include "../src/pocket_ident.c"
#include "../src/pocket_immortal.c"
#include "../src/pocket_jmp.c"
#include "../src/pocket_list.c"
#include "../src/pocket_macro.c"
#include "../src/pocket_math.c"
#include "../src/pocket_mem.c"
#include "../src/pocket_object.c"
#include "../src/pocket_parser.c"
#include "../src/pocket_print.c"
#include "../src/pocket_properties.c"
#include "../src/pocket_stack.c"
#include "../src/pocket_std.c"
#include "../src/pocket_string.c"
#include "../src/pocket_strsub.c"
#include "../src/pocket_symbol.c"
#include "../src/pocket_text.c"
#include "../src/pocket_throw.c"
#include "../src/pocket_user.c"
#include "../src/pocket_utils.c"
#include "../src/pocket_vm.c"
#include "../src/pocket_writer.c"

#endif // POCKET_LISP_ALL_H
