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

#undef  TARGET_DEFAULT
#define TARGET_DEFAULT	(MASK_NOP_FUN_DLLIMPORT)

#undef MATH_LIBRARY
#define MATH_LIBRARY ""

#define NO_IMPLICIT_EXTERN_C

#define TARGET_EXECUTABLE_SUFFIX ".exe"

#undef  TARGET_VERSION
#define TARGET_VERSION fprintf (stderr, " (arm cegcc)");

/* pedro: defined empty in arm.h, redefined in pe.h, and then here */
#undef SUBTARGET_CONDITIONAL_REGISTER_USAGE
#define SUBTARGET_CONDITIONAL_REGISTER_USAGE

#undef  MULTILIB_DEFAULTS
#define MULTILIB_DEFAULTS \
  { "marm", "mlittle-endian", "msoft-float", "mfpu=vfp", "mno-thumb-interwork" }  
#undef  FPUTYPE_DEFAULT
#define FPUTYPE_DEFAULT FPUTYPE_VFP

#undef CPP_SPEC
#define CPP_SPEC "%(cpp_cpu) %{posix:-D_POSIX_SOURCE} \
%{mno-win32:%{mno-cegcc: %emno-cegcc and mno-win32 are not compatible}} \
%{mno-cegcc: %{!ansi:%{mthreads: -D_MT }}} \
%{!mno-cegcc: -D__CEGCC32__ -D__CEGCC__ %{!ansi:-Dunix} -D__unix__ -D__unix } \
%{mwin32|mno-cegcc: -DWIN32 -D_WIN32 -D__WIN32 -D__WIN32__ } \
%{!nostdinc:%{!mno-win32|mno-cegcc: -idirafter ../include/w32api%s -idirafter ../../include/w32api%s }} \
"

#undef ASM_SPEC
#define ASM_SPEC "\
%{mbig-endian:-EB} \
%{mlittle-endian:-EL} \
%{mcpu=xscale:-mcpu=iwmmxt; mcpu=*:-mcpu=%*} \
%{march=*:-march=%*} \
%{mapcs-*:-mapcs-%*} \
%(subtarget_asm_float_spec) \
%{mthumb-interwork:-mthumb-interwork} \
%{msoft-float:-mfloat-abi=soft} %{mhard-float:-mfloat-abi=hard} \
%{mfloat-abi=*} %{mfpu=*} \
%(subtarget_extra_asm_spec)"

#undef SUBTARGET_EXTRA_SPECS
#define SUBTARGET_EXTRA_SPECS \
  { "subtarget_asm_float_spec", SUBTARGET_ASM_FLOAT_SPEC }, \
  { "mingw_include_path", DEFAULT_TARGET_MACHINE }

#undef SUBTARGET_ASM_FLOAT_SPEC
#define SUBTARGET_ASM_FLOAT_SPEC "\
%{!mfpu=*:-mfpu=vfp}"

#define EXTRA_OS_CPP_BUILTINS()

#define TARGET_OS_CPP_BUILTINS()				\
  do								\
  {								\
      builtin_define ("_M_ARM=1");				\
      builtin_define ("ARM=1");					\
      builtin_define_std ("UNDER_CE");				\
      builtin_define ("_UNICODE");				\
      builtin_define_std ("UNICODE");				\
      builtin_define ("__stdcall=__attribute__((__cdecl__))");	\
      builtin_define ("__cdecl=__attribute__((__cdecl__))");	\
      /* Even though linkonce works with static libs, this is needed 	\
          to compare typeinfo symbols across dll boundaries.  */	\
      builtin_define ("__GXX_MERGED_TYPEINFO_NAMES=0");		\
      EXTRA_OS_CPP_BUILTINS ();					\
  }								\
  while (0)

#undef  SUBTARGET_CPP_SPEC
#define SUBTARGET_CPP_SPEC " -D__pe__ "

/* Handle #pragma weak and #pragma pack. */
#define HANDLE_SYSV_PRAGMA 1
#define HANDLE_PRAGMA_PACK_PUSH_POP 1

/* Now we define the strings used to build the spec file.  */
#undef STARTFILE_SPEC
#define STARTFILE_SPEC "\
  %{shared|mdll: %{!mno-cegcc:dllcrt1%O%s} %{mno-cegcc:dllcrt2%O%s} } \
  %{!shared: %{!mdll: \
      %{!mno-cegcc:crt0%O%s} %{mno-cegcc:crt2%O%s} \
      %{mthreads:crtmt%O%s} %{!mthreads:crtst%O%s} \
  } } \
  "

