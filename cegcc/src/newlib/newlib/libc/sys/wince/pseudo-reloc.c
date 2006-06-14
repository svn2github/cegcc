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

#define	WIN32_LEAN_AND_MEAN
#include <windows.h>

extern char __RUNTIME_PSEUDO_RELOC_LIST__;
extern char __RUNTIME_PSEUDO_RELOC_LIST_END__;
extern char __image_base__;

typedef struct
  {
    DWORD addend;
    DWORD target;
  }
runtime_pseudo_reloc;

//#define DEBUG_PSEUDO_RELOC
//#define DEBUG_PSEUDO_RELOC_PRINTF

#ifdef DEBUG_PSEUDO_RELOC

#ifdef DEBUG_PSEUDO_RELOC_PRINTF
#define DGB_PRINT(FMT, ...) \
  do { \
  printf(FMT, __VA_ARGS__); \
  printf("\n"); \
  } while(0);
#else
#define DGB_PRINT(FMT, ...) \
  do { \
  WCHAR buf[512]; \
  wsprintfW(buf, L ## FMT, __VA_ARGS__); \
  MessageBoxW(0, buf, L"do_pseudo_reloc", 0); \
  } while(0);
#endif

#else
# define DGB_PRINT(FMT, ...) do ; while (0)
#endif

static void
do_pseudo_reloc (void* start, void* end, void* base)
{
  if (start != end)
    DGB_PRINT("s: %p, e: %p, b: %p", start, end, base);

  DWORD reloc_target;
  runtime_pseudo_reloc* r;
  for (r = (runtime_pseudo_reloc*) start; r < (runtime_pseudo_reloc*) end; r++)
    {
      /*
       * why is it that target comes "|= 0xf0000000"?
       * I don't know if this is a bug or a feature, 
       * probably something to do with 24-bit relocs.
       * for now, I just remove those four bits.
      */
      reloc_target = r->target & 0x0fffffff;
      DGB_PRINT("base: %p, r->target: 0x%x, r->addend: 0x%x", base, reloc_target, r->addend);
      DGB_PRINT("reloc_target: %p", (void*)reloc_target);
      DGB_PRINT("bef: *reloc_target: 0x%x", *(DWORD*)reloc_target);
      *((DWORD*) reloc_target) += r->addend;
      DGB_PRINT("aft: *reloc_target: 0x%x", *(DWORD*)reloc_target);
    }
}

void
_pei386_runtime_relocator ()
{
  do_pseudo_reloc (&__RUNTIME_PSEUDO_RELOC_LIST__,
		   &__RUNTIME_PSEUDO_RELOC_LIST_END__,
		   &__image_base__);
}
