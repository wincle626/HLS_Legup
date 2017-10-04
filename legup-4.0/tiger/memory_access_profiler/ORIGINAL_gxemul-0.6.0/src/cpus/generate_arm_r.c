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
 *
 *
 *  Generate functions for computing "reg" operands.
 */

#include <stdio.h>
#include <stdlib.h>


void sync_pc(void)
{
	printf("\tuint32_t tmp, low_pc = ((size_t)ic - (size_t)\n"
	    "\t    cpu->cd.arm.cur_ic_page)/sizeof(struct arm_instr_call);\n");
	printf("\ttmp = cpu->pc & ~((ARM_IC_ENTRIES_PER_PAGE-1) <<\n"
	    "\t    ARM_INSTR_ALIGNMENT_SHIFT);\n");
	printf("\ttmp += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT) + 8;\n");
}


void f(int s, int func, int only_name)
{
	int rm = func & 15;
	int c = (func >> 7) & 31;
	int t = (func >> 4) & 7;
	char name[200];
	int pc = rm == 15, rc = c >> 1;

	snprintf(name, sizeof(name), "arm_r%s_r%i_t%i_c%i", s? "s" : "",
	    rm, t, c);
	if (only_name) {
		printf("%s", name);
		return;
	}

	printf("uint32_t %s(struct cpu *cpu, struct arm_instr_call *ic)"
	    " {\n", name);
	if (pc)
		sync_pc();

	switch (t) {

	case 0:	/*  lsl c  (Logical Shift Left by constant)  */
		if (s) {
			printf("{ uint32_t x = ");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]", rm);
			printf(";\n");
			if (c != 0) {
				printf("cpu->cd.arm.flags &= ~ARM_F_C;\n");
				printf("if (x & 0x%x)\n"
				    "\tcpu->cd.arm.flags |= ARM_F_C;\n",
				    (int)(0x80000000 >> (c-1)));
				printf("x <<= %i;\n", c);
			}
			printf(" return x; }\n");
		} else {
			if (pc)
				printf("\treturn tmp");
			else
				printf("\treturn cpu->cd.arm.r[%i]", rm);
			if (c != 0)
				printf(" << %i", c);
			printf(";\n");
		}
		break;

	case 1:	/*  lsl Rc  (Logical Shift Left by register)  */
		if (s) {
			printf("{ uint32_t x = ");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]", rm);
			printf(";\n");
			printf("  uint32_t y = cpu->cd.arm.r[%i] & 255;\n", rc);
			printf("  if (y != 0) {\n");
			printf("    cpu->cd.arm.flags &= ~ARM_F_C;\n");
			printf("    if (y >= 32) return 0;\n");
			printf("    x <<= (y - 1);\n");
			printf("    if (x & 0x80000000)\n"
			    "\tcpu->cd.arm.flags |= ARM_F_C;\n");
			printf("    x <<= 1;\n");
			printf(" }\n");
			printf(" return x; }\n");
		} else {
			printf("{ uint32_t y = cpu->cd.arm.r[%i] & 255;\n", rc);
			printf("  uint32_t x =");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]", rm);
			printf(";\n");
			printf("if (y > 31) return 0; else x <<= y;\n");
			printf("return x; }\n");
		}
		break;

	case 2:	/*  lsr c  (Logical Shift Right by constant)  */
		/*  1..32  */
		if (s) {
			printf("{ uint32_t x = ");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]", rm);
			printf(";\n");
			if (c == 0)
				c = 32;
			printf("cpu->cd.arm.flags &= ~ARM_F_C;\n");
			printf("if (x & 0x%x)\n"
			    "\tcpu->cd.arm.flags |= ARM_F_C;\n",
			    (int)(1 << (c-1)));
			if (c == 32)
				printf("x = 0;\n");
			else
				printf("x >>= %i;\n", c);
			printf(" return x; }\n");
		} else {
			if (c == 0)
				printf("\treturn 0;\n");
			else {
				if (pc)
					printf("\treturn tmp");
				else
					printf("\treturn cpu->cd.arm.r[%i]",rm);
				printf(" >> %i;\n", c);
			}
		}
		break;

	case 3:	/*  lsr Rc  (Logical Shift Right by register)  */
		if (s) {
			printf("{ uint32_t x = ");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]", rm);
			printf(",y=cpu->cd.arm.r[%i]&255;\n", rc);
			printf("if(y==0) return x;\n");
			printf("cpu->cd.arm.flags &= ~ARM_F_C;\n");
			printf("if(y>31) y=32;\n");
			printf("y--; x >>= y;\n");
			printf("if (x & 1) "
			    "cpu->cd.arm.flags |= ARM_F_C;\n");
			printf(" return x >> 1; }\n");
		} else {
			printf("{ uint32_t y=cpu->cd.arm.r[%i]&255;\n", rc);
			printf("uint32_t x=");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]",rm);
			printf("; ");
			printf("if (y>=32) return 0;\n");
			printf("return x >> y; } ");
		}
		break;

	case 4:	/*  asr c  (Arithmetic Shift Right by constant)  */
		/*  1..32  */
		if (s) {
			printf("{ int32_t x = ");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]", rm);
			printf(";\n");
			if (c == 0)
				c = 32;
			printf("cpu->cd.arm.flags &= ~ARM_F_C;\n");
			printf("if (x & 0x%x)\n"
			    "\tcpu->cd.arm.flags |= ARM_F_C;\n",
			    (int)(1 << (c-1)));
			if (c == 32)
				printf("x = (x<0)? 0xffffffff : 0;\n");
			else
				printf("x >>= %i;\n", c);
			printf(" return x; }\n");
		} else {
			if (c == 0) {
				printf("\treturn ");
				if (pc)
					printf("tmp");
				else
					printf("cpu->cd.arm.r[%i]",rm);
				printf(" & 0x80000000? 0xffffffff : 0;\n");
			} else {
				printf("return (int32_t)");
				if (pc)
					printf("tmp");
				else
					printf("cpu->cd.arm.r[%i]",rm);
				printf(" >> %i;\n", c);
			}
		}
		break;

	case 5:	/*  asr Rc  (Arithmetic Shift Right by register)  */
		if (s) {
			printf("{ int32_t x = ");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]", rm);
			printf(",y=cpu->cd.arm.r[%i]&255;\n", rc);
			printf("if(y==0) return x;\n");
			printf("cpu->cd.arm.flags &= ~ARM_F_C;\n");
			printf("if(y>31) y=31;\n");
			printf("y--; x >>= y;\n");
			printf("if (x & 1) "
			    "cpu->cd.arm.flags |= ARM_F_C;\n");
			printf(" return (int32_t)x >> 1; }\n");
		} else {
			printf("{ int32_t y=cpu->cd.arm.r[%i]&255;\n", rc);
			printf("int32_t x=");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]",rm);
			printf("; ");
			printf("if (y>=31) return (x<0)?0xffffffff:0;\n");
			printf("return (int32_t)x >> y; } ");
		}
		break;

	case 6:	/*  ror c  OR rrx (Arithmetic Shift Right by constant)  */
		/*  0=rrx, 1..31=ror  */
		if (c == 0) {
			printf("{ uint64_t x=");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]",rm);
			printf("; if (cpu->cd.arm.flags & ARM_F_C)"
			    " x |= 0x100000000ULL;");
			if (s) {
				printf("cpu->cd.arm.flags &= ~ARM_F_C;"
				    "if(x&1) cpu->cd.arm.flags |= "
				    "ARM_F_C;");
			}
			printf("return x >> 1; }\n");
		} else if (s) {
			printf("{ uint64_t x = ");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]", rm);
			printf("; x |= (x << 32);\n");
			printf("cpu->cd.arm.flags &= ~ARM_F_C;\n");
			printf("if (x & 0x%x)\n"
			    "\tcpu->cd.arm.flags |= ARM_F_C;\n",
			    (int)(1 << (c-1)));
			printf(" return x >> %i; }\n", c);
		} else {
			printf("{ uint64_t x=");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]",rm);
			printf("; x |= (x << 32); ");
			printf("return x >> %i; }\n", c);
		}
		break;

	case 7:	/*  ror Rc  (Rotate Right by register)  */
		if (s) {
			printf("{ uint64_t x = ");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]", rm);
			printf("; int y=cpu->cd.arm.r[%i]&255;\n", rc);
			printf("if(y==0) return x;\n");
			printf("y --; y &= 31; x >>= y;\n");
			printf("cpu->cd.arm.flags &= ~ARM_F_C;\n");
			printf("if (x & 1) "
			    "cpu->cd.arm.flags |= ARM_F_C;\n");
			printf(" return x >> 1; }\n");
		} else {
			printf("{ int y=cpu->cd.arm.r[%i]&31;\n", rc);
			printf("uint64_t x=");
			if (pc)
				printf("tmp");
			else
				printf("cpu->cd.arm.r[%i]",rm);
			printf("; x |= (x << 32); ");
			printf("return (x >> y); } ");
		}
		break;

	default:
		printf("\tprintf(\"%s\\n\");\n", name);
		printf("\texit(1);  /*  TODO  */\n\treturn 0;\n");
	}

	printf("}\n");
}


