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
 *  Disk image support.
 *
 *  TODO:  diskimage_remove()? This would be useful for floppies in PC-style
 *	   machines, where disks may need to be swapped during boot etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "cpu.h"
#include "diskimage.h"
#include "machine.h"
#include "misc.h"


/*  #define debug fatal  */

extern int single_step;

static const char *diskimage_types[] = DISKIMAGE_TYPES;


/**************************************************************************/

/*
 *  my_fseek():
 *
 *  A helper function, like fseek() but takes off_t.  If the system has
 *  fseeko, then that is used. Otherwise I try to fake off_t offsets here.
 *
 *  The correct position is reached by seeking 2 billion bytes at a time
 *  (or less).  Note: This method is only used for SEEK_SET, for SEEK_CUR
 *  and SEEK_END, normal fseek() is used!
 *
 *  TODO: It seemed to work on Linux/i386, but not on Solaris/sparc (?).
 *  Anyway, most modern systems have fseeko(), so it shouldn't be a problem.
 */
static int my_fseek(FILE *f, off_t offset, int whence)
{
#ifdef HACK_FSEEKO
	if (whence == SEEK_SET) {
		int res = 0;
		off_t curoff = 0;
		off_t cur_step;

		fseek(f, 0, SEEK_SET);
		while (curoff < offset) {
			/*  How far to seek?  */
			cur_step = offset - curoff;
			if (cur_step > 2000000000)
				cur_step = 2000000000;
			res = fseek(f, cur_step, SEEK_CUR);
			if (res)
				return res;
			curoff += cur_step;
		}
		return 0;
	} else
		return fseek(f, offset, whence);
#else
	return fseeko(f, offset, whence);
#endif
}


/**************************************************************************/


/*
 *  diskimage_exist():
 *
 *  Returns 1 if the specified disk id (for a specific type) exists, 0
 *  otherwise.
 */
int diskimage_exist(struct machine *machine, int id, int type)
{
	struct diskimage *d = machine->first_diskimage;

	while (d != NULL) {
		if (d->type == type && d->id == id)
			return 1;
		d = d->next;
	}
	return 0;
}


/*
 *  diskimage_add_overlay():
 *
 *  Opens an overlay data file and its corresponding bitmap file, and adds
 *  the overlay to a disk image.
 */
void diskimage_add_overlay(struct diskimage *d, char *overlay_basename)
{
	struct diskimage_overlay overlay;
	size_t bitmap_name_len = strlen(overlay_basename) + 20;
	char *bitmap_name;

	CHECK_ALLOCATION(bitmap_name = (char *) malloc(bitmap_name_len));
	snprintf(bitmap_name, bitmap_name_len, "%s.map", overlay_basename);

	CHECK_ALLOCATION(overlay.overlay_basename = strdup(overlay_basename));
	overlay.f_data = fopen(overlay_basename, d->writable? "r+" : "r");
	if (overlay.f_data == NULL) {
		perror(overlay_basename);
		exit(1);
	}

	overlay.f_bitmap = fopen(bitmap_name, d->writable? "r+" : "r");
	if (overlay.f_bitmap == NULL) {
		perror(bitmap_name);
		fprintf(stderr, "Please create the map file first.\n");
		exit(1);
	}

	d->nr_of_overlays ++;

	CHECK_ALLOCATION(d->overlays = (struct diskimage_overlay *) realloc(d->overlays,
	    sizeof(struct diskimage_overlay) * d->nr_of_overlays));

	d->overlays[d->nr_of_overlays - 1] = overlay;

	free(bitmap_name);
}


/*
 *  diskimage_recalc_size():
 *
 *  Recalculate a disk's size by stat()-ing it.
 *  d is assumed to be non-NULL.
 */
