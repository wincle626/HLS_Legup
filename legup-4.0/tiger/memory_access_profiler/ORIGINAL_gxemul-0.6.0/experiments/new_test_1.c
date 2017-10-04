#include <stdio.h>
#include <stdlib.h>

typedef int int32_t;

struct cpu;

struct instr_call {
	void	(*f)(struct cpu *cpu, struct instr_call *ic);
/*	int	instr_len;  */
	void	*arg[3];
};

struct cpu {
	void	*curpage;
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
	int32_t *a = (int32_t *) ic->arg[0];
	int32_t *b = (int32_t *) ic->arg[1];
	int32_t *c = (int32_t *) ic->arg[2];

	*a = (*b) + (*c);
}


void f_end(struct cpu *cpu, struct instr_call *ic)
{
	cpu->nloops--;
	if (cpu->nloops > 0) {
		cpu->next_instr_call = cpu->curpage;
		return;
	}
	/*  printf(" %i", cpu->nloops); fflush(stdout);  */
	printf("Exiting correctly\n");
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

	cpu->nloops = atoi(argv[1]);
	ncalls = 1024 + 1;

	/*  Fill a range of nonsense calls:  */
	call_array = malloc(sizeof(struct instr_call) * ncalls);
	cpu->curpage = call_array;

	printf("ncalls = %i\n", ncalls);
	for (i=0; i<ncalls; i++) {
		if (i == ncalls-1) {
			call_array[i].f = f_end;
		} else {
			call_array[i].f = f_add;
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

