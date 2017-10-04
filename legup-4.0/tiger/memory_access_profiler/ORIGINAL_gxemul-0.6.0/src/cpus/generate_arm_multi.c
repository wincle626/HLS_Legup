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
 *  Generation of commonly used ARM load/store multiple instructions.
 *
 *  The main idea is to first check whether a load/store would be possible
 *  without going outside a page, and if so, use the host_load or _store
 *  arrays for quick access to emulated RAM. Otherwise, fall back to using
 *  the generic bdt_load() or bdt_store().
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

/*
 *  generate_opcode():
 *
 *  Given an ARM load/store multiple opcode, produce equivalent "hardcoded"
 *  C code which emulates the opcode.
 *
 *  TODO:
 *
 *	o)  On 64-bit hosts, load/store two registers at a time. This
 *	    feature depends both on the alignment of the base register,
 *	    and the specific set of registers being loaded/stored.
 *
 *	o)  Alignment checks. (Optional?)
 *
 *	o)  For accesses that cross page boundaries, use two pages using
 *	    the fast method instead of calling the generic function?
 */
void generate_opcode(uint32_t opcode)
{
	int p, u, s, w, load, r, n_regs, i, x;

	if ((opcode & 0x0e000000) != 0x08000000) {
		fprintf(stderr, "opcode 0x%08"PRIx32" is not an ldm/stm\n",
		    opcode);
		exit(1);
	}

	r = (opcode >> 16) & 15;
	p = opcode & 0x01000000? 1 : 0;
	u = opcode & 0x00800000? 1 : 0;
	s = opcode & 0x00400000? 1 : 0;
	w = opcode & 0x00200000? 1 : 0;
	load = opcode & 0x00100000? 1 : 0;
	n_regs = 0;
	for (i=0; i<16; i++)
		if (opcode & (1 << i))
			n_regs ++;

	/*  TODO: Check for register pairs, for 64-bit load/stores  */

	if (n_regs == 0) {
		fprintf(stderr, "opcode 0x%08"PRIx32" has no registers set\n",
		    opcode);
		exit(1);
	}

	if (s) {
		fprintf(stderr, "opcode 0x%08"PRIx32" has s-bit set\n", opcode);
		exit(1);
	}

	if (r == 15) {
		fprintf(stderr, "opcode 0x%08"PRIx32" has r=15\n", opcode);
		exit(1);
	}

	printf("\nvoid arm_instr_multi_0x%08"PRIx32"(struct cpu *cpu,"
	    " struct arm_instr_call *ic) {\n", opcode);

	printf("\tunsigned char *page;\n");
	printf("\tuint32_t addr = cpu->cd.arm.r[%i];\n", r);

	if (!load && opcode & 0x8000) {
		printf("\tuint32_t tmp_pc = ((size_t)ic - (size_t)\n\t"
		    "    cpu->cd.arm.cur_ic_page) / sizeof(struct "
		    "arm_instr_call);\n"
		    "\ttmp_pc = ((cpu->pc & ~((ARM_IC_ENTRIES_PER_PAGE-1)"
		    "\n\t    << ARM_INSTR_ALIGNMENT_SHIFT)))\n"
		    "\t    + (tmp_pc << ARM_INSTR_ALIGNMENT_SHIFT) + 12;\n");
	}

	if (p)
		printf("\taddr %s 4;\n", u? "+=" : "-=");

	printf("\tpage = cpu->cd.arm.host_%s[addr >> 12];\n",
	    load? "load" : "store");

	printf("\taddr &= 0xffc;\n");

	printf("\tif (");
	switch (p*2 + u) {
	case 0:	/*  post-decrement  */
		if (n_regs > 1)
			printf("addr >= 0x%x && ", 4*(n_regs-1));
		break;
	case 1:	/*  post-increment  */
		if (n_regs > 1)
			printf("addr <= 0x%x && ", 0x1000 - 4*n_regs);
		break;
	case 2:	/*  pre-decrement  */
		if (n_regs > 1)
			printf("addr >= 0x%x && ", 4*(n_regs-1));
		break;
	case 3:	/*  pre-increment  */
		if (n_regs > 1)
			printf("addr <= 0x%x && ", 0x1000 - 4*n_regs);
		break;
	}
	printf("page != NULL) {\n");

	printf("\t\tuint32_t *p = (uint32_t *) (page + addr);\n");

	if (u) {
		x = 0;
		for (i=0; i<=15; i++) {
			if (!(opcode & (1 << i)))
				continue;

			if (load && w && i == r) {
				/*  Skip the load if we're using writeback.  */
			} else if (load) {
				if (i == 15)
					printf("\t\tcpu->pc = p[%i];\n", x);
				else
					printf("\t\tcpu->cd.arm.r[%i] = "
					    "p[%i];\n", i, x);
			} else {
				if (i == 15)
					printf("\t\tp[%i] = tmp_pc;\n", x);
				else
					printf("\t\tp[%i] = cpu->cd.arm.r"
					    "[%i];\n", x, i);
			}

			x ++;
		}
	} else {
		/*  Decrementing, but do it incrementing anyway:  */
		x = -n_regs;
		for (i=0; i<=15; i++) {
			if (!(opcode & (1 << i)))
				continue;

			x ++;

			if (load && w && i == r) {
				/*  Skip the load if we're using writeback.  */
			} else if (load) {
				if (i == 15)
					printf("\t\tcpu->pc = p[%i];\n", x);
				else
					printf("\t\tcpu->cd.arm.r[%i] = "
					    "p[%i];\n", i, x);
			} else {
				if (i == 15)
					printf("\t\tp[%i] = tmp_pc;\n", x);
				else
					printf("\t\tp[%i] = "
					    "cpu->cd.arm.r[%i];\n", x, i);
			}
		}
	}

	if (w)
		printf("\t\tcpu->cd.arm.r[%i] %s %i;\n",
		    r, u? "+=" : "-=", 4*n_regs);

	if (load && opcode & 0x8000) {
		printf("\t\tquick_pc_to_pointers(cpu);\n");
	}

	printf("\t} else\n");
	printf("\t\tinstr(bdt_%s)(cpu, ic);\n", load? "load" : "store");

	printf("}\nY(multi_0x%08"PRIx32")\n", opcode);
}


