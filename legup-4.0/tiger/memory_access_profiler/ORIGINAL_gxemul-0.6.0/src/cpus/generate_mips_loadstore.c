/*
 *  Copyright (C) 2006-2009  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>


void print_function_name(int store, int size, int signedness, int endianness)
{
	if (store)
		printf("s");
	else {
		printf("l");
		if (!signedness)
			printf("u");
	}
	printf("%i", 1 << size);

	if (endianness >= 0) {
		/*  Special case so that Byte loads/stores are only
		    included ones (for little endian):  */
		if (size == 0 && endianness)
			printf("_le");
		else
			printf(endianness? "_be" : "_le");
	}
}


void loadstore(int mode32, int store, int size, int signedness, int endianness)
{
	if (store && signedness)
		return;

	if (size == 0 && endianness)
		return;

	printf("#if%sdef MODE32\n", mode32? "" : "n");

	if (store)
		printf("#define LS_STORE\n");
	else
		printf("#define LS_LOAD\n");

	printf("#define LS_N mips%s_instr_", mode32? "32" : "");
	print_function_name(store, size, signedness, endianness);
	printf("\n");

	printf("#define LS_GENERIC_N mips%s_generic_", mode32? "32" : "");
	print_function_name(store, size, signedness, -1);
	printf("\n");

	printf("#define LS_%i\n", 1 << size);
	printf("#define LS_SIZE %i\n", 1 << size);

	if (signedness && !store)
		printf("#define LS_SIGNED\n");

	if (endianness)
		printf("#define LS_BE\n");
	else
		printf("#define LS_LE\n");

	if (endianness == 0)
		printf("#define LS_INCLUDE_GENERIC\n");

	printf("#include \"cpu_mips_instr_loadstore.cc\"\n");

	if (endianness == 0)
		printf("#undef LS_INCLUDE_GENERIC\n");

	if (endianness)
		printf("#undef LS_BE\n");
	else
		printf("#undef LS_LE\n");

	if (signedness && !store)
		printf("#undef LS_SIGNED\n");

	printf("#undef LS_SIZE\n");
	printf("#undef LS_%i\n", 1 << size);

	printf("#undef LS_GENERIC_N\n");
	printf("#undef LS_N\n");

	if (store)
		printf("#undef LS_STORE\n");
	else
		printf("#undef LS_LOAD\n");

	printf("#endif\n");
}


int main(int argc, char *argv[])
{
	int store, mode32, size, signedness, endianness;

	printf("\n/*  AUTOMATICALLY GENERATED! Do not edit.  */\n\n");

	for (mode32=0; mode32<=1; mode32++)
	    for (endianness=0; endianness<=1; endianness++)
	      for (store=0; store<=1; store++)
		for (size=0; size<=3; size++)
		    for (signedness=0; signedness<=1; signedness++)
			loadstore(mode32, store, size, signedness,
			    endianness);

	/*  Array of pointers to fast load/store functions:  */
	for (mode32=0; mode32<=1; mode32++) {
		printf("#if%sdef MODE32\n", mode32? "" : "n");
		printf("\n\nvoid (*mips%s_loadstore[32])(struct cpu *, struct "
		    "mips_instr_call *) = {\n", mode32? "32" : "");
		for (endianness=0; endianness<=1; endianness++)
		   for (store=0; store<=1; store++)
		      for (size=0; size<=3; size++)
			  for (signedness=0; signedness<=1; signedness++) {
				if (store || size || signedness || endianness)
					printf(",\n");

				if (store && signedness) {
					printf("\tmips%s_instr_invalid",
					    mode32? "32" : "");
					continue;
				}

				printf("\tmips%s_instr_", mode32? "32" : "");
				print_function_name(store, size, signedness,
				    endianness);
			}
		printf(" };\n");
		printf("#endif\n");
	}

	/*  Array of pointers to the generic functions:  */
	for (mode32=0; mode32<=1; mode32++) {
		printf("#if%sdef MODE32\n", mode32? "" : "n");
		printf("\n\nvoid (*mips%s_loadstore_generic[16])("
		    "struct cpu *, struct mips_instr_call *) = {\n",
		    mode32? "32" : "");
		for (store=0; store<=1; store++)
		    for (size=0; size<=3; size++)
			for (signedness=0; signedness<=1; signedness++) {
				if (store || size || signedness)
					printf(",\n");

				if (store && signedness) {
					printf("\tmips%s_instr_invalid",
					    mode32? "32" : "");
					continue;
				}

				printf("\tmips%s_generic_", mode32? "32" : "");
				print_function_name(store, size, signedness,
				    -1);
			}
		printf(" };\n");
		printf("#endif\n");
	}

	return 0;
}

