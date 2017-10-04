/***********************************************************
 *  Copyright Univ. of Texas M.D. Anderson Cancer Center
 *  1992.
 *
 *	Launch, move, and record photon weight.
 ****/

#include "mcml.h"

#define COSZERO FIXEDPT_ONE - 1
#define COS90D  1

fixedpt get_uniform_fixed ( int *seed )

{
    int i;
    int i4_huge = 2147483647;
    int k;
    fixedpt r;
    k = *seed / 127773;
    // printf("k = %i \n", k);
    
    *seed = 16807 * ( *seed - k * 127773 ) - k * 2836;
    //printf("*seed = %i \n", *seed);
    
    if ( *seed < 0 )
    {
      *seed = *seed + i4_huge;
    }

    r = ((unsigned)(*seed & 0xFFFF));

    return r;
}

/***********************************************************
 *	Compute the specular reflection. 
 *
 *	If the first layer is a turbid medium, use the Fresnel
 *	reflection from the boundary of the first layer as the 
 *	specular reflectance.
 *
 *	If the first layer is glass, multiple reflections in
 *	the first layer is considered to get the specular
 *	reflectance.
 *
 *	The subroutine assumes the Layerspecs array is correctly 
 *	initialized.
 ****/
fixedpt Rspecular(LayerStruct * Layerspecs_Ptr)
{
  fixedpt r1, r2;
	/* direct reflections from the 1st and 2nd layers. */
  fixedpt temp;
  
  temp = fixedpt_div(Layerspecs_Ptr[0].n - Layerspecs_Ptr[1].n, Layerspecs_Ptr[0].n + Layerspecs_Ptr[1].n); //(Layerspecs_Ptr[0].n - Layerspecs_Ptr[1].n)
	 //  /(Layerspecs_Ptr[0].n + Layerspecs_Ptr[1].n);
  r1 = fixedpt_mul(temp, temp); //temp*temp;
  
  if((Layerspecs_Ptr[1].mua == 0) 
  && (Layerspecs_Ptr[1].mus == 0))  { /* glass layer. */
    temp = fixedpt_div(Layerspecs_Ptr[1].n - Layerspecs_Ptr[2].n, Layerspecs_Ptr[1].n + Layerspecs_Ptr[2].n);
		       //(Layerspecs_Ptr[1].n - Layerspecs_Ptr[2].n)
      //	  /(Layerspecs_Ptr[1].n + Layerspecs_Ptr[2].n);
    r2 = fixedpt_mul(temp, temp); //temp*temp;
    r1 = r1 + fixedpt_div(fixedpt_mul(fixedpt_mul(FIXEDPT_ONE - r1, FIXEDPT_ONE - r1), r2), (FIXEDPT_ONE - fixedpt_mul(r1, r2)));
    //    r1 = r1 + (1-r1)*(1-r1)*r2/(1-r1*r2);
  }
  
  return (r1);	
}

/***********************************************************
 *	Initialize a photon packet.
 ****/
void LaunchPhoton(fixedpt Rspecular,
				  LayerStruct  * Layerspecs_Ptr,
				  PhotonStruct * Photon_Ptr)
{
  //  printf("LaunchPhoton\n");
  Photon_Ptr->w	 	= FIXEDPT_ONE - Rspecular; // 1.0 - Rspecular;	
  //  printf("Launch W: ");
  //  fixedpt_print(Photon_Ptr->w);
  //  printf("\n");

  Photon_Ptr->dead 	= 0;
  Photon_Ptr->layer = 1;
  Photon_Ptr->s	= 0;
  Photon_Ptr->sleft= 0;
  
  Photon_Ptr->x 	= 0;	
  Photon_Ptr->y	 	= 0;	
  Photon_Ptr->z	 	= 0;	
  Photon_Ptr->ux	= 0;	
  Photon_Ptr->uy	= 0;	
  Photon_Ptr->uz	= FIXEDPT_ONE; //1.0;	
  
  if((Layerspecs_Ptr[1].mua == 0) 
  && (Layerspecs_Ptr[1].mus == 0))  { /* glass layer. */
    Photon_Ptr->layer 	= 2;
    Photon_Ptr->z	= Layerspecs_Ptr[2].z0;	
  }
}

