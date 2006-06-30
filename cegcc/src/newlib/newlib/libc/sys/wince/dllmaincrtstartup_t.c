#include <windows.h>

#include <malloc.h>
#include <stdlib.h>

int __DllMainCRTStartup(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved);

//#define DEBUG_DLL_LOAGING

/*
 * Leave as little static code as possible, 
 * most code in cegcc.dll
 *
 */
int _DllMainCRTStartup(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
  NKDbgPrintfW(L"_DllMainCRTStartup: reason %d\n", dwReason);

  if (dwReason==DLL_PROCESS_ATTACH)
  {
    void _pei386_runtime_relocator();
    _pei386_runtime_relocator();
  }

#ifdef DEBUG_DLL_LOAGING
#define NAME_LEN    64
  WCHAR cmdnameBufW[NAME_LEN];
  int len = GetModuleFileNameW(hinstDLL, cmdnameBufW, NAME_LEN);
  NKDbgPrintfW(L"_DllMainCRTStartup called for %s\n", cmdnameBufW);
  MessageBoxW(0, cmdnameBufW, L"_DllMainCRTStartup called", 0);
#endif

  if (FALSE == __DllMainCRTStartup(hinstDLL, dwReason, lpvReserved))
  {
    NKDbgPrintfW(L"_DllMainCRTStartup: __DllMainCRTStartup failed\n");
    return FALSE;
  }
  if (dwReason == DLL_PROCESS_ATTACH) {
      NKDbgPrintfW(L"_DllMainCRTStartup: calling __gccmain\n");
    __gccmain();
  }
  NKDbgPrintfW(L"_DllMainCRTStartup: calling DllMain\n");
  return DllMain(hinstDLL, dwReason, lpvReserved);
}