void diskimage_recalc_size(struct diskimage *d)
{
	struct stat st;
	int res;
	off_t size = 0;

	res = stat(d->fname, &st);
	if (res) {
		fprintf(stderr, "[ diskimage_recalc_size(): could not stat "
		    "'%s' ]\n", d->fname);
		return;
	}

	size = st.st_size;

	/*
	 *  TODO:  CD-ROM devices, such as /dev/cd0c, how can one
	 *  check how much data is on that cd-rom without reading it?
	 *  For now, assume some large number, hopefully it will be
	 *  enough to hold any cd-rom image.
	 */
	if (d->is_a_cdrom && size == 0)
		size = 762048000;

	d->total_size = size;
	d->ncyls = d->total_size / 1048576;

	/*  TODO: There is a mismatch between d->ncyls and d->cylinders,
	    SCSI-based stuff usually doesn't care.  TODO: Fix this.  */
}


/*
 *  diskimage_getsize():
 *
 *  Returns -1 if the specified disk id/type does not exists, otherwise
 *  the size of the disk image is returned.
 */
int64_t diskimage_getsize(struct machine *machine, int id, int type)
{
	struct diskimage *d = machine->first_diskimage;

	while (d != NULL) {
		if (d->type == type && d->id == id)
			return d->total_size;
		d = d->next;
	}
	return -1;
}


/*
 *  diskimage_get_baseoffset():
 *
 *  Returns -1 if the specified disk id/type does not exists, otherwise
 *  the base offset of the disk image is returned.
 */
int64_t diskimage_get_baseoffset(struct machine *machine, int id, int type)
{
	struct diskimage *d = machine->first_diskimage;

	while (d != NULL) {
		if (d->type == type && d->id == id)
			return d->override_base_offset;
		d = d->next;
	}
	return -1;
}


/*
 *  diskimage_getchs():
 *
 *  Returns the current CHS values of a disk image.
 */
void diskimage_getchs(struct machine *machine, int id, int type,
	int *c, int *h, int *s)
{
	struct diskimage *d = machine->first_diskimage;

	while (d != NULL) {
		if (d->type == type && d->id == id) {
			*c = d->cylinders;
			*h = d->heads;
			*s = d->sectors_per_track;
			return;
		}
		d = d->next;
	}
	fatal("diskimage_getchs(): disk id %i (type %i) not found?\n",
	    id, diskimage_types[type]);
	exit(1);
}


/*
 *  diskimage_access__cdrom():
 *
 *  This is a special-case function, called from diskimage__internal_access().
 *  On my FreeBSD 4.9 system, the cdrom device /dev/cd0c seems to not be able
 *  to handle something like "fseek(512); fread(512);" but it handles
 *  "fseek(2048); fread(512);" just fine.  So, if diskimage__internal_access()
 *  fails in reading a block of data, this function is called as an attempt to
 *  align reads at 2048-byte sectors instead.
 *
 *  (Ugly hack.  TODO: how to solve this cleanly?)
 *
 *  NOTE:  Returns the number of bytes read, 0 if nothing was successfully
 *  read. (These are not the same as diskimage_access()).
 */
#define	CDROM_SECTOR_SIZE	2048
static size_t diskimage_access__cdrom(struct diskimage *d, off_t offset,
	unsigned char *buf, size_t len)
{
	off_t aligned_offset;
	size_t bytes_read, total_copied = 0;
	unsigned char cdrom_buf[CDROM_SECTOR_SIZE];
	off_t buf_ofs, i = 0;

	/*  printf("diskimage_access__cdrom(): offset=0x%llx size=%lli\n",
	    (long long)offset, (long long)len);  */

	aligned_offset = (offset / CDROM_SECTOR_SIZE) * CDROM_SECTOR_SIZE;
	my_fseek(d->f, aligned_offset, SEEK_SET);

	while (len != 0) {
		bytes_read = fread(cdrom_buf, 1, CDROM_SECTOR_SIZE, d->f);
		if (bytes_read != CDROM_SECTOR_SIZE)
			return 0;

		/*  Copy (part of) cdrom_buf into buf:  */
		buf_ofs = offset - aligned_offset;
		while (buf_ofs < CDROM_SECTOR_SIZE && len != 0) {
			buf[i ++] = cdrom_buf[buf_ofs ++];
			total_copied ++;
			len --;
		}

		aligned_offset += CDROM_SECTOR_SIZE;
		offset = aligned_offset;
	}

	return total_copied;
}


