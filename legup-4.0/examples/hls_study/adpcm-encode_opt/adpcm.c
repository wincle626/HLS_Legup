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
/*************************************************************************/
/*                                                                       */
/*   SNU-RT Benchmark Suite for Worst Case Timing Analysis               */
/*   =====================================================               */
/*                              Collected and Modified by S.-S. Lim      */
/*                                           sslim@archi.snu.ac.kr       */
/*                                         Real-Time Research Group      */
/*                                        Seoul National University      */
/*                                                                       */
/*                                                                       */
/*        < Features > - restrictions for our experimental environment   */
/*                                                                       */
/*          1. Completely structured.                                    */
/*               - There are no unconditional jumps.                     */
/*               - There are no exit from loop bodies.                   */
/*                 (There are no 'break' or 'return' in loop bodies)     */
/*          2. No 'switch' statements.                                   */
/*          3. No 'do..while' statements.                                */
/*          4. Expressions are restricted.                               */
/*               - There are no multiple expressions joined by 'or',     */
/*                'and' operations.                                      */
/*          5. No library calls.                                         */
/*               - All the functions needed are implemented in the       */
/*                 source file.                                          */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*  FILE: adpcm.c                                                        */
/*  SOURCE : C Algorithms for Real-Time DSP by P. M. Embree              */
/*                                                                       */
/*  DESCRIPTION :                                                        */
/*                                                                       */
/*     CCITT G.722 ADPCM (Adaptive Differential Pulse Code Modulation)   */
/*     algorithm.                                                        */
/*     16khz sample rate data is stored in the array test_data[SIZE].    */
/*     Results are stored in the array compressed[SIZE] and result[SIZE].*/
/*     Execution time is determined by the constant SIZE (default value  */
/*     is 2000).                                                         */
/*                                                                       */
/*  REMARK :                                                             */
/*                                                                       */
/*  EXECUTION TIME :                                                     */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
/* DWARV cannot handle system-wide includes */
#define TESTMODE
//#define DWARV
#ifdef TESTMODE
#include <stdio.h>
#ifdef DWARV
  #include <stdlib.h> /* The Bluebee toolchain requires `malloc', so we need to include this header */
#endif /* DWARV */
#endif /* TESTMODE */

#ifdef DWARV
int encode (int xin1, int xin2, const int *h, int *tqmf, int xl, int xh, int szl, int *delay_bpl, int *delay_dltx, int spl, int *al1, int *al2, int *rlt1, int *rlt2, int sl, int el, int *il, int *detl, int dlt, const int *qq4_code4_table, int *nbl, int plt, int rlt, int szh, int *delay_dhx, int *delay_bph, int sph, int *ah1, int *ah2, int *rh1, int *rh2, int sh, int eh, int ih, int *deth, const int *qq2_code2_table, int dh, int *nbh, int ph, int yh, int *decis_levl, int *quant26bt_pos, int *quant26bt_neg, int *wl_code_table, int *ilb_table, int *plt1, int *plt2, int *wh_code_table, int *ph1, int *ph2);
int quantl (int el, int detl, int *decis_levl, int *quant26bt_pos, int *quant26bt_neg);
int logscl (int il, int nbl, int *wl_code_table);
int scalel (int nbl, int shift_constant, int *ilb_table);
int logsch (int ih, int nbh, int *wh_code_table);
void reset (int *tqmf, int *delay_bpl, int *delay_dltx, int *al1, int *al2, int *rlt1, int *rlt2, int *detl, int *nbl, int *delay_dhx, int *delay_bph, int *ah1, int *ah2, int *rh1, int *rh2, int *plt1, int *plt2, int *ph1, int *ph2, int *nbh, int *deth);
#else
int encode (int, int);
int quantl (int el, int detl);
int logscl (int il, int nbl);
int scalel (int nbl, int shift_constant);
int logsch (int ih, int nbh);
void reset ();
#endif /* DWARV */
void decode (int);
int filtez (int *bpl, int *dlt);
void upzero (int dlt, int *dlti, int *bli);
int filtep (int rlt1, int al1, int rlt2, int al2);
int uppol2 (int al1, int al2, int plt, int plt1, int plt2);
int uppol1 (int al1, int apl2, int plt, int plt1);

/* G722 C code */

/* variables for transimit quadrature mirror filter here */
int tqmf[24];

/* QMF filter coefficients:
scaled by a factor of 4 compared to G722 CCITT recomendation */
volatile const int h[24] = {
  12, -44, -44, 212, 48, -624, 128, 1448,
  -840, -3220, 3804, 15504, 15504, 3804, -3220, -840,
  1448, 128, -624, 48, 212, -44, -44, 12
};

int xl, xh;

/* variables for receive quadrature mirror filter here */
int accumc[11], accumd[11];

/* outputs of decode() */
int xout1, xout2;

int xs, xd;

/* variables for encoder (hi and lo) here */

int il, szl, spl, sl, el;

const int qq4_code4_table[16] = {
  0, -20456, -12896, -8968, -6288, -4240, -2584, -1200,
  20456, 12896, 8968, 6288, 4240, 2584, 1200, 0
};


const int qq6_code6_table[64] = {
  -136, -136, -136, -136, -24808, -21904, -19008, -16704,
  -14984, -13512, -12280, -11192, -10232, -9360, -8576, -7856,
  -7192, -6576, -6000, -5456, -4944, -4464, -4008, -3576,
  -3168, -2776, -2400, -2032, -1688, -1360, -1040, -728,
  24808, 21904, 19008, 16704, 14984, 13512, 12280, 11192,
  10232, 9360, 8576, 7856, 7192, 6576, 6000, 5456,
  4944, 4464, 4008, 3576, 3168, 2776, 2400, 2032,
  1688, 1360, 1040, 728, 432, 136, -432, -136
};