int main(int argc, char *argv[])
{
	int s, func, f_start, f_end;

	if (argc < 3) {
		fprintf(stderr, "usage: %s start end\n", argv[0]);
		exit(1);
	}

	f_start = strtol(argv[1], NULL, 0);
	f_end   = strtol(argv[2], NULL, 0);

	printf("/*\n *  DO NOT EDIT! AUTOMATICALLY GENERATED!\n */\n\n");
	printf("#include <stdio.h>\n");
	printf("#include <stdlib.h>\n");
	printf("#include \"cpu.h\"\n");
	printf("#include \"misc.h\"\n");
	printf("\n\n");

	if (f_start != 0 || f_end != 0) {
		for (s=0; s<=1; s++)
			for (func=f_start; func<=f_end; func++)
				f(s, func, 0);
	} else {
		for (s=0; s<=1; s++)
			for (func=0; func<=0xfff; func++) {
				printf("extern uint32_t ");
				f(s, func, 1);
				printf("(struct cpu *, struct arm_"
				    "instr_call *);\n");
			}

		printf("\nuint32_t (*arm_r[8192])(struct cpu *,"
		    " struct arm_instr_call *) = {\n");
		for (s=0; s<=1; s++)
			for (func=0; func<=0xfff; func++) {
				printf("\t");
				f(s, func, 1);
				if (s!=1 || func!=0xfff)
					printf(",");
				printf("\n");
			}
		printf("};\n\n");
	}

	return 0;
}

