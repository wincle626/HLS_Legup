/*
 *  Copyright (C) 2005-2009  Anders Gavare.  All rights reserved.
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


char *sizechar[4] = { "b", "h", "w", "d" };
char *modes[2] = { "", "32" };


void do_it(int mode)
{
	int n, load, size, zero, ignoreofs, update;

	n = 0;
	for (update=0; update<=1; update++)
	  for (ignoreofs=0; ignoreofs<=1; ignoreofs++)
	    for (load=0; load<=1; load++)
		for (zero=0; zero<=1; zero++)
		    for (size=0; size<4; size++) {
			if (!zero && !load)
				continue;
			if (load && !zero && size == 3)
				continue;

			switch (size) {
			case 0:	printf("#define LS_B\n"); break;
			case 1:	printf("#define LS_H\n"); break;
			case 2:	printf("#define LS_W\n"); break;
			case 3:	printf("#define LS_D\n"); break;
			}
			printf("#define LS_SIZE %i\n", 1 << size);
			if (zero)
				printf("#define LS_ZERO\n");
			if (load)
				printf("#define LS_LOAD\n");
			if (ignoreofs)
				printf("#define LS_IGNOREOFS\n");
			if (update)
				printf("#define LS_UPDATE\n");

			printf("#define LS_GENERIC_N ppc%s_generic_",
			    modes[mode]);
			if (load)
				printf("l");
			else
				printf("st");
			printf("%s", sizechar[size]);
			if (load) {
				if (zero)
					printf("z");
				else
					printf("a");
			}
			if (update)
				printf("u");
			printf("\n");

			printf("#define LS_N ppc%s_instr_", modes[mode]);
			if (load)
				printf("l");
			else
				printf("st");
			printf("%s", sizechar[size]);
			if (load && size < 3) {
				if (zero)
					printf("z");
				else
					printf("a");
			}
			if (update)
				printf("u");
			if (ignoreofs)
				printf("_0");
			printf("\n");

			printf("#include \"cpu_ppc_instr_loadstore.cc\"\n");

			printf("#undef LS_N\n");
			printf("#undef LS_GENERIC_N\n");
			switch (size) {
			case 0:	printf("#undef LS_B\n"); break;
			case 1:	printf("#undef LS_H\n"); break;
			case 2:	printf("#undef LS_W\n"); break;
			case 3:	printf("#undef LS_D\n"); break;
			}
			printf("#undef LS_SIZE\n");
			if (load)
				printf("#undef LS_LOAD\n");
			if (update)
				printf("#undef LS_UPDATE\n");
			if (zero)
				printf("#undef LS_ZERO\n");
			if (ignoreofs)
				printf("#undef LS_IGNOREOFS\n");
		    }

	/*  Indexed loads/stores:  */
	printf("#define LS_INDEXED\n");
	for (update=0; update<=1; update++)
	    for (load=0; load<=1; load++)
		for (zero=0; zero<=1; zero++)
		    for (size=0; size<4; size++) {
			if (!zero && !load)
				continue;
			if (load && !zero && size == 3)
				continue;

			switch (size) {
			case 0:	printf("#define LS_B\n"); break;
			case 1:	printf("#define LS_H\n"); break;
			case 2:	printf("#define LS_W\n"); break;
			case 3:	printf("#define LS_D\n"); break;
			}
			printf("#define LS_SIZE %i\n", 1 << size);
			if (zero)
				printf("#define LS_ZERO\n");
			if (load)
				printf("#define LS_LOAD\n");
			if (update)
				printf("#define LS_UPDATE\n");

			printf("#define LS_GENERIC_N ppc%s_generic_",
			    modes[mode]);
			if (load)
				printf("l");
			else
				printf("st");
			printf("%s", sizechar[size]);
			if (load) {
				if (zero)
					printf("z");
				else
					printf("a");
			}
			if (update)
				printf("u");
			printf("x");
			printf("\n");

			printf("#define LS_N ppc%s_instr_", modes[mode]);
			if (load)
				printf("l");
			else
				printf("st");
			printf("%s", sizechar[size]);
			if (load && size < 3) {
				if (zero)
					printf("z");
				else
					printf("a");
			}
			if (update)
				printf("u");
			printf("x");
			printf("\n");

			printf("#include \"cpu_ppc_instr_loadstore.cc\"\n");

			printf("#undef LS_N\n");
			printf("#undef LS_GENERIC_N\n");
			switch (size) {
			case 0:	printf("#undef LS_B\n"); break;
			case 1:	printf("#undef LS_H\n"); break;
			case 2:	printf("#undef LS_W\n"); break;
			case 3:	printf("#undef LS_D\n"); break;
			}
			printf("#undef LS_SIZE\n");
			if (load)
				printf("#undef LS_LOAD\n");
			if (update)
				printf("#undef LS_UPDATE\n");
			if (zero)
				printf("#undef LS_ZERO\n");
		    }

	printf("#undef LS_INDEXED\n");


	/*  Lookup tables for loads/stores:  */
	printf("\n\nvoid (*ppc%s_loadstore[64])(struct cpu *, struct "
	    "ppc_instr_call *) = {\n", modes[mode]);
	n = 0;
	for (update=0; update<=1; update++)
	  for (ignoreofs=0; ignoreofs<=1; ignoreofs++)
	    for (load=0; load<=1; load++)
		for (zero=0; zero<=1; zero++)
		    for (size=0; size<4; size++) {
			printf("\tppc%s_instr_", modes[mode]);

			if (load && !zero && size == 3) {
				printf("invalid");
				goto cont;
			}

			if (load)
				printf("l");
			else
				printf("st");
			printf("%s", sizechar[size]);
			if (load && size < 3) {
				if (zero)
					printf("z");
				else
					printf("a");
			}
			if (update)
				printf("u");
			if (ignoreofs)
				printf("_0");
cont:
			if (++n < 64)
				printf(",");
			printf("\n");
		    }

	printf("};\n\n");

	printf("\n\nvoid (*ppc%s_loadstore_indexed[32])(struct cpu *, struct "
	    "ppc_instr_call *) = {\n", modes[mode]);
	n = 0;
	for (update=0; update<=1; update++)
	    for (load=0; load<=1; load++)
		for (zero=0; zero<=1; zero++)
		    for (size=0; size<4; size++) {
			printf("\tppc%s_instr_", modes[mode]);

			if (load && !zero && size == 3) {
				printf("invalid");
				goto cont_x;
			}

			if (load)
				printf("l");
			else
				printf("st");
			printf("%s", sizechar[size]);
			if (load && size < 3) {
				if (zero)
					printf("z");
				else
					printf("a");
			}
			if (update)
				printf("u");
			printf("x");
cont_x:
			if (++n < 32)
				printf(",");
			printf("\n");
		    }

	printf("};\n\n");

	/*  Non-standard loads/stores:  */
	printf("#define LS_BYTEREVERSE\n"
	    "#define LS_INDEXED\n"

	    "#define LS_SIZE 2\n"
	    "#define LS_H\n"
	    "#define LS_GENERIC_N ppc%s_generic_lhbrx\n"
	    "#define LS_N ppc%s_instr_lhbrx\n"
	    "#define LS_LOAD\n"
	    "#include \"cpu_ppc_instr_loadstore.cc\"\n"
	    "#undef LS_LOAD\n"
	    "#undef LS_N\n"
	    "#undef LS_GENERIC_N\n"
	    "#define LS_GENERIC_N ppc%s_generic_sthbrx\n"
	    "#define LS_N ppc%s_instr_sthbrx\n"
	    "#include \"cpu_ppc_instr_loadstore.cc\"\n"
	    "#undef LS_N\n"
	    "#undef LS_GENERIC_N\n"
	    "#undef LS_H\n"
	    "#undef LS_SIZE\n"

	    "#define LS_SIZE 4\n"
	    "#define LS_W\n"
	    "#define LS_GENERIC_N ppc%s_generic_lwbrx\n"
	    "#define LS_N ppc%s_instr_lwbrx\n"
	    "#define LS_LOAD\n"
	    "#include \"cpu_ppc_instr_loadstore.cc\"\n"
	    "#undef LS_LOAD\n"
	    "#undef LS_N\n"
	    "#undef LS_GENERIC_N\n"
	    "#define LS_GENERIC_N ppc%s_generic_stwbrx\n"
	    "#define LS_N ppc%s_instr_stwbrx\n"
	    "#include \"cpu_ppc_instr_loadstore.cc\"\n"
	    "#undef LS_N\n"
	    "#undef LS_GENERIC_N\n"
	    "#undef LS_W\n"
	    "#undef LS_SIZE\n"

	    "#undef LS_INDEXED\n"
	    "#undef LS_BYTEREVERSE\n",
	    modes[mode], modes[mode], modes[mode], modes[mode],
	    modes[mode], modes[mode], modes[mode], modes[mode]);
}

int main(int argc, char *argv[])
{
	int mode;

	printf("\n/*  AUTOMATICALLY GENERATED! Do not edit.  */\n\n");

	for (mode = 0; mode <= 1; mode ++) {
		if (mode == 0)
			printf("#ifndef MODE32\n");
		else
			printf("#ifdef MODE32\n");

		do_it(mode);

		printf("#endif\n");
	}

	return 0;
}

