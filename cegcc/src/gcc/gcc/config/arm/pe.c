/* Subroutines for insn-output.c for Windows NT.
   Contributed by Douglas Rupp (drupp@cs.washington.edu)
   Copyright (C) 1995, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005
   Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "output.h"
#include "tree.h"
#include "flags.h"
#include "tm_p.h"
#include "toplev.h"
#include "hashtab.h"
#include "ggc.h"

/* arm/PE specific attribute support.

   arm/PE has two new attributes:
   dllexport - for exporting a function/variable that will live in a dll
   dllimport - for importing a function/variable from a dll

   Microsoft allows multiple declspecs in one __declspec, separating
   them with spaces.  We do NOT support this.  Instead, use __declspec
   multiple times.
*/

static tree associated_type (tree);
static bool pe_dllexport_p (tree);
static bool pe_dllimport_p (tree);
static void pe_mark_dllexport (tree);
static void pe_mark_dllimport (tree);

/* This is we how mark internal identifiers with dllimport or dllexport
   attributes.  */
#ifndef DLL_IMPORT_PREFIX
#define DLL_IMPORT_PREFIX "@i."
#endif
#ifndef DLL_EXPORT_PREFIX
#define DLL_EXPORT_PREFIX "@e."
#endif

/* Return the type that we should use to determine if DECL is
   imported or exported.  */

static tree
associated_type (tree decl)
{
  return  (DECL_CONTEXT (decl) && TYPE_P (DECL_CONTEXT (decl)))
            ?  DECL_CONTEXT (decl) : NULL_TREE;
}


/* Return true if DECL is a dllexport'd object.  */

static bool
pe_dllexport_p (tree decl)
{
  if (TREE_CODE (decl) != VAR_DECL
       && TREE_CODE (decl) != FUNCTION_DECL)
    return false;

  if (lookup_attribute ("dllexport", DECL_ATTRIBUTES (decl)))
    return true;

  /* Also mark class members of exported classes with dllexport.  */
  if (associated_type (decl)
      && lookup_attribute ("dllexport",
			    TYPE_ATTRIBUTES (associated_type (decl))))
    return arm_pe_type_dllexport_p (decl);

  return false;
}

static bool
pe_dllimport_p (tree decl)
{
  if (TREE_CODE (decl) != VAR_DECL
       && TREE_CODE (decl) != FUNCTION_DECL)
    return false;

  /* Lookup the attribute rather than rely on the DECL_DLLIMPORT_P flag.
     We may need to override an earlier decision.  */
  if (lookup_attribute ("dllimport", DECL_ATTRIBUTES (decl)))
    return true;

  /* The DECL_DLLIMPORT_P flag was set for decls in the class definition
     by  targetm.cxx.adjust_class_at_definition.  Check again to emit
     warnings if the class attribute has been overriden by an
     out-of-class definition.  */
  if (associated_type (decl)
      && lookup_attribute ("dllimport",
			    TYPE_ATTRIBUTES (associated_type (decl))))
    return arm_pe_type_dllimport_p (decl);

  return false;
}

/* Handle the -mno-fun-dllimport target switch.  */
bool
arm_pe_valid_dllimport_attribute_p (tree decl)
{
   if (TARGET_NOP_FUN_DLLIMPORT && TREE_CODE (decl) == FUNCTION_DECL)
     return false;
   return true;
}

/* Return nonzero if SYMBOL is marked as being dllexport'd.  */

int
arm_pe_dllexport_name_p (const char *symbol)
{
  return (strncmp (DLL_EXPORT_PREFIX, symbol,
		   strlen (DLL_EXPORT_PREFIX)) == 0);
}

/* Return nonzero if SYMBOL is marked as being dllimport'd.  */

int
arm_pe_dllimport_name_p (const char *symbol)
{
  return (strncmp (DLL_IMPORT_PREFIX, symbol,
		   strlen (DLL_IMPORT_PREFIX)) == 0);
}

/* Mark a DECL as being dllexport'd.
   Note that we override the previous setting (e.g.: dllimport).  */