/***********************************************************
 *	Choose (sample) a new theta angle for photon propagation
 *	according to the anisotropy.
 *
 *	If anisotropy g is 0, then
 *		cos(theta) = 2*rand-1.
 *	otherwise
 *		sample according to the Henyey-Greenstein function.
 *
 *	Returns the cosine of the polar deflection angle theta.
 ****/
fixedpt SpinTheta(fixedpt g, PhotonStruct *Photon_Ptr)
{
  fixedpt cost;
  
  if(g == 0) 
    cost = fixedpt_mul(FIXEDPT_TWO, get_uniform_fixed(&(Photon_Ptr->seed))) - FIXEDPT_ONE; //-1;
  else {
    //    double temp = (1-g*g)/(1-g+2*g*RandomNum());
    fixedpt temp = fixedpt_div(FIXEDPT_ONE - fixedpt_mul(g,g), FIXEDPT_ONE - g + fixedpt_mul(g << 1, get_uniform_fixed(&(Photon_Ptr->seed)))); 
    //    cost = (1+g*g - temp*temp)/(2*g);
    cost = fixedpt_div(FIXEDPT_ONE + fixedpt_mul(g, g) - fixedpt_mul(temp, temp), g << 1);
	if(cost < -FIXEDPT_ONE) cost = -FIXEDPT_ONE;
	else if(cost > FIXEDPT_ONE) cost = FIXEDPT_ONE;
  }
  return(cost);
}


/***********************************************************
 *	Choose a new direction for photon propagation by 
 *	sampling the polar deflection angle theta and the 
 *	azimuthal angle psi.
 *
 *	Note:
 *  	theta: 0 - pi so sin(theta) is always positive 
 *  	feel free to use sqrt() for cos(theta).
 * 
 *  	psi:   0 - 2pi 
 *  	for 0-pi  sin(psi) is + 
 *  	for pi-2pi sin(psi) is - 
 ****/