/*  Helper function.  */
static void overlay_set_block_in_use(struct diskimage *d,
	int overlay_nr, off_t ofs)
{
	off_t bit_nr = ofs / OVERLAY_BLOCK_SIZE;
	off_t bitmap_file_offset = bit_nr / 8;
	int res;
	unsigned char data;

	res = my_fseek(d->overlays[overlay_nr].f_bitmap,
	    bitmap_file_offset, SEEK_SET);
	if (res) {
		perror("my_fseek");
		fprintf(stderr, "Could not seek in bitmap file?"
		    " offset = %lli, read\n", (long long)bitmap_file_offset);
		exit(1);
	}

	/*  Read the original bitmap data, and OR in the new bit:  */
	res = fread(&data, 1, 1, d->overlays[overlay_nr].f_bitmap);
	if (res != 1)
		data = 0x00;

	data |= (1 << (bit_nr & 7));

	/*  Write it back:  */
	res = my_fseek(d->overlays[overlay_nr].f_bitmap,
	    bitmap_file_offset, SEEK_SET);
	if (res) {
		perror("my_fseek");
		fprintf(stderr, "Could not seek in bitmap file?"
		    " offset = %lli, write\n", (long long)bitmap_file_offset);
		exit(1);
	}
	res = fwrite(&data, 1, 1, d->overlays[overlay_nr].f_bitmap);
	if (res != 1) {
		fprintf(stderr, "Could not write to bitmap file. Aborting.\n");
		exit(1);
	}
}


/*  Helper function.  */
static int overlay_has_block(struct diskimage *d, int overlay_nr, off_t ofs)
{
	off_t bit_nr = ofs / OVERLAY_BLOCK_SIZE;
	off_t bitmap_file_offset = bit_nr / 8;
	int res;
	unsigned char data;

	res = my_fseek(d->overlays[overlay_nr].f_bitmap,
	    bitmap_file_offset, SEEK_SET);
	if (res != 0)
		return 0;

	/*  The seek succeeded, now read the bit:  */
	res = fread(&data, 1, 1, d->overlays[overlay_nr].f_bitmap);
	if (res != 1)
		return 0;

	if (data & (1 << (bit_nr & 7)))
		return 1;

	return 0;
}


/*
 *  fwrite_helper():
 *
 *  Internal helper function. Writes to a disk image file, or if the
 *  disk image has overlays, to the last overlay.
 */
static size_t fwrite_helper(off_t offset, unsigned char *buf,
	size_t len, struct diskimage *d)
{
	off_t curofs;

	/*  Fast return-path for the case when no overlays are used:  */
	if (d->nr_of_overlays == 0) {
		int res = my_fseek(d->f, offset, SEEK_SET);
		if (res != 0) {
			fatal("[ diskimage__internal_access(): fseek() failed"
			    " on disk id %i \n", d->id);
			return 0;
		}

		return fwrite(buf, 1, len, d->f);
	}

	if ((len & (OVERLAY_BLOCK_SIZE-1)) != 0) {
		fatal("TODO: overlay access (write), len not multiple of "
		    "overlay block size. not yet implemented.\n");
		fatal("len = %lli\n", (long long) len);
		abort();
	}
	if ((offset & (OVERLAY_BLOCK_SIZE-1)) != 0) {
		fatal("TODO: unaligned overlay access\n");
		fatal("offset = %lli\n", (long long) offset);
		abort();
	}

	/*  Split the write into OVERLAY_BLOCK_SIZE writes:  */
	for (curofs = offset; curofs < (off_t) (offset+len);
	     curofs += OVERLAY_BLOCK_SIZE) {
		/*  Always write to the last overlay:  */
		int overlay_nr = d->nr_of_overlays-1;
		off_t lenwritten;
		int res = my_fseek(d->overlays[overlay_nr].f_data,
		    curofs, SEEK_SET);
		if (res != 0) {
			fatal("[ diskimage__internal_access(): fseek()"
			    " failed on disk id %i \n", d->id);
			return 0;
		}

		lenwritten = fwrite(buf, 1, OVERLAY_BLOCK_SIZE,
		    d->overlays[overlay_nr].f_data);
		buf += OVERLAY_BLOCK_SIZE;

		/*  Mark this block in the last overlay as in use:  */
		overlay_set_block_in_use(d, overlay_nr, curofs);
	}

	return len;
}


