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
arithmetic/SoftFloat.html'.

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
//#define DWARV
#include <stdio.h>
#ifdef DWARV
#include <stdlib.h>
#endif
//#include "softfloat.c"


//---------------------------------------------
// PTHREADS
//---------------------------------------------
#include <pthread.h>

// number of threads:
#define NUM_ACCEL 3

#define N 36

// number of iterations per thread
#define OPS_PER_ACCEL (N/NUM_ACCEL)

struct thread_data{
   int  startidx;
   int  maxidx;
};

//---------------------------------------------
// PTHREADS
//---------------------------------------------

#include "milieu.h"
#include "softfloat.h"

/*----------------------------------------------------------------------------
| Floating-point rounding mode, extended double-precision rounding precision,
| and exception flags.
*----------------------------------------------------------------------------*/
#ifndef DWARV
int8 float_rounding_mode = float_round_nearest_even;
//int8 float_exception_flags = 0;
int8 float_exception_flags_add = 0;
int8 float_exception_flags_mul = 0;
int8 float_exception_flags_div = 0;
int8 float_exception_flags_sin = 0;


#endif

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
| Shifts `a' right by the number of bits given in `count'.  If any nonzero
| bits are shifted off, they are ``jammed'' into the least significant bit of
| the result by setting the least significant bit to 1.  The value of `count'
| can be arbitrarily large; in particular, if `count' is greater than 64, the
| result will be either 0 or 1, depending on whether `a' is zero or nonzero.
| The result is stored in the location pointed to by `zPtr'.
*----------------------------------------------------------------------------*/

INLINE void
shift64RightJamming (bits64 a, int16 count, bits64 * zPtr)
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
  *zPtr = z;

}

/*----------------------------------------------------------------------------
| Shifts the 128-bit value formed by concatenating `a0' and `a1' right by 64
| _plus_ the number of bits given in `count'.  The shifted result is at most
| 64 nonzero bits; this is stored at the location pointed to by `z0Ptr'.  The
| bits shifted off form a second 64-bit result as follows:  The _last_ bit
| shifted off is the most-significant bit of the extra result, and the other
| 63 bits of the extra result are all zero if and only if _all_but_the_last_
| bits shifted off were all zero.  This extra result is stored in the location
| pointed to by `z1Ptr'.  The value of `count' can be arbitrarily large.
|     (This routine makes more sense if `a0' and `a1' are considered to form
| a fixed-point value with binary point between `a0' and `a1'.  This fixed-
| point value is shifted right by the number of bits given in `count', and
| the integer part of the result is returned at the location pointed to by
| `z0Ptr'.  The fractional part of the result may be slightly corrupted as
| described above, and is returned at the location pointed to by `z1Ptr'.)
*----------------------------------------------------------------------------*/
/*this function is never called
INLINE void
shift64ExtraRightJamming (bits64 a0, bits64 a1, int16 count, bits64 * z0Ptr,
			  bits64 * z1Ptr)
{
  bits64 z0, z1;
  int8 negCount;
  negCount = (-count) & 63;

  if (count == 0)
    {
      z1 = a1;
      z0 = a0;
    }
  else if (count < 64)
    {
      z1 = (a0 << negCount) | (a1 != 0);
      z0 = a0 >> count;
    }
  else
    {
      if (count == 64)
	{
	  z1 = a0 | (a1 != 0);
	}
      else
	{
	  z1 = ((a0 | a1) != 0);
	}
      z0 = 0;
    }
  *z1Ptr = z1;
  *z0Ptr = z0;

}
*/
/*----------------------------------------------------------------------------
| Adds the 128-bit value formed by concatenating `a0' and `a1' to the 128-bit
| value formed by concatenating `b0' and `b1'.  Addition is modulo 2^128, so
| any carry out is lost.  The result is broken into two 64-bit pieces which
| are stored at the locations pointed to by `z0Ptr' and `z1Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
add128 (bits64 a0, bits64 a1, bits64 b0, bits64 b1, bits64 * z0Ptr,
	bits64 * z1Ptr)
{
  bits64 z1;

  z1 = a1 + b1;
  *z1Ptr = z1;
  *z0Ptr = a0 + b0 + (z1 < a1);

}

/*----------------------------------------------------------------------------
| Subtracts the 128-bit value formed by concatenating `b0' and `b1' from the
| 128-bit value formed by concatenating `a0' and `a1'.  Subtraction is modulo
| 2^128, so any borrow out (carry out) is lost.  The result is broken into two
| 64-bit pieces which are stored at the locations pointed to by `z0Ptr' and
| `z1Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
sub128 (bits64 a0, bits64 a1, bits64 b0, bits64 b1, bits64 * z0Ptr,
	bits64 * z1Ptr)
{

  *z1Ptr = a1 - b1;
  *z0Ptr = a0 - b0 - (a1 < b1);

}

/*----------------------------------------------------------------------------
| Multiplies `a' by `b' to obtain a 128-bit product.  The product is broken
| into two 64-bit pieces which are stored at the locations pointed to by
| `z0Ptr' and `z1Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
mul64To128 (bits64 a, bits64 b, bits64 * z0Ptr, bits64 * z1Ptr)
{
  bits32 aHigh, aLow, bHigh, bLow;
  bits64 z0, zMiddleA, zMiddleB, z1;

  aLow = a;
  aHigh = a >> 32;
  bLow = b;
  bHigh = b >> 32;
  z1 = ((bits64) aLow) * bLow;
  zMiddleA = ((bits64) aLow) * bHigh;
  zMiddleB = ((bits64) aHigh) * bLow;
  z0 = ((bits64) aHigh) * bHigh;
  zMiddleA += zMiddleB;
  z0 += (((bits64) (zMiddleA < zMiddleB)) << 32) + (zMiddleA >> 32);
  zMiddleA <<= 32;
  z1 += zMiddleA;
  z0 += (z1 < zMiddleA);
  *z1Ptr = z1;
  *z0Ptr = z0;

}

/*----------------------------------------------------------------------------
| Returns an approximation to the 64-bit integer quotient obtained by dividing
| `b' into the 128-bit value formed by concatenating `a0' and `a1'.  The
| divisor `b' must be at least 2^63.  If q is the exact quotient truncated
| toward zero, the approximation returned lies between q and q + 2 inclusive.
| If the exact quotient q is larger than 64 bits, the maximum positive 64-bit
| unsigned integer is returned.
*----------------------------------------------------------------------------*/

