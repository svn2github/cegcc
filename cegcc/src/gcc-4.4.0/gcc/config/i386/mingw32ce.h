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

#if 0
/* This appears to be obsolete */
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
#endif

#undef CPP_SPEC
#define CPP_SPEC "%{posix:-D_POSIX_SOURCE} %{mthreads:-D_MT} \
-D__COREDLL__ -D__MINGW32__ -D__MINGW32CE__ -D__CEGCC_VERSION__ \
-D_WIN32 -DWIN32 -DWINNT \
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
      builtin_define ("__GXX_MERGED_TYPEINFO_NAMES=1");		\
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
  %{shared|mdll: -e _DllMainCRTStartup} \
  "

/* in mingw32.h :
 * #undef STARTFILE_SPEC
 * #define STARTFILE_SPEC "%{shared|mdll:dllcrt2%O%s} \
 *   %{!shared:%{!mdll:crt2%O%s}} %{pg:gcrt2%O%s} \
 *   crtbegin.o%s"
*/

#undef STARTFILE_SPEC
#define STARTFILE_SPEC \
  "%{shared|mdll:dllcrt3%O%s} \
   %{!shared:%{!mdll:crt3%O%s}} %{pg:gcrt3%O%s}"


/* One entry from cygming.h, the other one is cegcc exception handling */
#ifdef SUBTARGET_ATTRIBUTE_TABLE
#undef SUBTARGET_ATTRIBUTE_TABLE
#endif
#define SUBTARGET_ATTRIBUTE_TABLE \
  /* { name, min_len, max_len, decl_req, type_req, fn_type_req, handler } */ \
  { "selectany", 0, 0, true, false, false, ix86_handle_selectany_attribute }, \
  { "exception_handler", 1, 2, true,  false, false, i386_handle_exception_handler_attribute }, \
  { "dft_exception_handler", 1, 2, true,  false, false, i386_dft_handle_exception_handler_attribute }

/* Write the extra assembler code needed to declare a function
   properly.  If we are generating SDB debugging information, this
   will happen automatically, so we only need to handle other cases.  */

/*
 * Note this is the structure of the PDATA entry created :
 * Offset 	Size 	Field 			Description
 * 0 		4 	Begin Address 		Virtual address of the corresponding function.
 * 4	 	4 	End Address 		Virtual address of the end of the function.
 * 8	 	4 	Exception Handler 	Pointer to the exception handler to be executed.
 * 12	 	4 	Handler Data 		Pointer to additional information to be passed to the handler.
 * 16	 	4 	Prolog End Address 	Virtual address of the end of the function prolog.
 */
#undef ASM_DECLARE_FUNCTION_NAME
#define ASM_DECLARE_FUNCTION_NAME(FILE, NAME, DECL)				\
  do										\
    {										\
      char *eh;									\
      eh = i386_exception_handler(FILE, NAME, DECL);				\
      if (eh) {									\
	asm_fprintf (FILE, "# %s has exception hander %s\n",			\
			NAME, eh);						\
	asm_fprintf (FILE, "\t.section .pdata\n");				\
        asm_fprintf (FILE, "\t.long _%s\n", NAME);				\
        asm_fprintf (FILE, "\t.long .L%s_end\n", NAME);				\
        asm_fprintf (FILE, "\t.long _%s\n", eh);				\
        asm_fprintf (FILE, "\t.long 0 /* .L%s_handler_data */\n", NAME);	\
        asm_fprintf (FILE, "\t.long 0 /* .L%s_prolog_end */\n", NAME);		\
	asm_fprintf (FILE, "\t.text\n");					\
	asm_fprintf (FILE, "\t.L%s_data:\n", NAME);				\
      }										\
      /* UWIN binutils bug workaround.  */					\
      if (0 && write_symbols != SDB_DEBUG)					\
	i386_pe_declare_function_type (FILE, NAME, TREE_PUBLIC (DECL));		\
      ASM_OUTPUT_LABEL (FILE, NAME);						\
    }										\
  while (0)

/*
 * An ARM specific trailer for function declarations.
 *
 * This one is needed for exception handlers : the entry in the pdata section
 * needs to know the size of the function for which we handle exceptions.
 */
#undef  ASM_DECLARE_FUNCTION_SIZE
#define ASM_DECLARE_FUNCTION_SIZE(STREAM, NAME, DECL)			\
    {									\
	if (i386_exception_handler(STREAM, NAME, DECL))		\
		asm_fprintf (STREAM, ".L%s_end:\n", NAME);		\
    }

/*
 * New macro to denote prologue end
 */
#undef ASM_X86_PROLOGUE_END
#define ASM_X86_PROLOGUE_END(FILE, NAME)				\
  {									\
    asm_fprintf (FILE, "# End of prologue\n");				\
  }

#undef SHLIB_LC
#define SHLIB_LC "-lmingw32 -lmingwex -lmsvcrt"

/*
 * See the message from Dave Korn dated 2009/06/01 15:44 on the cegcc mailing
 * list, and the gcc ChangeLog entry dated 2009-01-21, also by Dave.
 *
 * Based on that, we're replacing LIBGCC_SPEC by SHARED_LIBGCC_SPEC and
 * REAL_GCC_SPEC. This is based on cygwin's definition, which we extend
 * with the other libraries we need (our list was -lmingw32 -lgcc -lceoldname
 * -lmingwex prior to this).
 *
 * The REAL_LIBGCC_SPEC only contained support for the -mno-cygwin flag,
 * which is why there's no difference with SHARED_LIBGCC_SPEC here.
 */
#undef LIBGCC_SPEC

#undef SHARED_LIBGCC_SPEC
#ifdef ENABLE_SHARED_LIBGCC
#define SHARED_LIBGCC_SPEC " \
 %{mthreads:-lmingwthrd} -lmingw32 \
 %{static|static-libgcc:-lgcc -lgcc_eh} \
 %{!static: \
   %{!static-libgcc: \
     %{!shared: \
       %{!shared-libgcc:-lgcc -lgcc_eh} \
       %{shared-libgcc:-lgcc_s -lgcc} \
      } \
     %{shared:-lgcc_s -lgcc} \
    } \
  } \
  -lceoldname -lmingwex"
#else
#define SHARED_LIBGCC_SPEC " -lmingw32 -lgcc -lceoldname -lmingwex "
#endif

#undef REAL_LIBGCC_SPEC
#define REAL_LIBGCC_SPEC SHARED_LIBGCC_SPEC
