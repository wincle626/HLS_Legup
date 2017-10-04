/*
 *  Copyright (C) 2003-2009  Anders Gavare.  All rights reserved.
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
 *  Generic console support functions.
 *
 *  This module is used by individual device drivers, for example serial
 *  controllers, keyboards, or other devices which need to attach to the
 *  emulator's real stdin/stdout.
 *
 *  The idea is that several input and output streams (console handles) are
 *  allowed. As long as the number of input streams is <= 1, then everything
 *  can be done in the emulator's default terminal window.
 *
 *  If the number of inputs is more than 1, it is necessary to open up slave
 *  xterm windows for each input. (Otherwise the behaviour is undefined; i.e.
 *  which of two emulated serial controllers would get keyboard input?)
 *
 *  (If the -x command line option is NOT used, then slaves are not opened up.
 *  Instead, a warning message is printed, and input is not allowed.)
 *
 *  Note that console handles that _allow_ input but are not yet used for
 *  output are not counted. This allows a machine to have, say, 2 serial ports
 *  which can be used for both input and output, and it will still be possible
 *  to run in the default terminal window as long as only one of those serial
 *  ports is actually used.
 *
 *  xterms are opened up "on demand", when output is sent to them.
 *
 *  The MAIN console handle (fixed as handle nr 0) is the one used by the
 *  default terminal window. A machine which registers a serial controller,
 *  which should be used as the main way of communicating with guest operating
 *  systems running on that machine, should set machine->main_console_handle
 *  to the handle of the correct port on that controller.
 *
 *
 *  NOTE: The code in this module is mostly non-reentrant.
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <time.h>

#include "console.h"
#include "emul.h"
#include "machine.h"
#include "settings.h"


extern char *progname;
extern int verbose;
extern struct settings *global_settings;


static struct termios console_oldtermios;
static struct termios console_curtermios;

/*  For 'slave' mode:  */
static struct termios console_slave_tios;
static int console_slave_outputd;

static int console_initialized = 0;
static struct settings *console_settings = NULL;
static int console_stdout_pending;

#define	CONSOLE_FIFO_LEN	4096

static int console_mouse_x;		/*  absolute x, 0-based  */
static int console_mouse_y;		/*  absolute y, 0-based  */
static int console_mouse_fb_nr;		/*  framebuffer number of
					    host movement, 0-based  */
static int console_mouse_buttons;	/*  left=4, middle=2, right=1  */

static int allow_slaves = 0;

struct console_handle {
	int		in_use;
	int		in_use_for_input;
	int		using_xterm;
	int		inputonly;
	int		outputonly;
	int		warning_printed;

	char		*machine_name;
	char		*name;

	int		w_descriptor;
	int		r_descriptor;

	unsigned char	fifo[CONSOLE_FIFO_LEN];
	int		fifo_head;
	int		fifo_tail;
};

#define	NOT_USING_XTERM				0
#define	USING_XTERM_BUT_NOT_YET_OPEN		1
#define	USING_XTERM				2

/*  A simple array of console_handles  */
static struct console_handle *console_handles = NULL;
static int n_console_handles = 0;


/*
 *  console_deinit_main():
 *
 *  Restore host's console settings.
 */
void console_deinit_main(void)
{
	if (!console_initialized)
		return;

	tcsetattr(STDIN_FILENO, TCSANOW, &console_oldtermios);

	console_initialized = 0;
}


/*
 *  console_sigcont():
 *
 *  If the user presses CTRL-Z (to stop the emulator process) and then
 *  continues, the termios settings might have been invalidated. This
 *  function restores them.
 *
 *  (This function should be set as the SIGCONT signal handler in src/emul.c.)
 */
void console_sigcont(int x)
{
	if (!console_initialized)
		return;

	/*  Make sure that the correct (current) termios setting is active:  */
	tcsetattr(STDIN_FILENO, TCSANOW, &console_curtermios);

	/*  Reset the signal handler:  */
	signal(SIGCONT, console_sigcont);
}


/*
 *  start_xterm():
 *
 *  When using X11 (well, when allow_slaves is set), this routine tries to
 *  start up an xterm, with another copy of gxemul inside. The other gxemul
 *  copy is given arguments that will cause it to run console_slave().
 *
 *  TODO: This is ugly and hardcoded. Clean it up.
 */
