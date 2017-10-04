/*
 *  Copyright (C) 2004-2009  Anders Gavare.  All rights reserved.
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
 *  Floating point emulation routines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "float_emul.h"
#include "misc.h"


/*  #define IEEE_DEBUG  */


/*
 *  ieee_interpret_float_value():
 *
 *  Interprets a float value from binary IEEE format into an ieee_float_value
 *  struct.
 */
void ieee_interpret_float_value(uint64_t x, struct ieee_float_value *fvp,
	int fmt)
{
	int n_frac = 0, n_exp = 0;
	int i, nan, sign = 0, exponent;
	double fraction;

	memset(fvp, 0, sizeof(struct ieee_float_value));

	/*  n_frac and n_exp:  */
	switch (fmt) {
	case IEEE_FMT_S:	n_frac = 23; n_exp = 8; break;
	case IEEE_FMT_W:	n_frac = 31; n_exp = 0; break;
	case IEEE_FMT_D:	n_frac = 52; n_exp = 11; break;
	case IEEE_FMT_L:	n_frac = 63; n_exp = 0; break;
	default:fatal("ieee_interpret_float_value(): "
		    "unimplemented format %i\n", fmt);
	}

	/*  exponent:  */
	exponent = 0;
	switch (fmt) {
	case IEEE_FMT_W:
		x &= 0xffffffffULL;
	case IEEE_FMT_L:
		break;
	case IEEE_FMT_S:
		x &= 0xffffffffULL;
	case IEEE_FMT_D:
		exponent = (x >> n_frac) & ((1 << n_exp) - 1);
		exponent -= (1 << (n_exp-1)) - 1;
		break;
	default:fatal("ieee_interpret_float_value(): unimplemented "
		    "format %i\n", fmt);
	}

	/*  nan:  */
	nan = 0;
	switch (fmt) {
	case IEEE_FMT_S:
		if (x == 0x7fffffffULL || x == 0x7fbfffffULL)
			nan = 1;
		break;
	case IEEE_FMT_D:
		if (x == 0x7fffffffffffffffULL ||
		    x == 0x7ff7ffffffffffffULL)
			nan = 1;
		break;
	}

	if (nan) {
		fvp->f = 1.0;
		goto no_reasonable_result;
	}

	/*  fraction:  */
	fraction = 0.0;
	switch (fmt) {
	case IEEE_FMT_W:
		{
			int32_t r_int = x;
			fraction = r_int;
		}
		break;
	case IEEE_FMT_L:
		{
			int64_t r_int = x;
			fraction = r_int;
		}
		break;
	case IEEE_FMT_S:
	case IEEE_FMT_D:
		/*  sign:  */
		sign = (x >> 31) & 1;
		if (fmt == IEEE_FMT_D)
			sign = (x >> 63) & 1;

		fraction = 0.0;
		for (i=0; i<n_frac; i++) {
			int bit = (x >> i) & 1;
			fraction /= 2.0;
			if (bit)
				fraction += 1.0;
		}
		/*  Add implicit bit 0:  */
		fraction = (fraction / 2.0) + 1.0;
		break;
	default:fatal("ieee_interpret_float_value(): "
		    "unimplemented format %i\n", fmt);
	}

	/*  form the value:  */
	fvp->f = fraction;

#ifdef IEEE_DEBUG
	fatal("{ ieee: x=%016"PRIx64" sign=%i exponent=%i frac=%f ",
	    (uint64_t) x, sign, exponent, fraction);
#endif

	/*  TODO: this is awful for exponents of large magnitude.  */
	if (exponent > 0) {
		/*
		 *  NOTE / TODO:
		 *
		 *  This is an ulgy workaround on Alpha, where it seems that
		 *  multiplying by 2, 1024 times causes a floating point
		 *  exception. (Triggered by running for example NetBSD/pmax
		 *  2.0 emulated on an Alpha host.)
		 */
		if (exponent == 1024)
			exponent = 1023;

		while (exponent-- > 0)
			fvp->f *= 2.0;
	} else if (exponent < 0) {
		while (exponent++ < 0)
			fvp->f /= 2.0;
	}

	if (sign)
		fvp->f = -fvp->f;

no_reasonable_result:
	fvp->nan = nan;

#ifdef IEEE_DEBUG
	fatal("f=%f }\n", fvp->f);
#endif
}


