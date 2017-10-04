/*
 *  Copyright (C) 2005-2009  Anders Gavare.  All rights reserved.
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
 *  Set up an emulation by parsing a config file.
 *
 *  TODO: REWRITE THIS FROM SCRATCH! :-)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "diskimage.h"
#include "emul.h"
#include "machine.h"
#include "misc.h"
#include "net.h"


#define is_word_char(ch)	( \
	(ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || \
	ch == '_' || ch == '$' || (ch >= '0' && ch <= '9') )

#define	MAX_WORD_LEN		200

#define	EXPECT_WORD			1
#define	EXPECT_LEFT_PARENTHESIS		2
#define	EXPECT_RIGHT_PARENTHESIS	4

static int parenthesis_level = 0;


/*
 *  read_one_word():
 *
 *  Reads the file f until it has read a complete word. Whitespace is ignored,
 *  and also any exclamation mark ('!') and anything following an exclamation
 *  mark on a line.
 *
 *  Used internally by emul_parse_config().
 */
static void read_one_word(FILE *f, char *buf, int buflen, int *line,
	int expect)
{
	int ch;
	int done = 0;
	int curlen = 0;
	int in_word = 0;

	while (!done) {
		if (curlen >= buflen - 1)
			break;

		ch = fgetc(f);
		if (ch == EOF)
			break;

		if (in_word) {
			if (is_word_char(ch)) {
				buf[curlen++] = ch;
				if (curlen == buflen - 1)
					done = 1;
				continue;
			} else {
				if (ungetc(ch, f) == EOF) {
					fatal("ungetc() failed?\n");
					exit(1);
				}
				break;
			}
		}

		if (ch == '\n') {
			(*line) ++;
			continue;
		}

		if (ch == '\r')
			continue;

		if (ch == '!') {
			/*  Skip until newline:  */
			while (ch != '\n' && ch != EOF)
				ch = fgetc(f);
			if (ch == '\n')
				(*line) ++;
			continue;
		}

		if (ch == '{') {
			int depth = 1;

			/*  Skip until '}':  */
			while (depth > 0) {
				ch = fgetc(f);
				if (ch == '\n')
					(*line) ++;
				if (ch == '{')
					depth ++;
				if (ch == '}')
					depth --;
				if (ch == EOF) {
					fatal("line %i: unexpected EOF inside"
					    " a nested comment\n", *line);
					exit(1);
				}
			}
			continue;
		}

		/*  Whitespace?  */
		if (ch <= ' ')
			continue;

		if (ch == '"' || ch == '\'') {
			/*  This is a quoted word.  */
			int start_ch = ch;

			if (expect & EXPECT_LEFT_PARENTHESIS) {
				fatal("unexpected character '%c', line %i\n",
				    ch, *line);
				exit(1);
			}

			while (curlen < buflen - 1) {
				ch = fgetc(f);
				if (ch == '\n') {
					fatal("line %i: newline inside"
					    " quotes?\n", *line);
					exit(1);
				}
				if (ch == EOF) {
					fatal("line %i: EOF inside a quoted"
					    " string?\n", *line);
					exit(1);
				}
				if (ch == start_ch)
					break;
				buf[curlen++] = ch;
			}
			break;
		}

		if ((expect & EXPECT_WORD) && is_word_char(ch)) {
			buf[curlen++] = ch;
			in_word = 1;
			if (curlen == buflen - 1)
				done = 1;
		} else {
			if ((expect & EXPECT_LEFT_PARENTHESIS) && ch == '(') {
				parenthesis_level ++;
				buf[curlen++] = ch;
				break;
			}
			if ((expect & EXPECT_RIGHT_PARENTHESIS) && ch == ')') {
				parenthesis_level --;
				buf[curlen++] = ch;
				break;
			}

			fatal("unexpected character '%c', line %i\n",
			    ch, *line);
			exit(1);
		}
	}

	buf[curlen] = '\0';
}


#define	PARSESTATE_NONE			0
#define	PARSESTATE_EMUL			1
#define	PARSESTATE_NET			2
#define	PARSESTATE_MACHINE		3

