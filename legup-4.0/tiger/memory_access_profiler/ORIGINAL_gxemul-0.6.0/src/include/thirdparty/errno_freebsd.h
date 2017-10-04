/*-
 * Copyright (c) 1982, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)errno.h	8.5 (Berkeley) 1/21/94
 * $FreeBSD: src/sys/sys/errno.h,v 1.28 2005/04/02 12:33:28 das Exp $
 */

#ifndef _SYS_ERRNO_H_
#define _SYS_ERRNO_H_

#if 0
#ifndef _KERNEL
#include <sys/cdefs.h>
__BEGIN_DECLS
int *	__error(void);
__END_DECLS
#define	errno		(* __error())
#endif
#endif

#define FREEBSD_EPERM		1		/* Operation not permitted */
#define FREEBSD_ENOENT		2		/* No such file or directory */
#define FREEBSD_ESRCH		3		/* No such process */
#define FREEBSD_EINTR		4		/* Interrupted system call */
#define FREEBSD_EIO		5		/* Input/output error */
#define FREEBSD_ENXIO		6		/* Device not configured */
#define FREEBSD_E2BIG		7		/* Argument list too long */
#define FREEBSD_ENOEXEC		8		/* Exec format error */
#define FREEBSD_EBADF		9		/* Bad file descriptor */
#define FREEBSD_ECHILD		10		/* No child processes */
#define FREEBSD_EDEADLK		11		/* Resource deadlock avoided */
					/* 11 was EAGAIN */
#define FREEBSD_ENOMEM		12		/* Cannot allocate memory */
#define FREEBSD_EACCES		13		/* Permission denied */
#define FREEBSD_EFAULT		14		/* Bad address */
#ifndef _POSIX_SOURCE
#define FREEBSD_ENOTBLK		15		/* Block device required */
#endif
#define FREEBSD_EBUSY		16		/* Device busy */
#define FREEBSD_EEXIST		17		/* File exists */
#define FREEBSD_EXDEV		18		/* Cross-device link */
#define FREEBSD_ENODEV		19		/* Operation not supported by device */
#define FREEBSD_ENOTDIR		20		/* Not a directory */
#define FREEBSD_EISDIR		21		/* Is a directory */
#define FREEBSD_EINVAL		22		/* Invalid argument */
#define FREEBSD_ENFILE		23		/* Too many open files in system */
#define FREEBSD_EMFILE		24		/* Too many open files */
#define FREEBSD_ENOTTY		25		/* Inappropriate ioctl for device */
#ifndef _POSIX_SOURCE
#define FREEBSD_ETXTBSY		26		/* Text file busy */
#endif
#define FREEBSD_EFBIG		27		/* File too large */
#define FREEBSD_ENOSPC		28		/* No space left on device */
#define FREEBSD_ESPIPE		29		/* Illegal seek */
#define FREEBSD_EROFS		30		/* Read-only filesystem */
#define FREEBSD_EMLINK		31		/* Too many links */
#define FREEBSD_EPIPE		32		/* Broken pipe */

/* math software */
#define FREEBSD_EDOM		33		/* Numerical argument out of domain */
#define FREEBSD_ERANGE		34		/* Result too large */

/* non-blocking and interrupt i/o */
#define FREEBSD_EAGAIN		35		/* Resource temporarily unavailable */
#ifndef _POSIX_SOURCE
#define FREEBSD_EWOULDBLOCK	EAGAIN		/* Operation would block */
#define FREEBSD_EINPROGRESS	36		/* Operation now in progress */
#define FREEBSD_EALREADY	37		/* Operation already in progress */

/* ipc/network software -- argument errors */
#define FREEBSD_ENOTSOCK	38		/* Socket operation on non-socket */
#define FREEBSD_EDESTADDRREQ	39		/* Destination address required */
#define FREEBSD_EMSGSIZE	40		/* Message too long */
#define FREEBSD_EPROTOTYPE	41		/* Protocol wrong type for socket */
#define FREEBSD_ENOPROTOOPT	42		/* Protocol not available */
#define FREEBSD_EPROTONOSUPPORT	43		/* Protocol not supported */
#define FREEBSD_ESOCKTNOSUPPORT	44		/* Socket type not supported */
#define FREEBSD_EOPNOTSUPP	45		/* Operation not supported */
#define FREEBSD_ENOTSUP		EOPNOTSUPP	/* Operation not supported */
#define FREEBSD_EPFNOSUPPORT	46		/* Protocol family not supported */
#define FREEBSD_EAFNOSUPPORT	47		/* Address family not supported by protocol family */
#define FREEBSD_EADDRINUSE	48		/* Address already in use */
#define FREEBSD_EADDRNOTAVAIL	49		/* Can't assign requested address */

