/*
Idea 2007-06-06 on how to use a C compiler + linker as a native code generation backend.

  gcc native_cc_ld_test.i -Wall -O3 -fomit-frame-pointer -fpeephole -fno-builtin -c
  ld native_cc_ld_test.o -o native_cc_ld_test -e f -Ttext 0x1234560

  objdump -d native_cc_ld_test

The text part of that binary should then be easy to just copy directly into the
translation cache. (The address 0x12340040 in the example is where I want the code
fragment to end up in the cache.)  It might even be possible to only do the cc step,
and skip the ld step, if the code is position-independent.


A couple of tricks are used:

 o)  Note that the cpu and ic structs only contain just enough to mimic the cpu and ic
     structs in the emulator itself. The dummy fillers are there to make sure that
     the interesting fields (reg, next_ic, and ninstrs) end up at the correct offsets.

 o)  No #include directives are needed, if reasonable types are used (int, unsigned long long,
     etc). These will have to be detected before running the compiler. Also, this makes it
     possible to skip the preprocessor, i.e. output a .i file instead of a .c file.

 o)  Values in the cpu struct that are used are first loaded into local variables, used,
     and then stored back before any kind of return path (e.g. in a generic load/store,
     or at the end of the function, or on a non-samepage branch).

 o)  Delay slots are handled by setting a "condition", then executing the next instruction,
     then branching.  TODO: How about instructions in delay slots which may cause exceptions?

 o)  Samepage-branches can be implemented using C labels (goto).


Good:

 o)  Somewhat portable. The same mechanism could be used for amd64, Alpha, MIPS, and most likely
     several other host architectures.

 o)  A good optimizing compiler will generate very good code, probably much better code than
     I would be able to generate manually.


Bad:

 o)  Very high overhead. Calling cc + ld on my laptop takes 1/30th of a second, which is quite high.
     On my older Alpha workstation, it takes about 1/10th of a second. This means that the
     mechanism which desides whether or not to actually natively translate a block of code must
     take into account how much the overhead is vs how much time will be saved etc.

*/

struct cpu;

struct ic {
	void (*f)(struct cpu *, struct ic*);
	long arg[3];
};

struct cpu {
	char	dummy[800];
	int	reg[32];
	char	dummy2[80];
	struct ic*	next_ic;
	char	dummy3[120];
	int	ninstrs;
	void	*host_load[1048576];
	void	*host_store[1048576];
};

void f(struct cpu *cpu, struct ic *ic)
{
	int cond0;
	void (*g0)(struct cpu *, struct ic *) = (void (*)(struct cpu *, struct ic *)) 0x123801234560ULL;

	unsigned int r2 = cpu->reg[2];
	unsigned int r3 = cpu->reg[3];
	unsigned int r4 = cpu->reg[4];
	unsigned int r9 = cpu->reg[9];

	unsigned int addr0;
	unsigned char *page0;

	unsigned int ninstrs = cpu->ninstrs;

	ninstrs --;

L0:

	/*  st.b r3,r0,r2  */
	addr0 = r2;
	page0 = (unsigned char *) cpu->host_store[addr0 >> 12];
	if (page0 == (void *)0) {
		cpu->reg[2] = r2;
		cpu->reg[3] = r3;
		cpu->reg[4] = r4;
		cpu->reg[9] = r9;
		cpu->ninstrs = ninstrs;
		g0(cpu, ic + 0);
		return;
	}

	page0[addr0 & 0xfff] = r3;

	ninstrs ++;

	/*  addu r2,r2,1  */
	r2 = r2 + 1;
	ninstrs ++;

	/*  or r9,r0,r4  */
	r9 = r4;
	ninstrs ++;

	/*  bcnd.n gt0,r9,L0  */
	/*  subu r4, r4, 1  */
	cond0 = (int)r9 > 0;
	r4 = r4 - 1;
	ninstrs += 2;
	if (cond0)
		goto L0;

	cpu->reg[2] = r2;
	cpu->reg[3] = r3;
	cpu->reg[4] = r4;
	cpu->reg[9] = r9;
	cpu->ninstrs = ninstrs;

	cpu->next_ic = ic + 5;
}
