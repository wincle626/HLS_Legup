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
 *  ISO9660 CD-ROM "bootblock" handling.
 *
 *  There is really no bootblock; instead, the file which is to be booted
 *  is extracted into a temporary file, and started as if it was given
 *  as a normal file argument on the command line.
 *
 *  TODO: This is really ugly. It's a quick hack. All the magic constants
 *        need to be replaced with real code!
 *
 *        Instead of the current "could not find" message, it should really
 *        be more helpful, and print out the files found in the current
 *        directory, so that one can more easily choose the correct file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "cpu.h"
#include "diskimage.h"
#include "machine.h"
#include "misc.h"

/*  #define ISO_DEBUG  */


static void debug_print_volume_id_and_filename(int iso_type,
	unsigned char *buf, char *filename)
{
	/*  Volume ID:  */
	char str[35];
	int i, ofs = iso_type == 3? 48 : 40;

	memcpy(str, buf + ofs, sizeof(str));
	str[32] = '\0';  i = 31;

	while (i >= 0 && str[i]==' ')
		str[i--] = '\0';
	if (str[0])
		debug("\"%s\"", str);
	else {
		/*  System ID:  */
		ofs = iso_type == 3? 16 : 8;
		memcpy(str, buf + ofs, sizeof(str));
		str[32] = '\0';  i = 31;
		while (i >= 0 && str[i]==' ')
			str[i--] = '\0';
		if (str[0])
			debug("\"%s\"", str);
		else
			debug("(no ID)");
	}

	debug(":%s\n", filename);
}


/*
 *  iso_load_bootblock():
 *
 *  Try to load a kernel from an ISO 9660 disk image. iso_type is 1 for
 *  "CD001" (standard), 2 for "CDW01" (ECMA), and 3 for "CDROM" (Sierra).
 *
 *  TODO: This function uses too many magic offsets and so on; it should be
 *  cleaned up some day.
 *
 *  Returns 1 on success, 0 on failure.
 */
