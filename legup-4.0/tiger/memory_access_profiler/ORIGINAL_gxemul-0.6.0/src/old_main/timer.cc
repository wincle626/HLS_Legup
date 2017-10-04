/*
 *  Copyright (C) 2006-2009  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 *
 *  Timer framework. This is used by emulated clocks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "misc.h"
#include "timer.h"


/*  #define TEST  */


struct timer {
	struct timer	*next;

	double		freq;
	void		(*timer_tick)(struct timer *timer, void *extra);
	void		*extra;

	double		interval;
	double		next_tick_at;
};

static struct timer *first_timer = NULL;
struct timeval timer_start_tv;
static double timer_freq;
static int timer_countdown_to_next_gettimeofday;
static double timer_current_time;
static double timer_current_time_step;

static int timer_is_running;

#define	SECONDS_BETWEEN_GETTIMEOFDAY_SYNCH	1.65


/*
 *  timer_add():
 *
 *  Adds a virtual timer to the list of timers.
 *
 *  Return value is a pointer to a timer struct.
 */
struct timer *timer_add(double freq, void (*timer_tick)(struct timer *timer,
	void *extra), void *extra)
{
	struct timer *newtimer;

	CHECK_ALLOCATION(newtimer = (struct timer *) malloc(sizeof(struct timer)));

	if (freq <= 0.00000001)
		freq = 0.00000001;

	newtimer->freq = freq;
	newtimer->timer_tick = timer_tick;
	newtimer->extra = extra;

	newtimer->interval = 1.0 / freq;
	newtimer->next_tick_at = timer_current_time + newtimer->interval;

	newtimer->next = first_timer;
	first_timer = newtimer;

	return newtimer;
}


/*
 *  timer_remove():
 *
 *  Removes a virtual timer from the list of timers.
 */
void timer_remove(struct timer *t)
{
	struct timer *prev = NULL, *cur = first_timer;

	while (cur != NULL && cur != t) {
		prev = cur;
		cur = cur->next;
	}

	if (cur == t) {
		if (prev == NULL)
			first_timer = cur->next;
		else
			prev->next = cur->next;
		free(cur);
	} else {
		fprintf(stderr, "attempt to remove timer %p which "
		    "doesn't exist. aborting\n", t);
		exit(1);
	}
}


/*
 *  timer_update_frequency():
 *
 *  Changes the frequency of an existing timer.
 */
void timer_update_frequency(struct timer *t, double new_freq)
{
	if (t->freq == new_freq)
		return;

	t->freq = new_freq;

	if (new_freq <= 0.00000001)
		new_freq = 0.00000001;

	t->interval = 1.0 / new_freq;
	t->next_tick_at = timer_current_time + t->interval;
}


/*
 *  timer_tick():
 *
 *  Timer tick handler. This is where the interesting stuff happens.
 */
static void timer_tick(int signal_nr)
{
	struct timer *timer = first_timer;
	struct timeval tv;

	timer_current_time += timer_current_time_step;

	if ((--timer_countdown_to_next_gettimeofday) < 0) {
		gettimeofday(&tv, NULL);
		tv.tv_sec -= timer_start_tv.tv_sec;
		tv.tv_usec -= timer_start_tv.tv_usec;
		if (tv.tv_usec < 0) {
			tv.tv_usec += 1000000;
			tv.tv_sec --;
		}

#ifdef TIMER_DEBUG
		/*  For debugging/testing:  */
		{
			double diff = tv.tv_usec * 0.000001 + tv.tv_sec
			    - timer_current_time;
			printf("timer: lagging behind %f seconds\n", diff);
		}
#endif

		/*  Get exponentially closer to the real time, instead of
		    just changing to it directly:  */
		timer_current_time = ( (tv.tv_usec * 0.000001 + tv.tv_sec) +
		    timer_current_time ) / 2;

		timer_countdown_to_next_gettimeofday = (int64_t) (timer_freq *
		    SECONDS_BETWEEN_GETTIMEOFDAY_SYNCH);
	}

	while (timer != NULL) {
		while (timer_current_time >= timer->next_tick_at) {
			timer->timer_tick(timer, timer->extra);
			timer->next_tick_at += timer->interval;
		}

		timer = timer->next;
	}

#ifdef TEST
	printf("T"); fflush(stdout);
#endif
}


/*
 *  timer_start():
 *
 *  Set the interval timer to timer_freq Hz, and install the signal handler.
 */
void timer_start(void)
{
	struct timer *timer = first_timer;
	struct itimerval val;
	struct sigaction saction;

	if (timer_is_running)
		return;

	timer_is_running = 1;

	gettimeofday(&timer_start_tv, NULL);
	timer_current_time = 0.0;

	/*  Reset all timers:  */
	while (timer != NULL) {
		timer->next_tick_at = timer->interval;
		timer = timer->next;
	}
	val.it_interval.tv_sec = 0;
	val.it_interval.tv_usec = (int) (1000000.0 / timer_freq);
	val.it_value.tv_sec = 0;
	val.it_value.tv_usec = (int) (1000000.0 / timer_freq);

	memset(&saction, 0, sizeof(saction));
	saction.sa_handler = timer_tick;

	sigaction(SIGALRM, &saction, NULL);

	setitimer(ITIMER_REAL, &val, NULL);
}


/*
 *  timer_stop():
 *
 *  Deinstall the signal handler, and disable the interval timer.
 */
void timer_stop(void)
{
	struct itimerval val;
	struct sigaction saction;

	if (!timer_is_running)
		return;

	timer_is_running = 0;

	val.it_interval.tv_sec = 0;
	val.it_interval.tv_usec = 0;
	val.it_value.tv_sec = 0;
	val.it_value.tv_usec = 0;

	setitimer(ITIMER_REAL, &val, NULL);

	memset(&saction, 0, sizeof(saction));
	saction.sa_handler = NULL;

	sigaction(SIGALRM, &saction, NULL);
}


#ifdef TEST
static void timer_tick_test(struct timer *t, void *extra)
{
	printf((char *) extra); fflush(stdout);
}
#endif


/*
 *  timer_init():
 *
 *  Initialize the timer framework.
 */
void timer_init(void)
{
	first_timer = NULL;
	timer_current_time = 0.0;
	timer_is_running = 0;
	timer_countdown_to_next_gettimeofday = 0;

	timer_freq = TIMER_BASE_FREQUENCY;
	timer_current_time_step = 1.0 / timer_freq;

#ifdef TEST
	timer_add(0.5, timer_tick_test, "X");
	timer_add(10.0, timer_tick_test, ".");
	timer_add(200.0, timer_tick_test, " ");
	timer_start();
	while (1)
		sleep(999999);
#endif
}