int delay_bpl[6];

int delay_dltx[6];

const int wl_code_table[16] = {
  -60, 3042, 1198, 538, 334, 172, 58, -30,
  3042, 1198, 538, 334, 172, 58, -30, -60
};

const int ilb_table[32] = {
  2048, 2093, 2139, 2186, 2233, 2282, 2332, 2383,
  2435, 2489, 2543, 2599, 2656, 2714, 2774, 2834,
  2896, 2960, 3025, 3091, 3158, 3228, 3298, 3371,
  3444, 3520, 3597, 3676, 3756, 3838, 3922, 4008
};

int nbl;			/* delay line */
int al1, al2;
int plt, plt1, plt2;
int dlt;
int rlt, rlt1, rlt2;

/* decision levels - pre-multiplied by 8, 0 to indicate end */
const int decis_levl[30] = {
  280, 576, 880, 1200, 1520, 1864, 2208, 2584,
  2960, 3376, 3784, 4240, 4696, 5200, 5712, 6288,
  6864, 7520, 8184, 8968, 9752, 10712, 11664, 12896,
  14120, 15840, 17560, 20456, 23352, 32767
};

int detl;

/* quantization table 31 long to make quantl look-up easier,
last entry is for mil=30 case when wd is max */
const int quant26bt_pos[31] = {
  61, 60, 59, 58, 57, 56, 55, 54,
  53, 52, 51, 50, 49, 48, 47, 46,
  45, 44, 43, 42, 41, 40, 39, 38,
  37, 36, 35, 34, 33, 32, 32
};

/* quantization table 31 long to make quantl look-up easier,
last entry is for mil=30 case when wd is max */
const int quant26bt_neg[31] = {
  63, 62, 31, 30, 29, 28, 27, 26,
  25, 24, 23, 22, 21, 20, 19, 18,
  17, 16, 15, 14, 13, 12, 11, 10,
  9, 8, 7, 6, 5, 4, 4
};


int deth;
int sh;				/* this comes from adaptive predictor */
int eh;

const int qq2_code2_table[4] = {
  -7408, -1616, 7408, 1616
};

const int wh_code_table[4] = {
  798, -214, 798, -214
};


int dh, ih;
int nbh, szh;
int sph, ph, yh, rh;

int delay_dhx[6];

int delay_bph[6];

int ah1, ah2;
int ph1, ph2;
int rh1, rh2;

/* variables for decoder here */
int ilr, rl;
int dec_deth, dec_detl, dec_dlt;

int dec_del_bpl[6];

int dec_del_dltx[6];

int dec_plt, dec_plt1, dec_plt2;
int dec_szl, dec_spl, dec_sl;
int dec_rlt1, dec_rlt2, dec_rlt;
int dec_al1, dec_al2;
int dl;
int dec_nbl, dec_dh, dec_nbh;

/* variables used in filtez */
int dec_del_bph[6];

int dec_del_dhx[6];

int dec_szh;
/* variables used in filtep */
int dec_rh1, dec_rh2;
int dec_ah1, dec_ah2;
int dec_ph, dec_sph;

int dec_sh;

int dec_ph1, dec_ph2;

/* G722 encode function two ints in, one 8 bit output */

/* put input samples in xin1 = first value, xin2 = second value */
/* returns il and ih stored together */

int
abs (int n)
{
  int m;

  if (n >= 0)
    m = n;
  else
    m = -n;
  return m;
}

