/* Definitions of target machine for GNU compiler, for ARM with PE obj format.
   Copyright (C) 1995, 1996, 1999, 2000, 2002, 2003, 2004, 2005, 2007
   Free Software Foundation, Inc.
   Contributed by Doug Evans (dje@cygnus.com).
   
   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

/* Enable PE specific code.  */
#define ARM_PE		1

#define ARM_PE_FLAG_CHAR '@'

#define DLL_IMPORT_PREFIX "@i."
#define DLL_EXPORT_PREFIX "@e."

/* Ensure that @x. will be stripped from the function name.  */
#undef SUBTARGET_NAME_ENCODING_LENGTHS
#define SUBTARGET_NAME_ENCODING_LENGTHS  \
  case ARM_PE_FLAG_CHAR: return strlen(DLL_IMPORT_PREFIX);

#undef  USER_LABEL_PREFIX
#define USER_LABEL_PREFIX "_"


/* Run-time Target Specification.  */
#undef  TARGET_VERSION
#define TARGET_VERSION fputs (" (ARM/pe)", stderr)

/* Get tree.c to declare a target-specific specialization of
   merge_decl_attributes.  */
#define TARGET_DLLIMPORT_DECL_ATTRIBUTES 1

#undef  SUBTARGET_CPP_SPEC
#define SUBTARGET_CPP_SPEC "-D__pe__"

#undef  TARGET_DEFAULT
#define TARGET_DEFAULT	(MASK_NOP_FUN_DLLIMPORT)

#undef  MULTILIB_DEFAULTS
#define MULTILIB_DEFAULTS \
  { "marm", "mlittle-endian", "msoft-float", "mno-thumb-interwork" }  

#undef  WCHAR_TYPE
#define WCHAR_TYPE 	"short unsigned int"
#undef  WCHAR_TYPE_SIZE
#define WCHAR_TYPE_SIZE 16


/* Handle #pragma weak and #pragma pack. */
#define HANDLE_SYSV_PRAGMA 1
#define HANDLE_PRAGMA_PACK_PUSH_POP 1
/* Enable push_macro & pop_macro */
#define HANDLE_PRAGMA_PUSH_POP_MACRO 1

union tree_node;
#define TREE union tree_node *


/* r11 is fixed.  */
#undef  SUBTARGET_CONDITIONAL_REGISTER_USAGE
#define SUBTARGET_CONDITIONAL_REGISTER_USAGE \
  fixed_regs [11] = 1; \
  call_used_regs [11] = 1;

/* Don't allow flag_pic to propagate since gas may produce invalid code
   otherwise.  */

#undef  SUBTARGET_OVERRIDE_OPTIONS
#define SUBTARGET_OVERRIDE_OPTIONS					\
do {									\
  if (flag_pic)								\
    {									\
      warning (0, 							\
	"-f%s ignored for target (all code is position independent)",	\
	(flag_pic > 1) ? "PIC" : "pic");				\
      flag_pic = 0;							\
    }									\
} while (0)								\



/* PE/COFF uses explicit import from shared libraries.  */
#define MULTIPLE_SYMBOL_SPACES 1

#define TARGET_ASM_UNIQUE_SECTION arm_pe_unique_section
#define TARGET_ASM_FUNCTION_RODATA_SECTION default_no_function_rodata_section

#define SUPPORTS_ONE_ONLY 1

/* Switch into a generic section.  */
#undef  TARGET_ASM_NAMED_SECTION
#define TARGET_ASM_NAMED_SECTION  arm_pe_asm_named_section

/* Select attributes for named sections.  */
#undef TARGET_SECTION_TYPE_FLAGS
#define TARGET_SECTION_TYPE_FLAGS  arm_pe_section_type_flags


#define TARGET_ASM_FILE_START_FILE_DIRECTIVE true

/* Write the extra assembler code needed to declare a function
   properly.  If we are generating SDB debugging information, this
   will happen automatically, so we only need to handle other cases.  */
#undef ASM_DECLARE_FUNCTION_NAME
#define ASM_DECLARE_FUNCTION_NAME(FILE, NAME, DECL)			\
  do									\
    {									\
      arm_pe_maybe_record_exported_symbol (DECL, NAME, 0);		\
      if (write_symbols != SDB_DEBUG)					\
	arm_pe_declare_function_type (FILE, NAME, TREE_PUBLIC (DECL));	\
      if (TARGET_THUMB)							\
	fprintf (FILE, "\t.code 16\n");					\
      ASM_OUTPUT_LABEL (FILE, NAME);					\
    }									\
  while (0)

/* Output function declarations at the end of the file.  */
#undef TARGET_ASM_FILE_END
#define TARGET_ASM_FILE_END arm_pe_file_end

/* Define this macro if references to a symbol must be treated
   differently depending on something about the variable or
   function named by the symbol (such as what section it is in).

   We must mark dll symbols specially. Definitions of
   dllexport'd objects install some info in the .drectve section.
   References to dllimport'd objects are fetched indirectly via
   __imp_.  If both are declared, dllexport overrides.  This is also
   needed to implement one-only vtables: they go into their own
   section and we need to set DECL_SECTION_NAME so we do that here.
   Note that we can be called twice on the same decl.  */

