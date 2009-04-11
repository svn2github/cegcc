/* Implement cegcc's flavour of a gcc runtime for Windows CE on x86.

   Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004
   Free Software Foundation, Inc.

   Copyright (c) 2009, Danny Backx.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to
the Free Software Foundation, 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.  */

/* Same as for cygwin but without the MSVCRT */
#undef EXTRA_OS_CPP_BUILTINS
#define EXTRA_OS_CPP_BUILTINS()					\
  do								\
    {								\
      builtin_define ("__MINGW32__");			   	\
      builtin_define ("_WIN32");				\
      builtin_define_std ("WIN32");				\
      builtin_define_std ("WINNT");				\
    }								\
  while (0)

#undef CPP_SPEC
#define CPP_SPEC "%{posix:-D_POSIX_SOURCE} %{mthreads:-D_MT} \
-D__COREDLL__ -D__MINGW32__ -D__MINGW32CE__ -D__CEGCC_VERSION__ \
%{!nostdinc: -idirafter ../include/w32api%s -idirafter ../../include/w32api%s }"

#undef TARGET_OS_CPP_BUILTINS
#define TARGET_OS_CPP_BUILTINS()				\
  do								\
  {								\
      /* We currently define UNDER_CE to a non-value, as it seems \
         MSVC2005 does the same.  */ \
      builtin_define_std ("UNDER_CE");				\
      builtin_define ("_UNICODE");				\
      builtin_define_std ("UNICODE");				\
	  /* Let's just ignore stdcall, and fastcall.  */ \
      builtin_define ("__stdcall=__attribute__((__cdecl__))");	\
      builtin_define ("__fastcall=__attribute__((__cdecl__))");	\
      builtin_define ("__cdecl=__attribute__((__cdecl__))");		\
      if (!flag_iso)							\
        {								\
          builtin_define ("_stdcall=__attribute__((__cdecl__))");	\
          builtin_define ("_fastcall=__attribute__((__cdecl__))");	\
          builtin_define ("_cdecl=__attribute__((__cdecl__))");	\
        }								\
      /* Even though linkonce works with static libs, this is needed 	\
          to compare typeinfo symbols across dll boundaries.  */	\
      builtin_define ("__GXX_MERGED_TYPEINFO_NAMES=0");		\
  }                                                           \
  while (0)

/* Link with coredll, the main libc in the native SDK, 
   and to corelibc, a static lib that contains the start files, among other
   basic crt stuff.  */
#undef LIB_SPEC
#define LIB_SPEC "-lcoredll"

#undef LINK_SPEC
#define LINK_SPEC "\
  %{shared: %{mdll: %eshared and mdll are not compatible}} \
  %{shared: --shared} %{mdll:--dll} \
  %{static:-Bstatic} %{!static:-Bdynamic} \
  %{shared|mdll: -e DllMainCRTStartup} \
  "

#undef STARTFILE_SPEC
#define STARTFILE_SPEC "%{shared|mdll:dllcrt3%O%s} \
  %{!shared:%{!mdll:crt3%O%s}} %{pg:gcrt3%O%s}"


/* Include in the mingw32 libraries with libgcc */
#undef LIBGCC_SPEC
#define LIBGCC_SPEC \
  "%{mthreads:-lmingwthrd} -lmingw32 -lgcc -lceoldname -lmingwex"
