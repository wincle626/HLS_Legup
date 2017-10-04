#include "legup_mem.h"

#include "black_scholes.h"
#include "gaussian.h"
#include "parser.h"
#include "random.h"
#include "timer.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define NUM_THREADS 2

// Begin timer.c
double
get_seconds ()
{
  struct timeval tv;
  double s;

  assert (0 == gettimeofday (&tv, NULL));
  return (double) tv.tv_sec + 1.0e-6 * ((double) tv.tv_usec);
}

void
init_timer ()
{
  /* 
   * "Prime" gettimeofday().  This is helpful on some systems, 
   * as the first call in a program may be inaccurate.
   */
  double t = get_seconds ();
  return;
}
// End timer.c

// Begin parser.c
double 
to_double (const char* s)
{
  double val;

  errno = 0;
  val = strtod (s, NULL);
  if (errno != 0)
    {
      fprintf (stderr, "*** Can\'t read string \'%s\' as a double; errno = %d ***\n", s, errno);
      errno = 0;
      exit (EXIT_FAILURE);
    }
  return val;  
}

int
to_int (const char* s)
{
  int val;

  errno = 0;
  val = (int) strtol (s, NULL, 10);
  if (errno != 0)
    {
      fprintf (stderr, "*** Can\'t read string \'%s\' as an int; errno = %d ***\n", s, errno);
      errno = 0;
      exit (EXIT_FAILURE);
    }
  return val;  
}

void
parse_parameters (double* S, 
		  double* E, 
		  double* r, 
		  double* sigma, 
		  double* T, 
		  int* M, 
		  const char* filename)
{
  char line[400];
  FILE* stream = NULL;

  stream = fopen (filename, "r");
  if (stream == NULL)
    {
      fprintf (stderr, "*** Failed to open parameters file \'%s\' ***\n", filename);
      exit (EXIT_FAILURE);
    }

  if (NULL == fgets (line, 400, stream))
    {
      fprintf (stderr, "*** Failed to read parameter S from parameters file \'%s\' ***\n", filename);
      exit (EXIT_FAILURE);
    }
  *S = to_double (line);

  if (NULL == fgets (line, 400, stream))
    {
      fprintf (stderr, "*** Failed to read parameter E from parameters file \'%s\' ***\n", filename);
      exit (EXIT_FAILURE);
    }
  *E = to_double (line);

  if (NULL == fgets (line, 400, stream))
    {
      fprintf (stderr, "*** Failed to read parameter r from parameters file \'%s\' ***\n", filename);
      exit (EXIT_FAILURE);
    }
  *r = to_double (line);

  if (NULL == fgets (line, 400, stream))
    {
      fprintf (stderr, "*** Failed to read parameter sigma from parameters file \'%s\' ***\n", filename);
      exit (EXIT_FAILURE);
    }
  *sigma = to_double (line);

  if (NULL == fgets (line, 400, stream))
    {
      fprintf (stderr, "*** Failed to read parameter T from parameters file \'%s\' ***\n", filename);
      exit (EXIT_FAILURE);
    }
  *T = to_double (line);

  if (NULL == fgets (line, 400, stream))
    {
      fprintf (stderr, "*** Failed to read parameter M from parameters file \'%s\' ***\n", filename);
      exit (EXIT_FAILURE);
    }
  *M = to_double (line);
}
// End parser.c

// Begin random.c

int wiki_rand(int *m_w, int *m_z)
{
  *m_z = 36969 * (*m_z & 65535) + (*m_z >> 16);
  *m_w = 18000 * (*m_w & 65535) + (*m_w >> 16);

  return (*m_z << 16) + *m_w;
}

double
uniform_random_double (int *w_ptr, int *z_ptr)
{
  double d;
  d = (double) wiki_rand (w_ptr, z_ptr) / (double) RAND_MAX;
  return d;
}
// End random.c

// Start gaussian.c
void
init_gaussrand_state (gaussrand_state_t* state)
{
  state->V1 = 0.0;
  state->V2 = 0.0;
  state->S = 0.0;
  state->phase = 0;
}


