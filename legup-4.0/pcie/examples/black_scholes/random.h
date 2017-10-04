#ifndef _random_h
#define _random_h

#include <inttypes.h>

// From http://en.wikipedia.org/wiki/Random_number_generation#Computational_methods
int
wiki_rand (int *, int *);

/**
 * @param prng_stream [IN/OUT] Pointer to a "thread-local PRNG stream"
 *        object which was created by spawn_prng_stream().  
 *
 * @return Random double, uniformly distributed in the range [0,1).
 *
 * @note No mutual exclusion is done to protect the stream,
 *       so it should only used be called by one thread.  
 */
double
uniform_random_double (int *, int *);

#endif /* _random_h */