int iso_load_bootblock(struct machine *m, struct cpu *cpu,
	int disk_id, int disk_type, int iso_type, unsigned char *buf,
	int *n_loadp, char ***load_namesp)
{
	int filenr, dirlen, res = 0, res2, iadd = DEBUG_INDENTATION, found_dir;
	uint64_t dirofs, fileofs;
	ssize_t filelen;
	unsigned char *dirbuf = NULL, *dp, *match_entry = NULL, *filebuf = NULL;
	char *p, *filename_orig, *filename, *tmpfname = NULL;
	char **new_array;
	const char *tmpdir = getenv("TMPDIR");
	int tmpfile_handle;

	if (tmpdir == NULL)
		tmpdir = DEFAULT_TMP_DIR;

	CHECK_ALLOCATION(filename = strdup(cpu->machine->boot_kernel_filename));
	filename_orig = filename;

	debug("ISO9660 boot:\n");
	debug_indentation(iadd);

	debug_print_volume_id_and_filename(iso_type, buf, filename);


	/*
	 *  Traverse the directory structure to find the kernel.
	 */

	dirlen = buf[0x84] + 256*buf[0x85] + 65536*buf[0x86];
	if (dirlen != buf[0x8b] + 256*buf[0x8a] + 65536*buf[0x89])
		fatal("WARNING: Root directory length mismatch?\n");

	dirofs = (int64_t)(buf[0x8c] + (buf[0x8d] << 8) + (buf[0x8e] << 16) +
	    ((uint64_t)buf[0x8f] << 24)) * 2048;

#ifdef ISO_DEBUG
	debug("root = %i bytes at 0x%llx\n", dirlen, (long long)dirofs);
#endif

	CHECK_ALLOCATION(dirbuf = (unsigned char *) malloc(dirlen));
	res2 = diskimage_access(m, disk_id, disk_type, 0, dirofs, dirbuf,
	    dirlen);
	if (!res2) {
		fatal("Couldn't read the disk image. Aborting.\n");
		goto ret;
	}

	found_dir = 1;	/*  Assume root dir  */
	dp = dirbuf; filenr = 1;
	p = NULL;
	while (dp < dirbuf + dirlen) {
		size_t i, nlen = dp[0];
		int x = dp[2] + (dp[3] << 8) + (dp[4] << 16) +
		    ((uint64_t)dp[5] << 24);
		int y = dp[6] + (dp[7] << 8);
		char direntry[65];

		dp += 8;

		/*
		 *  As long as there is an \ or / in the filename, then we
		 *  have not yet found the directory.
		 */
		p = strchr(filename, '/');
		if (p == NULL)
			p = strchr(filename, '\\');

#ifdef ISO_DEBUG
		debug("%i%s: %i, %i, \"", filenr, filenr == found_dir?
		    " [CURRENT]" : "", x, y);
#endif
		for (i=0; i<nlen && i<sizeof(direntry)-1; i++)
			if (dp[i]) {
				direntry[i] = dp[i];
#ifdef ISO_DEBUG
				debug("%c", dp[i]);
#endif
			} else
				break;
#ifdef ISO_DEBUG
		debug("\"\n");
#endif
		direntry[i] = '\0';

		/*  A directory name match?  */
		if ((p != NULL && strncasecmp(filename, direntry, nlen) == 0
		    && nlen == (size_t)p - (size_t)filename && found_dir == y)
		    || (p == NULL && direntry[0] == '\0') ) {
			found_dir = filenr;
			if (p != NULL)
				filename = p+1;
			dirofs = 2048 * (int64_t)x;
		}

		dp += nlen;

		/*  16-bit aligned lenght:  */
		if (nlen & 1)
			dp ++;

		filenr ++;
	}

	p = strchr(filename, '/');
	if (p == NULL)
		p = strchr(filename, '\\');

	if (p != NULL) {
		char *blah = filename_orig;

		fatal("could not find '%s' in /", filename);

		/*  Print the first part of the filename:  */
		while (blah != filename)
			fatal("%c", *blah++);
		
		fatal("\n");
		goto ret;
	}

	/*  debug("dirofs = 0x%llx\n", (long long)dirofs);  */

	/*  Free the old dirbuf, and allocate a new one:  */
	free(dirbuf);
	CHECK_ALLOCATION(dirbuf = (unsigned char *) malloc(512));

	for (;;) {
		size_t len, i;

		/*  Too close to another sector? Then realign.  */
		if ((dirofs & 2047) + 70 > 2047) {
			dirofs = (dirofs | 2047) + 1;
			/*  debug("realign dirofs = 0x%llx\n", dirofs);  */
		}

		res2 = diskimage_access(m, disk_id, disk_type, 0, dirofs,
		    dirbuf, 256);
		if (!res2) {
			fatal("Couldn't read the disk image. Aborting.\n");
			goto ret;
		}

		dp = dirbuf;
		len = dp[0];
		if (len < 2)
			break;

		/*
		 *  TODO: Actually parse the directory entry!
		 *
		 *  Haha, this must be rewritten.
		 */

#if 0
/*  hahahaha  */
printf("filename = '%s'\n", filename);
{
int j;
for (j=32; j<len; j++)
	printf("%c", dp[j] >= ' ' && dp[j] < 128? dp[j] : '.');
printf("\n");
}
#endif

		for (i=32; i<len; i++) {
			if (i < len - strlen(filename))
				if (strncmp(filename, (char *)dp + i,
				    strlen(filename)) == 0) {
					/*  The filename was found somewhere
					    in the directory entry.  */
					if (match_entry != NULL) {
						fatal("TODO: I'm too lazy to"
						    " implement a correct "
						    "directory parser right "
						    "now... (BUG)\n");
						exit(1);
					}
					CHECK_ALLOCATION(match_entry = (unsigned char *)
					    malloc(512));
					memcpy(match_entry, dp, 512);
					break;
				}
		}

		if (match_entry != NULL)
			break;

		dirofs += len;
	}

	if (match_entry == NULL) {
		char *blah = filename_orig;

		fatal("could not find '%s' in /", filename);

		/*  Print the first part of the filename:  */
		while (blah != filename)
			fatal("%c", *blah++);
		
		fatal("\n");
		goto ret;
	}

	fileofs = match_entry[2] + (match_entry[3] << 8) +
	    (match_entry[4] << 16) + ((uint64_t)match_entry[5] << 24);
	filelen = match_entry[10] + (match_entry[11] << 8) +
	    (match_entry[12] << 16) + ((uint64_t)match_entry[13] << 24);
	fileofs *= 2048;

	/*  debug("filelen=%llx fileofs=%llx\n", (long long)filelen,
	    (long long)fileofs);  */

	CHECK_ALLOCATION(filebuf = (unsigned char *) malloc(filelen));

	CHECK_ALLOCATION(tmpfname = (char *) malloc(300));
	snprintf(tmpfname, 300, "%s/gxemul.XXXXXXXXXXXX", tmpdir);

	res2 = diskimage_access(m, disk_id, disk_type, 0, fileofs, filebuf,
	    filelen);
	if (!res2) {
		fatal("could not read the file from the disk image!\n");
		goto ret;
	}

	tmpfile_handle = mkstemp(tmpfname);
	if (tmpfile_handle < 0) {
		fatal("could not create %s\n", tmpfname);
		exit(1);
	}

	if (write(tmpfile_handle, filebuf, filelen) != filelen) {
		fatal("could not write to %s\n", tmpfname);
		perror("write");
		exit(1);
	}

	close(tmpfile_handle);

	debug("extracted %lli bytes into %s\n", (long long)filelen, tmpfname);

	/*  Add the temporary filename to the load_namesp array:  */
	(*n_loadp)++;
	CHECK_ALLOCATION(new_array = (char **) malloc(sizeof(char *) * (*n_loadp)));
	memcpy(new_array, *load_namesp, sizeof(char *) * (*n_loadp));
	*load_namesp = new_array;

	/*  This adds a Backspace char in front of the filename; this
	    is a special hack which causes the file to be removed once
	    it has been loaded.  */
	CHECK_ALLOCATION(tmpfname = (char *) realloc(tmpfname, strlen(tmpfname) + 2));
	memmove(tmpfname + 1, tmpfname, strlen(tmpfname) + 1);
	tmpfname[0] = 8;

	(*load_namesp)[*n_loadp - 1] = strdup(tmpfname);

	res = 1;

ret:
	if (dirbuf != NULL)
		free(dirbuf);

	if (filebuf != NULL)
		free(filebuf);

	if (match_entry != NULL)
		free(match_entry);

	if (tmpfname != NULL)
		free(tmpfname);

	free(filename_orig);

	debug_indentation(-iadd);
	return res;
}