void Spin(fixedpt g,
		  PhotonStruct * Photon_Ptr)
{

  const fixedpt cosTable[] = {65536, 65516, 65457, 65358, 65220, 65043, 64827, 64571, 64277, 63944, 63572, 63162, 62714, 62228, 61705, 61145, 60547, 59914, 59244, 58538, 57798, 57022, 56212, 55368, 54491, 53581, 52639, 51665, 50660, 49624, 48559, 47464, 46341, 45190, 44011, 42806, 41576, 40320, 39040, 37736, 36410, 35062, 33692, 32303, 30893, 29466, 28020, 26558, 25080, 23586, 22078, 20557, 19024, 17479, 15924, 14359, 12785, 11204, 9616, 8022, 6424, 4821, 3216, 1608, 0, -1608, -3216, -4821, -6424, -8022, -9616, -11204, -12785, -14359, -15924, -17479, -19024, -20557, -22078, -23586, -25080, -26558, -28020, -29466, -30893, -32303, -33692, -35062, -36410, -37736, -39040, -40320, -41576, -42806, -44011, -45190, -46341, -47464, -48559, -49624, -50660, -51665, -52639, -53581, -54491, -55368, -56212, -57022, -57798, -58538, -59244, -59914, -60547, -61145, -61705, -62228, -62714, -63162, -63572, -63944, -64277, -64571, -64827, -65043, -65220, -65358, -65457, -65516, -65536, -65516, -65457, -65358, -65220, -65043, -64827, -64571, -64277, -63944, -63572, -63162, -62714, -62228, -61705, -61145, -60547, -59914, -59244, -58538, -57798, -57022, -56212, -55368, -54491, -53581, -52639, -51665, -50660, -49624, -48559, -47464, -46341, -45190, -44011, -42806, -41576, -40320, -39040, -37736, -36410, -35062, -33692, -32303, -30893, -29466, -28020, -26558, -25080, -23586, -22078, -20557, -19024, -17479, -15924, -14359, -12785, -11204, -9616, -8022, -6424, -4821, -3216, -1608, 0, 1608, 3216, 4821, 6424, 8022, 9616, 11204, 12785, 14359, 15924, 17479, 19024, 20557, 22078, 23586, 25080, 26558, 28020, 29466, 30893, 32303, 33692, 35062, 36410, 37736, 39040, 40320, 41576, 42806, 44011, 45190, 46341, 47464, 48559, 49624, 50660, 51665, 52639, 53581, 54491, 55368, 56212, 57022, 57798, 58538, 59244, 59914, 60547, 61145, 61705, 62228, 62714, 63162, 63572, 63944, 64277, 64571, 64827, 65043, 65220, 65358, 65457, 65516, 65536};

  fixedpt cost, sint;	/* cosine and sine of the */
						/* polar deflection angle theta. */
  fixedpt cosp, sinp;	/* cosine and sine of the */
						/* azimuthal angle psi. */
  fixedpt ux = Photon_Ptr->ux;
  fixedpt uy = Photon_Ptr->uy;
  fixedpt uz = Photon_Ptr->uz;
  fixedpt psi;

  cost = SpinTheta(g, Photon_Ptr);
  sint = fixedpt_sqrt(FIXEDPT_ONE - fixedpt_mul(cost, cost)); //sqrt(1.0 - cost*cost);	
	/* sqrt() is faster than sin(). */

  fixedpt randnum = get_uniform_fixed(&(Photon_Ptr->seed)); //RandomNum();
  psi = fixedpt_mul(FIXEDPT_TWO_PI, randnum); //2.0*PI*RandomNum(); /* spin psi 0-2pi. */
  int index = (randnum & 0xFF00) >> 8;
  //  printf("%d\n", index);
  cosp = cosTable[index]; 
  //cosp = fixedpt_cos(psi); // cos(psi);

  if(psi<FIXEDPT_PI)
    sinp = fixedpt_sqrt(FIXEDPT_ONE - fixedpt_mul(cosp, cosp)); //sqrt(1.0 - cosp*cosp);	
	  /* sqrt() is faster than sin(). */
  else
    sinp = -fixedpt_sqrt(FIXEDPT_ONE - fixedpt_mul(cosp, cosp)); //- sqrt(1.0 - cosp*cosp);	
  
  if (fixedpt_abs(uz) > COSZERO) { //if(fabs(uz) > COSZERO)  { 	/* normal incident. */
    //    printf("NormalIncident\n");
    Photon_Ptr->ux = fixedpt_mul(sint, cosp); //    Photon_Ptr->ux = sint*cosp;
    Photon_Ptr->uy = fixedpt_mul(sint, sinp); //    Photon_Ptr->uy = sint*sinp;
    Photon_Ptr->uz = cost;
    if (uz < 0)
      Photon_Ptr->uz = -Photon_Ptr->uz;
//Photon_Ptr->uz = cost*SIGN(uz);	 JANDERS GO BACK HERE... WATCH IT!!!
	  /* SIGN() is faster than division. */
  }
  else  {		/* regular incident. */
    //    printf("RegularIncident\n");
    fixedpt temp = fixedpt_sqrt(FIXEDPT_ONE - fixedpt_mul(uz,uz)); //double temp = sqrt(1.0 - uz*uz);
    
    fixedpt firstClause = fixedpt_mul(ux, fixedpt_mul(uz, cosp)) - fixedpt_mul(uy, sinp);
    Photon_Ptr->ux = fixedpt_div(fixedpt_mul(sint, firstClause), temp) + fixedpt_mul(ux, cost);

      //sint*(ux*uz*cosp - uy*sinp)
      //			/temp + ux*cost;
    firstClause = fixedpt_mul(uy, fixedpt_mul(uz, cosp)) + fixedpt_mul(ux, sinp);
    
    Photon_Ptr->uy = fixedpt_div(fixedpt_mul(sint, firstClause), temp) + fixedpt_mul(uy, cost);

	//sint*(uy*uz*cosp + ux*sinp)
      //		/temp + uy*cost;
    Photon_Ptr->uz = fixedpt_mul(-sint, fixedpt_mul(cosp, temp)) + fixedpt_mul(uz, cost); //-sint*cosp*temp + uz*cost;
  }
#if 0
  printf("Spin: ");
  fixedpt_print(Photon_Ptr->ux);
  printf(" ");
  fixedpt_print(Photon_Ptr->uy);
  printf(" ");
  fixedpt_print(Photon_Ptr->uz);
#endif
}