#ifdef DWARV
int gfadfgadfg;
#pragma map generate_hw 0
/* DWARV: Compiles */
int encode (int xin1, int xin2, const int *h, int *tqmf, int xl, int xh, int szl, int *delay_bpl, int *delay_dltx, int spl, int *al1, int *al2, int *rlt1, int *rlt2, int sl, int el, int *il, int *detl, int dlt, const int *qq4_code4_table, int *nbl, int plt, int rlt, int szh, int *delay_dhx, int *delay_bph, int sph, int *ah1, int *ah2, int *rh1, int *rh2, int sh, int eh, int ih, int *deth, const int *qq2_code2_table, int dh, int *nbh, int ph, int yh, int *decis_levl, int *quant26bt_pos, int *quant26bt_neg, int *wl_code_table, int *ilb_table, int *plt1, int *plt2, int *wh_code_table, int *ph1, int *ph2) {
#else
int
__attribute__ ((noinline)) encode (int xin1, int xin2)
{
#endif /* DWARV */
  int i;
  const int *h_ptr;
  int *tqmf_ptr, *tqmf_ptr1;
  long int xa, xb;
  int decis;

/* transmit quadrature mirror filters implemented here */
  h_ptr = h;
  tqmf_ptr = tqmf;
  xa = (long) (*tqmf_ptr++) * (*h_ptr++);
  xb = (long) (*tqmf_ptr++) * (*h_ptr++);
/* main multiply accumulate loop for samples and coefficients */
  //loop1: for (i = 0; i < 10; i++)
  for (i = 0; i < 10; i++)
    {
      xa += (long) (*tqmf_ptr++) * (*h_ptr++);
      xb += (long) (*tqmf_ptr++) * (*h_ptr++);
    }
/* final mult/accumulate */
  xa += (long) (*tqmf_ptr++) * (*h_ptr++);
  xb += (long) (*tqmf_ptr) * (*h_ptr++);

/* update delay line tqmf */
  tqmf_ptr1 = tqmf_ptr - 2;
  //loop2: for (i = 0; i < 22; i++)
  for (i = 0; i < 22; i++)
    *tqmf_ptr-- = *tqmf_ptr1--;
  *tqmf_ptr-- = xin1;
  *tqmf_ptr = xin2;

/* scale outputs */
  xl = (xa + xb) >> 15;
  xh = (xa - xb) >> 15;

/* end of quadrature mirror filter code */

/* starting with lower sub band encoder */

/* filtez - compute predictor output section - zero section */
  //#pragma map call_hw VIRTEX5 0
  szl = filtez (delay_bpl, delay_dltx);

/* filtep - compute predictor output signal (pole section) */
#ifdef DWARV
rh1=rh1;

  //#pragma map call_hw VIRTEX5 0
  spl = filtep (*rlt1, *al1, *rlt2, *al2);
#else
  spl = filtep (rlt1, al1, rlt2, al2);
#endif /* DWARV */

/* compute the predictor output value in the lower sub_band encoder */
  sl = szl + spl;
  el = xl - sl;

#ifdef DWARV
/* quantl: quantize the difference signal */
rh1=rh1;
  
//#pragma map call_hw VIRTEX5 0
  *il = quantl (el, *detl, decis_levl, quant26bt_pos, quant26bt_neg);

/* computes quantized difference signal */
/* for invqbl, truncate by 2 lsbs, so mode = 3 */
   dlt = ((long) *detl * qq4_code4_table[*il >> 2]) >> 15;

/* logscl: updates logarithmic quant. scale factor in low sub band */
  //#pragma map call_hw VIRTEX5 0
  *nbl = logscl (*il, *nbl, wl_code_table);

/* scalel: compute the quantizer scale factor in the lower sub band */
/* calling parameters nbl and 8 (constant such that scalel can be scaleh) */
  //#pragma map call_hw VIRTEX5 0
  *detl = scalel (*nbl, 8, ilb_table);
#else
/* quantl: quantize the difference signal */
  il = quantl (el, detl);

/* computes quantized difference signal */
/* for invqbl, truncate by 2 lsbs, so mode = 3 */
  dlt = ((long) detl * qq4_code4_table[il >> 2]) >> 15;

/* logscl: updates logarithmic quant. scale factor in low sub band */
  nbl = logscl (il, nbl);

/* scalel: compute the quantizer scale factor in the lower sub band */
/* calling parameters nbl and 8 (constant such that scalel can be scaleh) */
  detl = scalel (nbl, 8);
#endif /* DWARV */

/* parrec - simple addition to compute recontructed signal for adaptive pred */
  plt = dlt + szl;

/* upzero: update zero section predictor coefficients (sixth order)*/
/* calling parameters: dlt, dlt1, dlt2, ..., dlt6 from dlt */
/*  bpli (linear_buffer in which all six values are delayed */
/* return params:      updated bpli, delayed dltx */
  //#pragma map call_hw VIRTEX5 0
  upzero (dlt, delay_dltx, delay_bpl);

#ifdef DWARV
/* uppol2- update second predictor coefficient apl2 and delay it as al2 */
/* calling parameters: al1, al2, plt, plt1, plt2 */
rh1=rh1;
  
//#pragma map call_hw VIRTEX5 0
  *al2 = uppol2 (*al1, *al2, plt, *plt1, *plt2);

/* uppol1 :update first predictor coefficient apl1 and delay it as al1 */
/* calling parameters: al1, apl2, plt, plt1 */
  //#pragma map call_hw VIRTEX5 0
  *al1 = uppol1 (*al1, *al2, plt, *plt1);
#else
/* uppol2- update second predictor coefficient apl2 and delay it as al2 */
/* calling parameters: al1, al2, plt, plt1, plt2 */
  al2 = uppol2 (al1, al2, plt, plt1, plt2);

/* uppol1 :update first predictor coefficient apl1 and delay it as al1 */
/* calling parameters: al1, apl2, plt, plt1 */
  al1 = uppol1 (al1, al2, plt, plt1);
#endif /* DWARV */

/* recons : compute recontructed signal for adaptive predictor */
  rlt = sl + dlt;

/* done with lower sub_band encoder; now implement delays for next time*/
#ifdef DWARV
  *rlt2 = *rlt1;
  *rlt1 = rlt;
  *plt2 = *plt1;
  *plt1 = plt;
#else
  rlt2 = rlt1;
  rlt1 = rlt;
  plt2 = plt1;
  plt1 = plt;
#endif /* DWARV */

/* high band encode */
rh1=rh1;

  //#pragma map call_hw VIRTEX5 0
  szh = filtez (delay_bph, delay_dhx);

#ifdef DWARV
rh1=rh1;

  //#pragma map call_hw VIRTEX5 0
  sph = filtep (*rh1, *ah1, *rh2, *ah2);
#else
  sph = filtep (rh1, ah1, rh2, ah2);
#endif /* DWARV */

/* predic: sh = sph + szh */
  sh = sph + szh;
/* subtra: eh = xh - sh */
  eh = xh - sh;

/* quanth - quantization of difference signal for higher sub-band */
/* quanth: in-place for speed params: eh, deth (has init. value) */
  if (eh >= 0)
    {
      ih = 3;			/* 2,3 are pos codes */
    }
  else
    {
      ih = 1;			/* 0,1 are neg codes */
    }
#ifdef DWARV
  decis = (564L * (long) *deth) >> 12L;
  if ( ((eh < 0) ? -eh : eh ) > decis)
#else
  decis = (564L * (long) deth) >> 12L;
  if (abs (eh) > decis)
#endif /* DWARV */
    ih--;			/* mih = 2 case */

#ifdef DWARV
/* compute the quantized difference signal, higher sub-band*/
  dh = ((long) *deth * qq2_code2_table[ih]) >> 15L;

/* logsch: update logarithmic quantizer scale factor in hi sub-band*/
  //#pragma map call_hw VIRTEX5 0
  *nbh = logsch (ih, *nbh, wh_code_table);

/* note : scalel and scaleh use same code, different parameters */
  //#pragma map call_hw VIRTEX5 0
  *deth = scalel (*nbh, 10, ilb_table);
#else
/* compute the quantized difference signal, higher sub-band*/
  dh = ((long) deth * qq2_code2_table[ih]) >> 15L;

/* logsch: update logarithmic quantizer scale factor in hi sub-band*/
  nbh = logsch (ih, nbh);

/* note : scalel and scaleh use same code, different parameters */
  deth = scalel (nbh, 10);
#endif /* DWARV */

/* parrec - add pole predictor output to quantized diff. signal */
  ph = dh + szh;

/* upzero: update zero section predictor coefficients (sixth order) */
/* calling parameters: dh, dhi, bphi */
/* return params: updated bphi, delayed dhx */
  upzero (dh, delay_dhx, delay_bph);

  
  
#ifdef DWARV
/* uppol2: update second predictor coef aph2 and delay as ah2 */
/* calling params: ah1, ah2, ph, ph1, ph2 */
  //#pragma map call_hw VIRTEX5 0
  *ah2 = uppol2 (*ah1, *ah2, ph, *ph1, *ph2);

/* uppol1:  update first predictor coef. aph2 and delay it as ah1 */
  //#pragma map call_hw VIRTEX5 0
  *ah1 = uppol1 (*ah1, *ah2, ph, *ph1);
#else
/* uppol2: update second predictor coef aph2 and delay as ah2 */
/* calling params: ah1, ah2, ph, ph1, ph2 */
  ah2 = uppol2 (ah1, ah2, ph, ph1, ph2);

/* uppol1:  update first predictor coef. aph2 and delay it as ah1 */
  ah1 = uppol1 (ah1, ah2, ph, ph1);
#endif /* DWARV */

/* recons for higher sub-band */
  yh = sh + dh;

/* done with higher sub-band encoder, now Delay for next time */
#ifdef DWARV
  *rh2 = *rh1;
  *rh1 = yh;
  *ph2 = *ph1;
  *ph1 = ph;

/* multiplex ih and il to get signals together */
  return (*il | (ih << 6));
#else
  rh2 = rh1;
  rh1 = yh;
  ph2 = ph1;
  ph1 = ph;

/* multiplex ih and il to get signals together */
  return (il | (ih << 6));
#endif /* DWARV */
rh1=rh1;
}

/* decode function, result in xout1 and xout2 */
void
decode (int input)
{
  int i;
  volatile long int xa1, xa2;		/* qmf accumulators */
  volatile const int *h_ptr;
  volatile int *ac_ptr, *ac_ptr1, *ad_ptr, *ad_ptr1;

/* split transmitted word from input into ilr and ih */
  ilr = input & 0x3f;
  ih = input >> 6;

/* LOWER SUB_BAND DECODER */

/* filtez: compute predictor output for zero section */
  dec_szl = filtez (dec_del_bpl, dec_del_dltx);

/* filtep: compute predictor output signal for pole section */
  dec_spl = filtep (dec_rlt1, dec_al1, dec_rlt2, dec_al2);

  dec_sl = dec_spl + dec_szl;

/* compute quantized difference signal for adaptive predic */
  dec_dlt = ((long) dec_detl * qq4_code4_table[ilr >> 2]) >> 15;

/* compute quantized difference signal for decoder output */
  dl = ((long) dec_detl * qq6_code6_table[il]) >> 15;

  rl = dl + dec_sl;

/* logscl: quantizer scale factor adaptation in the lower sub-band */
#ifdef DWARV
  dec_nbl = logscl (ilr, dec_nbl, wl_code_table);
#else
  dec_nbl = logscl (ilr, dec_nbl);
#endif /* DWARV */

/* scalel: computes quantizer scale factor in the lower sub band */
#ifdef DWARV
  dec_detl = scalel (dec_nbl, 8, ilb_table);
#else
  dec_detl = scalel (dec_nbl, 8);
#endif /* DWARV */

/* parrec - add pole predictor output to quantized diff. signal */
/* for partially reconstructed signal */
  dec_plt = dec_dlt + dec_szl;

/* upzero: update zero section predictor coefficients */
  upzero (dec_dlt, dec_del_dltx, dec_del_bpl);

/* uppol2: update second predictor coefficient apl2 and delay it as al2 */
  dec_al2 = uppol2 (dec_al1, dec_al2, dec_plt, dec_plt1, dec_plt2);

/* uppol1: update first predictor coef. (pole setion) */
  dec_al1 = uppol1 (dec_al1, dec_al2, dec_plt, dec_plt1);

/* recons : compute recontructed signal for adaptive predictor */
  dec_rlt = dec_sl + dec_dlt;

/* done with lower sub band decoder, implement delays for next time */
  dec_rlt2 = dec_rlt1;
  dec_rlt1 = dec_rlt;
  dec_plt2 = dec_plt1;
  dec_plt1 = dec_plt;

/* HIGH SUB-BAND DECODER */

/* filtez: compute predictor output for zero section */
  dec_szh = filtez (dec_del_bph, dec_del_dhx);

/* filtep: compute predictor output signal for pole section */
  dec_sph = filtep (dec_rh1, dec_ah1, dec_rh2, dec_ah2);

/* predic:compute the predictor output value in the higher sub_band decoder */
  dec_sh = dec_sph + dec_szh;

/* in-place compute the quantized difference signal */
  dec_dh = ((long) dec_deth * qq2_code2_table[ih]) >> 15L;

/* logsch: update logarithmic quantizer scale factor in hi sub band */
#ifdef DWARV
  dec_nbh = logsch (ih, dec_nbh, wh_code_table);
#else
  dec_nbh = logsch (ih, dec_nbh);
#endif /* DWARV */

/* scalel: compute the quantizer scale factor in the higher sub band */
#ifdef DWARV
  dec_deth = scalel (dec_nbh, 10, ilb_table);
#else
  dec_deth = scalel (dec_nbh, 10);
#endif /* DWARV */

/* parrec: compute partially recontructed signal */
  dec_ph = dec_dh + dec_szh;

/* upzero: update zero section predictor coefficients */
  upzero (dec_dh, dec_del_dhx, dec_del_bph);

/* uppol2: update second predictor coefficient aph2 and delay it as ah2 */
  dec_ah2 = uppol2 (dec_ah1, dec_ah2, dec_ph, dec_ph1, dec_ph2);

/* uppol1: update first predictor coef. (pole setion) */
  dec_ah1 = uppol1 (dec_ah1, dec_ah2, dec_ph, dec_ph1);

/* recons : compute recontructed signal for adaptive predictor */
  rh = dec_sh + dec_dh;

/* done with high band decode, implementing delays for next time here */
  dec_rh2 = dec_rh1;
  dec_rh1 = rh;
  dec_ph2 = dec_ph1;
  dec_ph1 = dec_ph;

/* end of higher sub_band decoder */

/* end with receive quadrature mirror filters */
  xd = rl - rh;
  xs = rl + rh;

/* receive quadrature mirror filters implemented here */
  h_ptr = h;
  ac_ptr = accumc;
  ad_ptr = accumd;
  xa1 = (long) xd *(*h_ptr++);
  xa2 = (long) xs *(*h_ptr++);
/* main multiply accumulate loop for samples and coefficients */
  for (i = 0; i < 10; i++)
    {
      xa1 += (long) (*ac_ptr++) * (*h_ptr++);
      xa2 += (long) (*ad_ptr++) * (*h_ptr++);
    }
/* final mult/accumulate */
  xa1 += (long) (*ac_ptr) * (*h_ptr++);
  xa2 += (long) (*ad_ptr) * (*h_ptr++);

/* scale by 2^14 */
  xout1 = xa1 >> 14;
  xout2 = xa2 >> 14;

/* update delay lines */
  ac_ptr1 = ac_ptr - 1;
  ad_ptr1 = ad_ptr - 1;
  for (i = 0; i < 10; i++)
    {
      *ac_ptr-- = *ac_ptr1--;
      *ad_ptr-- = *ad_ptr1--;
    }
  *ac_ptr = xd;
  *ad_ptr = xs;
}

/* clear all storage locations */

void
#ifdef DWARV
reset (int *tqmf, int *delay_bpl, int *delay_dltx, int *al1, int *al2, int *rlt1, int *rlt2, int *detl, int *nbl, int *delay_dhx, int *delay_bph, int *ah1, int *ah2, int *rh1, int *rh2, int *plt1, int *plt2, int *ph1, int *ph2, int *nbh, int *deth)
{
  int i;

  *detl = dec_detl = 32;		/* reset to min scale factor */
  *deth = dec_deth = 8;
  *nbl = *al1 = *al2 = *plt1 = *plt2 = *rlt1 = *rlt2 = 0;
  *nbh = *ah1 = *ah2 = *ph1 = *ph2 = *rh1 = *rh2 = 0;
  dec_nbl = dec_al1 = dec_al2 = dec_plt1 = dec_plt2 = dec_rlt1 = dec_rlt2 = 0;
  dec_nbh = dec_ah1 = dec_ah2 = dec_ph1 = dec_ph2 = dec_rh1 = dec_rh2 = 0;
#else
reset ()
{
  int i;

  detl = dec_detl = 32;		/* reset to min scale factor */
  deth = dec_deth = 8;
  nbl = al1 = al2 = plt1 = plt2 = rlt1 = rlt2 = 0;
  nbh = ah1 = ah2 = ph1 = ph2 = rh1 = rh2 = 0;
  dec_nbl = dec_al1 = dec_al2 = dec_plt1 = dec_plt2 = dec_rlt1 = dec_rlt2 = 0;
  dec_nbh = dec_ah1 = dec_ah2 = dec_ph1 = dec_ph2 = dec_rh1 = dec_rh2 = 0;
#endif /* DWARV */

  //loop1: for (i = 0; i < 6; i++)
  for (i = 0; i < 6; i++)
    {
      delay_dltx[i] = 0;
      delay_dhx[i] = 0;
      dec_del_dltx[i] = 0;
      dec_del_dhx[i] = 0;
    }

  //loop2: for (i = 0; i < 6; i++)
  for (i = 0; i < 6; i++)
    {
      delay_bpl[i] = 0;
      delay_bph[i] = 0;
      dec_del_bpl[i] = 0;
      dec_del_bph[i] = 0;
    }

  //loop3: for (i = 0; i < 24; i++)
  for (i = 0; i < 24; i++)
    tqmf[i] = 0;		// i<23

  //loop4: for (i = 0; i < 11; i++)
  for (i = 0; i < 11; i++)
    {
      accumc[i] = 0;
      accumd[i] = 0;
    }
}

/* filtez - compute predictor output signal (zero section) */
/* input: bpl1-6 and dlt1-6, output: szl */

int
filtez (int *bpl, int *dlt)
{
  int i;
  long int zl;
  zl = (long) (*bpl++) * (*dlt++);
  for (i = 1; i < 6; i++)
    zl += (long) (*bpl++) * (*dlt++);

  return ((int) (zl >> 14));	/* x2 here */
}

/* filtep - compute predictor output signal (pole section) */
/* input rlt1-2 and al1-2, output spl */

int
filtep (int rlt1, int al1, int rlt2, int al2)
{
  long int pl, pl2;
  pl = 2 * rlt1;
  pl = (long) al1 *pl;
  pl2 = 2 * rlt2;
  pl += (long) al2 *pl2;
  return ((int) (pl >> 15));
}

/* quantl - quantize the difference signal in the lower sub-band */
int
#ifdef DWARV
quantl (int el, int detl, int *decis_levl, int *quant26bt_pos, int *quant26bt_neg)
#else
quantl (int el, int detl)
#endif /* DWARV */
{
  int ril, mil;
  long int wd, decis;

/* abs of difference signal */
#ifdef DWARV
  wd = ( el < 0 ) ? -el : el;
#else
  wd = abs (el);
#endif /* DWARV */
/* determine mil based on decision levels and detl gain */
  for (mil = 0; mil < 30; mil++)
    {
      decis = (decis_levl[mil] * (long) detl) >> 15L;
      if (wd <= decis)
	break;
    }
/* if mil=30 then wd is less than all decision levels */
  if (el >= 0)
    ril = quant26bt_pos[mil];
  else
    ril = quant26bt_neg[mil];
  return (ril);
}

/* logscl - update log quantizer scale factor in lower sub-band */
/* note that nbl is passed and returned */

int
#ifdef DWARV
logscl (int il, int nbl, int *wl_code_table)
#else
logscl (int il, int nbl)
#endif /* DWARV */
{
  long int wd;
  wd = ((long) nbl * 127L) >> 7L;	/* leak factor 127/128 */
  nbl = (int) wd + wl_code_table[il >> 2];
  if (nbl < 0)
    nbl = 0;
  if (nbl > 18432)
    nbl = 18432;
  return (nbl);
}

/* scalel: compute quantizer scale factor in lower or upper sub-band*/

int
#ifdef DWARV
scalel (int nbl, int shift_constant, int *ilb_table)
#else
scalel (int nbl, int shift_constant)
#endif /* DWARV */
{
  int wd1, wd2, wd3;
  wd1 = (nbl >> 6) & 31;
  wd2 = nbl >> 11;
  wd3 = ilb_table[wd1] >> (shift_constant + 1 - wd2);
  return (wd3 << 3);
}

/* upzero - inputs: dlt, dlti[0-5], bli[0-5], outputs: updated bli[0-5] */
/* also implements delay of bli and update of dlti from dlt */

void
upzero (int dlt, int *dlti, int *bli)
{
  int i, wd2, wd3;
/*if dlt is zero, then no sum into bli */
  if (dlt == 0)
    {
      for (i = 0; i < 6; i++)
	{
	  bli[i] = (int) ((255L * bli[i]) >> 8L);	/* leak factor of 255/256 */
	}
    }
  else
    {
      for (i = 0; i < 6; i++)
	{
	  if ((long) dlt * dlti[i] >= 0)
	    wd2 = 128;
	  else
	    wd2 = -128;
	  wd3 = (int) ((255L * bli[i]) >> 8L);	/* leak factor of 255/256 */
	  bli[i] = wd2 + wd3;
	}
    }
/* implement delay line for dlt */
  dlti[5] = dlti[4];
  dlti[4] = dlti[3];
  dlti[3] = dlti[2];
  dlti[1] = dlti[0];
  dlti[0] = dlt;
}

/* uppol2 - update second predictor coefficient (pole section) */
/* inputs: al1, al2, plt, plt1, plt2. outputs: apl2 */

int
uppol2 (int al1, int al2, int plt, int plt1, int plt2)
{
  long int wd2, wd4;
  int apl2;
  wd2 = 4L * (long) al1;
  if ((long) plt * plt1 >= 0L)
    wd2 = -wd2;			/* check same sign */
  wd2 = wd2 >> 7;		/* gain of 1/128 */
  if ((long) plt * plt2 >= 0L)
    {
      wd4 = wd2 + 128;		/* same sign case */
    }
  else
    {
      wd4 = wd2 - 128;
    }
  apl2 = wd4 + (127L * (long) al2 >> 7L);	/* leak factor of 127/128 */

/* apl2 is limited to +-.75 */
  if (apl2 > 12288)
    apl2 = 12288;
  if (apl2 < -12288)
    apl2 = -12288;
  return (apl2);
}

/* uppol1 - update first predictor coefficient (pole section) */
/* inputs: al1, apl2, plt, plt1. outputs: apl1 */

int
uppol1 (int al1, int apl2, int plt, int plt1)
{
  long int wd2;
  int wd3, apl1;
  wd2 = ((long) al1 * 255L) >> 8L;	/* leak factor of 255/256 */
  if ((long) plt * plt1 >= 0L)
    {
      apl1 = (int) wd2 + 192;	/* same sign case */
    }
  else
    {
      apl1 = (int) wd2 - 192;
    }
/* note: wd3= .9375-.75 is always positive */
  wd3 = 15360 - apl2;		/* limit value */
  if (apl1 > wd3)
    apl1 = wd3;
  if (apl1 < -wd3)
    apl1 = -wd3;
  return (apl1);
}

/* logsch - update log quantizer scale factor in higher sub-band */
/* note that nbh is passed and returned */

int
#ifdef DWARV
logsch (int ih, int nbh, int *wh_code_table)
#else
logsch (int ih, int nbh)
#endif /* DWARV */
{
  int wd;
  wd = ((long) nbh * 127L) >> 7L;	/* leak factor 127/128 */
  nbh = wd + wh_code_table[ih];
  if (nbh < 0)
    nbh = 0;
  if (nbh > 22528)
    nbh = 22528;
  return (nbh);
}

/*
+--------------------------------------------------------------------------+
| * Test Vectors (added for CHStone)                                       |
|     test_data : input data                                               |
|     test_compressed : expected output data for "encode"                  |
|     test_result : expected output data for "decode"                      |
+--------------------------------------------------------------------------+
*/

#define SIZE 100
#define IN_END 100

const int test_data[SIZE] = {
  0x44, 0x44, 0x44, 0x44, 0x44,
  0x44, 0x44, 0x44, 0x44, 0x44,
  0x44, 0x44, 0x44, 0x44, 0x44,
  0x44, 0x44, 0x43, 0x43, 0x43,
  0x43, 0x43, 0x43, 0x43, 0x42,
  0x42, 0x42, 0x42, 0x42, 0x42,
  0x41, 0x41, 0x41, 0x41, 0x41,
  0x40, 0x40, 0x40, 0x40, 0x40,
  0x40, 0x40, 0x40, 0x3f, 0x3f,
  0x3f, 0x3f, 0x3f, 0x3e, 0x3e,
  0x3e, 0x3e, 0x3e, 0x3e, 0x3d,
  0x3d, 0x3d, 0x3d, 0x3d, 0x3d,
  0x3c, 0x3c, 0x3c, 0x3c, 0x3c,
  0x3c, 0x3c, 0x3c, 0x3c, 0x3b,
  0x3b, 0x3b, 0x3b, 0x3b, 0x3b,
  0x3b, 0x3b, 0x3b, 0x3b, 0x3b,
  0x3b, 0x3b, 0x3b, 0x3b, 0x3b,
  0x3b, 0x3b, 0x3b, 0x3b, 0x3b,
  0x3b, 0x3b, 0x3c, 0x3c, 0x3c,
  0x3c, 0x3c, 0x3c, 0x3c, 0x3c
};
int compressed[SIZE], result[SIZE];
const int test_compressed[SIZE] = {
  253, 222, 119, 186, 244,
  146, 32, 160, 236, 237,
  238, 240, 241, 241, 242,
  243, 244, 243, 244, 245,
  244, 244, 245, 245, 245,
  246, 246, 247, 247, 247,
  247, 248, 246, 247, 249,
  247, 248, 247, 248, 247,
  248, 247, 248, 247, 248,
  248, 246, 248, 247, 248,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0
};
volatile const int test_result[SIZE] = {
  0, -1, -1, 0, 0,
  -1, 0, 0, -1, -1,
  0, 0, 1, 1, 0,
  -2, -1, -2, 0, -3,
  1, 0, 0, -4, 1,
  1, 2, 11, 20, 18,
  20, 22, 28, 27, 31,
  31, 34, 31, 34, 34,
  38, 37, 42, 42, 44,
  41, 43, 42, 47, 45,
  47, 44, 45, 43, 46,
  45, 48, 46, 49, 48,
  51, 49, 52, 52, 55,
  53, 56, 55, 58, 57,
  59, 57, 60, 60, 60,
  54, 54, 53, 60, 62,
  62, 54, 55, 56, 59,
  53, 54, 56, 59, 53,
  56, 58, 59, 53, 56,
  58, 60, 54, 55, 57
};

void
adpcm_main ()
{
  int i = 0, j = 0;

#ifdef DWARV
  int *m_test_data = (int *) malloc (SIZE * sizeof (int));
  int *m_h = (int *) malloc (24 * sizeof (int));
  int *tqmf = (int *) malloc (24 * sizeof (int));
  int *delay_bpl = (int *) malloc (6 * sizeof (int));
  int *delay_dltx = (int *) malloc (6 * sizeof (int));
  int *al1 = (int *) malloc (sizeof (int));
  int *al2 = (int *) malloc (sizeof (int));
  int *rlt1 = (int *) malloc (sizeof (int));
  int *rlt2 = (int *) malloc (sizeof (int));
  int *m_il = (int *) malloc (sizeof (int));
  int *detl = (int *) malloc (sizeof (int));
  int *m_qq4_code4_table = (int *) malloc (16 * sizeof (int));
  int *nbl = (int *) malloc (sizeof (int));
  int *delay_dhx = (int *) malloc (6 * sizeof (int));
  int *delay_bph = (int *) malloc (6 * sizeof (int));
  int *ah1 = (int *) malloc (sizeof (int));
  int *ah2 = (int *) malloc (sizeof (int));
  int *rh1 = (int *) malloc (sizeof (int));
  int *rh2 = (int *) malloc (sizeof (int));
  int *m_qq2_code2_table = (int *) malloc (4 * sizeof (int));
  int *m_decis_levl = (int *) malloc (30 * sizeof (int));
  int *m_quant26bt_pos = (int *) malloc (31 * sizeof (int));
  int *m_quant26bt_neg = (int *) malloc (31 * sizeof (int));
  int *m_wl_code_table = (int *) malloc (16 * sizeof (int));
  int *m_ilb_table = (int *) malloc (32 * sizeof (int));
  int *plt1 = (int *) malloc (sizeof (int));
  int *plt2 = (int *) malloc (sizeof (int));
  int *m_wh_code_table = (int *) malloc (4 * sizeof (int));
  int *ph1 = (int *) malloc (sizeof (int));
  int *ph2 = (int *) malloc (sizeof (int));
  int *nbh = (int *) malloc (sizeof (int));
  int *deth = (int *) malloc (sizeof (int));

  for (i = 0; i < 24; i++)
    m_h[i] = h[i];
  for (i = 0; i < 16; i++)
  {
    m_qq4_code4_table[i] = qq4_code4_table[i];
    m_wl_code_table[i] = wl_code_table[i];
  }
  for (i = 0; i < 4; i++)
  {
    m_qq2_code2_table[i] = qq2_code2_table[i];
    m_wh_code_table[i] = wh_code_table[i];
  }
  for (i = 0; i < 30; i++)
    m_decis_levl[i] = decis_levl[i];
  for (i = 0; i < 31; i++)
  {
    m_quant26bt_pos[i] = quant26bt_pos[i];
    m_quant26bt_neg[i] = quant26bt_neg[i];
  }
  for (i = 0; i < 32; i++)
    m_ilb_table[i] = ilb_table[i];
  for (i = 0; i < SIZE; i++)
    m_test_data[i] = test_data[i];

/* reset, initialize required memory */
  reset (tqmf, delay_bpl, delay_dltx, al1, al2, rlt1, rlt2, detl, nbl, delay_dhx, delay_bph, ah1, ah2, rh1, rh2, plt1, plt2, ph1, ph2, nbh, deth);
#else
/* reset, initialize required memory */
  reset ();
#endif /* DWARV */

  j = 10;

#ifdef DWARV
//  for (i = 0; i < IN_END; i += 2)
  i=0;
    {
      #pragma map call_hw VIRTEX5 0
      compressed[i / 2] = encode (m_test_data[i], m_test_data[i + 1], m_h, tqmf, xl, xh, szl, delay_bpl, delay_dltx, spl, al1, al2, rlt1, rlt2, sl, el, m_il, detl, dlt, m_qq4_code4_table, nbl, plt, rlt, szh, delay_dhx, delay_bph, sph, ah1, ah2, rh1, rh2, sh, rh, ih, deth, m_qq2_code2_table, dh, nbh, ph, yh, m_decis_levl, m_quant26bt_pos, m_quant26bt_neg, m_wl_code_table, m_ilb_table, plt1, plt2, m_wh_code_table, ph1, ph2);

	/* Decode uses the global il, so we should set that to the right value. Otherwise, we'll get an error complaining about incorrect results */
	il = *m_il;
    }
  //for (i = 0; i < IN_END; i += 2)
    {
//      decode (compressed[i / 2]);
      result[i] = xout1;
      result[i + 1] = xout2;
    }
#else
  //for (i = 0; i < IN_END; i += 2)
  i=0;
    {
      compressed[i / 2] = encode (test_data[i], test_data[i + 1]);
    }
//  for (i = 0; i < IN_END; i += 2)
    {
      decode (compressed[i / 2]);
      result[i] = xout1;
      result[i + 1] = xout2;
    }
#endif /* DWARV */

#ifdef DWARV
  free(m_test_data);
  free(m_h);
  free(tqmf);
  free(delay_bpl);
  free(delay_dltx);
  free(al1);
  free(al2);
  free(rlt1);
  free(rlt2);
  free(m_il);
  free(detl);
  free(m_qq4_code4_table);
  free(nbl);
  free(delay_dhx);
  free(delay_bph);
  free(ah1);
  free(ah2);
  free(rh1);
  free(rh2);
  free(m_qq2_code2_table);
  free(m_decis_levl);
  free(m_quant26bt_pos);
  free(m_quant26bt_neg);
  free(m_wl_code_table);
  free(m_ilb_table);
  free(plt1);
  free(plt2);
  free(m_wh_code_table);
  free(ph1);
  free(ph2);
  free(nbh);
  free(deth);
#endif

}

int
main ()
{
  int i;
  int main_result;

      main_result = 0;
      adpcm_main ();
//       for (i = 0; i < IN_END / 2; i++)
  i=0;
	{
	  if (compressed[i] != test_compressed[i])
	    {
	      main_result = 1;
		printf( "compressed[%d] = %d instead of %d\n", i, compressed[i], test_compressed[i] );
	    }
	}
//       for (i = 0; i < IN_END; i++)
	{
	  if (result[i] != test_result[i])
	    {
	      main_result = 1;
		printf( "result[%d] = %d instead of %d\n", i, result[i], test_result[i] );
	    }
	}
      printf ("%d\n", main_result);
      return main_result;
    } 