static void start_xterm(int handle)
{
	int filedes[2];
	int filedesB[2];
	int res;
	size_t mlen;
	char **a;
	pid_t p;

	res = pipe(filedes);
	if (res) {
		printf("[ start_xterm(): pipe(): %i ]\n", errno);
		exit(1);
	}

	res = pipe(filedesB);
	if (res) {
		printf("[ start_xterm(): pipe(): %i ]\n", errno);
		exit(1);
	}

	/*  printf("filedes = %i,%i\n", filedes[0], filedes[1]);  */
	/*  printf("filedesB = %i,%i\n", filedesB[0], filedesB[1]);  */

	/*  NOTE/warning: Hardcoded max nr of args!  */
	CHECK_ALLOCATION(a = (char **) malloc(sizeof(char *) * 20));

	a[0] = getenv("XTERM");
	if (a[0] == NULL)
		a[0] = strdup("xterm");
	a[1] = strdup("-geometry");
	a[2] = strdup("80x25");
	a[3] = strdup("-title");
	mlen = strlen(console_handles[handle].name) +
	    strlen(console_handles[handle].machine_name) + 30;
	CHECK_ALLOCATION(a[4] = (char *) malloc(mlen));
	snprintf(a[4], mlen, "GXemul: %s %s",
	    console_handles[handle].machine_name,
	    console_handles[handle].name);
	a[5] = strdup("-e");
	a[6] = progname;
	CHECK_ALLOCATION(a[7] = (char *) malloc(80));
	snprintf(a[7], 80, "-WW@S%i,%i", filedes[0], filedesB[1]);
	a[8] = NULL;

	p = fork();
	if (p == -1) {
		printf("[ start_xterm(): ERROR while trying to "
		    "fork(): %i ]\n", errno);
		exit(1);
	} else if (p == 0) {
		close(filedes[1]);
		close(filedesB[0]);

		p = setsid();
		if (p < 0)
			printf("[ start_xterm(): ERROR while trying "
			    "to do a setsid(): %i ]\n", errno);

		res = execvp(a[0], a);
		printf("[ start_xterm(): ERROR while trying to "
		    "execvp(\"");
		while (a[0] != NULL) {
			printf("%s", a[0]);
			if (a[1] != NULL)
				printf(" ");
			a++;
		}
		printf("\"): %i ]\n", errno);
		if (errno == ENOENT)
			printf("[ Most probably you don't have xterm"
			    " in your PATH. Try again. ]\n");
		exit(1);
	}

	/*  TODO: free a and a[*]  */

	close(filedes[0]);
	close(filedesB[1]);

	console_handles[handle].using_xterm = USING_XTERM;

	/*
	 *  write to filedes[1], read from filedesB[0]
	 */

	console_handles[handle].w_descriptor = filedes[1];
	console_handles[handle].r_descriptor = filedesB[0];
}


/*
 *  d_avail():
 *
 *  Returns 1 if anything is available on a descriptor.
 */
