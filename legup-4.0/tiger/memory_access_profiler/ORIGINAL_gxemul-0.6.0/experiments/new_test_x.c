#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

/*  #define	N_CALLS_PER_PAGE	1024  */
#define	N_CALLS_PER_PAGE	128

#define MHZ	533

struct cpu;

struct instr_call {
	void	(*f)(struct cpu *cpu, struct instr_call *ic);
/*	int	instr_len;  */
	void	*arg[3];
};

struct cpu {
	void	*curpage;
	int	orig_nloops;
	int	nloops;

	struct instr_call *next_instr_call;
};

void r(struct cpu *cpu)
{
	struct instr_call *ic;

	for (;;) {
		ic = cpu->next_instr_call++;
		ic->f(cpu, ic);

		ic = cpu->next_instr_call++;
		ic->f(cpu, ic);

		ic = cpu->next_instr_call++;
		ic->f(cpu, ic);

		ic = cpu->next_instr_call++;
		ic->f(cpu, ic);

		ic = cpu->next_instr_call++;
		ic->f(cpu, ic);

		ic = cpu->next_instr_call++;
		ic->f(cpu, ic);

		ic = cpu->next_instr_call++;
		ic->f(cpu, ic);

		ic = cpu->next_instr_call++;
		ic->f(cpu, ic);
	}
}


void f_add(struct cpu *cpu, struct instr_call *ic)
{
#if 0
#if 0
	int *a = (int *) ic->arg[0];
	int *b = (int *) ic->arg[1];
	int *c = (int *) ic->arg[2];

	*a = (*b) + (*c);
#else
	int *a = (int *) ic->arg[0];
	int *b = (int *) ic->arg[1];
	int *c = (int *) ic->arg[2];
	int *d, *e, *f;

	ic = cpu->next_instr_call++;
	d = (int *) ic->arg[0];
	e = (int *) ic->arg[1];
	f = (int *) ic->arg[2];

	*a = (*b) + (*c);
	*d = (*e) + (*f);
#endif
#endif
}


void f_sub(struct cpu *cpu, struct instr_call *ic)
{
	int *a = (int *) ic->arg[0];
	int *b = (int *) ic->arg[1];
	int *c = (int *) ic->arg[2];
	*a = (*b) - (*c);
}


void f_and(struct cpu *cpu, struct instr_call *ic)
{
	int *a = (int *) ic->arg[0];
	int *b = (int *) ic->arg[1];
	int *c = (int *) ic->arg[2];
	*a = (*b) & (*c);
}


void f_or(struct cpu *cpu, struct instr_call *ic)
{
	int *a = (int *) ic->arg[0];
	int *b = (int *) ic->arg[1];
	int *c = (int *) ic->arg[2];
	*a = (*b) | (*c);
}


void f_end(struct cpu *cpu, struct instr_call *ic)
{
	struct rusage rusage;
	double t, mips;

	cpu->nloops--;
	if (cpu->nloops > 0) {
		cpu->next_instr_call = cpu->curpage;
		return;
	}

	getrusage(RUSAGE_SELF, &rusage);
	t = rusage.ru_utime.tv_sec + rusage.ru_utime.tv_usec / 1000000.0;
	mips = (float)(N_CALLS_PER_PAGE * cpu->orig_nloops) / t
	    / 1000000.0;

	printf("%.2f user seconds = %.1f MIPS.  %.1f cycles/emulated "
	    "instruction on a %i MHz machine\n", (float)t, (float)mips,
	    (float)MHZ/mips, MHZ);
	exit(1);
}


int main(int argc, char *argv[])
{
	int32_t tmp_a, tmp_b, tmp_c;
	struct instr_call *call_array;
	int i, ncalls;
	struct cpu *cpu = malloc(sizeof(struct cpu));

	if (argc <= 1) {
		fprintf(stderr, "usage: %s n\n", argv[0]);
		exit(1);
	}

	cpu->orig_nloops = cpu->nloops = atoi(argv[1]);
	ncalls = N_CALLS_PER_PAGE + 1;

	/*  Fill a range of nonsense calls:  */
	call_array = malloc(sizeof(struct instr_call) * ncalls);
	cpu->curpage = call_array;

	printf("ncalls = %i\n", ncalls);
	for (i=0; i<ncalls; i++) {
		if (i == ncalls-1) {
			call_array[i].f = f_end;
		} else {
#if 0
			switch (i & 3) {
			case 0:	call_array[i].f = f_add;
			case 1:	call_array[i].f = f_sub;
			case 2:	call_array[i].f = f_and;
			case 3:	call_array[i].f = f_or;
			}
#else
			call_array[i].f = f_add;
#endif

			call_array[i].arg[0] = &tmp_a;
			call_array[i].arg[1] = &tmp_b;
			call_array[i].arg[2] = &tmp_c;
		}
	}

	printf("running...\n");
	cpu->next_instr_call = &call_array[0];
	r(cpu);

	printf("ERROR!\n");
	return 0;
}