/*
 *  fread_helper():
 *
 *  Internal helper function. Reads from a disk image file, or if the
 *  disk image has overlays, from the last overlay that has the specific
 *  data (or the disk image file itself).
 */
static size_t fread_helper(off_t offset, unsigned char *buf,
	size_t len, struct diskimage *d)
{
	off_t curofs;
	size_t totallenread = 0;

	/*  Fast return-path for the case when no overlays are used:  */
	if (d->nr_of_overlays == 0) {
		int res = my_fseek(d->f, offset, SEEK_SET);
		if (res != 0) {
			fatal("[ diskimage__internal_access(): fseek() failed"
			    " on disk id %i \n", d->id);
			return 0;
		}

		return fread(buf, 1, len, d->f);
	}

	/*  Split the read into OVERLAY_BLOCK_SIZE reads:  */
	for (curofs=offset; len != 0;
	    curofs = (curofs | (OVERLAY_BLOCK_SIZE-1)) + 1) {
		/*  Find the overlay, if any, that has this block:  */
		off_t lenread, lentoread;
		int overlay_nr;
		for (overlay_nr = d->nr_of_overlays-1;
		    overlay_nr >= 0; overlay_nr --) {
			if (overlay_has_block(d, overlay_nr, curofs))
				break;
		}

		lentoread = len > OVERLAY_BLOCK_SIZE? OVERLAY_BLOCK_SIZE : len;

		if (overlay_nr >= 0) {
			/*  Read from overlay:  */
			int res = my_fseek(d->overlays[overlay_nr].f_data,
			    curofs, SEEK_SET);
			if (res != 0) {
				fatal("[ diskimage__internal_access(): fseek()"
				    " failed on disk id %i \n", d->id);
				return 0;
			}
			lenread = fread(buf, 1, lentoread,
			    d->overlays[overlay_nr].f_data);
		} else {
			/*  Read from the base disk image:  */
			int res = my_fseek(d->f, curofs, SEEK_SET);
			if (res != 0) {
				fatal("[ diskimage__internal_access(): fseek()"
				    " failed on disk id %i \n", d->id);
				return 0;
			}
			lenread = fread(buf, 1, lentoread, d->f);
		}

		if (lenread != lentoread) {
			fatal("[ INCOMPLETE READ from disk id %i, offset"
			    " %lli ]\n", d->id, (long long)curofs);
		}

		len -= lentoread;
		totallenread += lenread;
		buf += OVERLAY_BLOCK_SIZE;
	}

	return totallenread;
}


/*
 *  diskimage__internal_access():
 *
 *  Read from or write to a struct diskimage.
 *
 *  Returns 1 if the access completed successfully, 0 otherwise.
 */