/* ipc/network software -- operational errors */
#define FREEBSD_ENETDOWN	50		/* Network is down */
#define FREEBSD_ENETUNREACH	51		/* Network is unreachable */
#define FREEBSD_ENETRESET	52		/* Network dropped connection on reset */
#define FREEBSD_ECONNABORTED	53		/* Software caused connection abort */
#define FREEBSD_ECONNRESET	54		/* Connection reset by peer */
#define FREEBSD_ENOBUFS		55		/* No buffer space available */
#define FREEBSD_EISCONN		56		/* Socket is already connected */
#define FREEBSD_ENOTCONN	57		/* Socket is not connected */
#define FREEBSD_ESHUTDOWN	58		/* Can't send after socket shutdown */
#define FREEBSD_ETOOMANYREFS	59		/* Too many references: can't splice */
#define FREEBSD_ETIMEDOUT	60		/* Operation timed out */
#define FREEBSD_ECONNREFUSED	61		/* Connection refused */

#define FREEBSD_ELOOP		62		/* Too many levels of symbolic links */
#endif /* _POSIX_SOURCE */
#define FREEBSD_ENAMETOOLONG	63		/* File name too long */

/* should be rearranged */
#ifndef _POSIX_SOURCE
#define FREEBSD_EHOSTDOWN	64		/* Host is down */
#define FREEBSD_EHOSTUNREACH	65		/* No route to host */
#endif /* _POSIX_SOURCE */
#define FREEBSD_ENOTEMPTY	66		/* Directory not empty */

/* quotas & mush */
#ifndef _POSIX_SOURCE
#define FREEBSD_EPROCLIM	67		/* Too many processes */
#define FREEBSD_EUSERS		68		/* Too many users */
#define FREEBSD_EDQUOT		69		/* Disc quota exceeded */

/* Network File System */
#define FREEBSD_ESTALE		70		/* Stale NFS file handle */
#define FREEBSD_EREMOTE		71		/* Too many levels of remote in path */
#define FREEBSD_EBADRPC		72		/* RPC struct is bad */
#define FREEBSD_ERPCMISMATCH	73		/* RPC version wrong */
#define FREEBSD_EPROGUNAVAIL	74		/* RPC prog. not avail */
#define FREEBSD_EPROGMISMATCH	75		/* Program version wrong */
#define FREEBSD_EPROCUNAVAIL	76		/* Bad procedure for program */
#endif /* _POSIX_SOURCE */

#define FREEBSD_ENOLCK		77		/* No locks available */
#define FREEBSD_ENOSYS		78		/* Function not implemented */

#ifndef _POSIX_SOURCE
#define FREEBSD_EFTYPE		79		/* Inappropriate file type or format */
#define FREEBSD_EAUTH		80		/* Authentication error */
#define FREEBSD_ENEEDAUTH	81		/* Need authenticator */
#define FREEBSD_EIDRM		82		/* Identifier removed */
#define FREEBSD_ENOMSG		83		/* No message of desired type */
#define FREEBSD_EOVERFLOW	84		/* Value too large to be stored in data type */
#define FREEBSD_ECANCELED	85		/* Operation canceled */
#define FREEBSD_EILSEQ		86		/* Illegal byte sequence */
#define FREEBSD_ENOATTR		87		/* Attribute not found */

#define FREEBSD_EDOOFUS		88		/* Programming error */
#endif /* _POSIX_SOURCE */

#define FREEBSD_EBADMSG		89		/* Bad message */
#define FREEBSD_EMULTIHOP	90		/* Multihop attempted */
#define FREEBSD_ENOLINK		91		/* Link has been severed */
#define FREEBSD_EPROTO		92		/* Protocol error */

#ifndef _POSIX_SOURCE
#define FREEBSD_ELAST		92		/* Must be equal largest errno */
#endif /* _POSIX_SOURCE */

#ifdef _KERNEL
/* pseudo-errors returned inside kernel to modify return to process */
#define FREEBSD_ERESTART	(-1)		/* restart syscall */
#define FREEBSD_EJUSTRETURN	(-2)		/* don't modify regs, just return */
#define FREEBSD_ENOIOCTL	(-3)		/* ioctl not handled by this layer */
#define FREEBSD_EDIRIOCTL	(-4)		/* do direct ioctl in GEOM */
#endif

#endif
