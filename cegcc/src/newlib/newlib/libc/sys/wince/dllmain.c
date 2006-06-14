#define	WIN32_LEAN_AND_MEAN
#include <windows.h>

//#define DEBUG_DLL_LOAGING

BOOL DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID foo)
{
#ifdef DEBUG_DLL_LOAGING
#define NAME_LEN    64
  WCHAR cmdnameBufW[NAME_LEN];
  int len = GetModuleFileNameW(hinstDLL, cmdnameBufW, NAME_LEN);
  NKDbgPrintfW(L"default DllMain called for %s\n", cmdnameBufW);
  MessageBoxW(0, cmdnameBufW, L"default DllMain called", 0);
#endif

  switch (dwReason) {
  case DLL_PROCESS_ATTACH:
  case DLL_PROCESS_DETACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  default:
    break;
  }
  return TRUE;
}

