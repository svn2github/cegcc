/* pseudo-reloc.c

   Written by Egor Duda <deo@logos-m.ru>
   Ported to arm-wince-pe by Pedro Alves <pedro_alves@portugalmail.pt>

   THIS SOFTWARE IS NOT COPYRIGHTED

   This source code is offered for use in the public domain. You may
   use, modify or distribute it freely.

   This code is distributed in the hope that it will be useful but
   WITHOUT ANY WARRANTY. ALL WARRENTIES, EXPRESS OR IMPLIED ARE HEREBY
   DISCLAMED. This includes but is not limited to warrenties of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

extern char __RUNTIME_PSEUDO_RELOC_LIST__;
extern char __RUNTIME_PSEUDO_RELOC_LIST_END__;
extern char __image_base__;

/* From pseudo-reloc.c. It goes into cegcc.dll.  */
extern void __do_pseudo_reloc (void* start, void* end, void* base);

void
_pei386_runtime_relocator ()
{
  __do_pseudo_reloc (&__RUNTIME_PSEUDO_RELOC_LIST__,
		   &__RUNTIME_PSEUDO_RELOC_LIST_END__,
		   &__image_base__);
}
