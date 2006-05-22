#include <windows.h>

int __DllMainCRTStartup(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved);

//#define DEBUG_DLL_LOAGING

/*
 * Leave as little static code as possible, 
 * most code in cegcc.dll
 *
 */
int _DllMainCRTStartup(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
  NKDbgPrintfW("%S: reason %d\n", __FUNCTION__, dwReason);

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
    NKDbgPrintfW("%S: ____DllMainCRTStartup failed\n", __FUNCTION__);
    return FALSE;
  }
  if (dwReason == DLL_PROCESS_ATTACH) {
      NKDbgPrintfW("%S: calling __gccmain\n", __FUNCTION__);
    __gccmain();
  }
  NKDbgPrintfW("%S: calling DllMain\n", __FUNCTION__);
  return DllMain(hinstDLL, dwReason, lpvReserved);
}
