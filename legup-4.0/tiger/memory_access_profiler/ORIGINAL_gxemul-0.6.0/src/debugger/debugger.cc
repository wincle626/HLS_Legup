/*
 *  Copyright (C) 2004-2009  Anders Gavare.  All rights reserved.
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
 *  Single-step debugger.
 *
 *
 *  This entire module is very much non-reentrant. :-/  TODO: Fix.
 */

#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "console.h"
#include "cpu.h"
#include "device.h"
#include "debugger.h"
#include "diskimage.h"
#include "emul.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "net.h"
#include "settings.h"
#include "timer.h"
#include "x11.h"


extern int extra_argc;
extern char **extra_argv;
extern struct settings *global_settings;
extern int quiet_mode;


/*
 *  Global debugger variables:
 *
 *  TODO: Some of these should be moved to some other place!
 */

volatile int single_step = NOT_SINGLE_STEPPING;
volatile int exit_debugger;
int force_debugger_at_exit = 0;

volatile int single_step_breakpoint = 0;
int debugger_n_steps_left_before_interaction = 0;

int old_instruction_trace = 0;
int old_quiet_mode = 0;
int old_show_trace_tree = 0;


/*
 *  Private (global) debugger variables:
 */

static volatile int ctrl_c;

/*  Currently focused CPU, machine, and emulation:  */
int debugger_cur_cpu;
int debugger_cur_machine;
static struct machine *debugger_machine;
static struct emul *debugger_emul;

#define	MAX_CMD_BUFLEN		72
#define	N_PREVIOUS_CMDS		150
static char *last_cmd[N_PREVIOUS_CMDS];
static int last_cmd_index;

static char repeat_cmd[MAX_CMD_BUFLEN];

#define	MAGIC_UNTOUCHED		0x98ca76c2ffcc0011ULL

static uint64_t last_dump_addr = MAGIC_UNTOUCHED;
static uint64_t last_unasm_addr = MAGIC_UNTOUCHED;


/*
 *  debugger_readchar():
 */
char debugger_readchar(void)
{
	int ch;

	while ((ch = console_readchar(MAIN_CONSOLE)) < 0 && !exit_debugger) {
		/*  Check for X11 events:  */
		x11_check_event(debugger_emul);

		/*  Give up some CPU time:  */
		usleep(10000);
	}
	return ch;
}


/*
 *  debugger_activate():
 *
 *  This is a signal handler for CTRL-C.  It shouldn't be called directly,
 *  but setup code in emul.c sets the CTRL-C signal handler to use this
 *  function.
 */
void debugger_activate(int x)
{
	ctrl_c = 1;

	if (single_step != NOT_SINGLE_STEPPING) {
		/*  Already in the debugger. Do nothing.  */
		int i;
		for (i=0; i<MAX_CMD_BUFLEN; i++)
			console_makeavail(MAIN_CONSOLE, '\b');
		console_makeavail(MAIN_CONSOLE, ' ');
		console_makeavail(MAIN_CONSOLE, '\n');
		printf("^C");
		fflush(stdout);
	} else {
		/*  Enter the single step debugger.  */
		single_step = ENTER_SINGLE_STEPPING;

		/*  Discard any chars in the input queue:  */
		while (console_charavail(MAIN_CONSOLE))
			console_readchar(MAIN_CONSOLE);
	}

	/*  Clear the repeat-command buffer:  */
	repeat_cmd[0] = '\0';

	/*  Reactivate the signal handler:  */
	signal(SIGINT, debugger_activate);
}


/*
 *  show_breakpoint():
 */
static void show_breakpoint(struct machine *m, int i)
{
	printf("%3i: 0x", i);
	if (m->cpus[0]->is_32bit)
		printf("%08"PRIx32, (uint32_t) m->breakpoints.addr[i]);
	else
		printf("%016"PRIx64, (uint64_t) m->breakpoints.addr[i]);
	if (m->breakpoints.string[i] != NULL)
		printf(" (%s)", m->breakpoints.string[i]);
	printf("\n");
}


/****************************************************************************/


#include "debugger_cmds.cc"


/****************************************************************************/


/*
 *  debugger_assignment():
 *
 *  cmd contains something like "pc=0x80001000", or "r31=memcpy+0x40".
 */
