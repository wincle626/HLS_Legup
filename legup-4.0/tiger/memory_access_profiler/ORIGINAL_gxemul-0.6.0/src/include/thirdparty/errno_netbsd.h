/*  GXemul: $Id: errno_netbsd.h,v 1.1 2007-06-15 01:08:13 debug Exp $  */
/*	$NetBSD: errno.h,v 1.39 2006/10/31 00:38:07 cbiere NETBSD_Exp $	*/

#ifndef ERRNO_NETBSD_H
#define ERRNO_NETBSD_H

/*
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
 * 3. Neither the name of the University nor the names of its contributors
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
 */

#define NETBSD_EPERM		1		/* Operation not permitted */
#define NETBSD_ENOENT		2		/* No such file or directory */
#define NETBSD_ESRCH		3		/* No such process */
#define NETBSD_EINTR		4		/* Interrupted system call */
#define NETBSD_EIO		5		/* Input/output error */
#define NETBSD_ENXIO		6		/* Device not configured */
#define NETBSD_E2BIG		7		/* Argument list too long */
#define NETBSD_ENOEXEC		8		/* Exec format error */
#define NETBSD_EBADF		9		/* Bad file descriptor */
#define NETBSD_ECHILD		10		/* No child processes */
#define NETBSD_EDEADLK		11		/* Resource deadlock avoided */
					/* 11 was EAGAIN */
#define NETBSD_ENOMEM		12		/* Cannot allocate memory */
#define NETBSD_EACCES		13		/* Permission denied */
#define NETBSD_EFAULT		14		/* Bad address */
#define NETBSD_ENOTBLK		15		/* Block device required */
#define NETBSD_EBUSY		16		/* Device busy */
#define NETBSD_EEXIST		17		/* File exists */
#define NETBSD_EXDEV		18		/* Cross-device link */
#define NETBSD_ENODEV		19		/* Operation not supported by device */
#define NETBSD_ENOTDIR		20		/* Not a directory */
#define NETBSD_EISDIR		21		/* Is a directory */
#define NETBSD_EINVAL		22		/* Invalid argument */
#define NETBSD_ENFILE		23		/* Too many open files in system */
#define NETBSD_EMFILE		24		/* Too many open files */
#define NETBSD_ENOTTY		25		/* Inappropriate ioctl for device */
#define NETBSD_ETXTBSY		26		/* Text file busy */
#define NETBSD_EFBIG		27		/* File too large */
#define NETBSD_ENOSPC		28		/* No space left on device */
#define NETBSD_ESPIPE		29		/* Illegal seek */
#define NETBSD_EROFS		30		/* Read-only file system */
#define NETBSD_EMLINK		31		/* Too many links */
#define NETBSD_EPIPE		32		/* Broken pipe */

/* math software */
#define NETBSD_EDOM		33		/* Numerical argument out of domain */
#define NETBSD_ERANGE		34		/* Result too large or too small */

/* non-blocking and interrupt i/o */
#define NETBSD_EAGAIN		35		/* Resource temporarily unavailable */
#define NETBSD_EWOULDBLOCK	EAGAIN		/* Operation would block */
#define NETBSD_EINPROGRESS	36		/* Operation now in progress */
#define NETBSD_EALREADY	37		/* Operation already in progress */

/* ipc/network software -- argument errors */
#define NETBSD_ENOTSOCK	38		/* Socket operation on non-socket */
#define NETBSD_EDESTADDRREQ	39		/* Destination address required */
#define NETBSD_EMSGSIZE	40		/* Message too long */
#define NETBSD_EPROTOTYPE	41		/* Protocol wrong type for socket */
#define NETBSD_ENOPROTOOPT	42		/* Protocol option not available */
#define NETBSD_EPROTONOSUPPORT	43		/* Protocol not supported */
#define NETBSD_ESOCKTNOSUPPORT	44		/* Socket type not supported */
#define NETBSD_EOPNOTSUPP	45		/* Operation not supported */
#define NETBSD_EPFNOSUPPORT	46		/* Protocol family not supported */
#define NETBSD_EAFNOSUPPORT	47		/* Address family not supported by protocol family */
#define NETBSD_EADDRINUSE	48		/* Address already in use */
#define NETBSD_EADDRNOTAVAIL	49		/* Can't assign requested address */

