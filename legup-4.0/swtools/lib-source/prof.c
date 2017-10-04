
#include "profile_arm.h"

Entry entries[ARM_PROF_NUM_ENTRIES];

unsigned int legup_counters[ARM_PROF_NUM_LEGUP_COUNTERS];

int CURRENT;
int NEXT;

//#define ARM_PROF_DEBUG 1

// The PMU and L2 Cache events to monitor
extern int events[6];
extern int l2c_events[2];

// Get cycle counter value and store it in legup_counters[i]
void legup_start_counter(unsigned int i) {
    // get cycles
    unsigned long cycles;
    asm volatile("MRC p15, 0, %0, c9, c13, 0" : "=r"(cycles));

    legup_counters[i] = cycles;
}

// Return (cycle counter value - legup_counters[i])
unsigned int legup_stop_counter(unsigned int i) {
    // get cycles
    unsigned long cycles;
    asm volatile("MRC p15, 0, %0, c9, c13, 0" : "=r"(cycles));

    return cycles - legup_counters[i];
}

// initialize profiling
// this should be called by main() before any other profiling functions
void __legup_prof_init(void) {
    // set up h/w so we can use jtag uart
    // this must be done before any jtag uart can be used
    volatile unsigned int *GPIO1_DIR = (volatile unsigned int *)0xff709004;
    *GPIO1_DIR = 0x0f000000;
    volatile unsigned int *FPGA_BRIDGE_RESET =
        (volatile unsigned int *)0xFFD0501c;
    *FPGA_BRIDGE_RESET = 0x00000000;
    volatile unsigned int * WORD_IN_OCRAM = (volatile unsigned int* ) 0xffff0000;
    *WORD_IN_OCRAM = 0;

	CURRENT = -1;
	NEXT = 0;

	// initialize everything to zeros
	int i;
	for(i = 0; i < ARM_PROF_NUM_ENTRIES; i++)
	{
		int j;
		for(j = 0; j < 44; j++)
		{
			entries[i].fn_name[j] = 0;
		}
		entries[i].calls = 0;
		entries[i].cycles_self = 0;
        entries[i].cycles_hier = 0;
        for (j = 0; j < 6; j++) {
            entries[i].counts_self[j] = 0;
            entries[i].counts_hier[j] = 0;
        }
        for (j = 0; j < 2; j++) {
            entries[i].l2c_counts_self[j] = 0;
            entries[i].l2c_counts_hier[j] = 0;
        }
    }

#ifdef ARM_PROF_DEBUG
    printf("entries[] is at: %u\n", (unsigned int)&entries);
    printf("using event numbers: ");
#endif

    // set up the event counters and start them
	for(i = 0; i < 6; i++)
	{
#ifdef ARM_PROF_DEBUG
        printf("0x%x ", events[i]); // print event number
#endif
        // select event counter i using PMSELR
        asm volatile("MCR p15, 0, %0, c9, c12, 5" : : "r"(i));
        // set counter i to count event events[i] using PMXEVTYPER
        asm volatile("MCR p15, 0, %0, c9, c13, 1" : : "r"(events[i]));
        // start event counter i using PMCNTENSET register
        asm volatile("MCR p15, 0, %0, c9, c12, 1" : : "r"(1 << i));
    }
#ifdef ARM_PROF_DEBUG
    printf("\n");
#endif

    // Set L2C Event Counter Configuration Registers, leaving interrupts
    // disabled
    *L2C_EV_COUNTER0_CFG = l2c_events[0] << 2;
    *L2C_EV_COUNTER1_CFG = l2c_events[1] << 2;

    // Enable L2C Event Counters and reset the counts
    *L2C_EV_COUNTER_CTRL = 0x7;

    // start the cycle counter
    asm volatile("MCR p15, 0, %0, c9, c12, 1" : : "r"(1 << 31));

    // reset and enable cycle counter and event counters
    // by setting reset bits in PMCR
    unsigned long value;
    asm volatile("MRC p15, 0, %0, c9, c12, 0" : "=r"(value)); // read value
    value |= (1 << 0) | (1 << 1) | (1 << 2); // set reset bits and enable bit
	asm volatile("MCR p15, 0, %0, c9, c12, 0" : : "r"(value)); // write value
}


// update the counts
void update_counts(void)
{
    // stop counters by clearing the enable bit in the PMCR
    unsigned long value;
    asm volatile("MRC p15, 0, %0, c9, c12, 0" : "=r"(value)); // read value
    value &= ~(1 << 0); // clear enable bit
    asm volatile("MCR p15, 0, %0, c9, c12, 0" : : "r"(value)); // write value

    // Stop L2C event counters
    *L2C_EV_COUNTER_CTRL = 0x0;

    // get cycles
    unsigned long cycles;
    asm volatile("MRC p15, 0, %0, c9, c13, 0" : "=r"(cycles));

    // get event counters
    unsigned long counts[6];
    int i;
    for (i = 0; i < 6; i++) {
        asm volatile("MCR p15, 0, %0, c9, c12, 5" : : "r"(i));
        asm volatile("MRC p15, 0, %0, c9, c13, 2" : "=r"(counts[i]));
    }

    // L2C
    unsigned long l2c_counts[2];
    l2c_counts[0] = *L2C_EV_COUNTER0;
    l2c_counts[1] = *L2C_EV_COUNTER1;

    int ptr = CURRENT;

    // L2C
    entries[ptr].l2c_counts_self[0] += l2c_counts[0];
    entries[ptr].l2c_counts_self[1] += l2c_counts[1];

    entries[ptr].cycles_self += cycles;
    for (i = 0; i < 6; i++) {
        entries[ptr].counts_self[i] += counts[i];
    }

    while (ptr >= 0) {
        entries[ptr].cycles_hier += cycles;
        for (i = 0; i < 6; i++) {
            entries[ptr].counts_hier[i] += counts[i];
        }

        // L2C
        entries[ptr].l2c_counts_hier[0] += l2c_counts[0];
        entries[ptr].l2c_counts_hier[1] += l2c_counts[1];

        ptr = entries[ptr].parent;
    }
}

