// createfile.c
//
// Time-stamp: <07/06/01 10:05:31 keuchel@w2k>

#include <wchar.h>
#include <errno.h>

#include "sys/wcefile.h"
#include "sys/wceerror.h"

HANDLE
XCECreateFileW(
               LPCWSTR wfname,
               DWORD dwDesiredAccess,
               DWORD dwShareMode,
               LPSECURITY_ATTRIBUTES lpSecurityAttributes,
               DWORD dwCreationDisposition,
               DWORD dwFlagsAndAttributes,
               HANDLE hTemplateFile
               )
{
  HANDLE hFile;
  wchar_t wpath[MAX_PATH];
  wchar_t *p;
  DWORD dwError = 0;
  int len;

  len = wcslen(wfname);

  // This does not work when file is COM1: etc!
  if(wfname[len - 1] != ':')
  {
    XCEFixPathW(wfname, wpath);

    // In PocketEmulation, directories are created when
    // the file does not exist! Really stupid!

    if(dwCreationDisposition == OPEN_EXISTING)
    {
      DWORD dwAttr = XCEGetFileAttributesW(wpath);

      if(dwAttr == (DWORD) -1)
      {
        dwError = GetLastError();
        errno = _winerr2errno(dwError);
        return INVALID_HANDLE_VALUE;
      }
    }
  }
  else
  {
    wcscpy(wpath, wfname);
  }

  hFile = CreateFileW(wpath, dwDesiredAccess, dwShareMode,
    lpSecurityAttributes, dwCreationDisposition,
    dwFlagsAndAttributes, hTemplateFile);

  if(hFile == INVALID_HANDLE_VALUE)
  {
    dwError = GetLastError();

    if(dwError == ERROR_FILE_NOT_FOUND ||
      dwError == ERROR_PATH_NOT_FOUND)
    {
    }
    else
    {
      wchar_t buf[1024];
      swprintf(buf, L"WARNING: File problem: file=%s error=%d\n", wpath, dwError);
      int len = wcslen(buf);
      write(2, buf, len * sizeof(wchar_t));
    }

    errno = _winerr2errno(dwError);
  }

  return hFile;
}

HANDLE
XCECreateFileA(
               LPCSTR fname,
               DWORD dwDesiredAccess,
               DWORD dwShareMode,
               LPSECURITY_ATTRIBUTES lpSecurityAttributes,
               DWORD dwCreationDisposition,
               DWORD dwFlagsAndAttributes,
               HANDLE hTemplateFile
               )
{
  wchar_t wfname[MAX_PATH];
  HANDLE hFile;

  MultiByteToWideChar(CP_ACP, 0, fname, -1, wfname, COUNTOF(wfname));

  hFile = XCECreateFileW(wfname, dwDesiredAccess, dwShareMode,
    lpSecurityAttributes, dwCreationDisposition,
    dwFlagsAndAttributes, hTemplateFile);

  return hFile;
}