static int d_avail(int d)
{
	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(d, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	return select(d+1, &rfds, NULL, NULL, &tv);
}


/*
 *  console_makeavail():
 *
 *  Put a character in the queue, so that it will be avaiable,
 *  by inserting it into the char fifo.
 */
void console_makeavail(int handle, char ch)
{
	console_handles[handle].fifo[
	    console_handles[handle].fifo_head] = ch;
	console_handles[handle].fifo_head = (
	    console_handles[handle].fifo_head + 1) % CONSOLE_FIFO_LEN;

	if (console_handles[handle].fifo_head ==
	    console_handles[handle].fifo_tail)
		fatal("[ WARNING: console fifo overrun, handle %i ]\n", handle);
}


/*
 *  console_stdin_avail():
 *
 *  Returns 1 if a char is available from a handle's read descriptor,
 *  0 otherwise.
 */
static int console_stdin_avail(int handle)
{
	if (!console_handles[handle].in_use_for_input)
		return 0;

	if (!allow_slaves)
		return d_avail(STDIN_FILENO);

	if (console_handles[handle].using_xterm ==
	    USING_XTERM_BUT_NOT_YET_OPEN)
		return 0;

	return d_avail(console_handles[handle].r_descriptor);
}


/*
 *  console_charavail():
 *
 *  Returns 1 if a char is available in the fifo, 0 otherwise.
 */
int console_charavail(int handle)
{
	while (console_stdin_avail(handle)) {
		unsigned char ch[100];		/* = getchar(); */
		ssize_t len;
		int i, d;

		if (!allow_slaves)
			d = STDIN_FILENO;
		else
			d = console_handles[handle].r_descriptor;

		len = read(d, ch, sizeof(ch));

		for (i=0; i<len; i++) {
			/*  printf("[ %i: %i ]\n", i, ch[i]);  */

			if (!allow_slaves) {
				/*  Ugly hack: convert ctrl-b into ctrl-c.
				    (TODO: fix)  */
				if (ch[i] == 2)
					ch[i] = 3;
			}

			console_makeavail(handle, ch[i]);
		}
	}

	if (console_handles[handle].fifo_head ==
	    console_handles[handle].fifo_tail)
		return 0;

	return 1;
}


/*
 *  console_readchar():
 *
 *  Returns 0..255 if a char was available, -1 otherwise.
 */
int console_readchar(int handle)
{
	int ch;

	if (!console_charavail(handle))
		return -1;

	ch = console_handles[handle].fifo[console_handles[handle].fifo_tail];
	console_handles[handle].fifo_tail ++;
	console_handles[handle].fifo_tail %= CONSOLE_FIFO_LEN;

	return ch;
}


/*
 *  console_putchar():
 *
 *  Prints a char to stdout, and sets the console_stdout_pending flag.
 */
void console_putchar(int handle, int ch)
{
	char buf[1];

	if (!console_handles[handle].in_use_for_input &&
	    !console_handles[handle].outputonly)
		console_change_inputability(handle, 1);

	if (!allow_slaves) {
		/*  stdout:  */
		putchar(ch);

		/*  Assume flushes by OS or libc on newlines:  */
		if (ch == '\n')
			console_stdout_pending = 0;
		else
			console_stdout_pending = 1;

		return;
	}

	if (!console_handles[handle].in_use) {
		printf("[ console_putchar(): handle %i not in"
		    " use! ]\n", handle);
		return;
		}

	if (console_handles[handle].using_xterm ==
	    USING_XTERM_BUT_NOT_YET_OPEN)
		start_xterm(handle);

	buf[0] = ch;
	if (write(console_handles[handle].w_descriptor, buf, 1) != 1)
		perror("error writing to console handle");
}


/*
 *  console_flush():
 *
 *  Flushes stdout, if necessary, and resets console_stdout_pending to zero.
 */
void console_flush(void)
{
	if (console_stdout_pending)
		fflush(stdout);

	console_stdout_pending = 0;
}


/*
 *  console_mouse_coordinates():
 *
 *  Sets mouse coordinates. Called by for example an X11 event handler.
 *  x and y are absolute coordinates, fb_nr is where the mouse movement
 *  took place.
 */
void console_mouse_coordinates(int x, int y, int fb_nr)
{
	/*  TODO: fb_nr isn't used yet.  */

	console_mouse_x = x;
	console_mouse_y = y;
	console_mouse_fb_nr = fb_nr;
}


/*
 *  console_mouse_button():
 *
 *  Sets a mouse button to be pressed or released. Called by for example an
 *  X11 event handler.  button is 1 (left), 2 (middle), or 3 (right), and
 *  pressed = 1 for pressed, 0 for not pressed.
 */
void console_mouse_button(int button, int pressed)
{
	int mask = 1 << (3-button);

	if (pressed)
		console_mouse_buttons |= mask;
	else
		console_mouse_buttons &= ~mask;
}


/*
 *  console_getmouse():
 *
 *  Puts current mouse data into the variables pointed to by
 *  the arguments.
 */
void console_getmouse(int *x, int *y, int *buttons, int *fb_nr)
{
	*x = console_mouse_x;
	*y = console_mouse_y;
	*buttons = console_mouse_buttons;
	*fb_nr = console_mouse_fb_nr;
}


/*
 *  console_slave_sigint():
 */
static void console_slave_sigint(int x)
{
	char buf[1];

	/*  Send a ctrl-c:  */
	buf[0] = 3;
	if (write(console_slave_outputd, buf, sizeof(buf)) != sizeof(buf))
		perror("error writing to console handle");

	/*  Reset the signal handler:  */
	signal(SIGINT, console_slave_sigint);
}


/*
 *  console_slave_sigcont():
 *
 *  See comment for console_sigcont. This is for used by console_slave().
 */
static void console_slave_sigcont(int x)
{
	/*  Make sure our 'current' termios setting is active:  */
	tcsetattr(STDIN_FILENO, TCSANOW, &console_slave_tios);

	/*  Reset the signal handler:  */
	signal(SIGCONT, console_slave_sigcont);
}


/*
 *  console_slave():
 *
 *  This function is used when running with X11, and gxemul opens up
 *  separate xterms for each emulated terminal or serial port.
 */
void console_slave(const char *arg)
{
	int inputd;
	int len;
	const char *p;
	char buf[16384];

	/*  arg = '3,6' or similar, input and output descriptors  */
	/*  printf("console_slave(): arg = '%s'\n", arg);  */

	inputd = atoi(arg);
	p = strchr(arg, ',');
	if (p == NULL) {
		printf("console_slave(): bad arg '%s'\n", arg);
		sleep(5);
		exit(1);
	}
	console_slave_outputd = atoi(p+1);

	/*  Set the terminal to raw mode:  */
	tcgetattr(STDIN_FILENO, &console_slave_tios);

	console_slave_tios.c_lflag &= ~ICANON;
	console_slave_tios.c_cc[VTIME] = 0;
	console_slave_tios.c_cc[VMIN] = 1;
	console_slave_tios.c_lflag &= ~ECHO;
	console_slave_tios.c_iflag &= ~ICRNL;
	tcsetattr(STDIN_FILENO, TCSANOW, &console_slave_tios);

	signal(SIGINT, console_slave_sigint);
	signal(SIGCONT, console_slave_sigcont);

	for (;;) {
		/*  TODO: select() on both inputd and stdin  */

		if (d_avail(inputd)) {
			len = read(inputd, buf, sizeof(buf) - 1);
			if (len < 1)
				exit(0);
			buf[len] = '\0';
			printf("%s", buf);
			fflush(stdout);
		}

		if (d_avail(STDIN_FILENO)) {
			len = read(STDIN_FILENO, buf, sizeof(buf));
			if (len < 1)
				exit(0);
			if (write(console_slave_outputd, buf, len) != len)
				perror("error writing to console handle");
		}

		usleep(10000);
	}
}


/*
 *  console_new_handle():
 *
 *  Allocates a new console_handle struct, and returns a pointer to it.
 *
 *  For internal use.
 */
static struct console_handle *console_new_handle(const char *name, int *handlep)
{
	struct console_handle *chp;
	int i, n, found_free = -1;

	/*  Reuse an old slot, if possible:  */
	n = n_console_handles;
	for (i=0; i<n; i++)
		if (!console_handles[i].in_use) {
			found_free = i;
			break;
		}

	if (found_free == -1) {
		/*  Let's realloc console_handles[], to make room
		    for the new one:  */
		CHECK_ALLOCATION(console_handles = (struct console_handle *)
		    realloc(console_handles, sizeof(
		    struct console_handle) * (n_console_handles + 1)));
		found_free = n_console_handles;
		n_console_handles ++;
	}

	chp = &console_handles[found_free];
	memset(chp, 0, sizeof(struct console_handle));

	chp->in_use = 1;
	chp->machine_name = strdup("");
	CHECK_ALLOCATION(chp->name = strdup(name));

	*handlep = found_free;
	return chp;
}


/*
 *  console_start_slave():
 *
 *  When using X11:
 *
 *  This routine tries to start up an xterm, with another copy of gxemul
 *  inside. The other gxemul copy is given arguments that will cause it
 *  to run console_slave().
 *
 *  When not using X11:  Things will seem to work the same way without X11,
 *  but no xterm will actually be started.
 *
 *  consolename should be something like "serial 0".
 *
 *  If use_for_input is 1, input is allowed right from the start. (This
 *  can be upgraded later from 0 to 1 using the console_change_inputability()
 *  function.)
 *
 *  If use_for_input is CONSOLE_OUTPUT_ONLY, then this is an output-only stream.
 *
 *  On success, an integer >= 0 is returned. This can then be used as a
 *  'handle' when writing to or reading from an emulated console.
 *
 *  On failure, -1 is returned.
 */
int console_start_slave(struct machine *machine, const char *consolename,
	int use_for_input)
{
	struct console_handle *chp;
	int handle;

	if (machine == NULL || consolename == NULL) {
		printf("console_start_slave(): NULL ptr\n");
		exit(1);
	}

	chp = console_new_handle(consolename, &handle);
	chp->in_use_for_input = use_for_input;
	if (use_for_input == CONSOLE_OUTPUT_ONLY) {
		chp->outputonly = 1;
		chp->in_use_for_input = 0;
	}

	if (machine->machine_name != NULL) {
		CHECK_ALLOCATION(chp->machine_name =
		    strdup(machine->machine_name));
	} else {
		CHECK_ALLOCATION(chp->machine_name = strdup(""));
	}

	CHECK_ALLOCATION(chp->name = strdup(consolename));

	if (allow_slaves)
		chp->using_xterm = USING_XTERM_BUT_NOT_YET_OPEN;

	return handle;
}


/*
 *  console_start_slave_inputonly():
 *
 *  Similar to console_start_slave(), but doesn't open an xterm. This is
 *  useful for devices such as keyboard controllers, that need to have an
 *  input queue, but no xterm window associated with it.
 *
 *  On success, an integer >= 0 is returned. This can then be used as a
 *  'handle' when writing to or reading from an emulated console.
 *
 *  On failure, -1 is returned.
 */
int console_start_slave_inputonly(struct machine *machine, const char *consolename,
	int use_for_input)
{
	struct console_handle *chp;
	int handle;

	if (machine == NULL || consolename == NULL) {
		printf("console_start_slave(): NULL ptr\n");
		exit(1);
	}

	chp = console_new_handle(consolename, &handle);
	chp->inputonly = 1;
	chp->in_use_for_input = use_for_input;

	if (machine->name != NULL) {
		CHECK_ALLOCATION(chp->machine_name = strdup(machine->name));
	} else {
		CHECK_ALLOCATION(chp->machine_name = strdup(""));
	}

	CHECK_ALLOCATION(chp->name = strdup(consolename));

	return handle;
}


/*
 *  console_change_inputability():
 *
 *  Sets whether or not a console handle can be used for input. Return value
 *  is 1 if the change took place, 0 otherwise.
 */
int console_change_inputability(int handle, int inputability)
{
	int old;

	if (handle < 0 || handle >= n_console_handles) {
		fatal("console_change_inputability(): bad handle %i\n",
		    handle);
		exit(1);
	}

	old = console_handles[handle].in_use_for_input;
	console_handles[handle].in_use_for_input = inputability;

	if (inputability != 0) {
		if (console_warn_if_slaves_are_needed(0)) {
			console_handles[handle].in_use_for_input = old;
			if (!console_handles[handle].warning_printed) {
				fatal("%%\n%%  WARNING! Input to console ha"
				    "ndle \"%s\" wasn't enabled,\n%%  because "
				    "it", console_handles[handle].name);
				fatal(" would interfere with other inputs,\n"
				    "%%  and you did not use the -x command "
				    "line option!\n%%\n");
			}
			console_handles[handle].warning_printed = 1;
			return 0;
		}
	}

	return 1;
}


/*
 *  console_init_main():
 *
 *  Puts the host's console into single-character (non-canonical) mode.
 */
void console_init_main(struct emul *emul)
{
	int i, tra;

	if (console_initialized)
		return;

	tcgetattr(STDIN_FILENO, &console_oldtermios);
	memcpy(&console_curtermios, &console_oldtermios,
	    sizeof (struct termios));

	console_curtermios.c_lflag &= ~ICANON;
	console_curtermios.c_cc[VTIME] = 0;
	console_curtermios.c_cc[VMIN] = 1;

	console_curtermios.c_lflag &= ~ECHO;

	/*
	 *  Most guest OSes seem to work ok without ~ICRNL, but Linux on
	 *  DECstation requires it to be usable.  Unfortunately, clearing
	 *  out ICRNL makes tracing with '-t ... |more' akward, as you
	 *  might need to use CTRL-J instead of the enter key.  Hence,
	 *  this bit is only cleared if we're not tracing:
	 */
	tra = 0;
	for (i=0; i<emul->n_machines; i++)
		if (emul->machines[i]->show_trace_tree ||
		    emul->machines[i]->instruction_trace ||
		    emul->machines[i]->register_dump)
			tra = 1;
	if (!tra)
		console_curtermios.c_iflag &= ~ICRNL;

	tcsetattr(STDIN_FILENO, TCSANOW, &console_curtermios);

	console_stdout_pending = 1;
	console_handles[MAIN_CONSOLE].fifo_head = 0;
	console_handles[MAIN_CONSOLE].fifo_tail = 0;

	console_mouse_x = 0;
	console_mouse_y = 0;
	console_mouse_buttons = 0;

	console_initialized = 1;
}


/*
 *  console_debug_dump():
 *
 *  Dump debug info, if verbose >= 2.
 */
void console_debug_dump(struct machine *machine)
{
	int i, iadd = DEBUG_INDENTATION, listed_main = 0;

	if (verbose < 2)
		return;

	debug("console slaves (xterms): %s\n", allow_slaves?
	    "yes" : "no");

	debug("console handles:\n");
	debug_indentation(iadd);

	for (i=0; i<n_console_handles; i++) {
		if (!console_handles[i].in_use)
			continue;
		debug("%i: \"%s\"", i, console_handles[i].name);
		if (console_handles[i].using_xterm)
			debug(" [xterm]");
		if (console_handles[i].inputonly)
			debug(" [inputonly]");
		if (console_handles[i].outputonly)
			debug(" [outputonly]");
		if (i == machine->main_console_handle) {
			debug(" [MAIN CONSOLE]");
			listed_main = 1;
		}
		debug("\n");
	}

	debug_indentation(-iadd);

	if (!listed_main)
		fatal("WARNING! no main console handle?\n");
}


/*
 *  console_allow_slaves():
 *
 *  This function tells the console subsystem whether or not to open up
 *  slave xterms for each emulated serial controller.
 */
void console_allow_slaves(int allow)
{
	allow_slaves = allow;
}


/*
 *  console_are_slaves_allowed():
 *
 *  Returns the value of allow_slaves.
 */
int console_are_slaves_allowed(void)
{
	return allow_slaves;
}


/*
 *  console_warn_if_slaves_are_needed():
 *
 *  Prints an error (during startup of the emulator) if slave xterms are needed
 *  (i.e. there is more than one console handle in use which is used for
 *  INPUT), but they are not currently allowed.
 *
 *  This function should be called during startup (with init = 1), and every
 *  time a console handle changes/upgrades its in_use_for_input from 0 to 1.
 *
 *  If init is non-zero, this function doesn't return if there was a warning.
 *
 *  If init is zero, no warning is printed. 1 is returned if there were more
 *  than one input, 0 otherwise.
 */
int console_warn_if_slaves_are_needed(int init)
{
	int i, n = 0;

	if (allow_slaves)
		return 0;

	for (i=MAIN_CONSOLE+1; i<n_console_handles; i++)
		if (console_handles[i].in_use &&
		    console_handles[i].in_use_for_input &&
		    !console_handles[i].using_xterm)
			n ++;

	if (n > 1) {
		if (init) {
			fatal("#\n#  ERROR! More than one console input is "
			    "in use,\n#  but xterm slaves are not enabled.\n"
			    "#\n");
			fatal("#  Use -x to enable slave xterms.)\n#\n");
			for (i=MAIN_CONSOLE+1; i<n_console_handles; i++)
				if (console_handles[i].in_use &&
				    console_handles[i].in_use_for_input &&
				    !console_handles[i].using_xterm)
					fatal("#  console handle %i: '%s'\n",
					    i, console_handles[i].name);
			fatal("#\n");
			exit(1);
		}
		return 1;
	}

	return 0;
}


/*
 *  console_init():
 *
 *  This function should be called before any other console_*() function
 *  is used.
 */
void console_init(void)
{
	int handle;
	struct console_handle *chp;

	console_settings = settings_new();

	settings_add(global_settings, "console", 1,
            SETTINGS_TYPE_SUBSETTINGS, 0, console_settings);

	settings_add(console_settings, "allow_slaves", 0,
            SETTINGS_TYPE_INT, SETTINGS_FORMAT_YESNO, (void *)&allow_slaves);

	chp = console_new_handle("MAIN", &handle);
	if (handle != MAIN_CONSOLE) {
		printf("console_init(): fatal error: could not create"
		    " console 0: handle = %i\n", handle);
		exit(1);
	}

	chp->in_use_for_input = 1;
}


/*
 *  console_deinit():
 *
 *  Unregister settings registered by console_init().
 */
void console_deinit(void)
{
	settings_remove(console_settings, "allow_slaves");
	settings_remove(global_settings, "console");
}

