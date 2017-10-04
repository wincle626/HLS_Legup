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

char *cond[16] = {
	"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
	"hi", "ls", "ge", "lt", "gt", "le", "", "" };

char *op[16] = {
	"and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc",
	"tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn" };


static char *uppercase(char *l)
{
	static char staticbuf[1000];
	size_t i = 0;

	while (*l && i < sizeof(staticbuf)) {
		char u = *l++;
		if (u >= 'a' && u <= 'z')
			u -= 32;
		staticbuf[i++] = u;
	}
	if (i == sizeof(staticbuf))
		i--;
	staticbuf[i] = 0;
	return staticbuf;
}


int main(int argc, char *argv[])
{
	int n, a, reg, pc, s, c;

	printf("\n/*  AUTOMATICALLY GENERATED! Do not edit.  */\n\n");
	printf("#include <stdio.h>\n#include <stdlib.h>\n"
	    "#include \"cpu.h\"\n"
	    "#include \"misc.h\"\n"
	    "#define DYNTRANS_PC_TO_POINTERS arm_pc_to_pointers\n"
	    "#include \"quick_pc_to_pointers.h\"\n"
	    "#define reg(x) (*((uint32_t *)(x)))\n");
	printf("extern void arm_instr_nop(struct cpu *, "
	    "struct arm_instr_call *);\n");
	printf("extern void arm_instr_invalid(struct cpu *, "
	    "struct arm_instr_call *);\n");
	printf("extern void arm_pc_to_pointers(struct cpu *);\n");

	for (reg=0; reg<=2; reg++)
	  for (pc=0; pc<=1; pc++)
	    for (s=0; s<=1; s++)
	      for (a=0; a<16; a++) {
		if (a >= 8 && a <= 11 && s == 0)
			continue;
		if (reg == 2 && pc)
			continue;
		printf("#define A__NAME arm_instr_%s%s%s%s\n",
		    op[a], s? "s" : "", pc? "_pc" : "", reg?
		    (reg==2? "_regshort" : "_reg") : "");

		for (c=0; c<14; c++)
			printf("#define A__NAME__%s arm_instr_%s%s%s%s__%s\n",
			    cond[c], op[a], s? "s" : "", pc? "_pc" : "",
			    reg? (reg==2? "_regshort" : "_reg") : "", cond[c]);
		if (s)	printf("#define A__S\n");
		switch (reg) {
		case 1:	printf("#define A__REG\n"); break;
		case 2:	printf("#define A__REGSHORT\n"); break;
		}
		if (pc)	printf("#define A__PC\n");
		printf("#define A__%s\n", uppercase(op[a]));
		printf("#include \"cpu_arm_instr_dpi.cc\"\n");
		printf("#undef A__%s\n", uppercase(op[a]));
		if (s)	printf("#undef A__S\n");
		switch (reg) {
		case 1:	printf("#undef A__REG\n"); break;
		case 2:	printf("#undef A__REGSHORT\n"); break;
		}
		if (pc)	printf("#undef A__PC\n");
		for (c=0; c<14; c++)
			printf("#undef A__NAME__%s\n", cond[c]);
		printf("#undef A__NAME\n");
	      }

	printf("\n\tvoid (*arm_dpi_instr[2 * 2 * 2 * 16 * 16])(struct cpu *,\n"
	    "\t\tstruct arm_instr_call *) = {\n");
	n = 0;
	for (reg=0; reg<=1; reg++)
	  for (pc=0; pc<=1; pc++)
	    for (s=0; s<=1; s++)
	      for (a=0; a<16; a++)
		    for (c=0; c<16; c++) {
			if (c == 15)
				printf("\tarm_instr_nop");
			else if (a >= 8 && a <= 11 && s == 0)
				printf("\tarm_instr_invalid");
			else
				printf("\tarm_instr_%s%s%s%s%s%s",
				    op[a], s? "s" : "", pc? "_pc" : "",
				    reg? "_reg" : "",
				    c!=14? "__" : "", cond[c]);
			n++;
			if (n != 2 * 2 * 2 * 16 * 16)
				printf(",");
			printf("\n");
		  }

	printf("};\n\n");

	printf("\n\tvoid (*arm_dpi_instr_regshort[2 * 16 * 16])(struct cpu *,\n"
	    "\t\tstruct arm_instr_call *) = {\n");
	n = 0;
	for (s=0; s<=1; s++)
	    for (a=0; a<16; a++)
		for (c=0; c<16; c++) {
			if (c == 15)
				printf("\tarm_instr_nop");
			else if (a >= 8 && a <= 11 && s == 0)
				printf("\tarm_instr_invalid");
			else
				printf("\tarm_instr_%s%s_regshort%s%s",
				    op[a], s? "s" : "",
				    c!=14? "__" : "", cond[c]);
			n++;
			if (n != 2 * 16 * 16)
				printf(",");
			printf("\n");
		  }

	printf("};\n\n");

	return 0;
}