void debugger_assignment(struct machine *m, char *cmd)
{
	char *left, *right;
	int res_left, res_right;
	uint64_t tmp;
	uint64_t old_pc = m->cpus[0]->pc;	/*  TODO: multiple cpus?  */

	CHECK_ALLOCATION(left = (char *) malloc(MAX_CMD_BUFLEN));
	strlcpy(left, cmd, MAX_CMD_BUFLEN);
	right = strchr(left, '=');
	if (right == NULL) {
		fprintf(stderr, "internal error in the debugger\n");
		exit(1);
	}
	*right = '\0';

	/*  Remove trailing spaces in left:  */
	while (strlen(left) >= 1 && left[strlen(left)-1] == ' ')
		left[strlen(left)-1] = '\0';

	/*  Remove leading spaces in right:  */
	right++;
	while (*right == ' ' && *right != '\0')
		right++;

	/*  printf("left  = '%s'\nright = '%s'\n", left, right);  */

	res_right = debugger_parse_expression(m, right, 0, &tmp);
	switch (res_right) {
	case PARSE_NOMATCH:
		printf("No match for the right-hand side of the assignment.\n");
		break;
	case PARSE_MULTIPLE:
		printf("Multiple matches for the right-hand side of the "
		    "assignment.\n");
		break;
	default:
		res_left = debugger_parse_expression(m, left, 1, &tmp);
		switch (res_left) {
		case PARSE_NOMATCH:
			printf("No match for the left-hand side of the "
			    "assignment.\n");
			break;
		case PARSE_MULTIPLE:
			printf("Multiple matches for the left-hand side "
			    "of the assignment.\n");
			break;
		default:
			debugger_cmd_print(m, left);
		}
	}

	/*
	 *  If the PC has changed, then release any breakpoint we were
	 *  currently stopped at.
	 *
	 *  TODO: multiple cpus?
	 */
	if (old_pc != m->cpus[0]->pc)
		single_step_breakpoint = 0;

	free(left);
}


/*
 *  debugger_execute_cmd():
 */
void debugger_execute_cmd(char *cmd, int cmd_len)
{
	int i, n, i_match, matchlen;

	/*
	 *  Is there a '=' on the command line? Then try to do an
	 *  assignment.  (Only if there is just one word, followed
	 *  by the '=' sign. This makes it possible to use commands
	 *  such as "device add name addr=xyz".)
	 */
	if (strchr(cmd, '=') != NULL) {
		/*  Count the nr of words:  */
		int nw = 0, inword = 0;
		char *p = cmd;
		while (*p) {
			if (*p == '=')
				break;
			if (*p != ' ') {
				if (!inword)
					nw ++;
				inword = 1;
			} else
				inword = 0;
			p++;
		}

		if (nw == 1) {
			debugger_assignment(debugger_machine, cmd);
			return;
		}
	}

	i = 0;
	while (cmds[i].name != NULL)
		cmds[i++].tmp_flag = 0;

	/*  How many chars in cmd to match against:  */
	matchlen = 0;
	while (isalpha((int)cmd[matchlen]))
		matchlen ++;

	/*  Check for a command name match:  */
	n = i = i_match = 0;
	while (cmds[i].name != NULL) {
		if (strncasecmp(cmds[i].name, cmd, matchlen) == 0
		    && cmds[i].f != NULL) {
			cmds[i].tmp_flag = 1;
			i_match = i;
			n++;
		}
		i++;
	}

	/*  No match?  */
	if (n == 0) {
		printf("Unknown command '%s'. Type 'help' for help.\n", cmd);
		return;
	}

	/*  More than one match?  */
	if (n > 1) {
		printf("Ambiguous command '%s':  ", cmd);
		i = 0;
		while (cmds[i].name != NULL) {
			if (cmds[i].tmp_flag)
				printf("  %s", cmds[i].name);
			i++;
		}
		printf("\n");
		return;
	}

	/*  Exactly one match:  */
	if (cmds[i_match].f != NULL) {
		char *p = cmd + matchlen;
		/*  Remove leading whitespace from the args...  */
		while (*p != '\0' && *p == ' ')
			p++;

		/*  ... and run the command:  */
		cmds[i_match].f(debugger_machine, p);
	} else
		printf("FATAL ERROR: internal error in debugger.c:"
		    " no handler for this command?\n");
}


