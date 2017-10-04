/*
------------------------------------------------------------------------------
perfhex.c: code to generate code for a hash for perfect hashing.
(c) Bob Jenkins, December 31 1999
You may use this code in any way you wish, and it is free.  No warranty.
I hereby place this in the public domain.
Source is http://burtleburtle.net/bob/c/perfhex.c

The task of this file is to do the minimal amount of mixing needed to
find distinct (a,b) for each key when each key is a distinct ub4.  That
means trying all possible ways to mix starting with the fastest.  The
output is those (a,b) pairs and code in the *final* structure for producing
those pairs.
------------------------------------------------------------------------------
*/

#ifndef STANDARD
#include "standard.h"
#endif
#ifndef LOOKUPA
#include "lookupa.h"
#endif
#ifndef RECYCLE
#include "recycle.h"
#endif
#ifndef PERFECT
#include "perfect.h"
#endif

/*
* Guns aren't enough.  Bring out the Bomb.  Use tab[].
* This finds the initial (a,b) when we need to use tab[].
*
* We need to produce a different (a,b) every time this is called.  Try all
* reasonable cases, fastest first.
*
* The initial mix (which this determines) can be filled into final starting
* at line[1].  val is set and a,b are declared.  The final hash (at line[7])
* is a^tab[b] or a^scramble[tab[b]].
*
* The code will probably look like this, minus some stuff:
*     val += CONSTANT;
*     val ^= (val<<16);
*     val += (val>>8);
*     val ^= (val<<4);
*     b = (val >> l) & 7;
*     a = (val + (val<<m)) >> 29;
*     return a^scramble[tab[b]];
* Note that *a* and tab[b] will be computed in parallel by most modern chips.
*
* final->i is the current state of the state machine.
* final->j and final->k are counters in the loops the states simulate.
*/
static void hexn(keys, salt, alen, blen, final)
key     *keys;
ub4      salt;
ub4      alen;
ub4      blen;
gencode *final;
{
	key *mykey;
	ub4  alog = mylog2(alen);
	ub4  blog = mylog2(blen);

	// Always using case 7
	while (1) {
		ub4 addk = 0x9e3779b9*salt;

		if (!(final->j <= UB4BITS-blog)) {
			final->k++;
			if (!(final->k <= UB4BITS-alog)) {
				final->k = 0;
			}
			final->j = 0;
			final->i = 7;
			break;
		}
		
		for (mykey=keys; mykey; mykey=mykey->next_k) {
			ub4 val = mykey->hash_k + addk;
			/*if (final->highbit+1 - final->lowbit > 16)
				val ^= (val >> 16);*/
			if (final->highbit+1 - final->lowbit > 8)
				val += (val << 8);
			val ^= (val >> 4);
			mykey->b_k = (val >> final->j) & (blen-1);
			if (final->k == 0)
				mykey->a_k = val >> (UB4BITS-alog);
			else
				mykey->a_k = (val + (val << final->k)) >> (UB4BITS-alog);
		}
		sprintf(final->line[1], "  val += 0x%lx;\n", addk);
		prof_output.V1 = addk;
		
		/* CAN ASSUME THIS WON'T HAPPEN [for now] SINCE MAX DIFF IS CURRENTLY 12
		if (final->highbit+1 - final->lowbit > 16) {                   	// hnm
			sprintf(final->line[2], "  val ^= (val >> 16);\n");
			printf("error.  weird hash.\n"); exit(1);
		}*/
		if (final->highbit+1 - final->lowbit > 8) {                    	// hnn
			sprintf(final->line[3], "  val += (val << 8);\n");
			prof_output.V2 = 8;
		}
		sprintf(final->line[4], "  val ^= (val >> 4);\n");
		prof_output.V3 = 4;
		
		if (final->j == 0) {             								// hno: don't know how to reach this
			sprintf(final->line[5], "  b = val & 0x%lx;\n", blen-1);
			prof_output.B1 = 0;
			prof_output.B2 = blen-1;
		} else {														// hnp 
			sprintf(final->line[5], "  b = (val >> %ld) & 0x%lx;\n", final->j, blen-1);
			prof_output.B1 = final->j;
			prof_output.B2 = blen-1;
		}
		
		if (final->k == 0) {											// hnq
			sprintf(final->line[6], "  a = val >> %ld;\n", UB4BITS-alog);
			prof_output.A1 = 32;
			prof_output.A2 = UB4BITS-alog;
		} else {														// hnr
			sprintf(final->line[6], "  a = (val + (val << %ld)) >> %ld;\n", final->k, UB4BITS-alog);
			prof_output.A1 = final->k;
			prof_output.A2 = UB4BITS-alog;
		}

		final->j++;
		return;
	}
}



