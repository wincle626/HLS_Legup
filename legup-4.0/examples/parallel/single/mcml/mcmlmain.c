/***********************************************************
 *  Copyright Univ. of Texas M.D. Anderson Cancer Center
 *  1992.
 *
 *	main program for Monte Carlo simulation of photon
 *	distribution in multi-layered turbid media.
 *
 ****/

#include "mcml.h"
#include "fixedptc.c"
#include "mcmlgo.c"

void LaunchPhoton(fixedpt, LayerStruct *, PhotonStruct *);
void HopDropSpin(InputStruct  *,PhotonStruct *, fixedpt Results[50][40]);
fixedpt Rspecular(LayerStruct * );

fixedpt Absorption[50][40] = {0};
    
/***********************************************************
 *	Execute Monte Carlo simulation for one independent run.
 ****/
void DoOneRun(InputStruct *In_Ptr)
{
  long i_photon;	
  PhotonStruct photon;
  long num_photons = In_Ptr->num_photons;
  fixedpt Rspec = Rspecular(In_Ptr->layerspecs);

  i_photon = num_photons;

  for (i_photon = 0; i_photon < num_photons; i_photon++) {
    printf("Launch\n");
    photon.seed = i_photon + 2; /* seed the RNG differently for each photon */
    LaunchPhoton(Rspec, In_Ptr->layerspecs, &photon);
    do  HopDropSpin(In_Ptr, &photon, Absorption);
    while (!photon.dead);
  } 
    
}

/***********************************************************
 *	The argument to the command line is filename, if any.
 *	Macintosh does not support command line.
 ****/
int main() 
{
    InputStruct in_parm;

    in_parm.num_photons = 16; in_parm.Wth = 7; /*fixedpt_rconst(0.0001);*/
    in_parm.dz = 655/*fixedpt_rconst(0.01)*/; in_parm.dr = 655/*fixedpt_rconst(0.01)*/; in_parm.da = 102944/*fixedpt_rconst(1.5708)*/;
    in_parm.nz = 40; in_parm.nr = 50; in_parm.na = 1;
    in_parm.num_layers = 3;
    in_parm.layerspecs[0].n = FIXEDPT_ONE; // ambient
    in_parm.layerspecs[3+1].n = FIXEDPT_ONE; // ambient

    in_parm.layerspecs[1].z0 = 0/*fixedpt_rconst(0.0)*/; in_parm.layerspecs[1].z1 = 6554/*fixedpt_rconst(0.1)*/;
    in_parm.layerspecs[1].n = 89784/*fixedpt_rconst(1.37)*/; in_parm.layerspecs[1].mua = 65536/*fixedpt_rconst(1)*/; in_parm.layerspecs[1].mus = 6553600/*fixedpt_rconst(100)*/; in_parm.layerspecs[1].g = 58982/*fixedpt_rconst(0.9)*/; 
    in_parm.layerspecs[1].cos_crit0 = 44795/*fixedpt_rconst(0.683525)*/;
    in_parm.layerspecs[1].cos_crit1 = 0/*fixedpt_rconst(0)*/;

    in_parm.layerspecs[2].z0 = 6554/*fixedpt_rconst(0.1)*/; in_parm.layerspecs[2].z1 = 13107/*fixedpt_rconst(0.2)*/;
    in_parm.layerspecs[2].n = 89784/*fixedpt_rconst(1.37)*/; in_parm.layerspecs[2].mua = 65536/*fixedpt_rconst(1)*/; in_parm.layerspecs[2].mus = 655360/*fixedpt_rconst(10)*/; in_parm.layerspecs[2].g = 0/*fixedpt_rconst(0.0)*/; 
    in_parm.layerspecs[2].cos_crit0 = 0/*fixedpt_rconst(0)*/;
    in_parm.layerspecs[2].cos_crit1 = 0/*fixedpt_rconst(0)*/;

    in_parm.layerspecs[3].z0 = 13107/*fixedpt_rconst(0.2)*/; in_parm.layerspecs[3].z1 = 26214/*fixedpt_rconst(0.4)*/;
    in_parm.layerspecs[3].n = 89784/*fixedpt_rconst(1.37)*/; in_parm.layerspecs[3].mua = 131072/*fixedpt_rconst(2)*/; in_parm.layerspecs[3].mus = 655360/*fixedpt_rconst(10)*/; in_parm.layerspecs[3].g = 45875/*fixedpt_rconst(0.7)*/; 
    in_parm.layerspecs[3].cos_crit0 = 0/*fixedpt_rconst(0.0))*/;
    in_parm.layerspecs[3].cos_crit1 = 44795/*fixedpt_rconst(0.683525)*/;

    DoOneRun(&in_parm);
  
    fixedpt tally = 0;
    int i,j;
    for (i = 0; i < 50; i++)
      for (j = 0; j < 40; j++)
	tally += Absorption[i][j];

    printf("Result: %d\n", tally);
    if (tally == 739047)
      printf("RESULT: PASS\n");
    else
      printf("RESULT: FAIL\n");
    //    fixedpt_print(tally);

    return tally;
}