/***********************************************************
 *	Move the photon s away in the current layer of medium.  
 ****/
void Hop(PhotonStruct *	Photon_Ptr)
{
  fixedpt s = Photon_Ptr->s; // double s = Photon_Ptr->s;

  Photon_Ptr->x += fixedpt_mul(s, Photon_Ptr->ux); //  Photon_Ptr->x += s*Photon_Ptr->ux;
  Photon_Ptr->y += fixedpt_mul(s, Photon_Ptr->uy); //s*Photon_Ptr->uy;
  Photon_Ptr->z += fixedpt_mul(s, Photon_Ptr->uz); //s*Photon_Ptr->uz;
}			

/***********************************************************
 *	If uz != 0, return the photon step size in glass, 
 *	Otherwise, return 0.
 *
 *	The step size is the distance between the current 
 *	position and the boundary in the photon direction.
 *
 *	Make sure uz !=0 before calling this function.
 ****/
void StepSizeInGlass(PhotonStruct *  Photon_Ptr,
					 InputStruct  *  In_Ptr)
{
  //  printf("StepSizeInGlass\n");
  fixedpt dl_b;	/* step size to boundary. */
  short  layer = Photon_Ptr->layer;
  fixedpt uz = Photon_Ptr->uz;
  
  /* Stepsize to the boundary. */	
  if(uz>0)
    dl_b = fixedpt_div(In_Ptr->layerspecs[layer].z1 - Photon_Ptr->z, uz); //(In_Ptr->layerspecs[layer].z1 - Photon_Ptr->z)
      //	   /uz;
  else if(uz<0)
    dl_b = fixedpt_div(In_Ptr->layerspecs[layer].z0 - Photon_Ptr->z, uz); //(In_Ptr->layerspecs[layer].z0 - Photon_Ptr->z)
      //	   /uz;
  else
    dl_b = 0;
  
  Photon_Ptr->s = dl_b;
}

/***********************************************************
 *	Pick a step size for a photon packet when it is in 
 *	tissue.
 *	If the member sleft is zero, make a new step size 
 *	with: -log(rnd)/(mua+mus).
 *	Otherwise, pick up the leftover in sleft.
 *
 *	Layer is the index to layer.
 *	In_Ptr is the input parameters.
 ****/
void StepSizeInTissue(PhotonStruct * Photon_Ptr,
					  InputStruct  * In_Ptr)
{
  //  printf("StepSizeInTissue\n");
  short  layer = Photon_Ptr->layer;
  fixedpt mua = In_Ptr->layerspecs[layer].mua;
  fixedpt mus = In_Ptr->layerspecs[layer].mus;
  
  if(Photon_Ptr->sleft == 0) {  /* make a new step. */
    fixedpt rnd;

    do rnd = get_uniform_fixed(&(Photon_Ptr->seed));
      while( rnd == 0 );    /* avoid zero. */
    Photon_Ptr->s = -fixedpt_div(fixedpt_ln(rnd), mua+mus); // -log(rnd)/(mua+mus);
    //    printf("LOG: ");
    //    fixedpt_print(fixedpt_ln(rnd));
    //    printf("%g\n", log((double)rnd/FIXEDPT_ONE));
    //Photon_Ptr->s = -fixedpt_div(fixedpt_rconst(log((double)rnd/FIXEDPT_ONE)), mua+mus);
  }
  else {	/* take the leftover. */
    Photon_Ptr->s = fixedpt_div(Photon_Ptr->sleft, mua+mus); //Photon_Ptr->sleft/(mua+mus);
    Photon_Ptr->sleft = 0;
  }
}

/***********************************************************
 *	Check if the step will hit the boundary.
 *	Return 1 if hit boundary.
 *	Return 0 otherwise.
 *
 * 	If the projected step hits the boundary, the members
 *	s and sleft of Photon_Ptr are updated.
 ****/