double 
gaussrand1 (int *w_ptr,
            int *z_ptr,
	    gaussrand_state_t* gaussrand_state)
{
  /*
   * Source: http://c-faq.com/lib/gaussian.html
   * Discussed in Knuth and due originally to Marsaglia.
   */

  const int phase = gaussrand_state->phase;
  double X;

  if (phase == 0) 
    {
      double V1, V2, S;

      do {
	const double U1 = uniform_random_double(w_ptr, z_ptr);
	const double U2 = uniform_random_double(w_ptr, z_ptr);

	V1 = 2 * U1 - 1;
	V2 = 2 * U2 - 1;
	S = V1 * V1 + V2 * V2;
      } while (S >= 1 || S == 0);
      /* 
       * Save (pack) the state.  Note that we never needed to unpack
       * it, because the above DO loop is guaranteed to run at least
       * once (so S, V1, and V2 will all be written). 
       */
      gaussrand_state->S = S;
      gaussrand_state->V1 = V1;
      gaussrand_state->V2 = V2;

      X = V1 * sqrt (-2.0 * log (S) / S);
    } 
  else /* phase == 1 */
    {
      /* Unpack the state */
      const double S = gaussrand_state->S;
      const double V2 = gaussrand_state->V2;

      X = V2 * sqrt (-2.0 * log (S) / S);
    }

  /* Modify and pack the "phase" state */
  gaussrand_state->phase = 1 - phase;
  return X;
}
// End gaussian.c

// Begin black_scholes.c
/**
 * This function is what you compute for each iteration of
 * Black-Scholes.  You don't have to understand it; just call it.
 * "gaussian_random_number" is the current random number (from a
 * Gaussian distribution, which in our case comes from gaussrand1()).
 */
static inline double 
black_scholes_value (const double S,
		     const double E,
		     const double r,
		     const double sigma,
		     const double T,
		     const double gaussian_random_number)
{
  const double current_value = S * exp ( (r - (sigma*sigma) / 2.0) * T + 
					 sigma * sqrt (T) * gaussian_random_number );
  return exp (-r * T) * 
    ((current_value - E < 0.0) ? 0.0 : current_value - E);
  /* return exp (-r * T) * max_double (current_value - E, 0.0); */
}


/**
 * Compute the standard deviation of trials[0 .. M-1].
 */
static double
black_scholes_stddev (void* the_args)
{
  black_scholes_args_t* args = (black_scholes_args_t*) the_args;
  const double mean = args->mean;
  const int M = args->M;
  double variance = 0.0;
  int k;

  for (k = 0; k < M; k++)
    {
      const double diff = args->trials[k] - mean;
      /*
       * Just like when computing the mean, we scale each term of this
       * sum in order to avoid overflow.
       */
      variance += diff * diff / (double) M;
    }

  args->variance = variance;
  return sqrt (variance);
}


double black_scholes_iterate (const int S,
                              const int E,
                              const int M,
                              const double r,
                              const double sigma,
                              const double T,
                              const int seed,
                              double *trials)
{
  double mean = 0.0;

  /* Temporary variables */
  double gaussian_random_number;
  int k;
  int seed1 = seed;
  int seed2 = seed * seed + 1;
  gaussrand_state_t gaussrand_state;

  /* Initialize the Gaussian random number module for this thread */
  init_gaussrand_state (&gaussrand_state);
  
  /* Do the Black-Scholes iterations */
  for (k = 0; k < M; k++)
    {
      gaussian_random_number = gaussrand1 (&seed1, &seed2,
							&gaussrand_state);
      trials[k] = black_scholes_value (S, E, r, sigma, T, 
				       gaussian_random_number);

      /*
       * We scale each term of the sum in order to avoid overflow. 
       * This ensures that mean is never larger than the max
       * element of trials[0 .. M-1].
       */
      mean += trials[k] / (double) M;
    }

  return mean;

  /* 
   * We do the standard deviation computation as a second operation.
   */
}

/**
 * Take a pointer to a black_scholes_args_t struct, and return NULL.
 * (The return value is irrelevant, because all the interesting
 * information is written to the input struct.)  This function runs
 * Black-Scholes iterations, and computes the local part of the mean.
 */
static void*
black_scholes_thread (void* the_args)
{
  black_scholes_args_t* args = (black_scholes_args_t*) the_args;

  /* Unpack the IN/OUT struct */

  /* IN (read-only) parameters */
  const int S = args->S;
  const int E = args->E;
  const int M = args->M;
  const double r = args->r;
  const double sigma = args->sigma;
  const double T = args->T;
  const int seed = args->seed;

  assert(seed != 0 && "invalid seed");

  /* OUT (write-only) parameters */
  double* trials = args->trials;

  args->mean = black_scholes_iterate(S, E, M, r, sigma, T, seed, trials);

  return NULL;
}

