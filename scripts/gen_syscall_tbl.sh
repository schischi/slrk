#! /bin/bash

set -e

DEF_32="$1/arch/x86/include/generated/uapi/asm/unistd_32.h"
DEF_64="$1/arch/x86/include/generated/uapi/asm/unistd_64.h"
OUT=$2
declare -A table_32

echo Creating the syscall tables
while read l; do
    if grep -q "#define __NR_" <<< "$l"; then
        name=$(cut -d" " -f2 <<< "$l")
        N=$(cut -d" " -f3 <<< "$l")
        table_32[$name]=$N
    fi
done < $DEF_32

echo "#include <uapi/asm/unistd_64.h>" > $OUT
echo "int syscall_nr[] = {" >> $OUT
while read l; do
    if grep -q "#define __NR_" <<< "$l"; then
        name=$(cut -d" " -f2 <<< "$l")
        N=$(cut -d" " -f3 <<< "$l")
        if [ -z ${table_32[$name]} ]; then
            echo -e "\t[$name] = -1," >> $OUT
        else
            echo -e "\t[$name] = ${table_32[$name]}," >> $OUT
        fi
    fi
done < $DEF_64
echo "};" >> $OUT
