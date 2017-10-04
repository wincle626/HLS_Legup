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
#include <string.h>


/*  NOTE:  Static return buffer, so calling it multiple times in the
	same printf statement with the same argument works :-)  but not
	with different args. Hahaha. Really ugly.  */
char *uppercase(char *l)
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
	char *a, *b;

	if (argc != 3) {
		fprintf(stderr, "usage: %s arch Arch\n", argv[0]);
		fprintf(stderr, "Example: %s alpha Alpha\n", argv[0]);
		fprintf(stderr, "     or: %s arm ARM\n", argv[0]);
		exit(1);
	}

	a = argv[1];
	b = argv[2];


	printf("\n/*  AUTOMATICALLY GENERATED! Do not edit.  */\n\n");

	printf("#include <assert.h>\n");
	printf("#include \"debugger.h\"\n");

	printf("#define DYNTRANS_MAX_VPH_TLB_ENTRIES "
	    "%s_MAX_VPH_TLB_ENTRIES\n", uppercase(a));
	printf("#define DYNTRANS_ARCH %s\n", a);
	printf("#define DYNTRANS_%s\n", uppercase(a));

	/*  For 64-bit platforms, arch_L2N, and arch_L3N must be defined.  */
	printf("#ifndef DYNTRANS_32\n");
	printf("#define DYNTRANS_L2N %s_L2N\n"
	    "#define DYNTRANS_L3N %s_L3N\n"
	    "#if !defined(%s_L2N) || !defined(%s_L3N)\n"
	    "#error arch_L2N, and arch_L3N must be defined for this arch!\n"
	    "#endif\n",
	    uppercase(a), uppercase(a), uppercase(a), uppercase(a));
	printf("#define DYNTRANS_L2_64_TABLE %s_l2_64_table\n"
	    "#define DYNTRANS_L3_64_TABLE %s_l3_64_table\n", a, a);
	printf("#endif\n");

	/*  Default pagesize is 4KB.  */
	printf("#ifndef DYNTRANS_PAGESIZE\n"
	    "#define DYNTRANS_PAGESIZE 4096\n"
	    "#endif\n");

	printf("#define DYNTRANS_IC %s_instr_call\n", a);
	printf("#define DYNTRANS_IC_ENTRIES_PER_PAGE "
	    "%s_IC_ENTRIES_PER_PAGE\n", uppercase(a));
	printf("#define DYNTRANS_INSTR_ALIGNMENT_SHIFT "
	    "%s_INSTR_ALIGNMENT_SHIFT\n", uppercase(a));
	printf("#define DYNTRANS_TC_PHYSPAGE %s_tc_physpage\n", a);
	printf("#define DYNTRANS_INVALIDATE_TLB_ENTRY "
	    "%s_invalidate_tlb_entry\n", a);
	printf("#define DYNTRANS_ADDR_TO_PAGENR %s_ADDR_TO_PAGENR\n",
	    uppercase(a));
	printf("#define DYNTRANS_PC_TO_IC_ENTRY %s_PC_TO_IC_ENTRY\n",
	    uppercase(a));
	printf("#define DYNTRANS_TC_ALLOCATE "
	    "%s_tc_allocate_default_page\n", a);
	printf("#define DYNTRANS_TC_PHYSPAGE %s_tc_physpage\n", a);
	printf("#define DYNTRANS_PC_TO_POINTERS %s_pc_to_pointers\n", a);
	printf("#define DYNTRANS_PC_TO_POINTERS_GENERIC "
	    "%s_pc_to_pointers_generic\n", a);
	printf("#define COMBINE_INSTRUCTIONS %s_combine_instructions\n", a);
	printf("#define DISASSEMBLE %s_cpu_disassemble_instr\n", a);

	printf("\nextern volatile int single_step, single_step_breakpoint;"
	    "\nextern int debugger_n_steps_left_before_interaction;\n"
	    "extern int old_show_trace_tree;\n"
	    "extern int old_instruction_trace;\n"
	    "extern int old_quiet_mode;\n"
	    "extern int quiet_mode;\n");

	printf("\n/* instr uses the same names as in "
	    "cpu_%s_instr.c */\n#define instr(n) %s_instr_ ## n\n\n", a, a);

	printf("#ifdef DYNTRANS_DUALMODE_32\n"
	    "#define instr32(n) %s32_instr_ ## n\n\n", a);
	printf("#endif\n\n");

	printf("\n#define X(n) void %s_instr_ ## n(struct cpu *cpu, \\\n"
	    " struct %s_instr_call *ic)\n", a, a);

	printf("\n/*\n *  nothing:  Do nothing.\n *\n"
	    " *  The difference between this function and a \"nop\" "
	    "instruction is that\n *  this function does not increase "
	    "the program counter.  It is used to \"get out\" of running in "
	    "translated\n *  mode.\n */\n");
	printf("X(nothing)\n{\n");
	printf("\tcpu->cd.%s.next_ic --;\n", a);
	printf("}\n\n");

	/*  Ugly special hacks for SH[34]:  */
	if (strcasecmp(argv[1], "sh") == 0) {
	        printf("static struct %s_instr_call nothing_call = { "
		    "instr(nothing), {0,0} };\n", a);
	} else {
	        printf("static struct %s_instr_call nothing_call = { "
		    "instr(nothing), {0,0,0} };\n", a);
	}

	printf("\n");

	return 0;
}
