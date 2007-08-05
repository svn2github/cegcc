/* Definitions of target machine for GNU compiler, for ARM with WINCE-PE obj format.
   Copyright (C) 2003, 2004, 2005, 2006 Free Software Foundation, Inc.
   Contributed by Nick Clifton <nickc@redhat.com>

   Further development by Pedro Alves <pedro_alves@portugalmail.pt>

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

/* Enable WinCE specific code.  */
#define ARM_WINCE 1

#undef MATH_LIBRARY
#define MATH_LIBRARY ""

#define TARGET_EXECUTABLE_SUFFIX ".exe"

#undef  TARGET_VERSION
#define TARGET_VERSION fprintf (stderr, " (arm Windows CE/Native SDK)");

#undef SUBTARGET_CONDITIONAL_REGISTER_USAGE
#define SUBTARGET_CONDITIONAL_REGISTER_USAGE

#undef  MULTILIB_DEFAULTS
#define MULTILIB_DEFAULTS \
  { "marm", "mlittle-endian", "msoft-float", "mno-thumb-interwork" }

#undef SUBTARGET_CPU_DEFAULT
#define SUBTARGET_CPU_DEFAULT TARGET_CPU_arm8

/* We must store doubles in little endian order. Specifying the default
   of VFP assures that. To guaranty VFP insns won't be emitted by default,
   we default to float-abi=soft.  */
#undef  FPUTYPE_DEFAULT
#define FPUTYPE_DEFAULT FPUTYPE_VFP

#undef CPP_SPEC
#define CPP_SPEC "%(cpp_cpu) \
-DWIN32 -D_WIN32 -D__WIN32 -D__WIN32__ \
%{!nostdinc: -idirafter ../include/w32api%s -idirafter ../../include/w32api%s } \
"

#ifndef ASM_SPEC
#define ASM_SPEC "\
%{mbig-endian:-EB} \
%{mlittle-endian:-EL} \
%{mcpu=*:-mcpu=%*} \
%{march=*:-march=%*} \
%{mapcs-*:-mapcs-%*} \
%(subtarget_asm_float_spec) \
%{mthumb-interwork:-mthumb-interwork} \
%{msoft-float:-mfloat-abi=soft} %{mhard-float:-mfloat-abi=hard} \
%{mfloat-abi=*} %{mfpu=*} \
%(subtarget_extra_asm_spec)"
#endif

#define EXTRA_OS_CPP_BUILTINS()

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
      EXTRA_OS_CPP_BUILTINS ();					\
      {                                                       \
        /* Define these to be compatible MSFT's tools.  */    \
        char buf[64];                                         \
        int arch = arm_major_arch ();                         \
        sprintf (buf, "_M_ARM=%d", arch);                     \
        builtin_define (buf);                                 \
        if (arm_thumb_arch_p ())                              \
          {                                                   \
            sprintf (buf, "_M_ARMT=%d", arch);                \
            builtin_define (buf);                             \
          }                                                   \
        /* Always defined as empty.  */                       \
        builtin_define ("ARM=");                              \
      }                                                       \
  }                                                           \
  while (0)

/* Now we define the strings used to build the spec file.  */
#undef STARTFILE_SPEC
#define STARTFILE_SPEC ""

#undef LIBGCC_SPEC
#define LIBGCC_SPEC "-lgcc"

/* Link with coredll, the main libc in the native SDK, 
   and to corelibc, a static lib that contains the start files, among other
   basic crt stuff.  */
#undef LIB_SPEC
#define LIB_SPEC "-lcoredll -lcorelibc"

#undef LINK_SPEC
#define LINK_SPEC "\
  %{shared: %{mdll: %eshared and mdll are not compatible}} \
  %{shared: --shared} %{mdll:--dll} \
  %{static:-Bstatic} %{!static:-Bdynamic} \
  %{shared|mdll: -e DllMainCRTStartup} \
  "


/* Don't assume anything about the header files.  */
#define NO_IMPLICIT_EXTERN_C


/* Define types for compatibility with MS runtime.  */

#undef DEFAULT_SIGNED_CHAR
#define DEFAULT_SIGNED_CHAR 1

#undef SIZE_TYPE
#define SIZE_TYPE "unsigned int"

#undef PTRDIFF_TYPE
#define PTRDIFF_TYPE "int"

#undef WCHAR_TYPE_SIZE
#define WCHAR_TYPE_SIZE 16

#undef WCHAR_TYPE
#define WCHAR_TYPE "short unsigned int"

#undef WINT_TYPE
#define WINT_TYPE "short unsigned int"


#undef DWARF2_UNWIND_INFO
#define DWARF2_UNWIND_INFO 0