/* ipc/network software -- operational errors */
#define NETBSD_ENETDOWN	50		/* Network is down */
#define NETBSD_ENETUNREACH	51		/* Network is unreachable */
#define NETBSD_ENETRESET	52		/* Network dropped connection on reset */
#define NETBSD_ECONNABORTED	53		/* Software caused connection abort */
#define NETBSD_ECONNRESET	54		/* Connection reset by peer */
#define NETBSD_ENOBUFS		55		/* No buffer space available */
#define NETBSD_EISCONN		56		/* Socket is already connected */
#define NETBSD_ENOTCONN	57		/* Socket is not connected */
#define NETBSD_ESHUTDOWN	58		/* Can't send after socket shutdown */
#define NETBSD_ETOOMANYREFS	59		/* Too many references: can't splice */
#define NETBSD_ETIMEDOUT	60		/* Operation timed out */
#define NETBSD_ECONNREFUSED	61		/* Connection refused */

#define NETBSD_ELOOP		62		/* Too many levels of symbolic links */
#define NETBSD_ENAMETOOLONG	63		/* File name too long */

/* should be rearranged */
#define NETBSD_EHOSTDOWN	64		/* Host is down */
#define NETBSD_EHOSTUNREACH	65		/* No route to host */
#define NETBSD_ENOTEMPTY	66		/* Directory not empty */

/* quotas & mush */
#define NETBSD_EPROCLIM	67		/* Too many processes */
#define NETBSD_EUSERS		68		/* Too many users */
#define NETBSD_EDQUOT		69		/* Disc quota exceeded */

/* Network File System */
#define NETBSD_ESTALE		70		/* Stale NFS file handle */
#define NETBSD_EREMOTE		71		/* Too many levels of remote in path */
#define NETBSD_EBADRPC		72		/* RPC struct is bad */
#define NETBSD_ERPCMISMATCH	73		/* RPC version wrong */
#define NETBSD_EPROGUNAVAIL	74		/* RPC prog. not avail */
#define NETBSD_EPROGMISMATCH	75		/* Program version wrong */
#define NETBSD_EPROCUNAVAIL	76		/* Bad procedure for program */

#define NETBSD_ENOLCK		77		/* No locks available */
#define NETBSD_ENOSYS		78		/* Function not implemented */

#define NETBSD_EFTYPE		79		/* Inappropriate file type or format */
#define NETBSD_EAUTH		80		/* Authentication error */
#define NETBSD_ENEEDAUTH	81		/* Need authenticator */

/* SystemV IPC */
#define NETBSD_EIDRM		82		/* Identifier removed */
#define NETBSD_ENOMSG		83		/* No message of desired type */
#define NETBSD_EOVERFLOW	84		/* Value too large to be stored in data type */

/* Wide/multibyte-character handling, ISO/IEC 9899/AMD1:1995 */
#define NETBSD_EILSEQ		85		/* Illegal byte sequence */

/* From IEEE Std 1003.1-2001 */
/* Base, Realtime, Threads or Thread Priority Scheduling option errors */
#define NETBSD_ENOTSUP		86		/* Not supported */

/* Realtime option errors */
#define NETBSD_ECANCELED	87		/* Operation canceled */

/* Realtime, XSI STREAMS option errors */
#define NETBSD_EBADMSG		88		/* Bad or Corrupt message */

/* XSI STREAMS option errors  */
#define NETBSD_ENODATA		89		/* No message available */
#define NETBSD_ENOSR		90		/* No STREAM resources */
#define NETBSD_ENOSTR		91		/* Not a STREAM */
#define NETBSD_ETIME		92		/* STREAM ioctl timeout */

/* File system extended attribute errors */
#define NETBSD_ENOATTR		93		/* Attribute not found */

/* Realtime, XSI STREAMS option errors */
#define NETBSD_EMULTIHOP	94		/* Multihop attempted */ 
#define NETBSD_ENOLINK		95		/* Link has been severed */
#define NETBSD_EPROTO		96		/* Protocol error */

#define NETBSD_ELAST		96		/* Must equal largest errno */

#ifdef _KERNEL
/* pseudo-errors returned inside kernel to modify return to process */
#define NETBSD_EJUSTRETURN	-2		/* don't modify regs, just return */
#define NETBSD_ERESTART	-3		/* restart syscall */
#define NETBSD_EPASSTHROUGH	-4		/* ioctl not handled by this layer */
#define NETBSD_EDUPFD		-5		/* Dup given fd */
#define NETBSD_EMOVEFD		-6		/* Move given fd */
#endif

#endif	/*  ERRNO_NETBSD_H  */
