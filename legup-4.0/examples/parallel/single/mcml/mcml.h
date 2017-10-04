/***********************************************************
 *  Copyright Univ. of Texas M.D. Anderson Cancer Center
 *  1992.
 *
 *	Monte Carlo simulation of photon distribution in 
 *	multi-layered turbid media in ANSI Standard C.
 ****
 *	Starting Date:	10/1991.
 *	Current Date:	6/1992.
 *
 *	Lihong Wang, Ph. D.
 *	Steven L. Jacques, Ph. D.
 *	Laser Biology Research Laboratory - 17
 *	M.D. Anderson Cancer Center
 *	University of Texas
 *	1515 Holcombe Blvd.
 *	Houston, TX 77030
 *	USA
 *
 *	This program was based on:
 *	(1) The Pascal code written by Marleen Keijzer and 
 *	Steven L. Jacques in this laboratory in 1989, which
 *	deals with multi-layered turbid media.
 *
 *	(2) Algorithm for semi-infinite turbid medium by 
 *	S.A. Prahl, M. Keijzer, S.L. Jacques, A.J. Welch, 
 *	SPIE Institute Series Vol. IS 5 (1989), and by 
 *	A.N. Witt, The Astrophysical journal Supplement
 *	Series 35, 1-6 (1977).
 *	
 *	Major modifications include:
 *		. Conform to ANSI Standard C.
 *		. Removal of limit on number of array elements, 
 *		  because arrays in this program are dynamically 
 *		  allocated. This means that the program can accept 
 *		  any number of layers or gridlines as long as the 
 *		  memory permits.
 *		. Avoiding global variables whenever possible.  This
 *		  program has not used global variables so far.
 *		. Grouping variables logically using structures.
 *		. Top-down design, keep each subroutine clear & 
 *		  short.	
 *		. Reflectance and transmittance are angularly 
 *		  resolved.
 ****
 *	General Naming Conventions:
 *	Preprocessor names: all capital letters, 
 *		e.g. #define PREPROCESSORS
 *	Globals: first letter of each word is capital, no 
 *		underscores, 
 *		e.g. short GlobalVar;
 *	Dummy variables:  first letter of each word is capital,
 *		and words are connected by underscores, 
 *		e.g. void NiceFunction(char Dummy_Var);
 *	Local variables:  all lower cases, words are connected 
 *		by underscores,
 *		e.g. short local_var;
 *	Function names or data types:  same as Globals.
 *
 ****
 *	Dimension of length: cm.
 *
 ****/

#ifndef MCML
#define MCML
#include "fixedptc.h"

typedef char Boolean;

//#define PI 3.1415926
//#define WEIGHT 1E-4		/* Critical weight for roulette. */
#define CHANCE 0.1		/* Chance of roulette survival. */
#define CHANCE_FIXED 6554       /* in 16.16 FIXED POINT -- WATCH OUT JANDERS */
//#define STRLEN 256		/* String length. */

/****************** Stuctures *****************************/

/****
 *	Structure used to describe a photon packet.
 ****/
typedef struct {
  fixedpt x, y ,z;	/* Cartesian coordinates.[cm] */
  fixedpt ux, uy, uz;/* directional cosines of a photon. */
  fixedpt w;			/* weight. */
  Boolean dead;		/* 1 if photon is terminated. */
  short layer;		/* index to layer where the photon */
					/* packet resides. */
  fixedpt s;			/* current step size. [cm]. */
  fixedpt sleft;		/* step size left. dimensionless [-]. */

  int seed; /* seed for random number generator for this photon -- janders */
} PhotonStruct;

/****
 *	Structure used to describe the geometry and optical
 *	properties of a layer.
 *	z0 and z1 are the z coordinates for the upper boundary
 *	and lower boundary respectively.
 *
 *	cos_crit0 and cos_crit1 are the cosines of the 
 *	critical angle of total internal reflection for the
 *	upper boundary and lower boundary respectively.
 *	They are set to zero if no total internal reflection
 *	exists.
 *	They are used for computation speed.
 ****/
typedef struct {
  fixedpt z0, z1;	/* z coordinates of a layer. [cm] */
  fixedpt n;			/* refractive index of a layer. */
  fixedpt mua;	    /* absorption coefficient. [1/cm] */
  fixedpt mus;	    /* scattering coefficient. [1/cm] */
  fixedpt g;		    /* anisotropy. */
  
  fixedpt cos_crit0,	cos_crit1;	
} LayerStruct;

/****
 *	Input parameters for each independent run.
 *
 *	z and r are for the cylindrical coordinate system. [cm]
 *	a is for the angle alpha between the photon exiting 
 *	direction and the surface normal. [radian]
 *
 *	The grid line separations in z, r, and alpha
 *	directions are dz, dr, and da respectively.  The numbers 
 *	of grid lines in z, r, and alpha directions are
 *	nz, nr, and na respectively.
 *
 *	The member layerspecs will point to an array of 
 *	structures which store parameters of each layer. 
 *	This array has (number_layers + 2) elements. One
 *	element is for a layer.
 *	The layers 0 and (num_layers + 1) are for top ambient 
 *	medium and the bottom ambient medium respectively.
 ****/
typedef struct {

  long	 num_photons; 		/* to be traced. */
  fixedpt Wth; 				/* play roulette if photon */
							/* weight < Wth.*/
  
  fixedpt dz;				/* z grid separation.[cm] */ 
  fixedpt dr;				/* r grid separation.[cm] */
  fixedpt da;				/* alpha grid separation. */
							/* [radian] */
  short nz;					/* array range 0..nz-1. */
  short nr;					/* array range 0..nr-1. */
  short na;					/* array range 0..na-1. */
  
  short	num_layers;			/* number of layers. */
  LayerStruct layerspecs[5];	/* layer parameters. */	

} InputStruct;

#endif