static bits64
estimateDiv128To64 (bits64 a0, bits64 a1, bits64 b)
{
  bits64 b0, b1;
  bits64 rem0, rem1, term0, term1;
  bits64 z;

  if (b <= a0)
    return LIT64 (0xFFFFFFFFFFFFFFFF);
  b0 = b >> 32;
  z = (b0 << 32 <= a0) ? LIT64 (0xFFFFFFFF00000000) : (a0 / b0) << 32;
  mul64To128 (b, z, &term0, &term1);
  sub128 (a0, a1, term0, term1, &rem0, &rem1);
  while (((sbits64) rem0) < 0)
    {
      z -= LIT64 (0x100000000);
      b1 = b << 32;
      add128 (rem0, rem1, b0, b1, &rem0, &rem1);
    }
  rem0 = (rem0 << 32) | (rem1 >> 32);
  z |= (b0 << 32 <= rem0) ? 0xFFFFFFFF : rem0 / b0;
  return z;

}

/*----------------------------------------------------------------------------
| Returns the number of leading 0 bits before the most-significant 1 bit of
| `a'.  If `a' is zero, 32 is returned.
*----------------------------------------------------------------------------*/

static int8
countLeadingZeros32 (bits32 a)
{
  /*static*/ const int8 countLeadingZerosHigh[256] = {
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
INSTITUTE (possibly via similar legal warning) AGAINST ALL LOSSES, COSTS, OR
OTHER PROBLEMS INCURRED BY THEIR CUSTOMERS AND CLIENTS DUE TO THE SOFTWARE.

Derivative works are acceptable, even for commercial purposes, so long as
(1) the source code for the derivative work includes prominent notice that
the work is derivative, and (2) the source code includes prominent notice with
these four paragraphs for those parts of this code that are retained.

=============================================================================*/

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
float_raise (int8 flags, int type)
#endif
{
    if (type == 0)
        float_exception_flags_add |= flags;
    if (type == 1)
        float_exception_flags_mul |= flags;
    if (type == 2)
        float_exception_flags_div |= flags;
    if (type == 3)
        float_exception_flags_sin |= flags;

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
propagateFloat64NaN (float64 a, float64 b, int type)
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
    float_raise (float_flag_invalid, type);
#endif
  return bIsSignalingNaN ? b : aIsSignalingNaN ? a : bIsNaN ? b : a;

}
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

This C source file is part of the SoftFloat IEC/IEEE Floating-point Arithmetic
Package, Release 2b.

Written by John R. Hauser.  This work was made possible in part by the
International Computer Science Institute, located at Suite 600, 1947 Center
Street, Berkeley, California 94704.  Funding was partially provided by the
National Science Foundation under grant MIP-9311980.  The original version
of this code was written as part of a project to build a fixed-point vector
processor in collaboration with the University of California at Berkeley,
overseen by Profs. Nelson Morgan and John Wawrzynek.  More information
is available through the Web page `http://www.cs.berkeley.edu/~jhauser/
arithmetic/SoftFloat.html'.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort has
been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT TIMES
RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO PERSONS
AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ALL LOSSES,
COSTS, OR OTHER PROBLEMS THEY INCUR DUE TO THE SOFTWARE, AND WHO FURTHERMORE
EFFECTIVELY INDEMNIFY JOHN HAUSER AND THE INTERNATIONAL COMPUTER SCIENCE
INSTITUTE (possibly via similar legal warning) AGAINST ALL LOSSES, COSTS, OR
OTHER PROBLEMS INCURRED BY THEIR CUSTOMERS AND CLIENTS DUE TO THE SOFTWARE.

Derivative works are acceptable, even for commercial purposes, so long as
(1) the source code for the derivative work includes prominent notice that
the work is derivative, and (2) the source code includes prominent notice with
these four paragraphs for those parts of this code that are retained.

=============================================================================*/
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
| Normalizes the subnormal double-precision floating-point value represented
| by the denormalized significand `aSig'.  The normalized exponent and
| significand are stored at the locations pointed to by `zExpPtr' and
| `zSigPtr', respectively.
*----------------------------------------------------------------------------*/

static void
normalizeFloat64Subnormal (bits64 aSig, int16 * zExpPtr, bits64 * zSigPtr)
{
  int8 shiftCount;

  shiftCount = countLeadingZeros64 (aSig) - 11;
  *zSigPtr = aSig << shiftCount;
  *zExpPtr = 1 - shiftCount;

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
roundAndPackFloat64 (flag zSign, int16 zExp, bits64 zSig, int type)
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
	  float_raise (float_flag_overflow | float_flag_inexact, type);
#endif
	  return packFloat64 (zSign, 0x7FF, 0) - (roundIncrement == 0);
	}
      if (zExp < 0)
	{
	  isTiny = (float_detect_tininess == float_tininess_before_rounding)
	    || (zExp < -1)
	    || (zSig + roundIncrement < LIT64 (0x8000000000000000));
	  shift64RightJamming (zSig, -zExp, &zSig);
	  zExp = 0;
	  roundBits = zSig & 0x3FF;
	  if (isTiny && roundBits)
#ifdef DWARV
	    float_raise (float_flag_underflow, float_exception_flags);
#else
	    float_raise (float_flag_underflow, type);
#endif
	}
    }
  if (roundBits){
      if (type == 0)
        float_exception_flags_add |= float_flag_inexact;
      if (type == 1)
        float_exception_flags_mul |= float_flag_inexact;
      if (type == 2)
        float_exception_flags_div |= float_flag_inexact;
      if (type == 3)
        float_exception_flags_sin |= float_flag_inexact;
  }
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
normalizeRoundAndPackFloat64 (flag zSign, int16 zExp, bits64 zSig, int type)
#endif
{
  int8 shiftCount;

  shiftCount = countLeadingZeros64 (zSig) - 1;
#ifdef DWARV
  return roundAndPackFloat64 (zSign, zExp - shiftCount, zSig << shiftCount, float_rounding_mode,float_exception_flags);
#else
  return roundAndPackFloat64 (zSign, zExp - shiftCount, zSig << shiftCount, type);
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of converting the 32-bit two's complement integer `a'
| to the double-precision floating-point format.  The conversion is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float64
int32_to_float64 (int32 a)
{
  flag zSign;
  uint32 absA;
  int8 shiftCount;
  bits64 zSig;

  if (a == 0)
    return 0;
  zSign = (a < 0);
  absA = zSign ? -a : a;
  shiftCount = countLeadingZeros32 (absA) + 21;
  zSig = absA;

  return packFloat64 (zSign, 0x432 - shiftCount, zSig << shiftCount);
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
addFloat64Sigs (float64 a, float64 b, flag zSign, int type)
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
	    return propagateFloat64NaN (a, b, type);
#endif
	  return a;
	}
      if (bExp == 0)
	--expDiff;
      else
	bSig |= LIT64 (0x2000000000000000);
      shift64RightJamming (bSig, expDiff, &bSig);
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
	    return propagateFloat64NaN (a, b, type);
#endif
	  return packFloat64 (zSign, 0x7FF, 0);
	}
      if (aExp == 0)
	++expDiff;
      else
	{
	  aSig |= LIT64 (0x2000000000000000);
	}
      shift64RightJamming (aSig, -expDiff, &aSig);
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
	    return propagateFloat64NaN (a, b, type);
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
  return roundAndPackFloat64 (zSign, zExp, zSig, type);
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
subFloat64Sigs (float64 a, float64 b, flag zSign, int type)
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
	    return propagateFloat64NaN (a, b, type);
#endif
#ifdef DWARV
      float_raise (float_flag_invalid, float_exception_flags);
#else
      float_raise (float_flag_invalid, type);
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
	    return propagateFloat64NaN (a, b, type);
#endif
      return packFloat64 (zSign ^ 1, 0x7FF, 0);
    }
  if (aExp == 0)
    ++expDiff;
  else
    aSig |= LIT64 (0x4000000000000000);
  shift64RightJamming (aSig, -expDiff, &aSig);
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
	    return propagateFloat64NaN (a, b, type);
