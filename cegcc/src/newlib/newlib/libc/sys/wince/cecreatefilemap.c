// createfilemap.c
//
// Time-stamp: <03/01/02 22:34:54 keuchel@netwave.de>

#include <errno.h>
#include <wchar.h>

#include "sys/wcefile.h"

#define COUNTOF(X) (sizeof(X)/sizeof((X)[0]))

HANDLE
XCECreateFileForMappingW(LPCWSTR wfname,
					  DWORD dwDesiredAccess,
					  DWORD dwShareMode,
					  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
					  DWORD dwCreationDisposition,
					  DWORD dwFlagsAndAttributes,
					  HANDLE hTemplateFile
					  );

HANDLE
XCECreateFileForMappingA(
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

  hFile = XCECreateFileForMappingW(wfname, dwDesiredAccess, dwShareMode,
				   lpSecurityAttributes, dwCreationDisposition,
				   dwFlagsAndAttributes, hTemplateFile);

  return hFile;
}

HANDLE
XCECreateFileForMappingW(
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

  XCEFixPathW(wfname, wpath);

  SetLastError(0);

  hFile = CreateFileForMappingW(wpath, dwDesiredAccess, dwShareMode,
		      lpSecurityAttributes, dwCreationDisposition,
		      dwFlagsAndAttributes, hTemplateFile);

  if(hFile == INVALID_HANDLE_VALUE)
    {
      dwError = GetLastError();

      XCEShowMessageA("CreateFileForMapping: %d", dwError);

      errno = _winerr2errno(dwError);
    }

  return hFile;
}
