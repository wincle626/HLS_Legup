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
 */

#include <stdio.h>
#include <string.h>


void generate_multi(int store, int endianness, int n)
{
	int i, nr;

	printf("X(multi_%cw_%i_%ce)\n{\n",
	    store? 's' : 'l', n, endianness? 'b' : 'l');

	printf("\tuint32_t *page;\n"
	    "\tMODE_uint_t rX = reg(ic[0].arg[1])");
	for (i=0; i<n; i++)
		printf(", r%i", i);
	printf(";\n");

	for (i=0; i<n; i++)
		printf("\tMODE_uint_t addr%i = rX + (int32_t)ic[%i].arg[2];\n",
		    i, i);
	for (i=0; i<n; i++)
		printf("\tuint32_t index%i = addr%i >> 12;\n", i, i);

	printf("\tpage = (uint32_t *) cpu->cd.mips.host_%s[index0];\n",
	    store? "store" : "load");

	printf("\tif (cpu->delay_slot ||\n"
	    "\t    page == NULL");
	for (i=0; i<n; i++)
		printf(" || (addr%i & 3)", i);
	printf("\n\t   ");
	for (i=1; i<n; i++)
		printf(" || index%i != index0", i);
	printf(") {\n");

	nr = 2*2;
	if (store)
		nr += 8;
	else
		nr += 1;
	if (endianness)
		nr += 16;
	printf("\t\tmips32_loadstore[%i](cpu, ic);\n", nr);

	printf("\t\treturn;\n\t}\n");

	for (i=0; i<n; i++)
		printf("\taddr%i = (addr%i >> 2) & 0x3ff;\n", i, i);

	if (store) {
		for (i=0; i<n; i++)
			printf("\tr%i = reg(ic[%i].arg[0]);\n", i, i);
		for (i=0; i<n; i++)
			printf("\tr%i = %cE32_TO_HOST(r%i);\n", i,
			    endianness? 'B' : 'L', i);
		for (i=0; i<n; i++)
			printf("\tpage[addr%i] = r%i;\n", i, i);
	} else {
		for (i=0; i<n; i++)
			printf("\tr%i = page[addr%i];\n", i, i);
		for (i=0; i<n; i++)
			printf("\tr%i = %cE32_TO_HOST(r%i);\n", i,
			    endianness? 'B' : 'L', i);
		for (i=0; i<n; i++)
			printf("\treg(ic[%i].arg[0]) = r%i;\n", i, i);
	}

	printf("\tcpu->n_translated_instrs += %i;\n", n - 1);
	printf("\tcpu->cd.mips.next_ic += %i;\n", n - 1);

	printf("}\n\n");
}


int main(int argc, char *argv[])
{
	int store, endianness, n;

	printf("\n/*  AUTOMATICALLY GENERATED! Do not edit.  */\n\n");

	for (endianness=0; endianness<=1; endianness++)
		for (store=0; store<=1; store++)
			for (n=2; n<=4; n++)
				generate_multi(store, endianness, n);

	return 0;
}

