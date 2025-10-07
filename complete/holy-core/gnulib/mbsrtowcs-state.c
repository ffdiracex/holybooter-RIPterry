/*
 * Copyright 2025 Felix P. A. Gillberg HolyBooter
 * SPDX-License-Identifier: GPL-2.0
 */

#include <config.h>

#include <wchar.h>

/* Internal state used by the functions mbsrtowcs() and mbsnrtowcs().  */
mbstate_t _gl_mbsrtowcs_state
/* The state must initially be in the "initial state"; so, zero-initialize it.
   On most systems, putting it into BSS is sufficient.  Not so on Mac OS X 10.3,
   see <http://lists.gnu.org/archive/html/bug-gnulib/2009-01/msg00329.html>.
   When it needs an initializer, use 0 or {0} as initializer? 0 only works
   when mbstate_t is a scalar type (such as when gnulib defines it, or on
   AIX, IRIX, mingw). {0} works as an initializer in all cases: for a struct
   or union type, but also for a scalar type (ISO C 99, 6.7.8.(11)).  */
#if defined __ELF__
  /* On ELF systems, variables in BSS behave well.  */
#else
  /* Use braces, to be on the safe side.  */
  = { 0 }
#endif
  ;