/*
 *  debugger_readline():
 *
 *  Read a line from the terminal.
 */
static char *debugger_readline(void)
{
	int ch, i, j, n, i_match, reallen, cmd_len, cursor_pos;
	int read_from_index = last_cmd_index;
	char *cmd = last_cmd[last_cmd_index];

	cmd_len = 0; cmd[0] = '\0';
	printf("GXemul> ");
	fflush(stdout);

	ch = '\0';
	cmd_len = 0;
	cursor_pos = 0;

	while (ch != '\n' && !exit_debugger) {
		ch = debugger_readchar();

		if ((ch == '\b' || ch == 127) && cursor_pos > 0) {
			/*  Backspace.  */
			cursor_pos --;
			cmd_len --;
			memmove(cmd + cursor_pos, cmd + cursor_pos + 1,
			    cmd_len);
			cmd[cmd_len] = '\0';
			printf("\b");
			for (i=cursor_pos; i<cmd_len; i++)
				printf("%c", cmd[i]);
			printf(" \b");
			for (i=cursor_pos; i<cmd_len; i++)
				printf("\b");
		} else if (ch == 4 && cmd_len > 0 && cursor_pos < cmd_len) {
			/*  CTRL-D: Delete.  */
			cmd_len --;
			memmove(cmd + cursor_pos, cmd + cursor_pos + 1,
			    cmd_len);
			cmd[cmd_len] = '\0';
			for (i=cursor_pos; i<cmd_len; i++)
				printf("%c", cmd[i]);
			printf(" \b");
			for (i=cursor_pos; i<cmd_len; i++)
				printf("\b");
		} else if (ch == 1) {
			/*  CTRL-A: Start of line.  */
			while (cursor_pos > 0) {
				cursor_pos --;
				printf("\b");
			}
		} else if (ch == 2) {
			/*  CTRL-B: Backwards one character.  */
			if (cursor_pos > 0) {
				printf("\b");
				cursor_pos --;
			}
		} else if (ch == 5) {
			/*  CTRL-E: End of line.  */
			while (cursor_pos < cmd_len) {
				printf("%c", cmd[cursor_pos]);
				cursor_pos ++;
			}
		} else if (ch == 6) {
			/*  CTRL-F: Forward one character.  */
			if (cursor_pos < cmd_len) {
				printf("%c",
				    cmd[cursor_pos]);
				cursor_pos ++;
			}
		} else if (ch == 11) {
			/*  CTRL-K: Kill to end of line.  */
			for (i=0; i<MAX_CMD_BUFLEN; i++)
				console_makeavail(MAIN_CONSOLE, 4); /*  :-)  */
		} else if (ch == 14 || ch == 16) {
			/*  CTRL-P: Previous line in the command history,
			    CTRL-N: next line  */
			do {
				if (ch == 14 &&
				    read_from_index == last_cmd_index)
					break;
				if (ch == 16)
					i = read_from_index - 1;
				else
					i = read_from_index + 1;

				if (i < 0)
					i = N_PREVIOUS_CMDS - 1;
				if (i >= N_PREVIOUS_CMDS)
					i = 0;

				/*  Special case: pressing 'down'
				    to reach last_cmd_index:  */
				if (i == last_cmd_index) {
					read_from_index = i;
					for (i=cursor_pos; i<cmd_len;
					    i++)
						printf(" ");
					for (i=cmd_len-1; i>=0; i--)
						printf("\b \b");
					cmd[0] = '\0';
					cmd_len = cursor_pos = 0;
				} else if (last_cmd[i][0] != '\0') {
					/*  Copy from old line:  */
					read_from_index = i;
					for (i=cursor_pos; i<cmd_len;
					    i++)
						printf(" ");
					for (i=cmd_len-1; i>=0; i--)
						printf("\b \b");
					strlcpy(cmd,
					    last_cmd[read_from_index],
					    MAX_CMD_BUFLEN);
					cmd_len = strlen(cmd);
					printf("%s", cmd);
					cursor_pos = cmd_len;
				}
			} while (0);
		} else if (ch >= ' ' && cmd_len < MAX_CMD_BUFLEN-1) {
			/*  Visible character:  */
			memmove(cmd + cursor_pos + 1, cmd + cursor_pos,
			    cmd_len - cursor_pos);
			cmd[cursor_pos] = ch;
			cmd_len ++;
			cursor_pos ++;
			cmd[cmd_len] = '\0';
			printf("%c", ch);
			for (i=cursor_pos; i<cmd_len; i++)
				printf("%c", cmd[i]);
			for (i=cursor_pos; i<cmd_len; i++)
				printf("\b");
		} else if (ch == '\r' || ch == '\n') {
			ch = '\n';
			printf("\n");
		} else if (ch == '\t') {
			/*  Super-simple tab-completion:  */
			i = 0;
			while (cmds[i].name != NULL)
				cmds[i++].tmp_flag = 0;

			/*  Check for a (partial) command match:  */
			n = i = i_match = 0;
			while (cmds[i].name != NULL) {
				if (strncasecmp(cmds[i].name, cmd,
				    cmd_len) == 0) {
					cmds[i].tmp_flag = 1;
					i_match = i;
					n++;
				}
				i++;
			}

			switch (n) {
			case 0:	/*  Beep.  */
				printf("\a");
				break;
			case 1:	/*  Add the rest of the command:  */
				reallen = strlen(cmds[i_match].name);
				for (i=cmd_len; i<reallen; i++)
					console_makeavail(MAIN_CONSOLE,
					    cmds[i_match].name[i]);
				/*  ... and a space, if the command takes
				    any arguments:  */
				if (cmds[i_match].args != NULL &&
				    cmds[i_match].args[0] != '\0')
					console_makeavail(MAIN_CONSOLE, ' ');
				break;
			default:
				/*  Show all possible commands:  */
				printf("\a\n");	/*  Beep. :-)  */
				i = 0;		/*  i = cmds index  */
				j = 0;		/*  j = # of cmds printed  */
				while (cmds[i].name != NULL) {
					if (cmds[i].tmp_flag) {
						size_t q;
						if (j == 0)
							printf("  ");
						printf("%s",
						    cmds[i].name);
						j++;
						if (j != 6)
							for (q=0; q<13-strlen(
							    cmds[i].name); q++)
								printf(" ");
						if (j == 6) {
							printf("\n");
							j = 0;
						}
					}
					i++;
				}
				if (j != 0)
					printf("\n");
				printf("GXemul> ");
				for (i=0; i<cmd_len; i++)
					printf("%c", cmd[i]);
			}
		} else if (ch == 27) {
			/*  Escape codes: (cursor keys etc)  */
			while ((ch = console_readchar(MAIN_CONSOLE)) < 0)
				usleep(10000);
			if (ch == '[' || ch == 'O') {
				while ((ch = console_readchar(MAIN_CONSOLE))
				    < 0)
					usleep(10000);
				switch (ch) {
				case '2':	/*  2~ = ins  */
				case '5':	/*  5~ = pgup  */
				case '6':	/*  6~ = pgdn  */
					/*  TODO: Ugly hack, but might work.  */
					while ((ch = console_readchar(
					    MAIN_CONSOLE)) < 0)
						usleep(10000);
					/*  Do nothing for these keys.  */
					break;
				case '3':	/*  3~ = delete  */
					/*  TODO: Ugly hack, but might work.  */
					while ((ch = console_readchar(
					    MAIN_CONSOLE)) < 0)
						usleep(10000);
					console_makeavail(MAIN_CONSOLE, '\b');
					break;
				case 'A':	/*  Up.  */
					/*  Up cursor ==> CTRL-P  */
					console_makeavail(MAIN_CONSOLE, 16);
					break;
				case 'B':	/*  Down.  */
					/*  Down cursor ==> CTRL-N  */
					console_makeavail(MAIN_CONSOLE, 14);
					break;
				case 'C':
					/*  Right cursor ==> CTRL-F  */
					console_makeavail(MAIN_CONSOLE, 6);
					break;
				case 'D':	/*  Left  */
					/*  Left cursor ==> CTRL-B  */
					console_makeavail(MAIN_CONSOLE, 2);
					break;
				case 'F':
					/*  End ==> CTRL-E  */
					console_makeavail(MAIN_CONSOLE, 5);
					break;
				case 'H':
					/*  Home ==> CTRL-A  */
					console_makeavail(MAIN_CONSOLE, 1);
					break;
				}
			}
		}

		fflush(stdout);
	}

	if (exit_debugger)
		cmd[0] = '\0';

	return cmd;
}