// reset and start counters
void reset_counts(void)
{
    // reset and enable cycle counter and event counters
    // by setting reset bits in PMCR
    unsigned long value;
    asm volatile("MRC p15, 0, %0, c9, c12, 0" : "=r"(value)); // read value
    value |= (1 << 0) | (1 << 1) | (1 << 2); // set reset bits and enable bit
    asm volatile("MCR p15, 0, %0, c9, c12, 0" : : "r"(value)); // write value

    // Enable L2C Event Counters and reset the counters
    *L2C_EV_COUNTER_CTRL = 0x7;
}

// this is called at the beginning of a function
// it updates the counters for it's parent, and a new entry is added if this is
// the first time this function has been called
void __legup_prof_begin(char *fn_name) {
    // stop and get counter values
    update_counts();

    int PARENT = CURRENT;

    // determine if this function call is unique
    // look through all previous entries and check to see if they have the same
    // name and parent as this one
    int i;
	int unique = 1;
	for(i = 0; i < NEXT; i++)
	{
		int same_name = 1;
		int ndx = 0;
		while(fn_name[ndx] != 0 && ndx < (ARM_PROF_NAME_SIZE - 1))
		{
			if(fn_name[ndx] != entries[i].fn_name[ndx])
			{
				same_name = 0;
		   		break;
			}
			ndx++;
		}
		// it is possible that fn_name and entries[i].fn_name are not the same,
		// but fn_name is a substring of entries[i].fn_name. check for this case.
		if(fn_name[ndx] == 0 && entries[i].fn_name[ndx] != 0)
		{
			same_name = 0;
			// however... if it has a space, this is just padding, so it is actually the same
			if(entries[i].fn_name[ndx] == ' ')
			{
				same_name = 1;
			}
		}

		if(same_name)
		{
			// TODO: may want to make this check optional in case we just want to know how many times
			// each function is called, regardless of its parent

			// check if they have the same parent as well...
			if(entries[i].parent == PARENT)
			{
				CURRENT = i;
				unique = 0;
				break;
			}
		}
	}

	// if this function call is unique, increment the indices and use a new entry
	if(unique)
	{
		entries[NEXT].parent = PARENT;
		CURRENT = NEXT;
		NEXT++;
	}


	// copy name
	int ndx = 0;
	do {
		entries[CURRENT].fn_name[ndx] = fn_name[ndx];
		ndx++;
	} while(fn_name[ndx] != 0 && ndx < (ARM_PROF_NAME_SIZE - 1));
	// pad with spaces
	while(ndx < 16) // make sure name is at least 16 characters long (for formatting)
	{
		entries[CURRENT].fn_name[ndx] = ' ';
		ndx++;
	}
	entries[CURRENT].fn_name[ndx] = 0;	// make sure we null terminate string

	// update the number of times this function call has been made
	entries[CURRENT].calls++;

	// reset counters
    reset_counts();
}

void __legup_prof_end(void) {
    // stop and get counters
    // update counters
    update_counts();

    CURRENT = entries[CURRENT].parent;

    reset_counts();
}

void __legup_prof_print(void) {
    int hier = 1; // print out hierarchical data?
    // print the header
    printf("\t\t\t\t\t\t\t\t    Cycles\t\t\t  Events (hier)\n");
    printf("index\tfunction\t\tparent\t\t\tcalls\tself\thier\t");
    int i;
    for (i = 0; i < 6; i++) {
        printf("ev 0x%x\t", events[i]);
        if (hier)
            printf("\t");
    }
    printf("L2C 0x%x\t", l2c_events[0]);
    if (hier)
        printf("\t");
    printf("L2C 0x%x", l2c_events[1]);
    printf("\n");
    for (i = 0; i < 200; i++)
        printf("=");
    printf("\n");

    // print each entry
    int p = 0;
    while (p != NEXT) {
        // index
		printf("%d\t", p);

		// function
		printf("%s\t", entries[p].fn_name);

		// parent
		if(entries[p].parent >= 0)
		{
			printf("%s\t", entries[entries[p].parent].fn_name);
		}
		else
		{
			printf("---\t\t\t");
		}

		// number of calls
		printf("%d\t", entries[p].calls);

		// self cycles
		printf("%d\t", entries[p].cycles_self);

        // hier. cycles
        printf("%d\t", entries[p].cycles_hier);

        // cycle counters
        int j;
        for (j = 0; j < 6; j++) {
            printf("%d\t", entries[p].counts_self[j]);
            if (hier)
                printf("%d\t", entries[p].counts_hier[j]);
        }
        printf("%d\t", entries[p].l2c_counts_self[0]);
        if (hier)
            printf("%d\t", entries[p].l2c_counts_hier[0]);
        printf("%d\t", entries[p].l2c_counts_self[1]);
        if (hier)
            printf("%d\t", entries[p].l2c_counts_hier[1]);

        printf("\n");
        p++;
    }
}