int diskimage__internal_access(struct diskimage *d, int writeflag,
	off_t offset, unsigned char *buf, size_t len)
{
	ssize_t lendone;

	if (buf == NULL) {
		fprintf(stderr, "diskimage__internal_access(): buf = NULL\n");
		exit(1);
	}
	if (len == 0)
		return 1;
	if (d->f == NULL)
		return 0;

	if (writeflag) {
		if (!d->writable)
			return 0;

		lendone = fwrite_helper(offset, buf, len, d);
	} else {
		/*
		 *  Special case for CD-ROMs. Actually, this is not needed
		 *  for .iso images, only for physical CDROMS on some OSes,
		 *  such as FreeBSD.
		 */
		if (d->is_a_cdrom)
			lendone = diskimage_access__cdrom(d, offset, buf, len);
		else
			lendone = fread_helper(offset, buf, len, d);

		if (lendone < (ssize_t)len)
			memset(buf + lendone, 0, len - lendone);
	}

	/*  Incomplete data transfer? Then return failure:  */
	if (lendone != (ssize_t)len) {
#ifdef UNSTABLE_DEVEL
		fatal
#else
		debug
#endif
		    ("[ diskimage__internal_access(): disk_id %i, offset %lli"
		    ", transfer not completed. len=%i, len_done=%i ]\n",
		    d->id, (long long)offset, (int)len, (int)lendone);
		return 0;
	}

	return 1;
}


/*
 *  diskimage_access():
 *
 *  Read from or write to a disk image on a machine.
 *
 *  Returns 1 if the access completed successfully, 0 otherwise.
 */
int diskimage_access(struct machine *machine, int id, int type, int writeflag,
	off_t offset, unsigned char *buf, size_t len)
{
	struct diskimage *d = machine->first_diskimage;

	while (d != NULL) {
		if (d->type == type && d->id == id)
			break;
		d = d->next;
	}

	if (d == NULL) {
		fatal("[ diskimage_access(): ERROR: trying to access a "
		    "non-existant %s disk image (id %i)\n",
		    diskimage_types[type], id);
		return 0;
	}

	offset -= d->override_base_offset;
	if (offset < 0 && offset + d->override_base_offset >= 0) {
		debug("[ reading before start of disk image ]\n");
		/*  Returning zeros.  */
		memset(buf, 0, len);
		return 1;
	}

	return diskimage__internal_access(d, writeflag, offset, buf, len);
}


/*
 *  diskimage_add():
 *
 *  Add a disk image.  fname is the filename of the disk image.
 *  The filename may be prefixed with one or more modifiers, followed
 *  by a colon.
 *
 *	b	specifies that this is a bootable device
 *	c	CD-ROM (instead of a normal DISK)
 *	d	DISK (this is the default)
 *	f	FLOPPY (instead of SCSI)
 *	gH;S;	set geometry (H=heads, S=sectors per track, cylinders are
 *		automatically calculated). (This is ignored for floppies.)
 *	i	IDE (instead of SCSI)
 *	oOFS;	set base offset in bytes, when booting from an ISO9660 fs
 *	r       read-only (don't allow changes to the file)
 *	s	SCSI (this is the default)
 *	t	tape
 *	V	add an overlay to a disk image
 *	0-7	force a specific SCSI ID number
 *
 *  machine is assumed to be non-NULL.
 *  Returns an integer >= 0 identifying the disk image.
 */
