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


#include <windows.h>

typedef struct
  {
    DWORD addend;
    DWORD target;
  }
runtime_pseudo_reloc;

//#define DEBUG_PSEUDO_RELOC

//#define DEBUG_PRINTF
//#define DEBUG_OUTPUTDEBUGSTRING
#define DEBUG_MESSAGEBOX

#ifndef DEBUG_PSEUDO_RELOC
# define DGB_PRINT(FMT, ...) do ; while (0)
#else

#ifdef DEBUG_PRINTF
#define DGB_PRINT(FMT, ...) \
  do { \
    printf (FMT, ##__VA_ARGS__); \
    printf ("\n"); \
  } while(0);
#endif

#ifdef DEBUG_MESSAGEBOX
#define DGB_PRINT(FMT, ...) \
  do { \
    WCHAR buf[512]; \
    wsprintfW (buf, L ## FMT, ##__VA_ARGS__); \
    MessageBoxW (0, buf, L"do_pseudo_reloc", 0); \
  } while(0);
#endif

#ifdef DEBUG_OUTPUTDEBUGSTRING
#define DGB_PRINT(FMT, ...) \
  do { \
    WCHAR buf[512]; \
    wsprintfW (buf, L ## FMT, ##__VA_ARGS__); \
    OutputDebugStringW (buf); \
  } while(0);
#endif

#endif

void
__do_pseudo_reloc (void* start, void* end, void* base)
{
  runtime_pseudo_reloc* r;
  DWORD reloc_target;

  if (start == end) 
    return; /* Nothing to do.  */

  DGB_PRINT("s: %p, e: %p, b: %p", start, end, base);

  for (r = (runtime_pseudo_reloc*) start; r < (runtime_pseudo_reloc*) end; r++)
    {
      reloc_target = (DWORD) base + r->target;
      DGB_PRINT("base: %p, r->target: 0x%x, r->addend: 0x%x, reloc_target: 0x%x", 
        base, r->target, r->addend, reloc_target);
      DGB_PRINT("bef: *reloc_target: 0x%x", *(DWORD*)reloc_target);
      *((DWORD*) reloc_target) += r->addend;
      DGB_PRINT("aft: *reloc_target: 0x%x", *(DWORD*)reloc_target);
    }
}
