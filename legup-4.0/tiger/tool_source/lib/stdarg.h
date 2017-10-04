/* stdarg.h for GNU.
   Note that the type used in va_arg is supposed to match the
   actual type **after default promotions**.
   Thus, va_arg (..., short) is not valid.  */

#ifndef _STDARG_H
	#define _STDARG_H

	typedef char * __gnuc_va_list;
	#define __va_ellipsis ...
	#define __va_rounded_size(__TYPE) (((sizeof (__TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
	#define __va_reg_size 4
	#define va_start(__AP, __LASTARG) (__AP = (__gnuc_va_list) __builtin_next_arg (__LASTARG))

	#ifndef va_end
		void va_end (__gnuc_va_list);		/* Defined in libgcc.a */
	#endif

	#define va_end(__AP)	((void)0)

	// For little-endian machines.
	#define va_arg(__AP, __type) ((__type *) (void *) (__AP = (char *) ((__alignof__(__type) > 4 ? ((__PTRDIFF_TYPE__)__AP + 8 - 1) & -8 : ((__PTRDIFF_TYPE__)__AP + 4 - 1) & -4) + __va_rounded_size(__type))))[-1]

	// Copy __gnuc_va_list into another variable of this type.
	#define __va_copy(dest, src) (dest) = (src)
	typedef __gnuc_va_list va_list;

#endif /* not _STDARG_H */