/*
 *  debugger():
 *
 *  This is a loop, which reads a command from the terminal, and executes it.
 */
void debugger(void)
{
	int i, cmd_len;
	char *cmd;

	if (debugger_n_steps_left_before_interaction > 0) {
		debugger_n_steps_left_before_interaction --;
		return;
	}

	/*
	 *  Clear all dyntrans translations, because otherwise things would
	 *  become to complex to keep in sync.
	 */
	/*  TODO: In all machines  */
	for (i=0; i<debugger_machine->ncpus; i++)
		if (debugger_machine->cpus[i]->translation_cache != NULL) {
			cpu_create_or_reset_tc(debugger_machine->cpus[i]);
			debugger_machine->cpus[i]->
			    invalidate_translation_caches(
			    debugger_machine->cpus[i], 0, INVALIDATE_ALL);
		}

	/*  Stop timers while interacting with the user:  */
	timer_stop();

	exit_debugger = 0;

	while (!exit_debugger) {
		/*  Read a line from the terminal:  */
		cmd = debugger_readline();

		cmd_len = strlen(cmd);

		/*  Remove spaces:  */
		while (cmd_len > 0 && cmd[0]==' ')
			memmove(cmd, cmd+1, cmd_len --);
		while (cmd_len > 0 && cmd[cmd_len-1] == ' ')
			cmd[(cmd_len--)-1] = '\0';

		/*  No command? Then try reading another line.  */
		if (cmd_len == 0) {
			/*  Special case for repeated commands:  */
			if (repeat_cmd[0] != '\0')
				strlcpy(cmd, repeat_cmd, MAX_CMD_BUFLEN);
			else
				continue;
		} else {
			last_cmd_index ++;
			if (last_cmd_index >= N_PREVIOUS_CMDS)
				last_cmd_index = 0;

			repeat_cmd[0] = '\0';
		}

		debugger_execute_cmd(cmd, cmd_len);

		/*  Special hack for the "step" command:  */
		if (exit_debugger == -1)
			return;
	}

	/*  Start up timers again:  */
	timer_start();

	/*  ... and reset starttime, so that nr of instructions per second
	    can be calculated correctly:  */
	for (i=0; i<debugger_machine->ncpus; i++) {
		gettimeofday(&debugger_machine->cpus[i]->starttime, NULL);
		debugger_machine->cpus[i]->ninstrs_since_gettimeofday = 0;
	}

	single_step = NOT_SINGLE_STEPPING;
	debugger_machine->instruction_trace = old_instruction_trace;
	debugger_machine->show_trace_tree = old_show_trace_tree;
	quiet_mode = old_quiet_mode;
}


/*
 *  debugger_reset():
 *
 *  This function should be called before calling debugger(), when it is
 *  absolutely necessary that debugger() is interactive. Otherwise, it might
 *  return without doing anything, such as when single-stepping multiple
 *  instructions at a time.
 */
void debugger_reset(void)
{
	debugger_n_steps_left_before_interaction = 0;
}


/*
 *  debugger_init():
 *
 *  Must be called before any other debugger function is used.
 */
void debugger_init(struct emul *emul)
{
	int i;

	debugger_emul = emul;

	if (emul->n_machines < 1) {
		fprintf(stderr, "\nERROR: No machines, "
		    "cannot handle this situation yet.\n\n");
		exit(1);
	}

	debugger_machine = emul->machines[0];

	debugger_cur_cpu = 0;
	debugger_cur_machine = 0;

	for (i=0; i<N_PREVIOUS_CMDS; i++) {
		CHECK_ALLOCATION(last_cmd[i] = (char *) malloc(MAX_CMD_BUFLEN));
		last_cmd[i][0] = '\0';
	}

	last_cmd_index = 0;
	repeat_cmd[0] = '\0';
}