Boolean HitBoundary(PhotonStruct *  Photon_Ptr,
					InputStruct  *  In_Ptr)
{
  //  printf("HitBoundary\n");
  fixedpt dl_b;  /* length to boundary. */
  short  layer = Photon_Ptr->layer;
  fixedpt uz = Photon_Ptr->uz;
  Boolean hit;
  
  /* Distance to the boundary. */
  if(uz>0)
    dl_b = fixedpt_div(In_Ptr->layerspecs[layer].z1 - Photon_Ptr->z, uz); //(In_Ptr->layerspecs[layer].z1 
      //		- Photon_Ptr->z)/uz;	/* dl_b>0. */
  else if(uz<0)
    dl_b = fixedpt_div(In_Ptr->layerspecs[layer].z0 - Photon_Ptr->z, uz); //(In_Ptr->layerspecs[layer].z0 
      //		- Photon_Ptr->z)/uz;	/* dl_b>0. */
  
  if(uz != 0 && Photon_Ptr->s > dl_b) {
	  /* not horizontal & crossing. */
    fixedpt mut = In_Ptr->layerspecs[layer].mua 
				+ In_Ptr->layerspecs[layer].mus;

    Photon_Ptr->sleft = fixedpt_mul(Photon_Ptr->s - dl_b, mut); //(Photon_Ptr->s - dl_b)*mut;
    Photon_Ptr->s    = dl_b;
    hit = 1;
  }
  else
    hit = 0;
  
  return(hit);
}

/***********************************************************
 *	Drop photon weight inside the tissue (not glass).
 *
 *  The photon is assumed not dead. 
 *
 *	The weight drop is dw = w*mua/(mua+mus).
 *
 *	The dropped weight is assigned to the absorption array 
 *	elements.
 ****/
void Drop(InputStruct  *	In_Ptr, 
		  PhotonStruct *	Photon_Ptr,
	  fixedpt Results[50][40])
{
  fixedpt dwa;		/* absorbed weight.*/
  fixedpt x = Photon_Ptr->x;
  fixedpt y = Photon_Ptr->y;
  short  iz, ir;	/* index to z & r. */
  short  layer = Photon_Ptr->layer;
  fixedpt mua, mus;		

  //  printf("W: ");
  //  fixedpt_print(Photon_Ptr->w);
  //  printf("\n");

  /* compute array indices. */
  iz = fixedpt_div(Photon_Ptr->z, In_Ptr->dz) >> FIXEDPT_FBITS; //(short)(Photon_Ptr->z/In_Ptr->dz);
  if(iz>In_Ptr->nz-1) iz=In_Ptr->nz-1;
  
  ir = fixedpt_div(fixedpt_sqrt(fixedpt_mul(x,x) + fixedpt_mul(y,y)), In_Ptr->dr) >> FIXEDPT_FBITS; //(short)(sqrt(x*x+y*y)/In_Ptr->dr);
  if(ir>In_Ptr->nr-1) ir=In_Ptr->nr-1;
  
  /* update photon weight. */
  mua = In_Ptr->layerspecs[layer].mua;
  mus = In_Ptr->layerspecs[layer].mus;
  dwa = fixedpt_mul(Photon_Ptr->w, fixedpt_div(mua, mua+mus)); //  dwa = Photon_Ptr->w * mua/(mua+mus);
  Photon_Ptr->w -= dwa;
  
  /* assign dwa to the absorption array element. */
  Results[ir][iz]	+= dwa; // JAMES -- YOU SHOULD MUTEX THIS LINE IF THERE MAY BE CONCURRENT UPDATES
  //  printf("Drop: %d %d ", ir, iz);
  //  fixedpt_print(dwa);
  //  printf("\n");
}

/***********************************************************
 *	The photon weight is small, and the photon packet tries 
 *	to survive a roulette.
 ****/
void Roulette(PhotonStruct * Photon_Ptr)
{

  if(Photon_Ptr->w == 0)	
    Photon_Ptr->dead = 1;
  else if(get_uniform_fixed(&(Photon_Ptr->seed)) < CHANCE_FIXED) /* survived the roulette.*/
    Photon_Ptr->w = fixedpt_div(Photon_Ptr->w, CHANCE_FIXED); //    Photon_Ptr->w /= CHANCE;
  else 
    Photon_Ptr->dead = 1;
}