void
black_scholes (confidence_interval_t* interval,
	       const double S,
	       const double E,
	       const double r,
	       const double sigma,
	       const double T,
	       const int M)
{
  black_scholes_args_t args;
  double mean = 0.0;
  double stddev = 0.0;
  double conf_width = 0.0;
  double* trials = NULL;

  assert (M > 0);
  trials = (double*) malloc (M * sizeof (double));
  assert (trials != NULL);

  args.S = S;
  args.E = E;
  args.r = r;
  args.sigma = sigma;
  args.T = T;
  args.M = M;
  args.mean = 0.0;
  args.variance = 0.0;

  // pthread structures
  pthread_t pool[NUM_THREADS];
  black_scholes_args_t arg_array[NUM_THREADS];

  // Pre-allocate
  double* SHARED_MEM_trials = malloc_shared(M * sizeof(double), trials, LEGUP_RAM_LOCATION_ONCHIP);
  args.trials = SHARED_MEM_trials;

  assert(SHARED_MEM_trials && "Not enough shared memory");
  assert(M % NUM_THREADS == 0 && "M not divisible by number of threads");

  for (int i = 0; i < NUM_THREADS; i++) {
    int seed = i * i + 1; // can be changed to be non-deterministic
    arg_array[i] = (black_scholes_args_t){S, E, r, sigma, T, M / NUM_THREADS, seed, trials + i * M / NUM_THREADS, 0.0, 0.0};
    pthread_create(pool + i, NULL, black_scholes_thread, (void *)(arg_array + i));
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(pool[i], NULL);
    args.mean += arg_array[i].mean / NUM_THREADS;
  }

  // Post-copy and free
  memcpy_from_shared(trials, SHARED_MEM_trials, M * sizeof(double));
  free_shared(SHARED_MEM_trials);

  args.trials = trials;

  mean = args.mean;
  stddev = black_scholes_stddev (&args);

  conf_width = 1.96 * stddev / sqrt ((double) M);
  interval->min = mean - conf_width;
  interval->max = mean + conf_width;

  printf("Mean: %lf\nStandard deviation: %lf\n\n", mean, stddev);

  /* Clean up and exit */
    
  deinit_black_scholes_args (&args);
}


void
deinit_black_scholes_args (black_scholes_args_t* args)
{
  if (args != NULL)
    if (args->trials != NULL)
      {
	free (args->trials);
	args->trials = NULL;
      }
}
// End black_scholes.c

/**
 * Usage: ./argv[0] <filename>
 *
 * <filename> (don't include the angle brackets) is the name of 
 * a data file in the current directory containing the parameters
 * for the Black-Scholes simulation.  It has exactly six lines 
 * with no white space.  Put each parameter one to a line, with
 * an endline after it.  Here are the parameters:
 *
 * S
 * E
 * r
 * sigma
 * T
 * M
 */

int
main (int argc, char* argv[])
{
  confidence_interval_t interval;
  double S, E, r, sigma, T;
  int M = 0;
  char* filename = NULL;
  double t1, t2;
  
  if (argc < 2)
    {
      fprintf (stderr, 
	       "Usage: %s <filename>\n\n", argv[0]);
      exit (EXIT_FAILURE);
    }
  filename = argv[1];
  parse_parameters (&S, &E, &r, &sigma, &T, &M, filename);

  /* 
   * Make sure init_timer() is only called by one thread,
   * before all the other threads run!
   */
  init_timer ();

  /*
   * Run the benchmark and time it.
   */
  t1 = get_seconds ();
  black_scholes (&interval, S, E, r, sigma, T, M);
  t2 = get_seconds ();

  /*
   * A fun fact about C string literals (i.e., strings enclosed in
   * double quotes) is that the C preprocessor automatically
   * concatenates them if they are separated only by whitespace.
   */
  printf ("Black-Scholes benchmark:\n"
	  "------------------------\n"
	  "S        %g\n"
	  "E        %g\n"
	  "r        %g\n"
	  "sigma    %g\n"
	  "T        %g\n"
	  "M        %d\n",
	  S, E, r, sigma, T, M);
  printf ("Confidence interval: (%g, %g)\n", interval.min, interval.max);
  printf ("Total simulation time: %g seconds\n", t2 - t1);

  return 0;
}

