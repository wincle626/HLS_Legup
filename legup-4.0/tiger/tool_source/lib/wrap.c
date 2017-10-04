#include "stdio.h"
#include "../Debug_Stub/regdef.h"	// contains definitions of STACK_ADDR & PROF_ADDR

void flushICache();

// sdram is 0x00800000 to 0x00ffffff
void wrap() {
	int ret;
	unsigned long int ProfilerStatus;
	
	// Reset ProfilerStatus 
	*(volatile unsigned long int*)(STACK_ADDR) = 0;
	
	// Wait for profiler to be initialized
	do {
		flushICache();
		ProfilerStatus = *(volatile unsigned long int*)(STACK_ADDR);
		//printf("ProfilerStatus(init) = %x %x.\n", (int)((ProfilerStatus & 0xFFFF0000) >> 16), (int)(ProfilerStatus & 0xFFFF));
	} while (ProfilerStatus != 0xDEADBEEF);
	//printf("Profiler is Initialized.\n");

	// Run actual program
	ret = main();
	//printf("main() done, waiting for results...\n");
	
	// Put in some delay so that main can fully store
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	
	// Wait for profiler retrieval to be finished
	do {
		flushICache();
		ProfilerStatus = *(volatile unsigned long int*)(STACK_ADDR);
		//printf("ProfilerStatus(retrieve) = %x %x.\n", (int)((ProfilerStatus & 0xFFFF0000) >> 16), (int)(ProfilerStatus & 0xFFFF));
	} while (ProfilerStatus != 0x0CADFEED);
	//printf("Profiler results are ready.\n");

	//printf("Cycle Count = %d\n", ProfilerStatus);
	
	/*unsigned int V;
	int i;	
	for (i=0; i<12; i++) {
		V = *(volatile unsigned long int*)((STACK_ADDR) + i*4);
		printf("%x + %d --> %x\n", STACK_ADDR, i*4, V);
	}
	printf("Finished.\n\n");*/
}