#undef LIBGCC_SPEC
#define LIBGCC_SPEC \
  "%{mthreads:-lcegccthrd} %{!mno-cegcc: %{!static: -lcegcc } } -lgcc"

/* We have to dynamic link to get to the system DLLs.  All of libc, libm,
the Unix stuff is in cegcc.dll.  The import library is called
'libcegcc.dll.a'. For Windows applications, include more libraries, but
always include coredll.  We'd like to specify subsystem windows to
ld, but that doesn't work just yet.  */

#undef LIB_SPEC
#define LIB_SPEC "\
  %{!mno-cegcc: %{static: -lm -lc} } -lcoredll"

#undef LINK_SPEC
#define LINK_SPEC "\
  %{shared: %{mdll: %eshared and mdll are not compatible}} \
  %{shared: --shared} %{mdll:--dll} \
  %{static:-Bstatic} %{!static:-Bdynamic} \
  %{shared|mdll: -e DllMainCRTStartup} \
  "

#define ARM_WINCE 1

#undef  COMMON_ASM_OP
#define COMMON_ASM_OP	"\t.comm\t"

/* Output a common block.  */
#undef ASM_OUTPUT_COMMON
#define ASM_OUTPUT_COMMON(STREAM, NAME, SIZE, ROUNDED)	\
  do {							\
  if (arm_pe_dllexport_name_p (NAME))			\
  arm_pe_record_exported_symbol (NAME, 1);		\
  if (! arm_pe_dllimport_name_p (NAME))		\
    {							\
    fprintf ((STREAM), "\t.comm\t");			\
    assemble_name ((STREAM), (NAME));			\
    fprintf ((STREAM), ", %d\t%s %d\n",		\
    (int)(ROUNDED), ASM_COMMENT_START, (int)(SIZE));	\
    }							\
  } while (0)

/* Output the label for an initialized variable.  */
#undef ASM_DECLARE_OBJECT_NAME
#define ASM_DECLARE_OBJECT_NAME(STREAM, NAME, DECL)	\
  do {							\
  if (arm_pe_dllexport_name_p (NAME))			\
  arm_pe_record_exported_symbol (NAME, 1);		\
  ASM_OUTPUT_LABEL ((STREAM), (NAME));			\
  } while (0)

/* Write the extra assembler code needed to declare a function
properly.  If we are generating SDB debugging information, this
will happen automatically, so we only need to handle other cases.  */
#undef ASM_DECLARE_FUNCTION_NAME
#define ASM_DECLARE_FUNCTION_NAME(FILE, NAME, DECL)			\
  do									\
    {									\
    if (arm_pe_dllexport_name_p (NAME))				\
    arm_pe_record_exported_symbol (NAME, 0);			\
    if (write_symbols != SDB_DEBUG)					\
    arm_pe_declare_function_type (FILE, NAME, TREE_PUBLIC (DECL));	\
    ASM_OUTPUT_LABEL (FILE, NAME);					\
    }									\
    while (0)


/* Add an external function to the list of functions to be declared at
the end of the file.  */
#define ASM_OUTPUT_EXTERNAL(FILE, DECL, NAME)				\
  do									\
    {									\
    if (TREE_CODE (DECL) == FUNCTION_DECL)				\
    arm_pe_record_external_function ((DECL), (NAME));		\
    }									\
    while (0)

/* Declare the type properly for any external libcall.  */
#define ASM_OUTPUT_EXTERNAL_LIBCALL(FILE, FUN) \
  arm_pe_declare_function_type (FILE, XSTR (FUN, 0), 1)

/* This says out to put a global symbol in the BSS section.  */
#undef ASM_OUTPUT_ALIGNED_BSS
#define ASM_OUTPUT_ALIGNED_BSS(FILE, DECL, NAME, SIZE, ALIGN) \
  asm_output_aligned_bss ((FILE), (DECL), (NAME), (SIZE), (ALIGN))

/* Output function declarations at the end of the file.  */
#undef TARGET_ASM_FILE_END
#define TARGET_ASM_FILE_END arm_pe_file_end

/* Don't assume anything about the header files.  */
#define NO_IMPLICIT_EXTERN_C