int diskimage_add(struct machine *machine, char *fname)
{
	struct diskimage *d, *d2;
	int id = 0, override_heads=0, override_spt=0;
	int64_t bytespercyl, override_base_offset=0;
	char *cp;
	int prefix_b=0, prefix_c=0, prefix_d=0, prefix_f=0, prefix_g=0;
	int prefix_i=0, prefix_r=0, prefix_s=0, prefix_t=0, prefix_id=-1;
	int prefix_o=0, prefix_V=0;

	if (fname == NULL) {
		fprintf(stderr, "diskimage_add(): NULL ptr\n");
		return 0;
	}

	/*  Get prefix from fname:  */
	cp = strchr(fname, ':');
	if (cp != NULL) {
		while (fname <= cp) {
			char c = *fname++;
			switch (c) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				prefix_id = c - '0';
				break;
			case 'b':
				prefix_b = 1;
				break;
			case 'c':
				prefix_c = 1;
				break;
			case 'd':
				prefix_d = 1;
				break;
			case 'f':
				prefix_f = 1;
				break;
			case 'g':
				prefix_g = 1;
				override_heads = atoi(fname);
				while (*fname != '\0' && *fname != ';')
					fname ++;
				if (*fname == ';')
					fname ++;
				override_spt = atoi(fname);
				while (*fname != '\0' && *fname != ';' &&
				    *fname != ':')
					fname ++;
				if (*fname == ';')
					fname ++;
				if (override_heads < 1 ||
				    override_spt < 1) {
					fatal("Bad geometry: heads=%i "
					    "spt=%i\n", override_heads,
					    override_spt);
					exit(1);
				}
				break;
			case 'i':
				prefix_i = 1;
				break;
			case 'o':
				prefix_o = 1;
				override_base_offset = atoi(fname);
				while (*fname != '\0' && *fname != ':'
				    && *fname != ';')
					fname ++;
				if (*fname == ':' || *fname == ';')
					fname ++;
				if (override_base_offset < 0) {
					fatal("Bad base offset: %"PRIi64
					    "\n", override_base_offset);
					exit(1);
				}
				break;
			case 'r':
				prefix_r = 1;
				break;
			case 's':
				prefix_s = 1;
				break;
			case 't':
				prefix_t = 1;
				break;
			case 'V':
				prefix_V = 1;
				break;
			case ':':
				break;
			default:
				fprintf(stderr, "diskimage_add(): invalid "
				    "prefix char '%c'\n", c);
				exit(1);
			}
		}
	}

	/*  Allocate a new diskimage struct:  */
	CHECK_ALLOCATION(d = (struct diskimage *) malloc(sizeof(struct diskimage)));
	memset(d, 0, sizeof(struct diskimage));

	/*  Default to IDE disks...  */
	d->type = DISKIMAGE_IDE;

	/*  ... but some machines use SCSI by default:  */
	if (machine->machine_type == MACHINE_PMAX ||
	    machine->machine_type == MACHINE_ARC ||
	    machine->machine_type == MACHINE_MVME88K)
		d->type = DISKIMAGE_SCSI;

	if (prefix_i + prefix_f + prefix_s > 1) {
		fprintf(stderr, "Invalid disk image prefix(es). You can"
		    "only use one of i, f, and s\nfor each disk image.\n");
		exit(1);
	}

	if (prefix_i)
		d->type = DISKIMAGE_IDE;
	if (prefix_f)
		d->type = DISKIMAGE_FLOPPY;
	if (prefix_s)
		d->type = DISKIMAGE_SCSI;

	/*  Special case: Add an overlay for an already added disk image:  */
	if (prefix_V) {
		struct diskimage *dx = machine->first_diskimage;

		if (prefix_id < 0) {
			fprintf(stderr, "The 'V' disk image prefix requires"
			    " a disk ID to also be supplied.\n");
			exit(1);
		}

		while (dx != NULL) {
			if (d->type == dx->type && prefix_id == dx->id)
				break;
			dx = dx->next;
		}

		if (dx == NULL) {
			fprintf(stderr, "Bad ID supplied for overlay?\n");
			exit(1);
		}

		diskimage_add_overlay(dx, fname);

		/*  Free the preliminary d struct:  */
		free(d);

		/*  Don't add any disk image. This is an overlay!  */
		return -1;
	}

	/*  Add the new disk image in the disk image chain:  */
	d2 = machine->first_diskimage;
	if (d2 == NULL) {
		machine->first_diskimage = d;
	} else {
		while (d2->next != NULL)
			d2 = d2->next;
		d2->next = d;
	}

	if (prefix_o)
		d->override_base_offset = override_base_offset;

	CHECK_ALLOCATION(d->fname = strdup(fname));

	d->logical_block_size = 512;

	/*
	 *  Is this a tape, CD-ROM or a normal disk?
	 *
	 *  An intelligent guess, if no prefixes are used, would be that
	 *  filenames ending with .iso or .cdr are CD-ROM images.
	 */
	if (prefix_t) {
		d->is_a_tape = 1;
	} else {
		if (prefix_c ||
		    ((strlen(d->fname) > 4 &&
		    (strcasecmp(d->fname + strlen(d->fname) - 4, ".cdr") == 0 ||
		    strcasecmp(d->fname + strlen(d->fname) - 4, ".iso") == 0))
		    && !prefix_d)
		   ) {
			d->is_a_cdrom = 1;

			/*
			 *  This is tricky. Should I use 512 or 2048 here?
			 *  NetBSD/pmax 1.6.2 and Ultrix likes 512 bytes
			 *  per sector, but NetBSD 2.0_BETA suddenly ignores
			 *  this value and uses 2048 instead.
			 *
			 *  OpenBSD/arc doesn't like 2048, it requires 512
			 *  to work correctly.
			 *
			 *  TODO
			 */

#if 0
			if (machine->machine_type == MACHINE_PMAX)
				d->logical_block_size = 512;
			else
				d->logical_block_size = 2048;
#endif
			d->logical_block_size = 512;
		}
	}

	diskimage_recalc_size(d);

	if ((d->total_size == 720*1024 || d->total_size == 1474560
	    || d->total_size == 2949120 || d->total_size == 1228800)
	    && !prefix_i && !prefix_s)
		d->type = DISKIMAGE_FLOPPY;

	switch (d->type) {
	case DISKIMAGE_FLOPPY:
		if (d->total_size < 737280) {
			fatal("\nTODO: small (non-80-cylinder) floppies?\n\n");
			exit(1);
		}
		d->cylinders = 80;
		d->heads = 2;
		d->sectors_per_track = d->total_size / (d->cylinders *
		    d->heads * 512);
		break;
	default:/*  Non-floppies:  */
		d->heads = 16;
		d->sectors_per_track = 63;
		if (prefix_g) {
			d->chs_override = 1;
			d->heads = override_heads;
			d->sectors_per_track = override_spt;
		}
		bytespercyl = d->heads * d->sectors_per_track * 512;
		d->cylinders = d->total_size / bytespercyl;
		if (d->cylinders * bytespercyl < d->total_size)
			d->cylinders ++;
	}

	d->rpms = 3600;

	if (prefix_b)
		d->is_boot_device = 1;

	d->writable = access(fname, W_OK) == 0? 1 : 0;

	if (d->is_a_cdrom || prefix_r)
		d->writable = 0;

	d->f = fopen(fname, d->writable? "r+" : "r");
	if (d->f == NULL) {
		char *errmsg = (char *) malloc(200 + strlen(fname));
		snprintf(errmsg, 200+strlen(fname),
		    "could not fopen %s for reading%s", fname,
		    d->writable? " and writing" : "");
		perror(errmsg);
		exit(1);
	}

	/*  Calculate which ID to use:  */
	if (prefix_id == -1) {
		int free = 0, collision = 1;

		while (collision) {
			collision = 0;
			d2 = machine->first_diskimage;
			while (d2 != NULL) {
				/*  (don't compare against ourselves :)  */
				if (d2 == d) {
					d2 = d2->next;
					continue;
				}
				if (d2->id == free && d2->type == d->type) {
					collision = 1;
					break;
				}
				d2 = d2->next;
			}
			if (!collision)
				id = free;
			else
				free ++;
		}
	} else {
		id = prefix_id;
		d2 = machine->first_diskimage;
		while (d2 != NULL) {
			/*  (don't compare against ourselves :)  */
			if (d2 == d) {
				d2 = d2->next;
				continue;
			}
			if (d2->id == id && d2->type == d->type) {
				fprintf(stderr, "disk image id %i "
				    "already in use\n", id);
				exit(1);
			}
			d2 = d2->next;
		}
	}

	d->id = id;

	return id;
}


