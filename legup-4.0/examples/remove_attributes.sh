#!/bin/bash
# this script removes attributes from llvm 3.5 IR
# to match the format in llvm 2.9 IR
# llvm 2.9 is only used for the MIPS-I target
# since llvm 3.5 removed MIPS-I 

FILE=${1}

sed -i '/attributes #[0-9]\+/d' ${1}
sed -i '/; Function Attrs:/d' ${1}
sed -i 's/store volatile /volatile store /g' ${1}
sed -i 's/load volatile /volatile load /g' ${1}
sed -i 's/) #[0-9]\+/)/g' ${1}
sed -i 's/unnamed_addr #[0-9]\+//g' ${1}

PATTERNS=(
 ' none' ' alignment' ' alwaysinline' ' builtin'
 ' byval' ' inalloca' ' cold' ' inlinehint'
 ' inreg' ' jumptable' ' minsize' ' naked'
 ' nest' ' noalias' ' nobuiltin' ' nocapture'
 ' noduplicate' ' noimplicitfloat' ' noinline' ' nonlazybind'
 ' nonnull' ' dereferenceable' ' noredzone' ' noreturn'
 ' nounwind' ' optimizeforsize' ' optimizenone' ' readnone'
 ' readonly' ' returned' ' returnstwice'
 ' stackalignment' ' stackprotect' ' stackprotectreq' ' stackprotectstrong'
 ' structret ' ' sanitizeaddress' ' sanitizethread' ' sanitizememory'
 ' uwtable' ' endattrkinds')

#loop through each case
for (( i = 0 ; i < ${#PATTERNS[@]} ; i++ ))
do
    PATTERN=${PATTERNS[$i]}
    #echo "PATTERN ${PATTERN}"

    # if ${PATTERN} appears inside quotes, ie "${PATTERN}", replace ${PATTERN}
    # with 'some_really_long_temporary_string'
    sed -i "s/\(\".*\)${PATTERN}\(.*\"\)/\1some_really_long_temporary_string\2/g" ${1}

    # replace any ${PATTERN} not in quotes
    sed -i "s/${PATTERN}//g" ${1}

    # replace some_really_long_temporary_string with ${PATTERN}
    sed -i "s/some_really_long_temporary_string/${PATTERN}/g" ${1}
done

