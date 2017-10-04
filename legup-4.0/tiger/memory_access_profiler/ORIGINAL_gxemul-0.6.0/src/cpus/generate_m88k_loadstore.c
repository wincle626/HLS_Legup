/*
 *  Copyright (C) 2007-2009  Anders Gavare.  All rights reserved.
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
 *
 *
 *  Automatic generation of M88K loads and stores:
 *
 *	2 bits size (0=byte, 1=halfword, 2=word, 3=doubleword)
 *	1 bit load/store
 *	1 bit signedness (for byte and halfword loads only!)
 *	1 bit endianness
 *	1 bit scaledness (for halfword, word, and doublewords only,
 *			  and only when using register offset)
 *	1 bit ".usr" (only when using register offset)
 *	1 bit registeroffset
 */

#include <stdio.h>
#include <string.h>


#define	M88K_LOADSTORE_STORE		4
#define	M88K_LOADSTORE_SIGNEDNESS	8
#define	M88K_LOADSTORE_ENDIANNESS	16
#define	M88K_LOADSTORE_SCALEDNESS	32
#define	M88K_LOADSTORE_USR		64
#define	M88K_LOADSTORE_REGISTEROFFSET	128


int generated_functions = 0;


void print_function_name(int size, int store, int signedness,
	int endianness, int scaledness, int usr, int registeroffset)
{
	if (store)
		printf("st");
	else {
		printf("ld");
		if (!signedness)
			printf("_u");
	}
	printf("_%i", 1 << size);

	if (endianness >= 0) {
		/*  Special case so that Byte loads/stores are only
		    included ones (for little endian):  */
		if (size == 0 && endianness)
			printf("_le");
		else
			printf(endianness? "_be" : "_le");
	}

	if (scaledness && size > 0)
		printf("_scaled");

	if (usr)
		printf("_usr");

	if (registeroffset)
		printf("_regofs");
}


void loadstore(int size, int store, int signedness,
	int endianness, int scaledness, int usr, int registeroffset)
{
	if (store && signedness)
		return;

	if (size == 0 && endianness)
		return;

	if (size == 0 && scaledness)
		return;

	if (!registeroffset && usr)
		return;

	if (!registeroffset && scaledness)
		return;

	if (store)
		printf("#define LS_STORE\n");
	else
		printf("#define LS_LOAD\n");

	printf("#define LS_N m88k_instr_");
	print_function_name(size, store, signedness, endianness,
	    scaledness, usr, registeroffset);
	printf("\n");

	printf("#define LS_GENERIC_N m88k_generic_");
	print_function_name(size, store, signedness, -1,
	    scaledness, usr, registeroffset);
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

	if (scaledness)
		printf("#define LS_SCALED\n");

	if (usr)
		printf("#define LS_USR\n");

	if (registeroffset)
		printf("#define LS_REGOFS\n");

	printf("#include \"cpu_m88k_instr_loadstore.cc\"\n");
	generated_functions ++;

	if (registeroffset)
		printf("#undef LS_REGOFS\n");

	if (usr)
		printf("#undef LS_USR\n");

	if (scaledness)
		printf("#undef LS_SCALED\n");

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
}


int main(int argc, char *argv[])
{
	int size, store, signedness, endianness, scaledness, usr,
	    registeroffset;

	printf("\n/*  AUTOMATICALLY GENERATED! Do not edit.  */\n\n");

	for (registeroffset=0; registeroffset<=M88K_LOADSTORE_REGISTEROFFSET;
		      registeroffset+=M88K_LOADSTORE_REGISTEROFFSET)
	 for (usr=0; usr<=M88K_LOADSTORE_USR; usr+=M88K_LOADSTORE_USR)
	  for (scaledness=0; scaledness<=M88K_LOADSTORE_SCALEDNESS;
			scaledness+=M88K_LOADSTORE_SCALEDNESS)
	    for (endianness=0; endianness<=M88K_LOADSTORE_ENDIANNESS;
			  endianness+=M88K_LOADSTORE_ENDIANNESS)
	      for (signedness=0; signedness<=M88K_LOADSTORE_SIGNEDNESS;
			    signedness+=M88K_LOADSTORE_SIGNEDNESS)
	        for (store=0; store<=M88K_LOADSTORE_STORE;
			      store+=M88K_LOADSTORE_STORE)
		  for (size=0; size<=3; size++)
			loadstore(size, store, signedness,
			    endianness, scaledness, usr, registeroffset);

	/*  Array of pointers to fast load/store functions:  */
	printf("\n\nvoid (*m88k_loadstore[256])(struct cpu *, struct "
	    "m88k_instr_call *) = {\n");
	for (registeroffset=0; registeroffset<=M88K_LOADSTORE_REGISTEROFFSET;
		      registeroffset+=M88K_LOADSTORE_REGISTEROFFSET)
	 for (usr=0; usr<=M88K_LOADSTORE_USR; usr+=M88K_LOADSTORE_USR)
	  for (scaledness=0; scaledness<=M88K_LOADSTORE_SCALEDNESS;
			scaledness+=M88K_LOADSTORE_SCALEDNESS)
	    for (endianness=0; endianness<=M88K_LOADSTORE_ENDIANNESS;
			  endianness+=M88K_LOADSTORE_ENDIANNESS)
	      for (signedness=0; signedness<=M88K_LOADSTORE_SIGNEDNESS;
			    signedness+=M88K_LOADSTORE_SIGNEDNESS)
	        for (store=0; store<=M88K_LOADSTORE_STORE;
			      store+=M88K_LOADSTORE_STORE)
		  for (size=0; size<=3; size++) {
			if (store || size || signedness || endianness
			    || scaledness || usr || registeroffset)
				printf(",\n");

			if (store && signedness) {
				printf("\tNULL");
				continue;
			}

			if (!registeroffset && usr) {
				printf("\tNULL");
				continue;
			}

			if (!registeroffset && scaledness) {
				printf("\tNULL");
				continue;
			}

			if (size == 0 && scaledness) {
				printf("\tNULL");
				continue;
			}

			printf("\tm88k_instr_");
			print_function_name(size, store, signedness,
			    endianness, scaledness, usr, registeroffset);
		}
	printf(" };\n");

	fprintf(stderr, "%s: generated_functions = %i\n",
	    argv[0], generated_functions);

	return 0;
}

