/*
+--------------------------------------------------------------------------+
| CHStone : a suite of benchmark programs for C-based High-Level Synthesis |
| ======================================================================== |
|                                                                          |
| * Collected and Modified : Y. Hara, H. Tomiyama, S. Honda,               |
|                            H. Takada and K. Ishii                        |
|                            Nagoya University, Japan                      |
|                                                                          |
| * Remark :                                                               |
|    1. This source code is modified to unify the formats of the benchmark |
|       programs in CHStone.                                               |
|    2. Test vectors are added for CHStone.                                |
|    3. If "main_result" is 0 at the end of the program, the program is    |
|       correctly executed.                                                |
|    4. Please follow the copyright of each benchmark program.             |
+--------------------------------------------------------------------------+
*/
/*============================================================================

This C source fragment is part of the SoftFloat IEC/IEEE Floating-point
Arithmetic Package, Release 2b.

Written by John R. Hauser.  This work was made possible in part by the
International Computer Science Institute, located at Suite 600, 1947 Center
Street, Berkeley, California 94704.  Funding was partially provided by the
National Science Foundation under grant MIP-9311980.  The original version
of this code was written as part of a project to build a fixed-point vector
processor in collaboration with the University of California at Berkeley,
overseen by Profs. Nelson Morgan and John Wawrzynek.  More information
is available through the Web page `http://www.cs.berkeley.edu/~jhauser/
arithmetic/SoftFloat.html.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort has
been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT TIMES
RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO PERSONS
AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ALL LOSSES,
COSTS, OR OTHER PROBLEMS THEY INCUR DUE TO THE SOFTWARE, AND WHO FURTHERMORE
EFFECTIVELY INDEMNIFY JOHN HAUSER AND THE INTERNATIONAL COMPUTER SCIENCE
INSTITUTE (possibly via similar legal notice) AGAINST ALL LOSSES, COSTS, OR
OTHER PROBLEMS INCURRED BY THEIR CUSTOMERS AND CLIENTS DUE TO THE SOFTWARE.

Derivative works are acceptable, even for commercial purposes, so long as
(1) the source code for the derivative work includes prominent notice that
the work is derivative, and (2) the source code includes prominent notice with
these four paragraphs for those parts of this code that are retained.

=============================================================================*/

/*----------------------------------------------------------------------------
| Shifts `a' right by the number of bits given in `count'.  If any nonzero
| bits are shifted off, they are ``jammed'' into the least significant bit of
| the result by setting the least significant bit to 1.  The value of `count'
| can be arbitrarily large; in particular, if `count' is greater than 64, the
| result will be either 0 or 1, depending on whether `a' is zero or nonzero.
| The result is stored in the location pointed to by `zPtr'.
*----------------------------------------------------------------------------*/

//#define DWARV
#include <stdio.h> 
#ifdef DWARV
#include <stdlib.h>
#endif
//#include "softfloat.c"
#include "milieu.h"
#include "softfloat.h"

//---------------------------------------------
// PTHREADS
//---------------------------------------------
#include <pthread.h>

// number of threads:
#define NUM_ACCEL 15

#define N 46

// number of iterations per thread
#define OPS_PER_ACCEL (N/NUM_ACCEL)

struct thread_data{
   int  startidx;
   int  maxidx;
};

//---------------------------------------------
// PTHREADS
//---------------------------------------------




#ifndef DWARV
int8 float_rounding_mode = float_round_nearest_even;
int8 float_exception_flags = 0;
#endif


INLINE /*void*/bits64
shift64RightJamming (bits64 a, int16 count)//, bits64 * zPtr)
{
  bits64 z;

  if (count == 0)
    {
      z = a;
    }
  else if (count < 64)
    {
      z = (a >> count) | ((a << ((-count) & 63)) != 0);
    }
  else
    {
      z = (a != 0);
    }
  //*zPtr = z;
  return z;
}

/*----------------------------------------------------------------------------
| Returns the number of leading 0 bits before the most-significant 1 bit of
| `a'.  If `a' is zero, 32 is returned.
*----------------------------------------------------------------------------*/

