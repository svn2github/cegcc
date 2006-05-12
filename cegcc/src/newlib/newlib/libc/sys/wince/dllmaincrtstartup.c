#include "sys/wcebase.h"

extern void __call_exitprocs(int code, void*);
extern void __gccmain(void);

extern int DllMain( HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved );

//#define DEBUG_DLL_LOAGING

#define NAME_LEN    64

extern __declspec(dllimport) 
BOOL __handle_ce_reent_(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved);

DWORD __libc_ThreadIndex;

int __DllMainCRTStartup(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
  /* initialize _REENT */
  if (!__handle_ce_reent_(hinstDLL, dwReason, lpvReserved))
    return FALSE;

  WCHAR cmdnameBufW[NAME_LEN];
  int len = GetModuleFileNameW(hinstDLL, cmdnameBufW, NAME_LEN);

  switch (dwReason) {
  case DLL_PROCESS_ATTACH: {
    OutputDebugStringW(L"DLL_PROCESS_ATTACH:");
    OutputDebugStringW(cmdnameBufW);
#ifdef DEBUG_DLL_LOAGING
    MessageBoxW(0, cmdnameBufW, L"dll loaded", 0);
#endif
    //	__init_c__();
//    _shared_attach();
//    __gccmain();
#ifdef DEBUG_DLL_LOAGING
    OutputDebugStringW(L"dll inited:");
    OutputDebugStringW(cmdnameBufW);
    MessageBoxW(0, cmdnameBufW, L"dll inited", 0);
#endif
    break; }
  case DLL_PROCESS_DETACH:
    OutputDebugStringW(L"DLL_PROCESS_DETACH:");
    OutputDebugStringW(cmdnameBufW);
    //	__call_exitprocs(0, 0); // called in _startup
    //	_shared_dettach();
    break;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  default:
    break;
  }

  /* DllMain is called in _DllMainCRTStartup */
  return TRUE;
}