static char cur_net_ipv4net[50];
static char cur_net_ipv4len[50];
static char cur_net_local_port[10];
#define	MAX_N_REMOTE		20
#define	MAX_REMOTE_LEN		100
static char *cur_net_remote[MAX_N_REMOTE];
static int cur_net_n_remote;

static char cur_machine_name[50];
static char cur_machine_cpu[50];
static char cur_machine_type[50];
static char cur_machine_subtype[50];
static char cur_machine_bootname[150];
static char cur_machine_bootarg[250];
static char cur_machine_slowsi[10];
static char cur_machine_prom_emulation[10];
static char cur_machine_use_x11[10];
static char cur_machine_x11_scaledown[10];
static char cur_machine_byte_order[20];
static char cur_machine_random_mem[10];
static char cur_machine_random_cpu[10];
static char cur_machine_force_netboot[10];
static char cur_machine_start_paused[10];
static char cur_machine_ncpus[10];
static char cur_machine_n_gfx_cards[10];
static char cur_machine_serial_nr[10];
static char cur_machine_emulated_hz[10];
static char cur_machine_memory[10];
#define	MAX_N_LOAD		15
#define	MAX_LOAD_LEN		2000
static char *cur_machine_load[MAX_N_LOAD];
static int cur_machine_n_load;
#define	MAX_N_DISK		10
#define	MAX_DISK_LEN		2000
static char *cur_machine_disk[MAX_N_DISK];
static int cur_machine_n_disk;
#define	MAX_N_DEVICE		20
#define	MAX_DEVICE_LEN		400
static char *cur_machine_device[MAX_N_DISK];
static int cur_machine_n_device;
#define	MAX_N_X11_DISP		5
#define	MAX_X11_DISP_LEN	1000
static char *cur_machine_x11_disp[MAX_N_X11_DISP];
static int cur_machine_n_x11_disp;

#define WORD(w,var) {						\
		if (strcmp(word, w) == 0) {			\
			read_one_word(f, word, maxbuflen,	\
			    line, EXPECT_LEFT_PARENTHESIS);	\
			read_one_word(f, var, sizeof(var),	\
			    line, EXPECT_WORD);			\
			read_one_word(f, word, maxbuflen,	\
			    line, EXPECT_RIGHT_PARENTHESIS);	\
			return;					\
		}						\
	}

static void parse__machine(struct emul *e, FILE *f, int *in_emul, int *line,
	int *parsestate, char *word, size_t maxbuflen);


/*
 *  parse_on_off():
 *
 *  Returns 1 for "on", "yes", "enable", or "1".
 *  Returns 0 for "off", "no", "disable", or "0".
 *  Prints a fatal warning and exit()s for other values.
 */
int parse_on_off(char *s)
{
	if (strcasecmp(s, "on") == 0 || strcasecmp(s, "yes") == 0 ||
	    strcasecmp(s, "enable") == 0 || strcasecmp(s, "1") == 0)
		return 1;
	if (strcasecmp(s, "off") == 0 || strcasecmp(s, "no") == 0 ||
	    strcasecmp(s, "disable") == 0 || strcasecmp(s, "0") == 0)
		return 0;

	fprintf(stderr, "parse_on_off(): WARNING: unknown value '%s'\n", s);

	return 0;
}


/*
 *  parse__emul():
 *
 *  name, net, machine
 */