static void
pe_mark_dllexport (tree decl)
{
  const char *oldname;
  char  *newname;
  rtx rtlname;
  rtx symref;
  tree idp;

  rtlname = XEXP (DECL_RTL (decl), 0);
  if (GET_CODE (rtlname) == MEM)
    rtlname = XEXP (rtlname, 0);
  gcc_assert (GET_CODE (rtlname) == SYMBOL_REF);
  oldname = XSTR (rtlname, 0);
  if (arm_pe_dllimport_name_p (oldname))
    {
      warning (0, "inconsistent dll linkage for %q+D, dllexport assumed",
	       decl);
     /* Remove DLL_IMPORT_PREFIX.  */
      oldname += strlen (DLL_IMPORT_PREFIX);
    }
  else if (arm_pe_dllexport_name_p (oldname))
    return;  /*  already done  */

  newname = alloca (strlen (DLL_EXPORT_PREFIX) + strlen (oldname) + 1);
  sprintf (newname, "%s%s", DLL_EXPORT_PREFIX, oldname);

  /* We pass newname through get_identifier to ensure it has a unique
     address.  RTL processing can sometimes peek inside the symbol ref
     and compare the string's addresses to see if two symbols are
     identical.  */
  idp = get_identifier (newname);

  symref = gen_rtx_SYMBOL_REF (Pmode, IDENTIFIER_POINTER (idp));
  SYMBOL_REF_DECL (symref) = decl;
  XEXP (DECL_RTL (decl), 0) = symref;
}

/* Mark a DECL as being dllimport'd.  */

static void
pe_mark_dllimport (tree decl)
{
  const char *oldname;
  char  *newname;
  tree idp;
  rtx rtlname, newrtl;
  rtx symref;

  rtlname = XEXP (DECL_RTL (decl), 0);
  if (GET_CODE (rtlname) == MEM)
    rtlname = XEXP (rtlname, 0);
  gcc_assert (GET_CODE (rtlname) == SYMBOL_REF);
  oldname = XSTR (rtlname, 0);
  if (arm_pe_dllexport_name_p (oldname))
    {
      error ("%qs declared as both exported to and imported from a DLL",
             IDENTIFIER_POINTER (DECL_NAME (decl)));
      return;
    }
  else if (arm_pe_dllimport_name_p (oldname))
    {
      /* Already done, but do a sanity check to prevent assembler
	 errors.  */
      gcc_assert (DECL_EXTERNAL (decl) && TREE_PUBLIC (decl)
		  && DECL_DLLIMPORT_P (decl));
      return;
    }

  newname = alloca (strlen (DLL_IMPORT_PREFIX) + strlen (oldname) + 1);
  sprintf (newname, "%s%s", DLL_IMPORT_PREFIX, oldname);

  /* We pass newname through get_identifier to ensure it has a unique
     address.  RTL processing can sometimes peek inside the symbol ref
     and compare the string's addresses to see if two symbols are
     identical.  */
  idp = get_identifier (newname);

  symref = gen_rtx_SYMBOL_REF (Pmode, IDENTIFIER_POINTER (idp));
  SYMBOL_REF_DECL (symref) = decl;
  newrtl = gen_rtx_MEM (Pmode,symref);
  XEXP (DECL_RTL (decl), 0) = newrtl;

  DECL_DLLIMPORT_P (decl) = 1;
}

void
arm_pe_encode_section_info (tree decl, rtx rtl, int first)
{
  default_encode_section_info (decl, rtl, first);

  /* Mark the decl so we can tell from the rtl whether the object is
     dllexport'd or dllimport'd.  tree.c: merge_dllimport_decl_attributes
     handles dllexport/dllimport override semantics.  */

  if (pe_dllexport_p (decl))
    pe_mark_dllexport (decl);
  else if (pe_dllimport_p (decl))
    pe_mark_dllimport (decl);
  /* It might be that DECL has been declared as dllimport, but a
     subsequent definition nullified that.  Assert that
     tree.c: merge_dllimport_decl_attributes has removed the attribute
     before the RTL name was marked with the DLL_IMPORT_PREFIX.  */
  else
    gcc_assert (!((TREE_CODE (decl) == FUNCTION_DECL
	    	   || TREE_CODE (decl) == VAR_DECL)
		  && rtl != NULL_RTX
		  && GET_CODE (rtl) == MEM
		  && GET_CODE (XEXP (rtl, 0)) == MEM
		  && GET_CODE (XEXP (XEXP (rtl, 0), 0)) == SYMBOL_REF
		  && arm_pe_dllimport_name_p (XSTR (XEXP (XEXP (rtl, 0), 0), 0))));
}

/* Output a reference to a label. Symbols don't
   have a prefix (unless they are marked dllimport or dllexport).  */