#undef TARGET_ENCODE_SECTION_INFO
#define TARGET_ENCODE_SECTION_INFO  arm_pe_encode_section_info

#undef  TARGET_STRIP_NAME_ENCODING
#define TARGET_STRIP_NAME_ENCODING arm_pe_strip_name_encoding_full

/* Output a common block.  */
#undef ASM_OUTPUT_ALIGNED_DECL_COMMON
#define ASM_OUTPUT_ALIGNED_DECL_COMMON \
  arm_pe_asm_output_aligned_decl_common

/* Output the label for an initialized variable.  */
#undef ASM_DECLARE_OBJECT_NAME
#define ASM_DECLARE_OBJECT_NAME(STREAM, NAME, DECL)	\
do {							\
  arm_pe_maybe_record_exported_symbol (DECL, NAME, 1);	\
  ASM_OUTPUT_LABEL ((STREAM), (NAME));			\
} while (0)

/* Add an external function to the list of functions to be declared at
the end of the file.  */
#undef ASM_OUTPUT_EXTERNAL
#define ASM_OUTPUT_EXTERNAL(FILE, DECL, NAME)			\
  do								\
    {								\
      if (TREE_CODE (DECL) == FUNCTION_DECL)			\
        arm_pe_record_external_function ((DECL), (NAME));	\
    }								\
    while (0)

/* Declare the type properly for any external libcall.  */
#undef ASM_OUTPUT_EXTERNAL_LIBCALL
#define ASM_OUTPUT_EXTERNAL_LIBCALL(FILE, FUN) \
  arm_pe_declare_function_type (FILE, XSTR (FUN, 0), 1)

/* This says out to put a global symbol in the BSS section.  */
#undef ASM_OUTPUT_ALIGNED_BSS
#define ASM_OUTPUT_ALIGNED_BSS(FILE, DECL, NAME, SIZE, ALIGN) \
  asm_output_aligned_bss ((FILE), (DECL), (NAME), (SIZE), (ALIGN))

/* This implements the `alias' attribute.  */
#undef ASM_OUTPUT_DEF_FROM_DECLS
#define ASM_OUTPUT_DEF_FROM_DECLS(STREAM, DECL, TARGET) 		\
  do									\
    {									\
      const char *alias							\
	= IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (DECL));		\
      if (TREE_CODE (DECL) == FUNCTION_DECL)				\
        arm_pe_declare_function_type (STREAM, alias,			\
                                      TREE_PUBLIC (DECL));		\
      ASM_OUTPUT_DEF (STREAM, alias, IDENTIFIER_POINTER (TARGET));	\
    } while (0)

#define SUPPORTS_ONE_ONLY 1

#define SUBTARGET_ATTRIBUTE_TABLE \
  /* { name, min_len, max_len, decl_req, type_req, fn_type_req, handler } */ \
  { "selectany", 0, 0, true, false, false, arm_pe_handle_selectany_attribute }

/* Decide whether it is safe to use a local alias for a virtual function
   when constructing thunks.  */
#undef TARGET_USE_LOCAL_THUNK_ALIAS_P
#define TARGET_USE_LOCAL_THUNK_ALIAS_P(DECL) (!DECL_ONE_ONLY (DECL))

/* FIXME: SUPPORTS_WEAK && TARGET_HAVE_NAMED_SECTIONS is true,
   but for .jcr section to work we also need crtbegin and crtend
   objects.  */
#define TARGET_USE_JCR_SECTION 0


/* Support the ctors/dtors and other sections.  */

#define DRECTVE_SECTION_ASM_OP	"\t.section .drectve"

#define drectve_section() \
  (fprintf (asm_out_file, "%s\n", DRECTVE_SECTION_ASM_OP), \
   in_section = NULL)

#undef  TARGET_STRIP_NAME_ENCODING
#define TARGET_STRIP_NAME_ENCODING  arm_pe_strip_name_encoding

#define TARGET_VALID_DLLIMPORT_ATTRIBUTE_P arm_pe_valid_dllimport_attribute_p
#define TARGET_CXX_ADJUST_CLASS_AT_DEFINITION arm_pe_adjust_class_at_definition
#define TARGET_MANGLE_DECL_ASSEMBLER_NAME arm_pe_mangle_decl_assembler_name

#define ASM_WEAKEN_LABEL(FILE, NAME)	\
  do					\
    {					\
      fputs ("\t.weak\t", (FILE));	\
      assemble_name ((FILE), (NAME));	\
      fputc ('\n', (FILE));		\
    }					\
  while (0)


/* Hack.  See arm.h.  */
#undef REGISTER_TARGET_PRAGMAS
#define REGISTER_TARGET_PRAGMAS() do {					\
  REGISTER_TARGET_PRAGMAS_BASE ();					\
} while (0)

#ifdef HAVE_GAS_WEAK
#define SUPPORTS_WEAK 1
#endif