static void parse__emul(struct emul *e, FILE *f, int *in_emul, int *line,
	int *parsestate, char *word, size_t maxbuflen)
{
	if (word[0] == ')') {
		*parsestate = PARSESTATE_NONE;
		return;
	}

	if (strcmp(word, "name") == 0) {
		char tmp[200];
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_LEFT_PARENTHESIS);
		read_one_word(f, tmp, sizeof(tmp), line, EXPECT_WORD);
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_RIGHT_PARENTHESIS);
		if (e->name != NULL) {
			free(e->name);
			e->name = NULL;
		}
		CHECK_ALLOCATION(e->name = strdup(tmp));
		debug("name: \"%s\"\n", e->name);
		return;
	}

	if (strcmp(word, "net") == 0) {
		*parsestate = PARSESTATE_NET;
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_LEFT_PARENTHESIS);

		/*  Default net:  */
		strlcpy(cur_net_ipv4net, NET_DEFAULT_IPV4_MASK,
		    sizeof(cur_net_ipv4net));
		snprintf(cur_net_ipv4len, sizeof(cur_net_ipv4len), "%i",
		    NET_DEFAULT_IPV4_LEN);
		strlcpy(cur_net_local_port, "", sizeof(cur_net_local_port));
		cur_net_n_remote = 0;
		return;
	}

	if (strcmp(word, "machine") == 0) {
		*parsestate = PARSESTATE_MACHINE;
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_LEFT_PARENTHESIS);

		/*  A "zero state":  */
		cur_machine_name[0] = '\0';
		cur_machine_cpu[0] = '\0';
		cur_machine_type[0] = '\0';
		cur_machine_subtype[0] = '\0';
		cur_machine_bootname[0] = '\0';
		cur_machine_bootarg[0] = '\0';
		cur_machine_n_load = 0;
		cur_machine_n_disk = 0;
		cur_machine_n_device = 0;
		cur_machine_n_x11_disp = 0;
		cur_machine_slowsi[0] = '\0';
		cur_machine_prom_emulation[0] = '\0';
		cur_machine_use_x11[0] = '\0';
		cur_machine_x11_scaledown[0] = '\0';
		cur_machine_byte_order[0] = '\0';
		cur_machine_random_mem[0] = '\0';
		cur_machine_random_cpu[0] = '\0';
		cur_machine_force_netboot[0] = '\0';
		cur_machine_start_paused[0] = '\0';
		cur_machine_ncpus[0] = '\0';
		cur_machine_n_gfx_cards[0] = '\0';
		cur_machine_serial_nr[0] = '\0';
		cur_machine_emulated_hz[0] = '\0';
		cur_machine_memory[0] = '\0';
		return;
	}

	fatal("line %i: not expecting '%s' in an 'emul' section\n",
	    *line, word);
	exit(1);
}


/*
 *  parse__net():
 *
 *  Simple words: ipv4net, ipv4len, local_port
 *
 *  Complex: add_remote
 *
 *  TODO: more words? for example an option to disable the gateway? that would
 *  have to be implemented correctly in src/net.c first.
 */
static void parse__net(struct emul *e, FILE *f, int *in_emul, int *line,
	int *parsestate, char *word, size_t maxbuflen)
{
	int i;

	if (word[0] == ')') {
		/*  Finished with the 'net' section. Let's create the net:  */
		if (e->net != NULL) {
			fatal("line %i: more than one net isn't really "
			    "supported yet\n", *line);
			exit(1);
		}

		if (!cur_net_local_port[0])
			strlcpy(cur_net_local_port, "0",
			    sizeof(cur_net_local_port));

		e->net = net_init(e, NET_INIT_FLAG_GATEWAY,
		    cur_net_ipv4net, atoi(cur_net_ipv4len),
		    cur_net_remote, cur_net_n_remote,
		    atoi(cur_net_local_port), NULL);

		if (e->net == NULL) {
			fatal("line %i: fatal error: could not create"
			    " the net (?)\n", *line);
			exit(1);
		}

		for (i=0; i<cur_net_n_remote; i++) {
			free(cur_net_remote[i]);
			cur_net_remote[i] = NULL;
		}

		*parsestate = PARSESTATE_EMUL;
		return;
	}

	WORD("ipv4net", cur_net_ipv4net);
	WORD("ipv4len", cur_net_ipv4len);
	WORD("local_port", cur_net_local_port);

	if (strcmp(word, "add_remote") == 0) {
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_LEFT_PARENTHESIS);
		if (cur_net_n_remote >= MAX_N_REMOTE) {
			fprintf(stderr, "too many remote networks\n");
			exit(1);
		}

		CHECK_ALLOCATION(cur_net_remote[cur_net_n_remote] =
		    (char *) malloc(MAX_REMOTE_LEN));
		read_one_word(f, cur_net_remote[cur_net_n_remote],
		    MAX_REMOTE_LEN, line, EXPECT_WORD);
		cur_net_n_remote ++;
		read_one_word(f, word, maxbuflen, line,
		    EXPECT_RIGHT_PARENTHESIS);
		return;
	}

	fatal("line %i: not expecting '%s' in a 'net' section\n", *line, word);
	exit(1);
}


