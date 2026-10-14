#!/bin/sh
# Converts pocket lisp source into a C header that holds it as a #define string.
#
# usage: pk2h.sh <src.pk> <dst.h> <prefix>
#
# The include guard comes from the destination file name, the macro from the
# prefix (REPL -> REPL_CODE).

set -eu

if [ $# -ne 3 ]; then
    printf 'usage: %s <src.pk> <dst.h> <prefix>\n' "$0" >&2
    exit 1
fi

src=$1
dst=$2
prefix=$3

if [ ! -f "$src" ]; then
    printf '%s: cannot read %s\n' "$0" "$src" >&2
    exit 1
fi

guard=$(basename "$dst" | tr '[:lower:].-' '[:upper:]__')

tmp="$dst.tmp.$$"

{
    printf '#ifndef %s\n' "$guard"
    printf '#define %s\n' "$guard"
    printf '\n'
    printf '#define %s_CODE \\\n' "$prefix"
    awk '
        {
            line = $0
            gsub(/\\/, "\\\\", line)
            gsub(/"/, "\\\"", line)
            printf "\"%s\\n\" \\\n", line
        }
    ' "$src" | sed '$ s/ \\$//'
    printf '\n'
    printf '\n'
    printf '#endif // %s\n' "$guard"
} > "$tmp"

mv "$tmp" "$dst"