/*
 *  main():
 *
 *  Normal ARM code seems to only use about a few hundred of the 1^24 possible
 *  load/store multiple instructions. (I'm not counting the s-bit now.)
 *  Instead of having a linear array of 100s of entries, we can select a list
 *  to scan based on a few bits (*), and those lists will be shorter.
 *
 *  (*)  By running experiment_arm_multi.c on statistics gathered from running
 *       NetBSD/cats, it seems that choosing the following 8 bits results in
 *       the shortest linear lists:
 *
 *		  xxxx100P USWLnnnn llllllll llllllll
 *		           ^  ^ ^ ^        ^  ^ ^ ^	(0x00950154)
 */
int main(int argc, char *argv[])
{
	int i, j;
	int n_used[256];

	if (argc < 2) {
		fprintf(stderr, "usage: %s opcode [..]\n", argv[0]);
		exit(1);
	}

	printf("\n/*  AUTOMATICALLY GENERATED! Do not edit.  */\n\n"
	    "#include <stdio.h>\n"
	    "#include <stdlib.h>\n"
	    "#include \"cpu.h\"\n"
	    "#include \"misc.h\"\n"
	    "#define DYNTRANS_PC_TO_POINTERS arm_pc_to_pointers\n"
	    "#include \"quick_pc_to_pointers.h\"\n"
	    "#include \"arm_tmphead_1.h\"\n"
	    "\n#define instr(x) arm_instr_ ## x\n");
	printf("extern void arm_pc_to_pointers(struct cpu *);\n");
	printf("extern void arm_instr_nop(struct cpu *, "
	    "struct arm_instr_call *);\n");
	printf("extern void arm_instr_bdt_load(struct cpu *, "
	    "struct arm_instr_call *);\n");
	printf("extern void arm_instr_bdt_store(struct cpu *, "
	    "struct arm_instr_call *);\n");
	printf("\n\n");

	/*  Generate the opcode functions:  */
	for (i=1; i<argc; i++)
		generate_opcode(strtol(argv[i], NULL, 0));

	/*  Generate 256 small lookup tables:  */
	for (j=0; j<256; j++) {
		int n = 0, zz, zz0;
		for (i=1; i<argc; i++) {
			zz = strtol(argv[i], NULL, 0);
			zz = ((zz & 0x00800000) >> 16)
			    |((zz & 0x00100000) >> 14)
			    |((zz & 0x00040000) >> 13)
			    |((zz & 0x00010000) >> 12)
			    |((zz & 0x00000100) >>  5)
			    |((zz & 0x00000040) >>  4)
			    |((zz & 0x00000010) >>  3)
			    |((zz & 0x00000004) >>  2);
			if (zz == j)
				n++;
		}
		printf("\nuint32_t multi_opcode_%i[%i] = {\n", j, n+1);
		for (i=1; i<argc; i++) {
			zz = zz0 = strtol(argv[i], NULL, 0);
			zz = ((zz & 0x00800000) >> 16)
			    |((zz & 0x00100000) >> 14)
			    |((zz & 0x00040000) >> 13)
			    |((zz & 0x00010000) >> 12)
			    |((zz & 0x00000100) >>  5)
			    |((zz & 0x00000040) >>  4)
			    |((zz & 0x00000010) >>  3)
			    |((zz & 0x00000004) >>  2);
			if (zz == j)
				printf("\t0x%08x,\n", zz0);
		}
		printf("0 };\n");
	}

	/*  Generate 256 tables with function pointers:  */
	for (j=0; j<256; j++) {
		int n = 0, zz, zz0;
		for (i=1; i<argc; i++) {
			zz = strtol(argv[i], NULL, 0);
			zz = ((zz & 0x00800000) >> 16)
			    |((zz & 0x00100000) >> 14)
			    |((zz & 0x00040000) >> 13)
			    |((zz & 0x00010000) >> 12)
			    |((zz & 0x00000100) >>  5)
			    |((zz & 0x00000040) >>  4)
			    |((zz & 0x00000010) >>  3)
			    |((zz & 0x00000004) >>  2);
			if (zz == j)
				n++;
		}
		n_used[j] = n;
		if (n == 0)
			continue;
		printf("void (*multi_opcode_f_%i[%i])(struct cpu *,"
		    " struct arm_instr_call *) = {\n", j, n*16);
		for (i=1; i<argc; i++) {
			zz = zz0 = strtol(argv[i], NULL, 0);
			zz = ((zz & 0x00800000) >> 16)
			    |((zz & 0x00100000) >> 14)
			    |((zz & 0x00040000) >> 13)
			    |((zz & 0x00010000) >> 12)
			    |((zz & 0x00000100) >>  5)
			    |((zz & 0x00000040) >>  4)
			    |((zz & 0x00000010) >>  3)
			    |((zz & 0x00000004) >>  2);
			if (zz == j) {
				printf("\tarm_instr_multi_0x%08x__eq,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__ne,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__cs,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__cc,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__mi,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__pl,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__vs,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__vc,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__hi,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__ls,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__ge,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__lt,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__gt,\n", zz0);
				printf("\tarm_instr_multi_0x%08x__le,\n", zz0);
				printf("\tarm_instr_multi_0x%08x,\n", zz0);
				printf("\tarm_instr_nop,\n");
			}
		}
		printf("};\n");
	}


	printf("\nuint32_t *multi_opcode[256] = {\n");
	for (i=0; i<256; i++) {
		printf(" multi_opcode_%i,", i);
		if ((i % 4) == 0)
			printf("\n");
	}
	printf("};\n");

	printf("\nvoid (**multi_opcode_f[256])(struct cpu *,"
	    " struct arm_instr_call *) = {\n");
	for (i=0; i<256; i++) {
		if (n_used[i] > 0)
			printf(" multi_opcode_f_%i,", i);
		else
			printf(" NULL,");
		if ((i % 4) == 0)
			printf("\n");
	}
	printf("};\n");

	return 0;
}