/*
 *  diskimage_bootdev():
 *
 *  Returns the disk id of the device which we're booting from.  If typep is
 *  non-NULL, the type is returned as well.
 *
 *  If no disk was used as boot device, then -1 is returned. (In practice,
 *  this is used to fake network (tftp) boot.)
 */
int diskimage_bootdev(struct machine *machine, int *typep)
{
	struct diskimage *d;

	d = machine->first_diskimage;
	while (d != NULL) {
		if (d->is_boot_device) {
			if (typep != NULL)
				*typep = d->type;
			return d->id;
		}
		d = d->next;
	}

	d = machine->first_diskimage;
	if (d != NULL) {
		if (typep != NULL)
			*typep = d->type;
		return d->id;
	}

	return -1;
}


/*
 *  diskimage_getname():
 *
 *  Returns 1 if a valid disk image name was returned, 0 otherwise.
 */
int diskimage_getname(struct machine *machine, int id, int type,
	char *buf, size_t bufsize)
{
	struct diskimage *d = machine->first_diskimage;

	if (buf == NULL)
		return 0;

	while (d != NULL) {
		if (d->type == type && d->id == id) {
			char *p = strrchr(d->fname, '/');
			if (p == NULL)
				p = d->fname;
			else
				p ++;
			snprintf(buf, bufsize, "%s", p);
			return 1;
		}
		d = d->next;
	}
	return 0;
}


