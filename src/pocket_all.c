#ifndef POCKET_LISP_ALL_H
#define POCKET_LISP_ALL_H

#include "pocket_array.c"
#include "pocket_atom.c"
#include "pocket_blob.c"
#include "pocket_call.c"
#include "pocket_compiler.c"
#include "pocket_core.c"

#ifdef PK_CFLAG_DEBUG
    #include "pocket_debug.c"
#endif // PK_CFLAG_DEBUG

#include "pocket_destroy.c"
#include "pocket_env.c"
#include "pocket_eval.c"
#include "pocket_frame.c"
#include "pocket_fs.c"
#include "pocket_function.c"
#include "pocket_gc.c"
#include "pocket_generic.c"
#include "pocket_hashmap.c"
#include "pocket_ident.c"
#include "pocket_immortal.c"
#include "pocket_jmp.c"
#include "pocket_list.c"
#include "pocket_macro.c"
#include "pocket_math.c"
#include "pocket_mem.c"
#include "pocket_object.c"
#include "pocket_parser.c"
#include "pocket_print.c"
#include "pocket_properties.c"
#include "pocket_stack.c"
#include "pocket_std.c"
#include "pocket_string.c"
#include "pocket_strsub.c"
#include "pocket_symbol.c"
#include "pocket_text.c"
#include "pocket_throw.c"
#include "pocket_user.c"
#include "pocket_utils.c"
#include "pocket_vm.c"
#include "pocket_writer.c"

#endif // POCKET_LISP_ALL_H