void arm_pe_output_labelref (FILE *stream, const char *name)
{
  if (strncmp (name, DLL_IMPORT_PREFIX, strlen (DLL_IMPORT_PREFIX))
      == 0)
    /* A dll import */
    {
      asm_fprintf (stream, "__imp_%U%s", arm_strip_name_encoding (name));
    }
  else
    /* Everything else.  */
    {
      asm_fprintf (stream, "%U%s", arm_strip_name_encoding (name));
    }
}

void
arm_pe_unique_section (tree decl, int reloc)
{
  int len;
  const char *name, *prefix;
  char *string;

  name = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl));
  name = arm_strip_name_encoding (name);

  /* The object is put in, for example, section .text$foo.
     The linker will then ultimately place them in .text
     (everything from the $ on is stripped). Don't put
     read-only data in .rdata section to avoid a PE linker
     bug when .rdata$* grouped sections are used in code
     without a .rdata section.  */
  if (TREE_CODE (decl) == FUNCTION_DECL)
    prefix = ".text$";
  else if (decl_readonly_section (decl, reloc))
    prefix = ".rdata$";
  else
    prefix = ".data$";
  len = strlen (name) + strlen (prefix);
  string = alloca (len + 1);
  sprintf (string, "%s%s", prefix, name);

  DECL_SECTION_NAME (decl) = build_string (len, string);
}


/* The Microsoft linker requires that every function be marked as
DT_FCN.  When using gas on cygwin, we must emit appropriate .type
directives.  */

#include "gsyms.h"

/* Mark a function appropriately.  This should only be called for
functions for which we are not emitting COFF debugging information.
FILE is the assembler output file, NAME is the name of the
function, and PUBLIC is nonzero if the function is globally
visible.  */

void
arm_pe_declare_function_type (FILE *file, const char *name, int public)
{
  fprintf (file, "\t.def\t");
  assemble_name (file, name);
  fprintf (file, ";\t.scl\t%d;\t.type\t%d;\t.endef\n",
    public ? (int) C_EXT : (int) C_STAT,
    (int) DT_FCN << N_BTSHFT);
}

/* Keep a list of external functions.  */

struct extern_list GTY(())
{
  struct extern_list *next;
  tree decl;
  const char *name;
};

static GTY(()) struct extern_list *extern_head;

/* Assemble an external function reference.  We need to keep a list of
these, so that we can output the function types at the end of the
assembly.  We can't output the types now, because we might see a
definition of the function later on and emit debugging information
for it then.  */

void
arm_pe_record_external_function (tree decl, const char *name)
{
  struct extern_list *p;

  p = (struct extern_list *) ggc_alloc (sizeof *p);
  p->next = extern_head;
  p->decl = decl;
  p->name = name;
  extern_head = p;
}

/* Keep a list of exported symbols.  */

struct export_list GTY(())
{
  struct export_list *next;
  const char *name;
  int is_data;		/* used to type tag exported symbols.  */
};

static GTY(()) struct export_list *export_head;

/* Assemble an export symbol entry.  We need to keep a list of
these, so that we can output the export list at the end of the
assembly.  We used to output these export symbols in each function,
but that causes problems with GNU ld when the sections are
linkonce.  */

void
arm_pe_record_exported_symbol (const char *name, int is_data)
{
  struct export_list *p;

  p = (struct export_list *) ggc_alloc (sizeof *p);
  p->next = export_head;
  p->name = name;
  p->is_data = is_data;
  export_head = p;
}

/* This is called at the end of assembly.  For each external function
which has not been defined, we output a declaration now.  We also
output the .drectve section.  */

void
arm_pe_file_end (void)
{
  struct extern_list *p;

//  ix86_file_end ();

  for (p = extern_head; p != NULL; p = p->next)
  {
    tree decl;

    decl = p->decl;

    /* Positively ensure only one declaration for any given symbol.  */
    if (! TREE_ASM_WRITTEN (decl)
      && TREE_SYMBOL_REFERENCED (DECL_ASSEMBLER_NAME (decl)))
    {
      TREE_ASM_WRITTEN (decl) = 1;
      arm_pe_declare_function_type (asm_out_file, p->name,
        TREE_PUBLIC (decl));
    }
  }

  if (export_head)
  {
    struct export_list *q;
    drectve_section ();
    for (q = export_head; q != NULL; q = q->next)
    {
      fprintf (asm_out_file, "\t.ascii \" -export:%s%s\"\n",
        arm_strip_name_encoding (q->name),
        (q->is_data) ? ",data" : "");
    }
  }
}

#include "gt-pe.h"