/* find the highest and lowest bit where any key differs */
static void setlow(keys, final)
key     *keys;
gencode *final;
{
	ub4  lowbit;
	ub4  highbit;
	ub4  i;
	key *mykey;
	ub4  firstkey;

	/* mark the interesting bits in final->mask */
	final->diffbits = (ub4)0;
	if (keys) firstkey = keys->hash_k;
	
	for (mykey=keys;  mykey!=(key *)0;  mykey=mykey->next_k)
		final->diffbits |= (firstkey ^ mykey->hash_k);

	/* find the lowest interesting bit */
	for (i=0; i<UB4BITS; i++) {
		if (final->diffbits & (((ub4)1)<<i)) break;
	}
	final->lowbit = i;

	/* find the highest interesting bit */
	for (i=UB4BITS; i--; ){ 
		if (final->diffbits & (((ub4)1)<<i)) break;
	}
	final->highbit = i;
}

/* 
* Initialize (a,b) when keys are integers.
*
* Normally there's an initial hash which produces a number.  That hash takes
* an initializer.  Changing the initializer causes the initial hash to 
* produce a different (uniformly distributed) number without any extra work.
*
* Well, here we start with a number.  There's no initial hash.  Any mixing
* costs extra work.  So we go through a lot of special cases to minimize the
* mixing needed to get distinct (a,b).  For small sets of keys, it's often
* fastest to skip the final hash and produce the perfect hash from the number
* directly.
*
* The target user for this is switch statement optimization.  The common case
* is 3 to 16 keys, and instruction counts matter.  The competition is a 
* binary tree of branches.
*
* Return TRUE if we found a perfect hash and no more work is needed.
* Return FALSE if we just did an initial hash and more work is needed.
*/
int inithex(keys, nkeys, alen, blen, smax, salt, final, form)
key      *keys;                                          /* list of all keys */
ub4       nkeys;                                   /* number of keys to hash */
ub4       alen;                    /* (a,b) has a in 0..alen-1, a power of 2 */
ub4       blen;                    /* (a,b) has b in 0..blen-1, a power of 2 */
ub4       smax;                   /* maximum range of computable hash values */
ub4       salt;                     /* used to initialize the hash function */
gencode  *final;                          /* output, code for the final hash */
hashform *form;                                           /* user directives */
{
	setlow(keys, final);

	if (salt == 1) {
		final->used = 8;
		final->i = 7;		// MLA -- was originally set to 1, use 7 to make constant worst-case code
		final->j = final->k = final->l = final->m = final->n = final->o = 0;
		sprintf(final->line[0], "  ub4 a, b, rsl;\n");
		sprintf(final->line[1], "\n");
		sprintf(final->line[2], "\n");
		sprintf(final->line[3], "\n");
		sprintf(final->line[4], "\n");
		sprintf(final->line[5], "\n");
		sprintf(final->line[6], "\n");
		sprintf(final->line[7], "  rsl = (a^tab[b]);\n");	// assume blen < USE_SCRAMBLE = 4096
	}
	hexn(keys, salt, alen, blen, final);
	return FALSE;
}