#endif
      return a;
    }
  if (bExp == 0)
    --expDiff;
  else
    bSig |= LIT64 (0x4000000000000000);
  shift64RightJamming (bSig, expDiff, &bSig);
  aSig |= LIT64 (0x4000000000000000);
aBigger:
  zSig = aSig - bSig;
  zExp = aExp;
normalizeRoundAndPack:
  --zExp;
#ifdef DWARV
  return normalizeRoundAndPackFloat64 (zSign, zExp, zSig, float_rounding_mode, float_exception_flags);
#else
  return normalizeRoundAndPackFloat64 (zSign, zExp, zSig, type);
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of adding the double-precision floating-point values `a'
| and `b'.  The operation is performed according to the IEC/IEEE Standard for
| Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/
#ifdef DWARV
float64
float64_add (float64 a, float64 b, int8 float_rounding_mode, int8 float_exception_flags)
#else
float64
float64_add (float64 a, float64 b, int type)
#endif
{
  flag aSign, bSign;

  aSign = extractFloat64Sign (a);
  bSign = extractFloat64Sign (b);
  if (aSign == bSign)
#ifdef DWARV
    return addFloat64Sigs (a, b, aSign, float_rounding_mode, float_exception_flags);
#else
    return addFloat64Sigs (a, b, aSign, type);
#endif
  else
#ifdef DWARV
    return subFloat64Sigs (a, b, aSign, float_rounding_mode, float_exception_flags);
#else
    return subFloat64Sigs (a, b, aSign, type);
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of multiplying the double-precision floating-point values
| `a' and `b'.  The operation is performed according to the IEC/IEEE Standard
| for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/
#ifdef DWARV
float64
float64_mul (float64 a, float64 b, int8 float_rounding_mode, int8 float_exception_flags)
#else
float64
float64_mul (float64 a, float64 b, int type)
#endif
{
  flag aSign, bSign, zSign;
  int16 aExp, bExp, zExp;
  bits64 aSig, bSig, zSig0, zSig1;

  aSig = extractFloat64Frac (a);
  aExp = extractFloat64Exp (a);
  aSign = extractFloat64Sign (a);
  bSig = extractFloat64Frac (b);
  bExp = extractFloat64Exp (b);
  bSign = extractFloat64Sign (b);
  zSign = aSign ^ bSign;
  if (aExp == 0x7FF)
    {
      if (aSig || ((bExp == 0x7FF) && bSig))
#ifdef DWARV
	    return propagateFloat64NaN (a, b, float_exception_flags);
#else
	    return propagateFloat64NaN (a, b, type);
#endif
      if ((bExp | bSig) == 0)
	{
#ifdef DWARV
	  float_raise (float_flag_invalid, float_exception_flags);
#else
	  float_raise (float_flag_invalid, type);
#endif
	  return float64_default_nan;
	}
      return packFloat64 (zSign, 0x7FF, 0);
    }
  if (bExp == 0x7FF)
    {
      if (bSig)
#ifdef DWARV
	    return propagateFloat64NaN (a, b, float_exception_flags);
#else
	    return propagateFloat64NaN (a, b, type);
#endif
      if ((aExp | aSig) == 0)
	{
#ifdef DWARV
	  float_raise (float_flag_invalid, float_exception_flags);
#else
	  float_raise (float_flag_invalid, type);
#endif
	  return float64_default_nan;
	}
      return packFloat64 (zSign, 0x7FF, 0);
    }
  if (aExp == 0)
    {
      if (aSig == 0)
	return packFloat64 (zSign, 0, 0);
      normalizeFloat64Subnormal (aSig, &aExp, &aSig);
    }
  if (bExp == 0)
    {
      if (bSig == 0)
	return packFloat64 (zSign, 0, 0);
      normalizeFloat64Subnormal (bSig, &bExp, &bSig);
    }
  zExp = aExp + bExp - 0x3FF;
  aSig = (aSig | LIT64 (0x0010000000000000)) << 10;
  bSig = (bSig | LIT64 (0x0010000000000000)) << 11;
  mul64To128 (aSig, bSig, &zSig0, &zSig1);
  zSig0 |= (zSig1 != 0);
  if (0 <= (sbits64) (zSig0 << 1))
    {
      zSig0 <<= 1;
      --zExp;
    }
#ifdef DWARV
  return roundAndPackFloat64 (zSign, zExp, zSig0, float_rounding_mode, float_exception_flags);
#else
  return roundAndPackFloat64 (zSign, zExp, zSig0, type);
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of dividing the double-precision floating-point value `a'
| by the corresponding value `b'.  The operation is performed according to
| the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/
#ifdef DWARV
float64
float64_div (float64 a, float64 b, int8 float_rounding_mode, int8 float_exception_flags)
#else
float64
float64_div (float64 a, float64 b, int type)
#endif
{
  flag aSign, bSign, zSign;
  int16 aExp, bExp, zExp;
  bits64 aSig, bSig, zSig;
  bits64 rem0, rem1, term0, term1;

  aSig = extractFloat64Frac (a);
  aExp = extractFloat64Exp (a);
  aSign = extractFloat64Sign (a);
  bSig = extractFloat64Frac (b);
  bExp = extractFloat64Exp (b);
  bSign = extractFloat64Sign (b);
  zSign = aSign ^ bSign;
  if (aExp == 0x7FF)
    {
      if (aSig)
#ifdef DWARV
	    return propagateFloat64NaN (a, b, float_exception_flags);
#else
	    return propagateFloat64NaN (a, b, type);
#endif
      if (bExp == 0x7FF)
	{
	  if (bSig)
#ifdef DWARV
	    return propagateFloat64NaN (a, b, float_exception_flags);
#else
	    return propagateFloat64NaN (a, b, type);
#endif
#ifdef DWARV
	  float_raise (float_flag_invalid, float_exception_flags);
#else
	  float_raise (float_flag_invalid, type);
#endif
	  return float64_default_nan;
	}
      return packFloat64 (zSign, 0x7FF, 0);
    }
  if (bExp == 0x7FF)
    {
      if (bSig)
#ifdef DWARV
	    return propagateFloat64NaN (a, b, float_exception_flags);
#else
	    return propagateFloat64NaN (a, b, type);
#endif
      return packFloat64 (zSign, 0, 0);
    }
  if (bExp == 0)
    {
      if (bSig == 0)
	{
	  if ((aExp | aSig) == 0)
	    {
#ifdef DWARV
	      float_raise (float_flag_invalid, float_exception_flags);
#else
	      float_raise (float_flag_invalid, type);
#endif
	      return float64_default_nan;
	    }
#ifdef DWARV
	  float_raise (float_flag_divbyzero, float_exception_flags);
#else
	  float_raise (float_flag_divbyzero, type);
#endif
	  return packFloat64 (zSign, 0x7FF, 0);
	}
      normalizeFloat64Subnormal (bSig, &bExp, &bSig);
    }
  if (aExp == 0)
    {
      if (aSig == 0)
	return packFloat64 (zSign, 0, 0);
      normalizeFloat64Subnormal (aSig, &aExp, &aSig);
    }
  zExp = aExp - bExp + 0x3FD;
  aSig = (aSig | LIT64 (0x0010000000000000)) << 10;
  bSig = (bSig | LIT64 (0x0010000000000000)) << 11;
  if (bSig <= (aSig + aSig))
    {
      aSig >>= 1;
      ++zExp;
    }
  zSig = estimateDiv128To64 (aSig, 0, bSig);
  if ((zSig & 0x1FF) <= 2)
    {
      mul64To128 (bSig, zSig, &term0, &term1);
      sub128 (aSig, 0, term0, term1, &rem0, &rem1);
      while ((sbits64) rem0 < 0)
	{
	  --zSig;
	  add128 (rem0, rem1, 0, bSig, &rem0, &rem1);
	}
      zSig |= (rem1 != 0);
    }
#ifdef DWARV
  return roundAndPackFloat64 (zSign, zExp, zSig, float_rounding_mode, float_exception_flags);
#else
  return roundAndPackFloat64 (zSign, zExp, zSig, type);
#endif

}

/*----------------------------------------------------------------------------
| Returns 1 if the double-precision floating-point value `a' is less than or
| equal to the corresponding value `b', and 0 otherwise.  The comparison is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/
#ifdef DWARV
flag
float64_le (float64 a, float64 b, int8 float_exception_flags)
#else
flag
float64_le (float64 a, float64 b, int type)
#endif
{
  flag aSign, bSign;

  if (((extractFloat64Exp (a) == 0x7FF) && extractFloat64Frac (a))
      || ((extractFloat64Exp (b) == 0x7FF) && extractFloat64Frac (b)))
    {
#ifdef DWARV
      float_raise (float_flag_invalid, float_exception_flags);
#else
      float_raise (float_flag_invalid, type);
#endif
      return 0;
    }
  aSign = extractFloat64Sign (a);
  bSign = extractFloat64Sign (b);
  if (aSign != bSign)
    return aSign || ((bits64) ((a | b) << 1) == 0);
  return (a == b) || (aSign ^ (a < b));

}
#ifdef DWARV
flag
float64_ge (float64 a, float64 b, int8 float_exception_flags)
#else
flag
float64_ge (float64 a, float64 b, int type)
#endif
{
#ifdef DWARV
  return float64_le (b, a, float_exception_flags, type);
#else
  return float64_le (b, a, type);
#endif
}

// added by hiroyuki@acm.org
float64
float64_neg (float64 x)
{
  return (((~x) & 0x8000000000000000ULL) | (x & 0x7fffffffffffffffULL));
}
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
/*
 * Copyright (C) 2008
 * Y. Hara, H. Tomiyama, S. Honda, H. Takada and K. Ishii
 * Nagoya University, Japan
 * All rights reserved.
 *
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis. The authors disclaims any and all warranties, 
 * whether express, implied, or statuary, including any implied warranties or 
 * merchantability or of fitness for a particular purpose. In no event shall the
 * copyright-holder be liable for any incidental, punitive, or consequential damages
 * of any kind whatsoever arising from the use of these programs. This disclaimer
 * of warranty extends to the user of these programs and user's customers, employees,
 * agents, transferees, successors, and assigns.
 *
 */
float64
float64_abs (float64 x)
{
  return (x & 0x7fffffffffffffffULL);
}

#ifdef DWARV
float64
local_sin (float64 rad, int8 float_rounding_mode, int8 float_exception_flags)
#else
__attribute__((noinline))
float64
local_sin (float64 rad)
#endif
{
  float64 app;
  float64 diff;
  float64 m_rad2;
  int inc;

  app = diff = rad;
  inc = 1; 
#ifdef DWARV
    m_rad2 = float64_neg (float64_mul (rad, rad, float_rounding_mode, float_exception_flags));
    do
    {
/*TODO: error in sim but actually equal:  0x3fd5e3a82b09bda7 = 0.342020074872 vs soft: 0x3fd5e3a82b09bf3e = 0.342020074872
-> why are the last 3 hexdigits ingnored? */
      diff = float64_div (float64_mul (diff, m_rad2, float_rounding_mode, float_exception_flags),
                          int32_to_float64 ((2 * inc) * (2 * inc + 1)), float_rounding_mode, float_exception_flags);
      app = float64_add (app, diff, float_rounding_mode,float_exception_flags);
      inc++;
    }
    while (float64_ge (float64_abs (diff), 0x3ee4f8b588e368f1ULL, float_exception_flags));	// 0.00001 
#else  
  m_rad2 = float64_neg (float64_mul (rad, rad, 3));
  do
    {
      diff = float64_div (float64_mul (diff, m_rad2, 3),
			  int32_to_float64 ((2 * inc) * (2 * inc + 1)), 3);
      app = float64_add (app, diff, 3);
      inc++;
    }
  while (float64_ge (float64_abs (diff), 0x3ee4f8b588e368f1ULL, 3));	/* 0.00001 */
#endif
  return app;
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
|     test_in : input data                                                 |
|     test_out : expected output data                                      |
+--------------------------------------------------------------------------+
*/
const float64 test_in[N] = {
  0x3fd65717fced55c1ULL,	/*2   PI/9  */
  0x3fc65717fced55c1ULL,	/*1   PI/18 */
  0x0000000000000000ULL,	/*0      0  */
  0x3fe0c151fdb20051ULL,	/*3   PI/6  */
  0x3fe65717fced55c1ULL,	/*4  2PI/9  */
  0x3febecddfc28ab31ULL,	/*5  5PI/18 */
  0x3ff0c151fdb20051ULL,	/*6   PI/3  */
  0x3ff38c34fd4fab09ULL,	/*7  7PI/18 */
  0x3ff65717fced55c1ULL,	/*8  4PI/9  */
  0x3ff921fafc8b0079ULL,	/*9   PI/2  */
  0x3ffbecddfc28ab31ULL,	/*10  5PI/9  */
  0x3ffeb7c0fbc655e9ULL,	/*11 11PI/18 */
  0x4000c151fdb20051ULL,	/*12  2PI/3  */
  0x400226c37d80d5adULL,	/*13 13PI/18 */
  0x40038c34fd4fab09ULL,	/*14  7PI/9  */
  0x4004f1a67d1e8065ULL,	/*15  5PI/6  */
  0x40065717fced55c1ULL,	/*16  8PI/9  */
  0x4007bc897cbc2b1dULL,	/*17 17PI/18 */
  0x400921fafc8b0079ULL,	/*18   PI    */
  0x400a876c7c59d5d5ULL,	/*19 19PI/18 */
  0x400becddfc28ab31ULL,	/*20 10PI/9  */
  0x400d524f7bf7808dULL,	/*21  7PI/6  */
  0x400eb7c0fbc655e9ULL,	/*22 11PI/9  */
  0x40100e993dca95a3ULL,	/*23 23PI/18 */
  0x4010c151fdb20051ULL,	/*24  8PI/6  */
  0x4011740abd996affULL,	/*25 25PI/18 */
  0x401226c37d80d5adULL,	/*26 13PI/9  */
  0x4012d97c3d68405bULL,	/*27  3PI/2  */
  0x40138c34fd4fab09ULL,	/*28 14PI/9  */
  0x40143eedbd3715b7ULL,	/*29 29PI/18 */
  0x4014f1a67d1e8065ULL,	/*30 15PI/9  */
  0x4015a45f3d05eb13ULL,	/*31 31PI/18 */
  0x40165717fced55c1ULL,	/*32 16PI/9  */
  0x401709d0bcd4c06fULL,	/*33 33PI/18 */
  0x4017bc897cbc2b1dULL,	/*34 17PI/9  */
  0x40186f423ca395cbULL		/*35 35PI/18 */
};

const float64 test_out[N] = {
  0x3fd5e3a82b09bf3eULL,	/*2  0.342020 */
  0x3fc63a1a335aadcdULL,	/*1  0.173648 */
  0x0000000000000000ULL,	/*0  0.000000 */
  0x3fdfffff91f9aa91ULL,	/*3  0.500000 */
  0x3fe491b716c242e3ULL,	/*4  0.642787 */
  0x3fe8836f672614a6ULL,	/*5  0.766044 */
  0x3febb67ac40b2bedULL,	/*6  0.866025 */
  0x3fee11f6127e28adULL,	/*7  0.939693 */
  0x3fef838b6adffac0ULL,	/*8  0.984808 */
  0x3fefffffe1cbd7aaULL,	/*9  1.000000 */
  0x3fef838bb0147989ULL,	/*10  0.984808 */
  0x3fee11f692d962b4ULL,	/*11  0.939693 */
  0x3febb67b77c0142dULL,	/*12  0.866026 */
  0x3fe883709d4ea869ULL,	/*13  0.766045 */
  0x3fe491b81d72d8e8ULL,	/*14  0.642788 */
  0x3fe00000ea5f43c8ULL,	/*15  0.500000 */
  0x3fd5e3aa4e0590c5ULL,	/*16  0.342021 */
  0x3fc63a1d2189552cULL,	/*17  0.173648 */
  0x3ea6aedffc454b91ULL,	/*18  0.000001 */
  0xbfc63a1444ddb37cULL,	/*19 -0.173647 */
  0xbfd5e3a4e68f8f3eULL,	/*20 -0.342019 */
  0xbfdffffd494cf96bULL,	/*21 -0.499999 */
  0xbfe491b61cb9a3d3ULL,	/*22 -0.642787 */
  0xbfe8836eb2dcf815ULL,	/*23 -0.766044 */
  0xbfebb67a740aae32ULL,	/*24 -0.866025 */
  0xbfee11f5912d2157ULL,	/*25 -0.939692 */
  0xbfef838b1ac64afcULL,	/*26 -0.984808 */
  0xbfefffffc2e5dc8fULL,	/*27 -1.000000 */
  0xbfef838b5ea2e7eaULL,	/*28 -0.984808 */
  0xbfee11f7112dae27ULL,	/*29 -0.939693 */
  0xbfebb67c2c31cb4aULL,	/*30 -0.866026 */
  0xbfe883716e6fd781ULL,	/*31 -0.766045 */
  0xbfe491b9cd1b5d56ULL,	/*32 -0.642789 */
  0xbfe000021d0ca30dULL,	/*33 -0.500001 */
  0xbfd5e3ad0a69caf7ULL,	/*34 -0.342021 */
  0xbfc63a23c48863ddULL		/*35 -0.173649 */
};

void* sin_thread(void* threadarg) {
    
    int i, main_result=0;
	struct thread_data* arg = (struct thread_data*) threadarg;
	int startidx = arg->startidx;
	int maxidx = arg->maxidx;
	for (i=startidx; i<maxidx; i++)
	{
	  float64 result;
	  result = local_sin (test_in[i]);
      main_result += (result == test_out[i]);

	}

    pthread_exit((void*)main_result);
}

int
main ()
{
  int main_result;
  int i;

      main_result = 0;

  pthread_t threads[NUM_ACCEL];
  int result[NUM_ACCEL]={0};
  struct thread_data data[NUM_ACCEL];

  for (i=0; i<NUM_ACCEL; i++) {
      // initialize structs to pass into accels
      data[i].startidx = i*OPS_PER_ACCEL;
      data[i].maxidx = (i+1)*OPS_PER_ACCEL;
  }
  if(NUM_ACCEL*OPS_PER_ACCEL != N)
    data[NUM_ACCEL-1].maxidx = N;

  //launch threads
  for (i=0; i<NUM_ACCEL; i++) {
      pthread_create(&threads[i], NULL, sin_thread, (void *)&data[i]);
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
  for (i = 0; i < N; i++)
	{
	  float64 result;
	  result = local_sin (test_in[i]);
	  main_result += (result != test_out[i]);

	  printf
	    ("input=%016llx expected=%016llx output=%016llx (%lf)\n",
	     test_in[i], test_out[i], result, ullong_to_double (result));
	}
    */
      printf ("Result: %d\n", main_result);
      return main_result;
    }
