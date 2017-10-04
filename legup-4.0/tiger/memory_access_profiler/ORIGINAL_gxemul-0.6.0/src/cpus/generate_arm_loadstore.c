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
#include <stdlib.h>


char *cond[16] = {
	"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
	"hi", "ls", "ge", "lt", "gt", "le", "", "" };

int main(int argc, char *argv[])
{
	int l, b, w=0, h, s, u=0, p=0, reg, c, n;
	int only_array = 0;

	if (argc == 1) {
		only_array = 1;
	} else {
		if (argc != 4) {
			fprintf(stderr, "puw missing?\n");
			exit(1);
		}
		p = atoi(argv[1]);
		u = atoi(argv[2]);
		w = atoi(argv[3]);
	}

	printf("\n/*  AUTOMATICALLY GENERATED! Do not edit.  */\n\n");
	printf("#include <stdio.h>\n#include <stdlib.h>\n"
	    "#include \"cpu.h\"\n"
	    "#include \"machine.h\"\n"
	    "#include \"memory.h\"\n"
	    "#include \"misc.h\"\n"
	    "#define DYNTRANS_PC_TO_POINTERS arm_pc_to_pointers\n"
	    "#include \"quick_pc_to_pointers.h\"\n"
	    "#define reg(x) (*((uint32_t *)(x)))\n");
	printf("extern void arm_instr_nop(struct cpu *, "
	    "struct arm_instr_call *);\n");
	printf("extern void arm_instr_invalid(struct cpu *, "
	    "struct arm_instr_call *);\n");
	printf("extern void arm_pc_to_pointers(struct cpu *);\n");

    if (!only_array)
	for (reg=0; reg<=1; reg++)
	      for (b=0; b<=1; b++)
		  for (l=0; l<=1; l++) {
			printf("#define A__NAME__general arm_instr_%s_"
			    "%s_%s_%s_%s_%s__general\n",
			    l?"load":"store", w? "w1" : "w0",
			    b? "byte" : "word", u? "u1" : "u0",
			    p? "p1" : "p0", reg? "reg" : "imm");

			printf("#define A__NAME arm_instr_%s_%s_%s_%s_%s_%s\n",
			    l? "load" : "store", w? "w1" : "w0",
			    b? "byte" : "word", u? "u1" : "u0",
			    p? "p1" : "p0", reg? "reg" : "imm");
			for (c=0; c<14; c++)
				printf("#define A__NAME__%s arm_instr_%s_"
				    "%s_%s_%s_%s_%s__%s\n",
				    cond[c], l?"load":"store", w? "w1" : "w0",
				    b? "byte" : "word", u? "u1" : "u0",
				    p? "p1" : "p0", reg? "reg" : "imm",cond[c]);

			printf("#define A__NAME_PC arm_instr_%s_%s_%s_%s_"
			    "%s_%s_pc\n", l? "load" : "store", w? "w1" : "w0",
			    b? "byte" : "word", u? "u1" : "u0",
			    p? "p1" : "p0", reg? "reg" : "imm");
			for (c=0; c<14; c++)
				printf("#define A__NAME_PC__%s arm_instr_%s_"
				    "%s_%s_%s_%s_%s_pc__%s\n",
				    cond[c], l?"load":"store", w? "w1" : "w0",
				    b? "byte" : "word", u? "u1" : "u0",
				    p? "p1" : "p0", reg? "reg" : "imm",cond[c]);

			if (l)	printf("#define A__L\n");
			if (w)	printf("#define A__W\n");
			if (b)	printf("#define A__B\n");
			if (u)	printf("#define A__U\n");
			if (p)	printf("#define A__P\n");
			if (reg)printf("#define A__REG\n");
			printf("#include \"cpu_arm_instr_loadstore.cc\"\n");
			if (l)	printf("#undef A__L\n");
			if (w)	printf("#undef A__W\n");
			if (b)	printf("#undef A__B\n");
			if (u)	printf("#undef A__U\n");
			if (p)	printf("#undef A__P\n");
			if (reg)printf("#undef A__REG\n");
			for (c=0; c<14; c++)
				printf("#undef A__NAME__%s\n", cond[c]);
			for (c=0; c<14; c++)
				printf("#undef A__NAME_PC__%s\n", cond[c]);
			printf("#undef A__NAME__general\n");
			printf("#undef A__NAME_PC\n");
			printf("#undef A__NAME\n");
		  }

	if (only_array) {
		for (reg=0; reg<=1; reg++)
		  for (p=0; p<=1; p++)
		    for (u=0; u<=1; u++)
		      for (b=0; b<=1; b++)
			for (w=0; w<=1; w++)
			  for (l=0; l<=1; l++)
			    for (c=0; c<16; c++) {
				if (c == 15)
					continue;

				printf("void arm_instr_%s_%s_%s"
				    "_%s_%s_%s%s%s(struct cpu *, struct "
				    "arm_instr_call *);",
				    l? "load" : "store",
				    w? "w1" : "w0",
				    b? "byte" : "word",
				    u? "u1" : "u0",
				    p? "p1" : "p0",
				    reg? "reg" : "imm",
				    c!=14? "__" : "", cond[c]);
				printf("void arm_instr_%s_%s_%s_%s_%s_%s_pc%s%s"
				    "(struct cpu *, struct "
				    "arm_instr_call *);\n",
				    l? "load" : "store",
				    w? "w1" : "w0",
				    b? "byte" : "word",
				    u? "u1" : "u0",
				    p? "p1" : "p0",
				    reg? "reg" : "imm",
				    c!=14? "__" : "", cond[c]);
			  }

		printf("\n\tvoid (*arm_load_store_instr[1024])(struct cpu *,\n"
		    "\t\tstruct arm_instr_call *) = {\n");
		n = 0;
		for (reg=0; reg<=1; reg++)
		  for (p=0; p<=1; p++)
		    for (u=0; u<=1; u++)
		      for (b=0; b<=1; b++)
			for (w=0; w<=1; w++)
			  for (l=0; l<=1; l++)
			    for (c=0; c<16; c++) {
				if (c == 15)
					printf("\tarm_instr_nop");
				else
					printf("\tarm_instr_%s_%s_%s"
					    "_%s_%s_%s%s%s",
					    l? "load" : "store",
					    w? "w1" : "w0",
					    b? "byte" : "word",
					    u? "u1" : "u0",
					    p? "p1" : "p0",
					    reg? "reg" : "imm",
					    c!=14? "__" : "", cond[c]);
				n++;
				if (n!=2*2*2*2*2*2*16)
					printf(",");
				printf("\n");
			  }

		printf("};\n\n");

		/*  Load/store with the pc register:  */
		printf("\n\tvoid (*arm_load_store_instr_pc[1024])"
		    "(struct cpu *,\n\t\tstruct arm_instr_call *) = {\n");
		n = 0;
		for (reg=0; reg<=1; reg++)
		  for (p=0; p<=1; p++)
		    for (u=0; u<=1; u++)
		      for (b=0; b<=1; b++)
			for (w=0; w<=1; w++)
			  for (l=0; l<=1; l++)
			    for (c=0; c<16; c++) {
				if (c == 15)
					printf("\tarm_instr_nop");
				else
					printf("\tarm_instr_%s_%s_%s_"
					    "%s_%s_%s_pc%s%s",
					    l? "load" : "store",
					    w? "w1" : "w0",
					    b? "byte" : "word",
					    u? "u1" : "u0",
					    p? "p1" : "p0",
					    reg? "reg" : "imm",
					    c!=14? "__" : "", cond[c]);
				n++;
				if (n!=2*2*2*2*2*2*16)
					printf(",");
				printf("\n");
			  }

		printf("};\n\n");
	}


	/*  "Addressing mode 3":  */

    if (!only_array)
	for (reg=0; reg<=1; reg++)
	      for (h=0; h<=1; h++)
		  for (s=0; s<=1; s++)
		    for (l=0; l<=1; l++) {
			if (s==0 && h==0)
				continue;
			/*  l=0, s=1, h=0 means LDRD  */
			/*  l=0, s=1, h=1 means STRD  */

			printf("#define A__NAME__general arm_instr_%s_"
			    "%s_%s_%s_%s_%s_%s__general\n",
			    l?"load":"store", w? "w1" : "w0",
			    s? "signed" : "unsigned",
			    h? "halfword" : "byte", u? "u1" : "u0",
			    p? "p1" : "p0", reg? "reg" : "imm");

			printf("#define A__NAME arm_instr_%s_%s_%s_%s_"
			    "%s_%s_%s\n", l? "load" : "store", w? "w1" : "w0",
			    s? "signed" : "unsigned",
			    h? "halfword" : "byte", u? "u1" : "u0",
			    p? "p1" : "p0", reg? "reg" : "imm");
			for (c=0; c<14; c++)
				printf("#define A__NAME__%s arm_instr_%s_"
				    "%s_%s_%s_%s_%s_%s__%s\n",
				    cond[c], l?"load":"store", w? "w1" : "w0",
				    s? "signed" : "unsigned",
				    h? "halfword" : "byte", u? "u1" : "u0",
				    p? "p1" : "p0", reg? "reg" : "imm",cond[c]);

			printf("#define A__NAME_PC arm_instr_%s_%s_%s_%s_%s_"
			    "%s_%s_pc\n", l? "load" : "store", w? "w1" : "w0",
			    s? "signed" : "unsigned",
			    h? "halfword" : "byte", u? "u1" : "u0",
			    p? "p1" : "p0", reg? "reg" : "imm");
			for (c=0; c<14; c++)
				printf("#define A__NAME_PC__%s arm_instr_%s_"
				    "%s_%s_%s_%s_%s_%s_pc__%s\n",
				    cond[c], l?"load":"store", w? "w1" : "w0",
				    s? "signed" : "unsigned",
				    h? "halfword" : "byte", u? "u1" : "u0",
				    p? "p1" : "p0", reg? "reg" : "imm",cond[c]);

			if (s)	printf("#define A__SIGNED\n");
			if (l)	printf("#define A__L\n");
			if (w)	printf("#define A__W\n");
			if (h)	printf("#define A__H\n");
			else	printf("#define A__B\n");
			if (u)	printf("#define A__U\n");
			if (p)	printf("#define A__P\n");
			if (reg)printf("#define A__REG\n");
			printf("#include \"cpu_arm_instr_loadstore.cc\"\n");
			if (s)	printf("#undef A__SIGNED\n");
			if (l)	printf("#undef A__L\n");
			if (w)	printf("#undef A__W\n");
			if (h)	printf("#undef A__H\n");
			else	printf("#undef A__B\n");
			if (u)	printf("#undef A__U\n");
			if (p)	printf("#undef A__P\n");
			if (reg)printf("#undef A__REG\n");
			for (c=0; c<14; c++)
				printf("#undef A__NAME__%s\n", cond[c]);
			for (c=0; c<14; c++)
				printf("#undef A__NAME_PC__%s\n", cond[c]);
			printf("#undef A__NAME__general\n");
			printf("#undef A__NAME_PC\n");
			printf("#undef A__NAME\n");
		  }

	if (only_array) {
		for (reg=0; reg<=1; reg++)
		  for (p=0; p<=1; p++)
		    for (u=0; u<=1; u++)
		      for (h=0; h<=1; h++)
			for (w=0; w<=1; w++)
			  for (s=0; s<=1; s++)
			   for (l=0; l<=1; l++)
			    for (c=0; c<16; c++) {
				if (c == 15)
					continue;
				else if (s==0 && h==0)
					continue;

				printf("void arm_instr_%s_%s_%s_%s_%s_%s_%s%s%s"
				   "(struct cpu *, struct arm_instr_call *);\n",
				    l? "load" : "store",
				    w? "w1" : "w0",
				    s? "signed" : "unsigned",
				    h? "halfword" : "byte",
				    u? "u1" : "u0", p? "p1" : "p0",
				    reg? "reg" : "imm",
				    c!=14? "__" : "", cond[c]);
				printf("void arm_instr_%s_%s_%s_%s_%s_%s_"
				    "%s_pc%s%s(struct cpu *, struct "
				    "arm_instr_call *);\n",
				    l? "load" : "store",
				    w? "w1" : "w0",
				    s? "signed" : "unsigned",
				    h? "halfword" : "byte",
				    u? "u1" : "u0", p? "p1" : "p0",
				    reg? "reg" : "imm",
				    c!=14? "__" : "", cond[c]);
			  }

		printf("\n\tvoid (*arm_load_store_instr_3[2048])"
		    "(struct cpu *,\n\t\tstruct arm_instr_call *) = {\n");
		n = 0;
		for (reg=0; reg<=1; reg++)
		  for (p=0; p<=1; p++)
		    for (u=0; u<=1; u++)
		      for (h=0; h<=1; h++)
			for (w=0; w<=1; w++)
			  for (s=0; s<=1; s++)
			   for (l=0; l<=1; l++)
			    for (c=0; c<16; c++) {
				if (c == 15)
					printf("\tarm_instr_nop");
				else if (s==0 && h==0)
					printf("\tarm_instr_invalid");
				else
					printf("\tarm_instr_%s_%s_%s_%s_"
					    "%s_%s_%s%s%s",
					    l? "load" : "store",
					    w? "w1" : "w0",
					    s? "signed" : "unsigned",
					    h? "halfword" : "byte",
					    u? "u1" : "u0", p? "p1" : "p0",
					    reg? "reg" : "imm",
					    c!=14? "__" : "", cond[c]);
				n++;
				if (n!=2*2*2*2*2*2*2*16)
					printf(",");
				printf("\n");
			  }

		printf("};\n\n");

		/*  Load/store with the pc register:  */
		printf("\n\tvoid (*arm_load_store_instr_3_pc[2048])"
		    "(struct cpu *,\n\t\tstruct arm_instr_call *) = {\n");
		n = 0;
		for (reg=0; reg<=1; reg++)
		  for (p=0; p<=1; p++)
		    for (u=0; u<=1; u++)
		      for (h=0; h<=1; h++)
			for (w=0; w<=1; w++)
			  for (s=0; s<=1; s++)
			   for (l=0; l<=1; l++)
			    for (c=0; c<16; c++) {
				if (c == 15)
					printf("\tarm_instr_nop");
				else if (s==0 && h==0)
					printf("\tarm_instr_invalid");
				else
					printf("\tarm_instr_%s_%s_%s_%s_%s_%s_"
					    "%s_pc%s%s", l? "load" : "store",
					    w? "w1" : "w0",
					    s? "signed" : "unsigned",
					    h? "halfword" : "byte",
					    u? "u1" : "u0", p? "p1" : "p0",
					    reg? "reg" : "imm",
					    c!=14? "__" : "", cond[c]);
				n++;
				if (n!=2*2*2*2*2*2*2*16)
					printf(",");
				printf("\n");
			  }

		printf("};\n\n");
	}

	return 0;
}