/***********************************************************
 *	Compute the Fresnel reflectance.
 *
 *	Make sure that the cosine of the incident angle a1
 *	is positive, and the case when the angle is greater 
 *	than the critical angle is ruled out.
 *
 * 	Avoid trigonometric function operations as much as
 *	possible, because they are computation-intensive.
 ****/
fixedpt RFresnel(fixedpt n1,	/* incident refractive index.*/
				fixedpt n2,	/* transmit refractive index.*/
				fixedpt ca1,	/* cosine of the incident */
							/* angle. 0<a1<90 degrees. */
				fixedpt * ca2_Ptr)  /* pointer to the */
							/* cosine of the transmission */
							/* angle. a2>0. */
{
  //  printf("RFresnel\n");
  fixedpt r;
  
  if(n1==n2) {			  	/** matched boundary. **/
    *ca2_Ptr = ca1;
    r = 0;
  }
  else if(ca1>COSZERO) {	/** normal incident. **/
    *ca2_Ptr = ca1;
    r = fixedpt_div(n2-n1, n2 + n1); //(n2-n1)/(n2+n1);
    r = fixedpt_mul(r,r); //r *= r;
  }
  else if(ca1<COS90D)  {	/** very slant. **/
    *ca2_Ptr = 0;
    r = FIXEDPT_ONE; //1.0;
  }
  else  {			  		/** general. **/
    fixedpt sa1, sa2;	
	  /* sine of the incident and transmission angles. */
    fixedpt ca2;
    
    sa1 = fixedpt_sqrt(FIXEDPT_ONE - fixedpt_mul(ca1, ca1)); //sqrt(1-ca1*ca1);
    sa2 = fixedpt_div(fixedpt_mul(n1,sa1), n2); // n1*sa1/n2;
    if(sa2>=FIXEDPT_ONE) {	
	  /* double check for total internal reflection. */
      *ca2_Ptr = 0;
      r = FIXEDPT_ONE; //= 1.0;
    }
    else  {
      fixedpt cap, cam;	/* cosines of the sum ap or */
						/* difference am of the two */
						/* angles. ap = a1+a2 */
						/* am = a1 - a2. */
      fixedpt sap, sam;	/* sines. */
      
      *ca2_Ptr = ca2 = fixedpt_sqrt(FIXEDPT_ONE - fixedpt_mul(sa2, sa2)); //sqrt(1-sa2*sa2);
      
      cap = fixedpt_mul(ca1, ca2) - fixedpt_mul(sa1, sa2); //      cap = ca1*ca2 - sa1*sa2; /* c+ = cc - ss. */
      cam = fixedpt_mul(ca1, ca2) + fixedpt_mul(sa1, sa2); //cam = ca1*ca2 + sa1*sa2; /* c- = cc + ss. */
      sap = fixedpt_mul(sa1, ca2) + fixedpt_mul(ca1, sa2); //sap = sa1*ca2 + ca1*sa2; /* s+ = sc + cs. */
      sam = fixedpt_mul(sa1, ca2) - fixedpt_mul(ca1, sa2); //sam = sa1*ca2 - ca1*sa2; /* s- = sc - cs. */

      fixedpt tempRHS = fixedpt_mul(cam, cam) + fixedpt_mul(cap, cap);
      tempRHS = fixedpt_div(tempRHS, fixedpt_mul(sap, fixedpt_mul(sap, fixedpt_mul(cam, cam))));
      r = fixedpt_mul(FIXEDPT_ONE_HALF, fixedpt_mul(sam, fixedpt_mul(sam, tempRHS)));
      //   r = 0.5*sam*sam*(cam*cam+cap*cap)/(sap*sap*cam*cam); 
		/* rearranged for speed. */
    }
  }
  return(r);
}

/***********************************************************
 *	Record the photon weight exiting the first layer(uz<0), 
 *	no matter whether the layer is glass or not, to the 
 *	reflection array.
 *
 *	Update the photon weight as well.
 ****/
void RecordR(fixedpt			Refl,	/* reflectance. */
			 InputStruct  *	In_Ptr,
			 PhotonStruct *	Photon_Ptr,
	     fixedpt Results[50][40])
{
  Photon_Ptr->w = fixedpt_mul(Photon_Ptr->w, Refl);
  //  Photon_Ptr->w *= Refl;
}

