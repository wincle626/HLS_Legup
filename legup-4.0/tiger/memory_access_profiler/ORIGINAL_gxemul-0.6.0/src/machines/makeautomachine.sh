#!/bin/sh
###############################################################################
#
#  Copyright (C) 2005-2009  Anders Gavare.  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
#  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
#  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
#  SUCH DAMAGE.


printf "Generating automachine.cc... "

rm -f automachine.cc

printf "/*\n *  DO NOT EDIT. AUTOMATICALLY CREATED\n */\n\n" >> automachine.cc

cat automachine_head.cc >> automachine.cc

printf "3"
rm -f .index
for a in *.cc; do
	B=`grep COMMENT $a`
	if [ z"$B" != z ]; then
		printf "$a " >> .index
		echo "$B"|cut -d : -f 2- >> .index
	fi
done

printf "2"
for a in machine_*.cc; do
	B=`grep MACHINE_REGISTER $a`
	if [ z"$B" != z ]; then
		C=`grep MACHINE_REGISTER $a | cut -d \( -f 2|cut -d \) -f 1`
		for B in $C; do
			printf "void machine_register_$B(void);\n" >> automachine.cc
		done
	fi
done

cat automachine_middle.cc >> automachine.cc

printf "1"
for a in machine_*.cc; do
	B=`grep MACHINE_REGISTER $a`
	if [ z"$B" != z ]; then
		C=`grep MACHINE_REGISTER $a | cut -d \( -f 2|cut -d \) -f 1`
		for B in $C; do
			printf "\tmachine_register_$B();\n" >> automachine.cc
		done
	fi
done

cat automachine_tail.cc >> automachine.cc

printf " done\n"