#define DWARF2_DEBUGGING_INFO 1

#undef  PREFERRED_DEBUGGING_TYPE
#define PREFERRED_DEBUGGING_TYPE DWARF2_DEBUG

#undef HAVE_AS_DWARF2_DEBUG_LINE
#define HAVE_AS_DWARF2_DEBUG_LINE 1

/* Use section relative relocations for debugging offsets.  Unlike
   other targets that fake this by putting the section VMA at 0, PE
   won't allow it.  */
#define ASM_OUTPUT_DWARF_OFFSET(FILE, SIZE, LABEL)    \
  do {                                                \
    if (SIZE != 4)                                    \
      abort ();                                       \
                                                      \
    fputs ("\t.secrel32\t", FILE);                    \
    assemble_name (FILE, LABEL);                      \
  } while (0)


/* Prefix for internally generated assembler labels.  If we aren't using
   underscores, we are using prefix `.'s to identify labels that should
   be ignored.  */
/* If user-symbols don't have underscores,
   then it must take more than `L' to identify
   a label that should be ignored.  */

#undef  LPREFIX
#define LPREFIX ".L"

#undef LOCAL_LABEL_PREFIX
#define LOCAL_LABEL_PREFIX "."

/* The prefix to add to user-visible assembler symbols.  
   Arm Windows CE is not underscored.  */

#undef  USER_LABEL_PREFIX
#define USER_LABEL_PREFIX ""

/* This is how to store into the string BUF
   the symbol_ref name of an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.
   This is suitable for output with `assemble_name'.  */
#undef  ASM_GENERATE_INTERNAL_LABEL
#define ASM_GENERATE_INTERNAL_LABEL(BUF,PREFIX,NUMBER)	\
  sprintf ((BUF), ".%s%ld", (PREFIX), (long)(NUMBER))

/* We have to re-define this to prevent any conflicts.  */
#undef ARM_MCOUNT_NAME
#define ARM_MCOUNT_NAME "_mcount"



/* We do bitfields MSVC-compatibly by default.  
   We choose to be compatible with Microsoft's ARM and Thumb compilers,
   which always return aggregates in memory.  */
#undef TARGET_DEFAULT
#define TARGET_DEFAULT (MASK_NOP_FUN_DLLIMPORT | \
                        MASK_MS_BITFIELD_LAYOUT | \
                        MASK_RETURN_AGGREGATES_IN_MEMORY)

/* A bit-field declared as `int' forces `int' alignment for the struct.  */
#undef PCC_BITFIELD_TYPE_MATTERS
#define PCC_BITFIELD_TYPE_MATTERS 1
#define GROUP_BITFIELDS_BY_ALIGN TYPE_NATIVE(rec)

#undef  DEFAULT_STRUCTURE_SIZE_BOUNDARY
#define DEFAULT_STRUCTURE_SIZE_BOUNDARY 8

#undef TREE

#ifndef BUFSIZ
# undef FILE
#endif

/*
 * An ARM specific header for function declarations.
 *
 * This one is needed for exception handlers : the entry in the pdata section
 * needs to know the size of the function for which we handle exceptions.
 */
#undef ASM_DECLARE_FUNCTION_NAME
#define ASM_DECLARE_FUNCTION_NAME(STREAM, NAME, DECL)			\
  do									\
    {									\
      char *eh;								\
      eh = arm_exception_handler(STREAM, NAME, DECL);			\
      if (eh)								\
        {								\
          asm_fprintf (STREAM, "%@ %s has exception handler %s\n",	\
			       NAME, eh);				\
          asm_fprintf (STREAM, "\t.section .pdata\n");			\
          asm_fprintf (STREAM, "\t.word %s\n", NAME);			\
          asm_fprintf (STREAM, "\t.word 0xc0000002 | "			\
			"((((.L%s_end - %s) / 4) & 0x3ffffff) << 8) "	\
			"/* _cegcc_%s size */\n", NAME, NAME, NAME);	\
          asm_fprintf (STREAM, "\t.text\n");				\
          asm_fprintf (STREAM, ".L%s_data:\n", NAME);			\
          asm_fprintf (STREAM, "\t.word %s /* .L%s_handler */\n",	\
			       eh, NAME);				\
          asm_fprintf (STREAM, "\t.word 0 /* .L%s_handler_data */\n",	\
			       NAME);					\
        }								\
      ARM_PE_DECLARE_FUNCTION_NAME(STREAM, NAME, DECL);			\
    }									\
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
	if (arm_exception_handler(STREAM, NAME, DECL))			\
		asm_fprintf (STREAM, ".L%s_end:\n", NAME);		\
    }