/***********************************************************
 *	Record the photon weight exiting the last layer(uz>0), 
 *	no matter whether the layer is glass or not, to the 
 *	transmittance array.
 *
 *	Update the photon weight as well.
 ****/
void RecordT(fixedpt 		Refl,
			 InputStruct  *	In_Ptr,
			 PhotonStruct *	Photon_Ptr,
	     fixedpt Results[50][40])
{
  Photon_Ptr->w = fixedpt_mul(Photon_Ptr->w, Refl); // Photon_Ptr->w *= Refl;
}

/***********************************************************
 *	Decide whether the photon will be transmitted or 
 *	reflected on the upper boundary (uz<0) of the current 
 *	layer.
 *
 *	If "layer" is the first layer, the photon packet will 
 *	be partially transmitted and partially reflected if 
 *	PARTIALREFLECTION is set to 1,
 *	or the photon packet will be either transmitted or 
 *	reflected determined statistically if PARTIALREFLECTION 
 *	is set to 0.
 *
 *	Record the transmitted photon weight as reflection.  
 *
 *	If the "layer" is not the first layer and the photon 
 *	packet is transmitted, move the photon to "layer-1".
 *
 *	Update the photon parmameters.
 ****/
void CrossUpOrNot(InputStruct  *	In_Ptr, 
				  PhotonStruct *	Photon_Ptr,
		  fixedpt Results[50][40])
{
  //  printf("CrossUpOrNot\n");
  fixedpt uz = Photon_Ptr->uz; /* z directional cosine. */
  fixedpt uz1;	/* cosines of transmission alpha. always */
				/* positive. */
  fixedpt r=0;	/* reflectance */
  short  layer = Photon_Ptr->layer;
  fixedpt ni = In_Ptr->layerspecs[layer].n;
  fixedpt nt = In_Ptr->layerspecs[layer-1].n;
  
  /* Get r. */
  if( - uz <= In_Ptr->layerspecs[layer].cos_crit0) 
    r = FIXEDPT_ONE; //    r=1.0;		      /* total internal reflection. */
  else r = RFresnel(ni, nt, -uz, &uz1);
  
  if(get_uniform_fixed(&(Photon_Ptr->seed)) > r) {		/* transmitted to layer-1. */
    if(layer==1)  {
      Photon_Ptr->uz = -uz1;
      RecordR(0, In_Ptr, Photon_Ptr, Results);
      Photon_Ptr->dead = 1;
    }
    else {
      Photon_Ptr->layer--;
      Photon_Ptr->ux = fixedpt_mul(Photon_Ptr->ux, fixedpt_div(ni, nt)); //      Photon_Ptr->ux *= ni/nt;
      Photon_Ptr->uy = fixedpt_mul(Photon_Ptr->uy, fixedpt_div(ni, nt)); //      Photon_Ptr->uy *= ni/nt;
      Photon_Ptr->uz = -uz1;
    }
  }
  else 						/* reflected. */
    Photon_Ptr->uz = -uz;
}

/***********************************************************
 *	Decide whether the photon will be transmitted  or be 
 *	reflected on the bottom boundary (uz>0) of the current 
 *	layer.
 *
 *	If the photon is transmitted, move the photon to 
 *	"layer+1". If "layer" is the last layer, record the 
 *	transmitted weight as transmittance. See comments for 
 *	CrossUpOrNot.
 *
 *	Update the photon parmameters.
 ****/
