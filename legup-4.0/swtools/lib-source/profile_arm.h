#ifndef __INSTRUMENT_H__
#define __INSTRUMENT_H__

#define ARM_PROF_NUM_ENTRIES 1024
#define ARM_PROF_NUM_LEGUP_COUNTERS 64
#define ARM_PROF_NAME_SIZE 84

#include "uart.h"

struct Entry {
	char fn_name[ARM_PROF_NAME_SIZE];
    int parent;
    unsigned int calls;
    unsigned int cycles_self;
    unsigned int cycles_hier;
    unsigned int counts_self[6];
    unsigned int counts_hier[6];
    unsigned int l2c_counts_self[2];
    unsigned int l2c_counts_hier[2];
};
typedef struct Entry Entry;

void update_counts(void);

void reset_counts(void);

int events[6] = {
	0x10,
	0x12,
	0x60,
	0x61,
	0x68,
	0x70
};

int l2c_events[2] = {
    // 0x7,    // Instruction read hit in L2C
    // 0x8     // Instruction read lookup in L2C
    0x2, // Data read hit in L2C.
    0x3  // Data read lookup in L2C, subsequent hit or miss.
};

// L2C Registers
volatile unsigned int *L2C_BASE =
    (volatile unsigned int *)0xfffef000;
volatile unsigned int *L2C_EV_COUNTER_CTRL =
    (volatile unsigned int *)0xfffef200;
volatile unsigned int *L2C_EV_COUNTER0_CFG =
    (volatile unsigned int *)0xfffef204;
volatile unsigned int *L2C_EV_COUNTER1_CFG =
    (volatile unsigned int *)0xfffef208;
volatile unsigned int *L2C_EV_COUNTER0 =
    (volatile unsigned int *)0xfffef20c;
volatile unsigned int *L2C_EV_COUNTER1 =
    (volatile unsigned int *)0xfffef210;

// volatile unsigned int * L2C_ID = (volatile unsigned int* ) 0xfffef000;
// volatile unsigned int * L2C_TYPE = (volatile unsigned int* ) 0xfffef004;
// volatile unsigned int * L2C_CONTROL = (volatile unsigned int* ) 0xfffef100;
// volatile unsigned int * L2C_AUX_CONTROL = (volatile unsigned int* )
// 0xfffef104;
#endif
