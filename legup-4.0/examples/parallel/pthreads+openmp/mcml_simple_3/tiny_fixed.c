
/**
 * Tiny MCML benchmark.  
 * "Simulates light propagation from a point source in an infinite medium with isotropic scattering."
 * Original source: http://omic.ogi.edu/software/mc
 */
#define SHELL_MAX  101
#define NUMPHOTONS 16
#define NUM_ACCEL 4
#define OMP_ACCEL 3
#define OPS_PER_ACCEL NUMPHOTONS/NUM_ACCEL

#include <pthread.h>
#include <stdio.h>
#include "fixedptc.c"
#include "tiny_fixed.h"

#define LEGUP
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

const fixedpt mu_a = 131072; // fixedpt_fromint(2);			   /* Absorption Coefficient in 1/cm !!non-zero!! */
const fixedpt mu_s = 1310720; //fixedpt_fromint(20);			   /* Reduced Scattering Coefficient in 1/cm */
const fixedpt microns_per_shell = 327680; //fixedpt_fromint(50); /* Thickness of spherical shells in microns */
//long   i;
const long photons = NUMPHOTONS;
const fixedpt albedo = 59578; 
const fixedpt shells_per_mfp = 595782; 
fixedpt heat[SHELL_MAX] = {0};

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

typedef struct 
{
  fixedpt x, y, z, u, v, w, weight;
  
} photon;

void processPhoton(int seed) {
  
  photon p;

  p.x = 0; p.y = 0; p.z = 0;					/*launch*/  
  p.u = 0; p.v = 0; p.w = FIXEDPT_ONE; 
  p.weight = FIXEDPT_ONE; 
  fixedpt t, xi1, xi2;
  long shell;

  for (;;) {
    fixedpt r0 = get_uniform_fixed(&seed) + 1; 
    t = -fixedpt_ln(r0);
    p.x += fixedpt_mul(t,p.u); //t * u;
    p.y += fixedpt_mul(t,p.v); //t * v;
    p.z += fixedpt_mul(t,p.w); //t * w;  
    
    fixedpt jay = fixedpt_mul(fixedpt_sqrt(fixedpt_mul(p.x,p.x) + fixedpt_mul(p.y,p.y)+ fixedpt_mul(p.z,p.z)), shells_per_mfp);
    shell = jay >> FIXEDPT_FBITS;
    //	    shell=(fixedpt_mul(fixedpt_sqrt(fixedpt_mul(x,x) + fixedpt_mul(y,y)+ fixedpt_mul(z,z)), shells_per_mfp)) >> FIXEDPT_FBITS; //sqrt(x*x+y*y+z*z)*shells_per_mfp;	/*absorb*/
    if (shell > SHELL_MAX-1) {
        shell = SHELL_MAX-1;
    }

	pthread_mutex_lock (&mutex1);
    heat[shell] += fixedpt_mul(FIXEDPT_ONE-albedo, p.weight); //(1.0-albedo)*weight;
 	pthread_mutex_unlock (&mutex1);

    p.weight = fixedpt_mul(p.weight, albedo);
    
    for(;;) {				
      fixedpt r1, r2;/*new direction*/
      r1 = get_uniform_fixed(&seed);
      r2 = get_uniform_fixed(&seed); 
      xi1 = (r1 << 1) - FIXEDPT_ONE;
      xi2 = (r2 << 1) - FIXEDPT_ONE;
      //if ((t=xi1*xi1+xi2*xi2)<=1) break;
      t = fixedpt_mul(xi1,xi1) + fixedpt_mul(xi2,xi2);
      if (t <= FIXEDPT_ONE) 
	break;
    }
    if (t == 0)
      t = 1;
    p.u = (t << 1) - FIXEDPT_ONE; //2.0 * t - 1.0;
    fixedpt temp = fixedpt_sqrt(fixedpt_div(FIXEDPT_ONE-fixedpt_mul(p.u,p.u),t));
    p.v = fixedpt_mul(xi1, temp); // xi1 * sqrt((1-u*u)/t);
    p.w = fixedpt_mul(xi2, temp); // xi2 * sqrt((1-u*u)/t);
    
    if (p.weight < 66){  // 66 = 0.001 in fixedpt 					/*roulette*/
	if (get_uniform_fixed(&seed) > 6554) break;  // 6554 = 0.1 in fixedpt
      p.weight = fixedpt_div(p.weight, 6554); // /= 0.1;
    }
  }  
}

void *process(void* threadarg) {
//void process(int* threadarg) {
  int* arg = (int*) threadarg;
  int offset = *arg;
  int i;
  #pragma omp parallel for num_threads(OMP_ACCEL) private(i)  
  for (i = 0; i < photons/NUM_ACCEL; i++)
  {
    //printf("i = %d, seed = %d\n", i, seeds[i+offset]);
    processPhoton(seeds[i+offset]);
  }	
  pthread_exit(NULL);
}

int main () 
{
    #ifdef LEGUP
    legup_start_counter(0);
    #endif
  int sum=0;
	pthread_t threads[NUM_ACCEL];
	int data[NUM_ACCEL];
    int i,j;
	for (i=0; i<NUM_ACCEL; i++) {
		data[i] = i*OPS_PER_ACCEL;
	}

	//launch threads
	//for (i=0; i<1; i++) {
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, process, (void *)&data[i]);
	}
	 
	//join the threads
	//for (i=0; i<1; i++) {
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], NULL);
	}

//  process(&data[0]);
//  process(&data[1]);
//  process(&data[2]);
//  process(&data[3]);
  for (i=0;i<SHELL_MAX-1;i++) {
    //    fixedpt_print(heat[i]);
    sum ^= heat[i]; // janders -- check correctness by XOR'ing all the values
  }
    #ifdef LEGUP
    int perf_counter = legup_stop_counter(0);
    printf("perf_counter = %d\n", perf_counter);
    #endif
  printf("Result: %d\n", sum);
  if (sum == 56060)
    printf("RESULT: PASS\n");
  else
    printf("RESULT: FAIL\n");
  return sum;
}