void CrossDnOrNot(InputStruct  *	In_Ptr, 
				  PhotonStruct *	Photon_Ptr,
		  fixedpt Results[50][40])
{
  //  printf("CrossDnOrNot\n");
  fixedpt uz = Photon_Ptr->uz; /* z directional cosine. */
  fixedpt uz1;	/* cosines of transmission alpha. */
  fixedpt r=0;	/* reflectance */
  short  layer = Photon_Ptr->layer;
  fixedpt ni = In_Ptr->layerspecs[layer].n;
  fixedpt nt = In_Ptr->layerspecs[layer+1].n;
  
  /* Get r. */
  if( uz <= In_Ptr->layerspecs[layer].cos_crit1) 
    r= FIXEDPT_ONE; //1.0;		/* total internal reflection. */
  else r = RFresnel(ni, nt, uz, &uz1);
  
  if(get_uniform_fixed(&(Photon_Ptr->seed)) > r) {		/* transmitted to layer+1. */
    if(layer == In_Ptr->num_layers) {
      Photon_Ptr->uz = uz1;
      RecordT(0, In_Ptr, Photon_Ptr, Results);
      Photon_Ptr->dead = 1;
    }
    else {
      Photon_Ptr->layer++;
      Photon_Ptr->ux = fixedpt_mul(Photon_Ptr->ux, fixedpt_div(ni, nt));  //      Photon_Ptr->ux *= ni/nt;
      Photon_Ptr->uy = fixedpt_mul(Photon_Ptr->uy, fixedpt_div(ni, nt));  //      Photon_Ptr->uy *= ni/nt;
      Photon_Ptr->uz = uz1;
    }
  }
  else 						/* reflected. */
    Photon_Ptr->uz = -uz;
}

/***********************************************************
 ****/
void CrossOrNot(InputStruct  *	In_Ptr, 
				PhotonStruct *	Photon_Ptr,
		fixedpt Results[50][40])
{
  if(Photon_Ptr->uz < 0)
    CrossUpOrNot(In_Ptr, Photon_Ptr, Results);
  else
    CrossDnOrNot(In_Ptr, Photon_Ptr, Results);
}

/***********************************************************
 *	Move the photon packet in glass layer.
 *	Horizontal photons are killed because they will
 *	never interact with tissue again.
 ****/
void HopInGlass(InputStruct  * In_Ptr,
				PhotonStruct * Photon_Ptr,
		fixedpt Results[50][40])
{
  //  double dl;     /* step size. 1/cm */
  
  if(Photon_Ptr->uz == 0) { 
	/* horizontal photon in glass is killed. */
    Photon_Ptr->dead = 1;
  }
  else {
    StepSizeInGlass(Photon_Ptr, In_Ptr);
    Hop(Photon_Ptr);
    CrossOrNot(In_Ptr, Photon_Ptr, Results);
  }
}

/***********************************************************
 *	Set a step size, move the photon, drop some weight, 
 *	choose a new photon direction for propagation.  
 *
 *	When a step size is long enough for the photon to 
 *	hit an interface, this step is divided into two steps. 
 *	First, move the photon to the boundary free of 
 *	absorption or scattering, then decide whether the 
 *	photon is reflected or transmitted.
 *	Then move the photon in the current or transmission 
 *	medium with the unfinished stepsize to interaction 
 *	site.  If the unfinished stepsize is still too long, 
 *	repeat the above process.  
 ****/
void HopDropSpinInTissue(InputStruct  *  In_Ptr,
						 PhotonStruct *  Photon_Ptr,
			 fixedpt Results[50][40])
{

  StepSizeInTissue(Photon_Ptr, In_Ptr);
  
  if(HitBoundary(Photon_Ptr, In_Ptr)) {
    Hop(Photon_Ptr);	/* move to boundary plane. */
    CrossOrNot(In_Ptr, Photon_Ptr, Results);
  }
  else {
    Hop(Photon_Ptr);
    Drop(In_Ptr, Photon_Ptr, Results);
    Spin(In_Ptr->layerspecs[Photon_Ptr->layer].g, 
		Photon_Ptr);
  }
}

/***********************************************************
 ****/
void HopDropSpin(InputStruct  *  In_Ptr,
				 PhotonStruct *  Photon_Ptr,
		 fixedpt Results[50][40])
{
  short layer = Photon_Ptr->layer;

  if((In_Ptr->layerspecs[layer].mua == 0) 
  && (In_Ptr->layerspecs[layer].mus == 0)) 
	/* glass layer. */
    HopInGlass(In_Ptr, Photon_Ptr, Results);
  else
    HopDropSpinInTissue(In_Ptr, Photon_Ptr, Results);
  
  if( Photon_Ptr->w < In_Ptr->Wth && !Photon_Ptr->dead) 
    Roulette(Photon_Ptr);
}