/*
 *  parse__machine():
 */
static void parse__machine(struct emul *e, FILE *f, int *in_emul, int *line,
	int *parsestate, char *word, size_t maxbuflen)
{
	int r, i;

	if (word[0] == ')') {
		/*  Finished with the 'machine' section.  */
		struct machine *m;

		if (!cur_machine_name[0])
			strlcpy(cur_machine_name, "no_name",
			    sizeof(cur_machine_name));

		m = emul_add_machine(e, cur_machine_name);

		r = machine_name_to_type(cur_machine_type, cur_machine_subtype,
		    &m->machine_type, &m->machine_subtype, &m->arch);
		if (!r)
			exit(1);

		if (cur_machine_cpu[0])
			CHECK_ALLOCATION(m->cpu_name = strdup(cur_machine_cpu));

		if (!cur_machine_use_x11[0])
			strlcpy(cur_machine_use_x11, "no",
			    sizeof(cur_machine_use_x11));
		m->x11_md.in_use = parse_on_off(cur_machine_use_x11);

		if (!cur_machine_slowsi[0])
			strlcpy(cur_machine_slowsi, "no",
			    sizeof(cur_machine_slowsi));
		m->slow_serial_interrupts_hack_for_linux =
		    parse_on_off(cur_machine_slowsi);

		if (!cur_machine_prom_emulation[0])
			strlcpy(cur_machine_prom_emulation, "yes",
			    sizeof(cur_machine_prom_emulation));
		m->prom_emulation = parse_on_off(cur_machine_prom_emulation);

		if (!cur_machine_random_mem[0])
			strlcpy(cur_machine_random_mem, "no",
			    sizeof(cur_machine_random_mem));
		m->random_mem_contents =
		    parse_on_off(cur_machine_random_mem);

		if (!cur_machine_random_cpu[0])
			strlcpy(cur_machine_random_cpu, "no",
			    sizeof(cur_machine_random_cpu));
		m->use_random_bootstrap_cpu =
		    parse_on_off(cur_machine_random_cpu);

		m->byte_order_override = NO_BYTE_ORDER_OVERRIDE;
		if (cur_machine_byte_order[0]) {
			if (strncasecmp(cur_machine_byte_order, "big", 3) == 0)
				m->byte_order_override = EMUL_BIG_ENDIAN;
			else if (strncasecmp(cur_machine_byte_order, "little",
			    6) == 0)
				m->byte_order_override = EMUL_LITTLE_ENDIAN;
			else {
				fatal("Byte order must be big-endian or"
				    " little-endian\n");
				exit(1);
			}
		}

		if (!cur_machine_force_netboot[0])
			strlcpy(cur_machine_force_netboot, "no",
			    sizeof(cur_machine_force_netboot));
		m->force_netboot = parse_on_off(cur_machine_force_netboot);

		if (!cur_machine_start_paused[0])
			strlcpy(cur_machine_start_paused, "no",
			    sizeof(cur_machine_start_paused));
		m->start_paused = parse_on_off(cur_machine_start_paused);

		/*  NOTE: Default nr of CPUs is 0:  */
		if (!cur_machine_ncpus[0])
			strlcpy(cur_machine_ncpus, "0",
			    sizeof(cur_machine_ncpus));
		m->ncpus = atoi(cur_machine_ncpus);

		if (cur_machine_n_gfx_cards[0])
			m->n_gfx_cards = atoi(cur_machine_n_gfx_cards);

		if (cur_machine_serial_nr[0]) {
			m->serial_nr = atoi(cur_machine_serial_nr);
			e->next_serial_nr = m->serial_nr+1;
		}

		if (cur_machine_emulated_hz[0]) {
			m->emulated_hz = mystrtoull(cur_machine_emulated_hz,
			    NULL, 0);
		}

		/*  NOTE: Default nr of CPUs is 0:  */
		if (!cur_machine_memory[0])
			strlcpy(cur_machine_memory, "0",
			    sizeof(cur_machine_memory));
		m->physical_ram_in_mb = atoi(cur_machine_memory);

		if (!cur_machine_x11_scaledown[0])
			m->x11_md.scaledown = 1;
		else {
			m->x11_md.scaledown = atoi(cur_machine_x11_scaledown);
			if (m->x11_md.scaledown < 0) {
				m->x11_md.scaleup = 0 - m->x11_md.scaledown;
				m->x11_md.scaledown = 1;
			}
			if (m->x11_md.scaledown < 1) {
				fprintf(stderr, "Invalid scaledown value"
				    " (%i)\n", m->x11_md.scaledown);
				exit(1);
			}
		}

		for (i=0; i<cur_machine_n_disk; i++) {
			diskimage_add(m, cur_machine_disk[i]);
			free(cur_machine_disk[i]);
			cur_machine_disk[i] = NULL;
		}

		m->boot_kernel_filename = strdup(cur_machine_bootname);

		if (cur_machine_bootarg[0])
			m->boot_string_argument = strdup(cur_machine_bootarg);

		for (i=0; i<cur_machine_n_x11_disp; i++) {
			m->x11_md.n_display_names ++;
			CHECK_ALLOCATION(m->x11_md.display_names = (char **) realloc(
			    m->x11_md.display_names, m->x11_md.n_display_names
			    * sizeof(char *)));
			CHECK_ALLOCATION(m->x11_md.display_names[
			    m->x11_md.n_display_names-1] =
			    strdup(cur_machine_x11_disp[i]));
			free(cur_machine_x11_disp[i]);
			cur_machine_x11_disp[i] = NULL;
		}

		emul_machine_setup(m,
		    cur_machine_n_load, cur_machine_load,
		    cur_machine_n_device, cur_machine_device);

		for (i=0; i<cur_machine_n_device; i++) {
			free(cur_machine_device[i]);
			cur_machine_device[i] = NULL;
		}

		for (i=0; i<cur_machine_n_load; i++) {
			free(cur_machine_load[i]);
			cur_machine_load[i] = NULL;
		}

		*parsestate = PARSESTATE_EMUL;
		return;
	}

	WORD("name", cur_machine_name);
	WORD("cpu", cur_machine_cpu);
	WORD("type", cur_machine_type);
	WORD("subtype", cur_machine_subtype);
	WORD("bootname", cur_machine_bootname);
	WORD("bootarg", cur_machine_bootarg);
	WORD("slow_serial_interrupts_hack_for_linux", cur_machine_slowsi);
	WORD("prom_emulation", cur_machine_prom_emulation);
	WORD("use_x11", cur_machine_use_x11);
	WORD("x11_scaledown", cur_machine_x11_scaledown);
	WORD("byte_order", cur_machine_byte_order);
	WORD("random_mem_contents", cur_machine_random_mem);
	WORD("use_random_bootstrap_cpu", cur_machine_random_cpu);
	WORD("force_netboot", cur_machine_force_netboot);
	WORD("ncpus", cur_machine_ncpus);
	WORD("serial_nr", cur_machine_serial_nr);
	WORD("n_gfx_cards", cur_machine_n_gfx_cards);
	WORD("emulated_hz", cur_machine_emulated_hz);
	WORD("memory", cur_machine_memory);
	WORD("start_paused", cur_machine_start_paused);

	if (strcmp(word, "load") == 0) {
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_LEFT_PARENTHESIS);
		if (cur_machine_n_load >= MAX_N_LOAD) {
			fprintf(stderr, "too many loads\n");
			exit(1);
		}
		CHECK_ALLOCATION(cur_machine_load[cur_machine_n_load] =
		    (char*) malloc(MAX_LOAD_LEN));
		read_one_word(f, cur_machine_load[cur_machine_n_load],
		    MAX_LOAD_LEN, line, EXPECT_WORD);
		cur_machine_n_load ++;
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_RIGHT_PARENTHESIS);
		return;
	}

	if (strcmp(word, "disk") == 0) {
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_LEFT_PARENTHESIS);
		if (cur_machine_n_disk >= MAX_N_DISK) {
			fprintf(stderr, "too many disks\n");
			exit(1);
		}
		CHECK_ALLOCATION(cur_machine_disk[cur_machine_n_disk] =
		    (char*) malloc(MAX_DISK_LEN));
		read_one_word(f, cur_machine_disk[cur_machine_n_disk],
		    MAX_DISK_LEN, line, EXPECT_WORD);
		cur_machine_n_disk ++;
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_RIGHT_PARENTHESIS);
		return;
	}

	if (strcmp(word, "device") == 0) {
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_LEFT_PARENTHESIS);
		if (cur_machine_n_device >= MAX_N_DEVICE) {
			fprintf(stderr, "too many devices\n");
			exit(1);
		}
		CHECK_ALLOCATION(cur_machine_device[cur_machine_n_device] =
		    (char*) malloc(MAX_DEVICE_LEN));
		read_one_word(f, cur_machine_device[cur_machine_n_device],
		    MAX_DEVICE_LEN, line, EXPECT_WORD);
		cur_machine_n_device ++;
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_RIGHT_PARENTHESIS);
		return;
	}

	if (strcmp(word, "add_x11_display") == 0) {
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_LEFT_PARENTHESIS);
		if (cur_machine_n_x11_disp >= MAX_N_X11_DISP) {
			fprintf(stderr, "too many x11 displays\n");
			exit(1);
		}
		CHECK_ALLOCATION(cur_machine_x11_disp[cur_machine_n_x11_disp] =
		    (char*) malloc(MAX_X11_DISP_LEN));
		read_one_word(f, cur_machine_x11_disp[cur_machine_n_x11_disp],
		    MAX_X11_DISP_LEN, line, EXPECT_WORD);
		cur_machine_n_x11_disp ++;
		read_one_word(f, word, maxbuflen,
		    line, EXPECT_RIGHT_PARENTHESIS);
		return;
	}

	fatal("line %i: not expecting '%s' in a 'machine' section\n",
	    *line, word);
	exit(1);
}


