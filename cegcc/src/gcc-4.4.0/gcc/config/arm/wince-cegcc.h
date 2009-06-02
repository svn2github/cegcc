/* Definitions of target machine for GNU compiler, for ARM with WINCE-PE obj format.
   Copyright (C) 2003, 2004, 2005, 2006 Free Software Foundation, Inc.

   Based on work contributed by Nick Clifton <nickc@redhat.com>

   Further development by Pedro Alves <pedro_alves@portugalmail.pt>
   and Danny Backx.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#undef  TARGET_VERSION
#define TARGET_VERSION fprintf (stderr, " (ARM/cegcc)");

#undef CPP_SPEC
#define CPP_SPEC "%(cpp_cpu) %{posix:-D_POSIX_SOURCE} \
  -D__CEGCC32__ -D__CEGCC__ -D__CEGCC_VERSION__ \
  %{!ansi:-Dunix} -D__unix__ -D__unix \
  %{mwin32: -DWIN32 -D_WIN32 -D__WIN32 -D__WIN32__ } \
  %{!nostdinc:%{!mno-win32: -idirafter ../include/w32api%s -idirafter ../../include/w32api%s }} \
"

#undef STARTFILE_SPEC
#define STARTFILE_SPEC "\
  %{shared|mdll:dllcrt1%O%s } \
  %{!shared: %{!mdll: crt0%O%s \
      %{mthreads:crtmt%O%s} %{!mthreads:crtst%O%s} \
  } } \
  %{pg: gcrt3%O%s} \
  "

/* We have to dynamic link to get to the system DLLs.  All of libc, libm,
   the Unix stuff is in cegcc.dll.  The import library is called
   'libcegcc.dll.a'. For Windows applications, include more libraries, but
   always include coredll.  We'd like to specify subsystem windows to
   ld, but that doesn't work just yet.  */

#undef LIB_SPEC
#define LIB_SPEC "%{static: -lm -lc} \
	%{pg: -lgmon} \
	-lcoredll"

#undef LINK_SPEC
#define LINK_SPEC "\
  %{shared: %{mdll: %eshared and mdll are not compatible}} \
  %{shared: --shared} %{mdll:--dll} \
  %{static:-Bstatic} %{!static:-Bdynamic} \
  %{shared|mdll: -e DllMainCRTStartup} \
  "


/*
 * See the message from Dave Korn dated 2009/06/01 15:44 on the cegcc mailing
 * list, and the gcc ChangeLog entry dated 2009-01-21, also by Dave.
 *
 * Based on that, we're replacing LIBGCC_SPEC by SHARED_LIBGCC_SPEC and
 * REAL_GCC_SPEC. This is based on cygwin's definition, which we extend
 * with the other libraries we need.
 *
 * The old definition :
 * "%{mthreads:-lcegccthrd} %{!static: -lcegcc } -lgcc"
 */
#undef LIBGCC_SPEC

#undef SHARED_LIBGCC_SPEC
#ifdef ENABLE_SHARED_LIBGCC
#define SHARED_LIBGCC_SPEC " \
 %{mthreads:-lcegccthrd} \
 %{!static: -lcegcc } \
 %{static|static-libgcc:-lgcc -lgcc_eh} \
 %{!static: \
   %{!static-libgcc: \
     %{!shared: \
       %{!shared-libgcc:-lgcc -lgcc_eh} \
       %{shared-libgcc:-lgcc_s -lgcc} \
      } \
     %{shared:-lgcc_s -lgcc} \
    } \
  } "
#else
#define SHARED_LIBGCC_SPEC "%{mthreads:-lcegccthrd} %{!static: -lcegcc } -lgcc"
#endif

#undef REAL_LIBGCC_SPEC
#define REAL_LIBGCC_SPEC SHARED_LIBGCC_SPEC
