#!/bin/sh
set -e

input="$1"
name="${2:-$(basename "$input" | sed 's/\.[^.]*$//')}"
upper=$(echo "$name" | tr '[:lower:]' '[:upper:]')

echo "#ifndef POCKET_${upper}_H"
echo "#define POCKET_${upper}_H"
echo ""
printf "#define ${upper}_SRC \""

awk '{
    gsub(/\\/, "\\\\")
    gsub(/"/, "\\\"")
    gsub(/\t/, "\\t")
    if (NR > 1) printf "\\n"
    printf "%s", $0
} END { printf "\"\n" }' "$input"

echo ""
echo "#endif"
