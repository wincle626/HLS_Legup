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

	printf("\n/*\n *  AUTOMATICALLY GENERATED! Do not edit.\n */\n\n");

	printf("extern size_t dyntrans_cache_size;\n");

	printf("#ifdef DYNTRANS_32\n");
	printf("#define MODE32\n");
	printf("#endif\n");

	printf("#define DYNTRANS_FUNCTION_TRACE_DEF "
	    "%s_cpu_functioncall_trace\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_FUNCTION_TRACE_DEF\n\n");

	printf("#define DYNTRANS_INIT_TABLES "
	    "%s_cpu_init_tables\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_INIT_TABLES\n\n");

	printf("#define DYNTRANS_TC_ALLOCATE_DEFAULT_PAGE_DEF "
	    "%s_tc_allocate_default_page\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_TC_ALLOCATE_DEFAULT_PAGE_DEF\n\n");

	printf("#define DYNTRANS_INVAL_ENTRY\n");
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_INVAL_ENTRY\n\n");

	printf("#define DYNTRANS_INVALIDATE_TC "
	    "%s_invalidate_translation_caches\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_INVALIDATE_TC\n\n");

	printf("#define DYNTRANS_INVALIDATE_TC_CODE "
	    "%s_invalidate_code_translation\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_INVALIDATE_TC_CODE\n\n");

	printf("#define DYNTRANS_UPDATE_TRANSLATION_TABLE "
	    "%s_update_translation_table\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_UPDATE_TRANSLATION_TABLE\n\n");

	printf("#define MEMORY_RW %s_memory_rw\n", a);
	printf("#define MEM_%s\n", uppercase(a));
	printf("#include \"memory_rw.cc\"\n");
	printf("#undef MEM_%s\n", uppercase(a));
	printf("#undef MEMORY_RW\n\n");

	printf("#define DYNTRANS_PC_TO_POINTERS_FUNC %s_pc_to_pointers\n", a);
	printf("#define DYNTRANS_PC_TO_POINTERS_GENERIC "
	    "%s_pc_to_pointers_generic\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_PC_TO_POINTERS_FUNC\n\n");
	printf("#undef DYNTRANS_PC_TO_POINTERS_GENERIC\n\n");


	printf("#define COMBINE_INSTRUCTIONS %s_combine_instructions\n", a);
	printf("#ifndef DYNTRANS_32\n");
	printf("#define reg(x) (*((uint64_t *)(x)))\n");
	printf("#define MODE_uint_t uint64_t\n");
	printf("#define MODE_int_t int64_t\n");
	printf("#else\n");
	printf("#define reg(x) (*((uint32_t *)(x)))\n");
	printf("#define MODE_uint_t uint32_t\n");
	printf("#define MODE_int_t int32_t\n");
	printf("#endif\n");
	printf("#define COMBINE(n) %s_combine_ ## n\n", a);
	printf("#include \"quick_pc_to_pointers.h\"\n");
	printf("#include \"cpu_%s_instr.cc\"\n\n", a);

	printf("#define DYNTRANS_RUN_INSTR_DEF %s_run_instr\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_RUN_INSTR_DEF\n\n");


	printf("#ifdef DYNTRANS_DUALMODE_32\n");
	printf("#undef COMBINE_INSTRUCTIONS\n");
	printf("#define COMBINE_INSTRUCTIONS %s32_combine_instructions\n", a);
	printf("#undef X\n#undef instr\n#undef reg\n"
	    "#define X(n) void %s32_instr_ ## n(struct cpu *cpu, \\\n"
	    "\tstruct %s_instr_call *ic)\n", a, a);
	printf("#define instr(n) %s32_instr_ ## n\n", a);
	printf("#ifdef HOST_LITTLE_ENDIAN\n");
	printf("#define reg(x) ( *((uint32_t *)(x)) )\n");
	printf("#else\n");
	printf("#define reg(x) ( *((uint32_t *)(x)+1) )\n");
	printf("#endif\n");
	printf("#define MODE32\n");
	printf("#undef MODE_uint_t\n#undef MODE_int_t\n");
	printf("#define MODE_uint_t uint32_t\n");
	printf("#define MODE_int_t int32_t\n");

	printf("#define DYNTRANS_INVAL_ENTRY\n");
	printf("#undef DYNTRANS_INVALIDATE_TLB_ENTRY\n"
	    "#define DYNTRANS_INVALIDATE_TLB_ENTRY "
	    "%s32_invalidate_tlb_entry\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_INVAL_ENTRY\n\n");
	printf("#define DYNTRANS_INVALIDATE_TC "
	    "%s32_invalidate_translation_caches\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_INVALIDATE_TC\n\n");
	printf("#define DYNTRANS_INVALIDATE_TC_CODE "
	    "%s32_invalidate_code_translation\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_INVALIDATE_TC_CODE\n\n");
	printf("#define DYNTRANS_UPDATE_TRANSLATION_TABLE "
	    "%s32_update_translation_table\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_UPDATE_TRANSLATION_TABLE\n\n");
	printf("#define DYNTRANS_PC_TO_POINTERS_FUNC %s32_pc_to_pointers\n", a);
	printf("#define DYNTRANS_PC_TO_POINTERS_GENERIC "
	    "%s32_pc_to_pointers_generic\n", a);
	printf("#undef DYNTRANS_PC_TO_POINTERS\n"
	    "#define DYNTRANS_PC_TO_POINTERS %s32_pc_to_pointers\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_PC_TO_POINTERS_FUNC\n\n");
	printf("#undef DYNTRANS_PC_TO_POINTERS_GENERIC\n\n");
	printf("#undef COMBINE\n");
	printf("#define COMBINE(n) %s32_combine_ ## n\n", a);
	printf("#include \"quick_pc_to_pointers.h\"\n");
	printf("#include \"cpu_%s_instr.cc\"\n", a);

	printf("\n#undef DYNTRANS_PC_TO_POINTERS\n"
	    "#define DYNTRANS_PC_TO_POINTERS %s_pc_to_pointers\n"
	    "#define DYNTRANS_PC_TO_POINTERS32 %s32_pc_to_pointers\n\n", a, a);

	printf("#define DYNTRANS_RUN_INSTR_DEF %s32_run_instr\n", a);
	printf("#include \"cpu_dyntrans.cc\"\n");
	printf("#undef DYNTRANS_RUN_INSTR_DEF\n\n");

	printf("#endif /*  DYNTRANS_DUALMODE_32  */\n\n\n");


	printf("CPU_FAMILY_INIT(%s,\"%s\")\n\n", a, b);

	return 0;
}

