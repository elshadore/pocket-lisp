#include "pocket_internals.h"

PKAtom pk_fs_slurp_str(PKStr file_path) {
    if (file_path.length == 0) {
        return pk_error();
    }
    char *cfile_path = pk_malloc((file_path.length + 1) * sizeof(char));
    (void)memcpy(cfile_path, file_path.c, file_path.length);
    cfile_path[file_path.length] = '\0';

    FILE *file = fopen(cfile_path, "r");
    pk_free(cfile_path);
    if (file == NULL) {
        return pk_error();
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    if (length < 0) {
        fclose(file);
        return pk_error();
    }
    rewind(file);
    
    PKAtom result = pk_string_alloc(length);
    if (pk_is_error(result)) {
        fclose(file);
        return result;
    }
    
    size_t read = fread(result->string.str.c, 1, (size_t)length, file);
    fclose(file);
    
    if ((size_t)length != read) {
        return pk_error();
    }
    
    return result;
}

PKAtom pk_fs_slurp(PKAtom file_path) {
    pk_propagate(pk_typecheck_string(file_path));
    return pk_fs_slurp_str(file_path->string.str);
}
