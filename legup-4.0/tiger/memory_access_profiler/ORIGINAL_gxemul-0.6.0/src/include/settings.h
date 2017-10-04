#ifndef	SETTINGS_H
#define	SETTINGS_H

/*
 *  Copyright (C) 2006-2010  Anders Gavare.  All rights reserved.
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
 */

#include <inttypes.h>

#define	GLOBAL_SETTINGS_NAME	"settings"

struct settings;

/*  Storage types:  */
#define	SETTINGS_TYPE_SUBSETTINGS	1
#define	SETTINGS_TYPE_STRING		2
#define	SETTINGS_TYPE_INT		3
#define	SETTINGS_TYPE_INT8		4
#define	SETTINGS_TYPE_INT16		5
#define	SETTINGS_TYPE_INT32		6
#define	SETTINGS_TYPE_INT64		7
#define	SETTINGS_TYPE_UINT		8
#define	SETTINGS_TYPE_UINT8		9
#define	SETTINGS_TYPE_UINT16		10
#define	SETTINGS_TYPE_UINT32		11
#define	SETTINGS_TYPE_UINT64		12

/*  Presentation formats:  */
#define	SETTINGS_FORMAT_DECIMAL		1	/*  -123  */
#define	SETTINGS_FORMAT_HEX8		2	/*  0x12  */
#define	SETTINGS_FORMAT_HEX16		3	/*  0x1234  */
#define	SETTINGS_FORMAT_HEX32		4	/*  0x80000000  */
#define	SETTINGS_FORMAT_HEX64		5	/*  0xffffffff80000000  */
#define	SETTINGS_FORMAT_BOOL		6	/*  true, false  */
#define	SETTINGS_FORMAT_YESNO		7	/*  yes, no  */
#define	SETTINGS_FORMAT_STRING		8	/*  %s  */


/*
 *  settings.c:
 */

struct settings *settings_new(void);
void settings_destroy(struct settings *settings);

void settings_debugdump(struct settings *settings, const char *prefix,
	int recurse);

void settings_add(struct settings *settings, const char *name, int writable,
	int type, int format, void *ptr);
void settings_remove(struct settings *settings, const char *name);
void settings_remove_all(struct settings *settings);

int settings_access(struct settings *settings, const char *fullname,
	int writeflag, uint64_t *valuep);

/*  Result codes from settings_access:  */
#define	SETTINGS_OK			1
#define	SETTINGS_NAME_NOT_FOUND		2
#define	SETTINGS_READONLY		3


#endif	/*  SETTINGS_H  */