/*
 *  diskimage_is_a_cdrom():
 *
 *  Returns 1 if a disk image is a CDROM, 0 otherwise.
 */
int diskimage_is_a_cdrom(struct machine *machine, int id, int type)
{
	struct diskimage *d = machine->first_diskimage;

	while (d != NULL) {
		if (d->type == type && d->id == id)
			return d->is_a_cdrom;
		d = d->next;
	}
	return 0;
}


/*
 *  diskimage_is_a_tape():
 *
 *  Returns 1 if a disk image is a tape, 0 otherwise.
 *
 *  (Used in src/machine.c, to select 'rz' vs 'tz' for DECstation
 *  boot strings.)
 */
int diskimage_is_a_tape(struct machine *machine, int id, int type)
{
	struct diskimage *d = machine->first_diskimage;

	while (d != NULL) {
		if (d->type == type && d->id == id)
			return d->is_a_tape;
		d = d->next;
	}
	return 0;
}


/*
 *  diskimage_dump_info():
 *
 *  Debug dump of all diskimages that are loaded for a specific machine.
 */
void diskimage_dump_info(struct machine *machine)
{
	int i, iadd = DEBUG_INDENTATION;
	struct diskimage *d = machine->first_diskimage;

	while (d != NULL) {
		debug("diskimage: %s\n", d->fname);
		debug_indentation(iadd);

		switch (d->type) {
		case DISKIMAGE_SCSI:
			debug("SCSI");
			break;
		case DISKIMAGE_IDE:
			debug("IDE");
			break;
		case DISKIMAGE_FLOPPY:
			debug("FLOPPY");
			break;
		default:
			debug("UNKNOWN type %i", d->type);
		}

		debug(" %s", d->is_a_tape? "TAPE" :
			(d->is_a_cdrom? "CD-ROM" : "DISK"));
		debug(" id %i, ", d->id);
		debug("%s, ", d->writable? "read/write" : "read-only");

		if (d->type == DISKIMAGE_FLOPPY)
			debug("%lli KB", (long long) (d->total_size / 1024));
		else
			debug("%lli MB", (long long) (d->total_size / 1048576));

		if (d->type == DISKIMAGE_FLOPPY || d->chs_override)
			debug(" (CHS=%i,%i,%i)", d->cylinders, d->heads,
			    d->sectors_per_track);
		else
			debug(" (%lli sectors)", (long long)
			   (d->total_size / 512));

		if (d->is_boot_device)
			debug(" (BOOT)");
		debug("\n");

		for (i=0; i<d->nr_of_overlays; i++) {
			debug("overlay %i: %s\n",
			    i, d->overlays[i].overlay_basename);
		}

		debug_indentation(-iadd);

		d = d->next;
	}
}