/*
 *  ieee_store_float_value():
 *
 *  Generates a 64-bit IEEE-formated value in a specific format.
 */
uint64_t ieee_store_float_value(double nf, int fmt, int nan)
{
	int n_frac = 0, n_exp = 0, signofs=0;
	int i, exponent;
	uint64_t r = 0, r2;
	int64_t r3;

	/*  n_frac and n_exp:  */
	switch (fmt) {
	case IEEE_FMT_S:	n_frac = 23; n_exp = 8; signofs = 31; break;
	case IEEE_FMT_W:	n_frac = 31; n_exp = 0; signofs = 31; break;
	case IEEE_FMT_D:	n_frac = 52; n_exp = 11; signofs = 63; break;
	case IEEE_FMT_L:	n_frac = 63; n_exp = 0; signofs = 63; break;
	default:fatal("ieee_store_float_value(): unimplemented format"
		    " %i\n", fmt);
	}

	if ((fmt == IEEE_FMT_S || fmt == IEEE_FMT_D) && nan)
		goto store_nan;

	/*  fraction:  */
	switch (fmt) {
	case IEEE_FMT_W:
	case IEEE_FMT_L:
		/*
		 *  This causes an implicit conversion of double to integer.
		 *  If nf < 0.0, then r2 will begin with a sequence of binary
		 *  1's, which is ok.
		 */
		r3 = (int64_t) nf;
		r2 = r3;
		r |= r2;

		if (fmt == IEEE_FMT_W)
			r &= 0xffffffffULL;
		break;
	case IEEE_FMT_S:
	case IEEE_FMT_D:
#ifdef IEEE_DEBUG
		fatal("{ ieee store f=%f ", nf);
#endif
		/*  sign bit:  */
		if (nf < 0.0) {
			r |= ((uint64_t)1 << signofs);
			nf = -nf;
		}

		/*
		 *  How to convert back from double to exponent + fraction:
		 *  The fraction should be 1.xxx, that is
		 *  1.0 <= fraction < 2.0
		 *
		 *  This method is very slow but should work:
		 *  (TODO: Fix the performance problem!)
		 */
		exponent = 0;
		while (nf < 1.0 && exponent > -1023) {
			nf *= 2.0;
			exponent --;
		}
		while (nf >= 2.0 && exponent < 1023) {
			nf /= 2.0;
			exponent ++;
		}

		/*  Here:   1.0 <= nf < 2.0  */
#ifdef IEEE_DEBUG
		fatal(" nf=%f", nf);
#endif
		nf -= 1.0;	/*  remove implicit first bit  */
		for (i=n_frac-1; i>=0; i--) {
			nf *= 2.0;
			if (nf >= 1.0) {
				r |= ((uint64_t)1 << i);
				nf -= 1.0;
			}
		}

		/*  Insert the exponent into the resulting word:  */
		/*  (First bias, then make sure it's within range)  */
		exponent += (((uint64_t)1 << (n_exp-1)) - 1);
		if (exponent < 0)
			exponent = 0;
		if (exponent >= ((int64_t)1 << n_exp))
			exponent = ((int64_t)1 << n_exp) - 1;
		r |= (uint64_t)exponent << n_frac;

		/*  Special case for 0.0:  */
		if (exponent == 0)
			r = 0;

#ifdef IEEE_DEBUG
		fatal(" exp=%i, r = %016"PRIx64" }\n", exponent, (uint64_t) r);
#endif
		break;
	default:/*  TODO  */
		fatal("ieee_store_float_value(): unimplemented format %i\n",
		    fmt);
	}

store_nan:
	if (nan) {
		if (fmt == IEEE_FMT_S)
			r = 0x7fffffffULL;
		else if (fmt == IEEE_FMT_D)
			r = 0x7fffffffffffffffULL;
		else
			r = 0x7fffffffULL;
	}

	if (fmt == IEEE_FMT_S || fmt == IEEE_FMT_W)
		r &= 0xffffffff;

	return r;
}