/*
 *  emul_parse_config():
 *
 *  Set up an emulation by parsing a config file.
 */
void emul_parse_config(struct emul *e, char *fname)
{
	FILE *f = fopen(fname, "r");
	char word[MAX_WORD_LEN];
	int in_emul = 0;
	int line = 1;
	int parsestate = PARSESTATE_EMUL;

	/*  debug("emul_parse_config()\n");  */
	if (f == NULL) {
		perror(fname);
		exit(1);
	}

	while (!feof(f)) {
		read_one_word(f, word, sizeof(word), &line,
		    EXPECT_WORD | EXPECT_RIGHT_PARENTHESIS);
		if (!word[0])
			break;

		/*  debug("word = '%s'\n", word);  */

		switch (parsestate) {
		case PARSESTATE_EMUL:
			parse__emul(e, f, &in_emul, &line, &parsestate,
			    word, sizeof(word));
			break;
		case PARSESTATE_NET:
			parse__net(e, f, &in_emul, &line, &parsestate,
			    word, sizeof(word));
			break;
		case PARSESTATE_MACHINE:
			parse__machine(e, f, &in_emul, &line, &parsestate,
			    word, sizeof(word));
			break;
		case PARSESTATE_NONE:
			break;
		default:
			fatal("INTERNAL ERROR in emul_parse.c ("
			    "parsestate %i is not imlemented yet?)\n",
			    parsestate);
			exit(1);
		}
	}

	if (parenthesis_level != 0) {
		fatal("EOF but not enough right parentheses?\n");
		exit(1);
	}

	fclose(f);
}