#define SUPPORTS_ONE_ONLY 1

/* Windows uses explicit import from shared libraries.  */
#define MULTIPLE_SYMBOL_SPACES 1

#undef SIZE_TYPE
#define SIZE_TYPE "unsigned int"

#undef PTRDIFF_TYPE
#define PTRDIFF_TYPE "int"

#undef WCHAR_TYPE_SIZE
#define WCHAR_TYPE_SIZE 16

#undef WCHAR_TYPE
#define WCHAR_TYPE "short unsigned int"

/* Define as short unsigned for compatibility with MS runtime.  */
#undef WINT_TYPE
#define WINT_TYPE "short unsigned int"

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

#undef  LPREFIX
#define LPREFIX ".L"

#undef LOCAL_LABEL_PREFIX
#define LOCAL_LABEL_PREFIX "."

/* The prefix to add to user-visible assembler symbols.  */

#undef  USER_LABEL_PREFIX
#define USER_LABEL_PREFIX ""


/* If user-symbols don't have underscores,
   then it must take more than `L' to identify
   a label that should be ignored.  */

/* This is how to store into the string BUF
   the symbol_ref name of an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.
   This is suitable for output with `assemble_name'.  */

#undef  ASM_GENERATE_INTERNAL_LABEL
#define ASM_GENERATE_INTERNAL_LABEL(BUF,PREFIX,NUMBER)	\
  sprintf ((BUF), ".%s%ld", (PREFIX), (long)(NUMBER))


/* Emit code to check the stack when allocating more that 4000
   bytes in one go.  */

#define CHECK_STACK_LIMIT 4000

/* By default, target has a 80387, uses IEEE compatible arithmetic,
   returns float values in the 387 and needs stack probes.
   We also align doubles to 64-bits for MSVC default compatibility.  */

#undef TARGET_SUBTARGET_DEFAULT
#define TARGET_SUBTARGET_DEFAULT \
   (MASK_IEEE_FP | MASK_ALIGN_DOUBLE)

/* ### Needs better testing!*/
/* Native complier aligns internal doubles in structures on dword boundaries.  */
#undef	BIGGEST_FIELD_ALIGNMENT
#define BIGGEST_FIELD_ALIGNMENT 64

/* A bit-field declared as `int' forces `int' alignment for the struct.  */
#undef PCC_BITFIELD_TYPE_MATTERS
#define PCC_BITFIELD_TYPE_MATTERS 1
#define GROUP_BITFIELDS_BY_ALIGN TYPE_NATIVE(rec)

/* Enable alias attribute support.  */
#ifndef SET_ASM_OP
#define SET_ASM_OP "\t.set\t"
#endif


/* This implements the `alias' attribute.  */
#undef	ASM_OUTPUT_DEF_FROM_DECLS
#define	ASM_OUTPUT_DEF_FROM_DECLS(STREAM, DECL, TARGET) 		\
  do									\
    {									\
      const char *alias;						\
      rtx rtlname = XEXP (DECL_RTL (DECL), 0);				\
      if (GET_CODE (rtlname) == SYMBOL_REF)				\
	alias = XSTR (rtlname, 0);					\
      else								\
	abort ();							\
      if (TREE_CODE (DECL) == FUNCTION_DECL)				\
	arm_pe_declare_function_type (STREAM, alias,			\
				       TREE_PUBLIC (DECL));		\
      ASM_OUTPUT_DEF (STREAM, alias, IDENTIFIER_POINTER (TARGET));	\
    } while (0)

/* FIXME: SUPPORTS_WEAK && TARGET_HAVE_NAMED_SECTIONS is true,
   but for .jcr section to work we also need crtbegin and crtend
   objects.  */
#define TARGET_USE_JCR_SECTION 0

/* Decide whether it is safe to use a local alias for a virtual function
   when constructing thunks.  */
#undef TARGET_USE_LOCAL_THUNK_ALIAS_P
#define TARGET_USE_LOCAL_THUNK_ALIAS_P(DECL) (!DECL_ONE_ONLY (DECL))

#define SUBTARGET_ATTRIBUTE_TABLE \
  /* { name, min_len, max_len, decl_req, type_req, fn_type_req, handler } */ \
  { "selectany", 0, 0, true, false, false, arm_pe_handle_selectany_attribute }

#undef TREE

#ifndef BUFSIZ
# undef FILE
#endif
