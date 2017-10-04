#!/bin/bash

#
# Usage: ./replace_cmov.sh input.s > output.s
#
# This script modifies mips32 assembly so that it will run on the mips1 Tiger
# processor.
#
# This is a temporary solution until either:
# 1) The LLVM mips target is modified to do this
# 2) We begin to use Blair's new MIPS processor
#
# The LLVM mips target was also modified so it doesn't produce DSP instructions.
#


# This awk command does a few things:
# * replace MIPS conditional move instructions (movn, movz) with an equivalent
#   instruction sequence that is supported by mips1
# * adds a nop after every lw
# * removes teq (trap if equal) instructions that are not supported by mips1

# replace:
#	movz $1, $2, $3
# with:
#	bne  $3, $zero, $mytag###
#	add  $1, $2, $zero
#mytag###:

# replace:
#	movn $1, $2, $3
# with:
#	beq  $3, $zero, $mytag###
#	add  $1, $2, $zero
#mytag###:

# replace:
#	mul  $1, $2, $3
# with:
#	multu $2, $3
#	mflo  $1

awk '
$1=="movz" {print "\tbne\t" $4 ", $zero, $mytag" NR \
	"\n\tnop" \
	"\n\tadd\t" $2 " " $3 " $zero" \
	"\n$mytag" NR ":"} \
$1=="movn" {print "\tbeq\t" $4 ", $zero, $mytag" NR \
	"\n\tnop" \
	"\n\tadd\t" $2 " " $3 " $zero" \
	"\n$mytag" NR ":"} \
$1=="mul" {print "\tmultu\t" $3 $4 \
	"\n\tmflo\t" substr($2, 1, length($2) -1) } \
$1=="lw" {print $0 "\n\tnop"} \
$1!="movz" && $1!="movn" && $1!="teq" && $1!="lw" && $1!="mul" {print $0} \
' $1