static int8
countLeadingZeros32 (bits32 a)
{
  /*static const*/ int8 countLeadingZerosHigh[256] = {
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  int8 shiftCount;

  shiftCount = 0;
  if (a < 0x10000)
    {
      shiftCount += 16;
      a <<= 16;
    }
  if (a < 0x1000000)
    {
      shiftCount += 8;
      a <<= 8;
    }
  shiftCount += countLeadingZerosHigh[a >> 24];
  return shiftCount;

}

/*----------------------------------------------------------------------------
| Returns the number of leading 0 bits before the most-significant 1 bit of
| `a'.  If `a' is zero, 64 is returned.
*----------------------------------------------------------------------------*/

static int8
countLeadingZeros64 (bits64 a)
{
  int8 shiftCount;

  shiftCount = 0;
  if (a < ((bits64) 1) << 32)
    {
      shiftCount += 32;
    }
  else
    {
      a >>= 32;
    }
  shiftCount += countLeadingZeros32 (a);
  return shiftCount;

}

/*----------------------------------------------------------------------------
| Underflow tininess-detection mode, statically initialized to default value.
| (The declaration in `softfloat.h' must match the `int8' type here.)
*----------------------------------------------------------------------------*/
#define float_detect_tininess float_tininess_before_rounding

/*----------------------------------------------------------------------------
| Raises the exceptions specified by `flags'.  Floating-point traps can be
| defined here if desired.  It is currently not possible for such a trap
| to substitute a result value.  If traps are not implemented, this routine
| should be simply `float_exception_flags |= flags;'.
*----------------------------------------------------------------------------*/
#ifdef DWARV
void
float_raise (int8 flags, int8 float_exception_flags)
#else
void
float_raise (int8 flags)
#endif
{
  float_exception_flags |= flags;

}


/*----------------------------------------------------------------------------
| The pattern for a default generated double-precision NaN.
*----------------------------------------------------------------------------*/
#define float64_default_nan LIT64( 0x7FFFFFFFFFFFFFFF )

/*----------------------------------------------------------------------------
| Returns 1 if the double-precision floating-point value `a' is a NaN;
| otherwise returns 0.
*----------------------------------------------------------------------------*/

flag
float64_is_nan (float64 a)
{

  return (LIT64 (0xFFE0000000000000) < (bits64) (a << 1));

}

/*----------------------------------------------------------------------------
| Returns 1 if the double-precision floating-point value `a' is a signaling
| NaN; otherwise returns 0.
*----------------------------------------------------------------------------*/

flag
float64_is_signaling_nan (float64 a)
{

  return (((a >> 51) & 0xFFF) == 0xFFE) && (a & LIT64 (0x0007FFFFFFFFFFFF));

}

/*----------------------------------------------------------------------------
| Takes two double-precision floating-point values `a' and `b', one of which
| is a NaN, and returns the appropriate NaN result.  If either `a' or `b' is a
| signaling NaN, the invalid exception is raised.
*----------------------------------------------------------------------------*/
#ifdef DWARV
static float64
propagateFloat64NaN (float64 a, float64 b, int8 float_exception_flags)
#else
static float64
propagateFloat64NaN (float64 a, float64 b)
#endif
{
  flag aIsNaN, aIsSignalingNaN, bIsNaN, bIsSignalingNaN;

  aIsNaN = float64_is_nan (a);
  aIsSignalingNaN = float64_is_signaling_nan (a);
  bIsNaN = float64_is_nan (b);
  bIsSignalingNaN = float64_is_signaling_nan (b);
  a |= LIT64 (0x0008000000000000);
  b |= LIT64 (0x0008000000000000);
  if (aIsSignalingNaN | bIsSignalingNaN)
#ifdef DWARV
    float_raise (float_flag_invalid, float_exception_flags);
#else
    float_raise (float_flag_invalid);
#endif
  return bIsSignalingNaN ? b : aIsSignalingNaN ? a : bIsNaN ? b : a;

}
/*----------------------------------------------------------------------------
| Floating-point rounding mode, extended double-precision rounding precision,
| and exception flags.
*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
| Primitive arithmetic functions, including multi-word arithmetic, and
| division and square root approximations.  (Can be specialized to target if
| desired.)
*----------------------------------------------------------------------------*/
//#include "softfloat-macros"

/*----------------------------------------------------------------------------
| Functions and definitions to determine:  (1) whether tininess for underflow
| is detected before or after rounding by default, (2) what (if anything)
| happens when exceptions are raised, (3) how signaling NaNs are distinguished
| from quiet NaNs, (4) the default generated quiet NaNs, and (5) how NaNs
| are propagated from function inputs to output.  These details are target-
| specific.
*----------------------------------------------------------------------------*/
//#include "softfloat-specialize"

/*----------------------------------------------------------------------------
| Returns the fraction bits of the double-precision floating-point value `a'.
*----------------------------------------------------------------------------*/

INLINE bits64
extractFloat64Frac (float64 a)
{

  return a & LIT64 (0x000FFFFFFFFFFFFF);

}

/*----------------------------------------------------------------------------
| Returns the exponent bits of the double-precision floating-point value `a'.
*----------------------------------------------------------------------------*/

INLINE int16
extractFloat64Exp (float64 a)
{

  return (a >> 52) & 0x7FF;

}

/*----------------------------------------------------------------------------
| Returns the sign bit of the double-precision floating-point value `a'.
*----------------------------------------------------------------------------*/

INLINE flag
extractFloat64Sign (float64 a)
{

  return a >> 63;

}

/*----------------------------------------------------------------------------
| Packs the sign `zSign', exponent `zExp', and significand `zSig' into a
| double-precision floating-point value, returning the result.  After being
| shifted into the proper positions, the three fields are simply added
| together to form the result.  This means that any integer portion of `zSig'
| will be added into the exponent.  Since a properly normalized significand
| will have an integer portion equal to 1, the `zExp' input should be 1 less
| than the desired result exponent whenever `zSig' is a complete, normalized
| significand.
*----------------------------------------------------------------------------*/

INLINE float64
packFloat64 (flag zSign, int16 zExp, bits64 zSig)
{

  return (((bits64) zSign) << 63) + (((bits64) zExp) << 52) + zSig;

}

/*----------------------------------------------------------------------------
| Takes an abstract floating-point value having sign `zSign', exponent `zExp',
| and significand `zSig', and returns the proper double-precision floating-
| point value corresponding to the abstract input.  Ordinarily, the abstract
| value is simply rounded and packed into the double-precision format, with
| the inexact exception raised if the abstract input cannot be represented
| exactly.  However, if the abstract value is too large, the overflow and
| inexact exceptions are raised and an infinity or maximal finite value is
| returned.  If the abstract value is too small, the input value is rounded
| to a subnormal number, and the underflow and inexact exceptions are raised
| if the abstract input cannot be represented exactly as a subnormal double-
| precision floating-point number.
|     The input significand `zSig' has its binary point between bits 62
| and 61, which is 10 bits to the left of the usual location.  This shifted
| significand must be normalized or smaller.  If `zSig' is not normalized,
| `zExp' must be 0; in that case, the result returned is a subnormal number,
| and it must not require rounding.  In the usual case that `zSig' is
| normalized, `zExp' must be 1 less than the ``true'' floating-point exponent.
| The handling of underflow and overflow follows the IEC/IEEE Standard for
| Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/
#ifdef DWARV
static float64
roundAndPackFloat64 (flag zSign, int16 zExp, bits64 zSig, int8 float_rounding_mode, int8 float_exception_flags)
#else
static float64
roundAndPackFloat64 (flag zSign, int16 zExp, bits64 zSig)
#endif
{
  int8 roundingMode;
  flag roundNearestEven, isTiny;
  int16 roundIncrement, roundBits;

  roundingMode = float_rounding_mode;
  roundNearestEven = (roundingMode == float_round_nearest_even);
  roundIncrement = 0x200;
  if (!roundNearestEven)
    {
      if (roundingMode == float_round_to_zero)
	{
	  roundIncrement = 0;
	}
      else
	{
	  roundIncrement = 0x3FF;
	  if (zSign)
	    {
	      if (roundingMode == float_round_up)
		roundIncrement = 0;
	    }
	  else
	    {
	      if (roundingMode == float_round_down)
		roundIncrement = 0;
	    }
	}
    }
  roundBits = zSig & 0x3FF;
  if (0x7FD <= (bits16) zExp)
    {
      if ((0x7FD < zExp)
	  || ((zExp == 0x7FD) && ((sbits64) (zSig + roundIncrement) < 0)))
	{
#ifdef DWARV
	  float_raise (float_flag_overflow | float_flag_inexact, float_exception_flags);
#else
	  float_raise (float_flag_overflow | float_flag_inexact);
#endif
	  return packFloat64 (zSign, 0x7FF, 0) - (roundIncrement == 0);
	}
      if (zExp < 0)
	{
	  isTiny = (float_detect_tininess == float_tininess_before_rounding)
	    || (zExp < -1)
	    || (zSig + roundIncrement < LIT64 (0x8000000000000000));
	  zSig = shift64RightJamming (zSig, -zExp);//, &zSig);
	  zExp = 0;
	  roundBits = zSig & 0x3FF;
	  if (isTiny && roundBits)
#ifdef DWARV
	    float_raise (float_flag_underflow, float_exception_flags);
#else
	    float_raise (float_flag_underflow);
#endif
	}
    }
  if (roundBits)
    float_exception_flags |= float_flag_inexact;
  zSig = (zSig + roundIncrement) >> 10;
  zSig &= ~(((roundBits ^ 0x200) == 0) & roundNearestEven);
  if (zSig == 0)
    zExp = 0;

  return packFloat64 (zSign, zExp, zSig);
}

/*----------------------------------------------------------------------------
| Takes an abstract floating-point value having sign `zSign', exponent `zExp',
| and significand `zSig', and returns the proper double-precision floating-
| point value corresponding to the abstract input.  This routine is just like
| `roundAndPackFloat64' except that `zSig' does not have to be normalized.
| Bit 63 of `zSig' must be zero, and `zExp' must be 1 less than the ``true''
| floating-point exponent.
*----------------------------------------------------------------------------*/
#ifdef DWARV
static float64
normalizeRoundAndPackFloat64 (flag zSign, int16 zExp, bits64 zSig, int8 float_rounding_mode, int8 float_exception_flags)
#else
static float64
normalizeRoundAndPackFloat64 (flag zSign, int16 zExp, bits64 zSig)
#endif
{
  int8 shiftCount;

  shiftCount = countLeadingZeros64 (zSig) - 1;
#ifdef DWARV
  return roundAndPackFloat64 (zSign, zExp - shiftCount, zSig << shiftCount, float_rounding_mode, float_exception_flags);
#else
  return roundAndPackFloat64 (zSign, zExp - shiftCount, zSig << shiftCount);
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of adding the absolute values of the double-precision
| floating-point values `a' and `b'.  If `zSign' is 1, the sum is negated
| before being returned.  `zSign' is ignored if the result is a NaN.
| The addition is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/
#ifdef DWARV
static float64
addFloat64Sigs (float64 a, float64 b, flag zSign, int8 float_rounding_mode, int8 float_exception_flags)
#else
static float64
addFloat64Sigs (float64 a, float64 b, flag zSign)
#endif
{
  int16 aExp, bExp, zExp;
  bits64 aSig, bSig, zSig;
  int16 expDiff;

  aSig = extractFloat64Frac (a);
  aExp = extractFloat64Exp (a);
  bSig = extractFloat64Frac (b);
  bExp = extractFloat64Exp (b);
  expDiff = aExp - bExp;
  aSig <<= 9;
  bSig <<= 9;
  if (0 < expDiff)
    {
      if (aExp == 0x7FF)
	{
	  if (aSig)
#ifdef DWARV
	    return propagateFloat64NaN (a, b, float_exception_flags);
#else
	    return propagateFloat64NaN (a, b);
#endif
	  return a;
	}
      if (bExp == 0)
	--expDiff;
      else
	bSig |= LIT64 (0x2000000000000000);
      bSig = shift64RightJamming (bSig, expDiff);//, &bSig);
      zExp = aExp;
    }
  else if (expDiff < 0)
    {
      if (bExp == 0x7FF)
	{
	  if (bSig)
#ifdef DWARV
	    return propagateFloat64NaN (a, b, float_exception_flags);
#else
	    return propagateFloat64NaN (a, b);
#endif
	  return packFloat64 (zSign, 0x7FF, 0);
	}
      if (aExp == 0)
	++expDiff;
      else
	{
	  aSig |= LIT64 (0x2000000000000000);
	}
      aSig = shift64RightJamming (aSig, -expDiff);//, &aSig);
      zExp = bExp;
    }
  else
    {
      if (aExp == 0x7FF)
	{
	  if (aSig | bSig)
#ifdef DWARV
	    return propagateFloat64NaN (a, b, float_exception_flags);
#else
	    return propagateFloat64NaN (a, b);
#endif
	  return a;
	}
      if (aExp == 0)
	return packFloat64 (zSign, 0, (aSig + bSig) >> 9);
      zSig = LIT64 (0x4000000000000000) + aSig + bSig;
      zExp = aExp;
      goto roundAndPack;
    }
  aSig |= LIT64 (0x2000000000000000);
  zSig = (aSig + bSig) << 1;
  --zExp;
  if ((sbits64) zSig < 0)
    {
      zSig = aSig + bSig;
      ++zExp;
    }
roundAndPack:
#ifdef DWARV
  return roundAndPackFloat64 (zSign, zExp, zSig, float_rounding_mode, float_exception_flags);
#else
  return roundAndPackFloat64 (zSign, zExp, zSig);
#endif
}

/*----------------------------------------------------------------------------
| Returns the result of subtracting the absolute values of the double-
| precision floating-point values `a' and `b'.  If `zSign' is 1, the
| difference is negated before being returned.  `zSign' is ignored if the
| result is a NaN.  The subtraction is performed according to the IEC/IEEE
| Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/
#ifdef DWARV
static float64
subFloat64Sigs (float64 a, float64 b, flag zSign, int8 float_rounding_mode, int8 float_exception_flags)
#else
static float64
subFloat64Sigs (float64 a, float64 b, flag zSign)
#endif
{
  int16 aExp, bExp, zExp;
  bits64 aSig, bSig, zSig;
  int16 expDiff;

  aSig = extractFloat64Frac (a);
  aExp = extractFloat64Exp (a);
  bSig = extractFloat64Frac (b);
  bExp = extractFloat64Exp (b);
  expDiff = aExp - bExp;
  aSig <<= 10;
  bSig <<= 10;
 if (0 < expDiff)
   goto aExpBigger;
 if (expDiff < 0)
    goto bExpBigger;
  if (aExp == 0x7FF)
    {
      if (aSig | bSig)
#ifdef DWARV
	    return propagateFloat64NaN (a, b, float_exception_flags);
#else
	    return propagateFloat64NaN (a, b);
#endif
#ifdef DWARV
      float_raise (float_flag_invalid, float_exception_flags);
#else
      float_raise (float_flag_invalid);
#endif
      return float64_default_nan;
    }
  if (aExp == 0)
    {
      aExp = 1;
      bExp = 1;
    }
  if (bSig < aSig)
    goto aBigger;
  if (aSig < bSig)
    goto bBigger;
  return packFloat64 (float_rounding_mode == float_round_down, 0, 0);
bExpBigger:
  if (bExp == 0x7FF)
    {
      if (bSig)
#ifdef DWARV
	    return propagateFloat64NaN (a, b, float_exception_flags);
#else
	    return propagateFloat64NaN (a, b);
#endif
      return packFloat64 (zSign ^ 1, 0x7FF, 0);
    }
  if (aExp == 0)
    ++expDiff;
  else
    aSig |= LIT64 (0x4000000000000000);
  aSig = shift64RightJamming (aSig, -expDiff);//, &aSig);
  bSig |= LIT64 (0x4000000000000000);
bBigger:
  zSig = bSig - aSig;
  zExp = bExp;
  zSign ^= 1;
  goto normalizeRoundAndPack;
aExpBigger:
  if (aExp == 0x7FF)
    {
      if (aSig)
#ifdef DWARV
	    return propagateFloat64NaN (a, b, float_exception_flags);
#else
	    return propagateFloat64NaN (a, b);
#endif
      return a;
    }
  if (bExp == 0)
    --expDiff;
  else
    bSig |= LIT64 (0x4000000000000000);
 bSig = shift64RightJamming (bSig, expDiff);//, &bSig);
  aSig |= LIT64 (0x4000000000000000);
aBigger:
  zSig = aSig - bSig;
  zExp = aExp;
normalizeRoundAndPack:
  --zExp;
#ifdef DWARV
  return normalizeRoundAndPackFloat64 (zSign, zExp, zSig, float_rounding_mode, float_exception_flags);
#else
  return normalizeRoundAndPackFloat64 (zSign, zExp, zSig);
#endif
}

/*----------------------------------------------------------------------------
| Returns the result of adding the double-precision floating-point values `a'
| and `b'.  The operation is performed according to the IEC/IEEE Standard for
| Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/
#pragma map generate_hw
#ifdef DWARV
float64
float64_add (float64 a, float64 b, int8 float_rounding_mode, int8 float_exception_flags)
#else
__attribute__((noinline))
float64
float64_add (float64 a, float64 b)
#endif
{
  flag aSign, bSign;

  aSign = extractFloat64Sign (a);
  bSign = extractFloat64Sign (b);

  if (aSign == bSign)
#ifdef DWARV
    return addFloat64Sigs (a, b, aSign, float_rounding_mode, float_exception_flags);
#else
    return addFloat64Sigs (a, b, aSign);
#endif
 else
#ifdef DWARV
    return subFloat64Sigs (a, b, aSign,float_rounding_mode, float_exception_flags);
#else
    return subFloat64Sigs (a, b, aSign);
#endif
a=b;
}

double
ullong_to_double (unsigned long long x)
{
  union
  {
    double d;
    unsigned long long ll;
  } t;

  t.ll = x;
  return t.d;
}

/*
+--------------------------------------------------------------------------+
| * Test Vectors (added for CHStone)                                       |
|     a_input, b_input : input data                                        |
|     z_output : expected output data                                      |
+--------------------------------------------------------------------------+
*/
const float64 a_input[N] = {
  0xBFF8000000000000ULL,	/*41 -1.5 */
  0xC000000000000000ULL,	/*15 -2.0 */
  0xBFF8000000000000ULL,	/*23 -1.5 */
  0x4000000000000000ULL,	/*3 2.0 */
  0x7FF8000000000000ULL,	/*0 nan */
  0x7FF0000000000000ULL,	/*1 inf */
  0x4000000000000000ULL,	/*2 2.0 */
  0x3FF0000000000000ULL,	/*4 1.0 */
  0x3FF0000000000000ULL,	/*5 1.0 */
  0x0000000000000000ULL,	/*6 0.0 */
  0x3FF8000000000000ULL,	/*7 1.5 */
  0x7FF8000000000000ULL,	/*8 nan */
  0x7FF0000000000000ULL,	/*9 inf */
  0x0000000000000000ULL,	/*10 0.0 */
  0x3FF8000000000000ULL,	/*11 1.5 */
  0xFFF8000000000000ULL,	/*12 nan */
  0xFFF0000000000000ULL,	/*13 -inf */
  0xC000000000000000ULL,	/*14 -2.0 */
  0xBFF0000000000000ULL,	/*16 -1.0 */
  0xBFF0000000000000ULL,	/*17 -1.0 */
  0x8000000000000000ULL,	/*18 -0.0 */
  0xBFF8000000000000ULL,	/*19 -1.5 */
  0xFFF8000000000000ULL,	/*20 nan */
  0xFFF0000000000000ULL,	/*21 -inf */
  0x8000000000000000ULL,	/*22 -0.0 */
  0x7FF8000000000000ULL,	/*24 nan */
  0x7FF0000000000000ULL,	/*25 inf */
  0x3FF0000000000000ULL,	/*26 1.0 */
  0x3FF0000000000000ULL,	/*27 1.0 */
  0x3FF0000000000000ULL,	/*28 1.0 */
  0x0000000000000000ULL,	/*29 0.0 */
  0x3FF8000000000000ULL,	/*30 1.5 */
  0x7FF8000000000000ULL,	/*31 nan */
  0x7FF0000000000000ULL,	/*32 inf */
  0x3FF0000000000000ULL,	/*33 1.0 */
  0x4000000000000000ULL,	/*34 2.0 */
  0xFFF0000000000000ULL,	/*35 -inf */
  0xFFF0000000000000ULL,	/*36 -inf */
  0xBFF0000000000000ULL,	/*37 -1.0 */
  0xBFF0000000000000ULL,	/*38 -1.0 */
  0xBFF0000000000000ULL,	/*39 -1.0 */
  0x8000000000000000ULL,	/*40 -0.0 */
  0xFFF8000000000000ULL,	/*42 nan */
  0xFFF0000000000000ULL,	/*43 -inf */
  0xBFF0000000000000ULL,	/*44 -1.0 */
  0xC000000000000000ULL		/*45 -2.0 */
};

const float64 b_input[N] = {
  0x4000000000000000ULL,	/*41 2.0 */
  0xBFF8000000000000ULL,	/*15 -1.5 */
  0xBFF0000000000000ULL,	/*23 -1.0 */
  0x3FF8000000000000ULL,	/*3 1.5 */
  0x3FF0000000000000ULL,	/*0 1.0 */
  0x3FF0000000000000ULL,	/*1 1.0 */
  0x0000000000000000ULL,	/*2 0.0 */
  0x7FF8000000000000ULL,	/*4 nan */
  0x7FF0000000000000ULL,	/*5 inf */
  0x4000000000000000ULL,	/*6 2.0 */
  0x4000000000000000ULL,	/*7 2.0 */
  0x7FF0000000000000ULL,	/*8 inf */
  0x7FF0000000000000ULL,	/*9 inf */
  0x0000000000000000ULL,	/*10 0.0 */
  0x3FF0000000000000ULL,	/*11 1.0 */
  0xBFF0000000000000ULL,	/*12 -1.0 */
  0xBFF0000000000000ULL,	/*13 -1.0 */
  0x8000000000000000ULL,	/*14 -0.0 */
  0xFFF8000000000000ULL,	/*16 nan */
  0xFFF0000000000000ULL,	/*17 -inf */
  0xC000000000000000ULL,	/*18 -2.0 */
  0xC000000000000000ULL,	/*19 -2.0 */
  0xFFF0000000000000ULL,	/*20 -inf */
  0xFFF0000000000000ULL,	/*21 -inf */
  0x8000000000000000ULL,	/*22 -inf */
  0xFFF0000000000000ULL,	/*24 -inf */
  0xFFF0000000000000ULL,	/*25 -inf */
  0xBFF0000000000000ULL,	/*26 -1.0 */
  0xFFF8000000000000ULL,	/*27 nan */
  0xFFF0000000000000ULL,	/*28 -inf */
  0xBFF0000000000000ULL,	/*29 -1.0 */
  0xC000000000000000ULL,	/*30 -2.0 */
  0xBFF0000000000000ULL,	/*31 -1.0 */
  0xBFF0000000000000ULL,	/*32 -1.0 */
  0x8000000000000000ULL,	/*33 -0.0 */
  0xBFF8000000000000ULL,	/*34 -1.5 */
  0x7FF8000000000000ULL,	/*35 nan */
  0x7FF0000000000000ULL,	/*36 inf */
  0x3FF0000000000000ULL,	/*37 1.0 */
  0x7FF8000000000000ULL,	/*38 nan */
  0x7FF0000000000000ULL,	/*39 inf */
  0x3FF0000000000000ULL,	/*40 1.0 */
  0x3FF0000000000000ULL,	/*42 1.0 */
  0x3FF0000000000000ULL,	/*43 1.0 */
  0x0000000000000000ULL,	/*44 0.0 */
  0x3FF8000000000000ULL		/*45 1.5 */
};

const float64 z_output[N] = {
  0x3FE0000000000000ULL,	/*41 0.5 */
  0xC00C000000000000ULL,	/*15 -3.5 */
  0xC004000000000000ULL,	/*23 -2.5 */
  0x400C000000000000ULL,	/*3 3.5 */
  0x7FF8000000000000ULL,	/*0 nan */
  0x7FF0000000000000ULL,	/*1 inf */
  0x4000000000000000ULL,	/*2 2.0 */
  0x7FF8000000000000ULL,	/*4 nan */
  0x7FF0000000000000ULL,	/*5 inf */
  0x4000000000000000ULL,	/*6 2.0 */
  0x400C000000000000ULL,	/*7 3.5 */
  0x7FF8000000000000ULL,	/*8 nan */
  0x7FF0000000000000ULL,	/*9 inf */
  0x0000000000000000ULL,	/*10 0.0 */
  0x4004000000000000ULL,	/*11 2.5 */
  0xFFF8000000000000ULL,	/*12 nan */
  0xFFF0000000000000ULL,	/*13 -inf */
  0xC000000000000000ULL,	/*14 -2.0 */
  0xFFF8000000000000ULL,	/*16 nan */
  0xFFF0000000000000ULL,	/*17 -inf */
  0xC000000000000000ULL,	/*18 -2.0 */
  0xC00C000000000000ULL,	/*19 -3.5 */
  0xFFF8000000000000ULL,	/*20 nan */
  0xFFF0000000000000ULL,	/*21 -inf */
  0x8000000000000000ULL,	/*22 -0.0 */
  0x7FF8000000000000ULL,	/*24 nan */
  0x7FFFFFFFFFFFFFFFULL,	/*25 nan */
  0x0000000000000000ULL,	/*26 0.0 */
  0xFFF8000000000000ULL,	/*27 nan */
  0xFFF0000000000000ULL,	/*28 -inf */
  0xBFF0000000000000ULL,	/*29 -1.0 */
  0xBFE0000000000000ULL,	/*30 -0.5 */
  0x7FF8000000000000ULL,	/*31 nan */
  0x7FF0000000000000ULL,	/*32 inf */
  0x3FF0000000000000ULL,	/*33 1.0 */
  0x3FE0000000000000ULL,	/*34 0.5 */
  0x7FF8000000000000ULL,	/*35 nan */
  0x7FFFFFFFFFFFFFFFULL,	/*36 nan */
  0x0000000000000000ULL,	/*37 0.0 */
  0x7FF8000000000000ULL,	/*38 nan */
  0x7FF0000000000000ULL,	/*39 inf */
  0x3FF0000000000000ULL,	/*40 1.0 */
  0xFFF8000000000000ULL,	/*42 nan */
  0xFFF0000000000000ULL,	/*43 -inf */
  0xBFF0000000000000ULL,	/*44 -1.0 */
  0xBFE0000000000000ULL		/*45 -0.5 */
};

void* fadd_thread(void* threadarg) {
    
    int i, main_result=0;
	struct thread_data* arg = (struct thread_data*) threadarg;
	int startidx = arg->startidx;
	int maxidx = arg->maxidx;
	for (i=startidx; i<maxidx; i++)
	{
	  float64 result;
	  result = float64_add (a_input[i], b_input[i]);
	  main_result += (result == z_output[i]);

	}

    pthread_exit((void*)main_result);
}


int
main ()
{
  int main_result = 0;
  int i;
  float64 x1, x2;
  /*a_input,b_input*/

  pthread_t threads[NUM_ACCEL];
  int result[NUM_ACCEL]={0};
  struct thread_data data[NUM_ACCEL];

  for (i=0; i<NUM_ACCEL; i++) {
      // initialize structs to pass into accels
      data[i].startidx = i*OPS_PER_ACCEL;
      if (i == NUM_ACCEL-1) {
          data[i].maxidx = N;
      } else {
          data[i].maxidx = (i+1)*OPS_PER_ACCEL;
      }
      //printf("Range: %d - %d\n", data[i].startidx, data[i].maxidx);
  }

  //launch threads
  for (i=0; i<NUM_ACCEL; i++) {
      pthread_create(&threads[i], NULL, fadd_thread, (void *)&data[i]);
  }

  //join the threads
  for (i=0; i<NUM_ACCEL; i++) {
      pthread_join(threads[i], (void**)&result[i]);
  }

  for (i=0; i<NUM_ACCEL; i++) {
      main_result += result[i];
	  //main_result += (result != test_out[i]);
	  //printf
	  //  ("input=%016llx expected=%016llx output=%016llx (%lf)\n",
	  //   test_in[i], test_out[i], result, ullong_to_double (result));
  }



  /*
#ifdef DWARV
  int8 *m_float_rounding_mode; 
  int8 *m_float_exception_flags;
  float64 *m_a_input, *m_b_input, *m_z_output;

  m_a_input = (float64 *)malloc( N * sizeof(float64));
  m_b_input = (float64 *)malloc( N * sizeof(float64));
  m_z_output = (float64 *)malloc( N * sizeof(float64));
  m_float_rounding_mode =(int8 *)malloc(sizeof(int8)); 
  m_float_exception_flags=(int8 *)malloc(sizeof(int8)); 

  *m_float_rounding_mode = float_round_nearest_even;
  *m_float_exception_flags = 0; 

  for (i = 0; i < N; i++)
  {
    m_a_input[i] = a_input[i];
    m_b_input[i] = b_input[i];
    m_z_output[i] = z_output[i];
  }
#endif
      main_result = 0;
      for (i = 0; i < N; i++)
	{
	  float64 result;
#ifdef DWARV
	  x1 = m_a_input[i];
	  x2 = m_b_input[i];
#else
	  x1 = a_input[i];
	  x2 = b_input[i];
#endif
#ifdef DWARV
	  result = float64_add (x1, x2, *m_float_rounding_mode, *m_float_exception_flags);//doubt
#else
	  result = float64_add (x1, x2);
#endif
#ifdef DWARV
	  main_result += (result != m_z_output[i]);
#else
	  //main_result += (result != z_output[i]);
	  main_result += (result == z_output[i]);
#endif

	  printf
	    ("a_input=%016llx b_input=%016llx expected=%016llx output=%016llx (%lf)\n",
	     a_input[i], b_input[i], z_output[i], result,
	     ullong_to_double (result));
	}
      printf ("%d\n", main_result);
#ifdef DWARV
    free(m_a_input);
    free(m_b_input);
    free(m_z_output);
#endif
*/

    printf ("Sum: %d\n", main_result);
    if (main_result == N) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
    }

      return main_result;
    }
