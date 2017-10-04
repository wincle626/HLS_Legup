#ifndef _timer_h
#define _timer_h

/**
 * Call this in the master thread before you create any other threads,
 * and only call it once!  It initializes the timer system.
 */
void
init_timer ();

/**
 * Return the number of seconds (as a double-precision floating-point
 * number) since some time in the past.  Use it to compute time
 * intervals.  You can expect ~ microsecond (1.0e-6) timer resolution.
 * Any event that lasts 10 microseconds, according to this timer, has
 * at least 10% uncertainty in the timing.  Any event that lasts 1
 * microsecond, according to this timer, has 100% uncertainty in the
 * timing!  You may need to repeat events if the uncertainty is large.
 * 
 * @note You should ALWAYS report timer resolution!!!
 */
double
get_seconds ();

#endif /* _timer_h */
