// findfile.c
//
// Time-stamp: <12/02/01 14:42:40 keuchel@keuchelnt>

#include <wchar.h>

#include "sys/wcebase.h"
#include "sys/wcefile.h"

#include "sys/sysconf.h"

typedef struct _WIN32_FIND_DATAA {
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
	DWORD dwReserved0;
	DWORD dwReserved1;
	CHAR  cFileName[ MAX_PATH ];
	CHAR  cAlternateFileName[ 14 ];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

HANDLE __IMPORT
XCEFindFirstFileW(const wchar_t *oldpath, LPWIN32_FIND_DATAW lpfd);


HANDLE __IMPORT
XCEFindFirstFileA(const char *lpName, LPWIN32_FIND_DATAA lpfd)
{
  HANDLE hFind;
  WIN32_FIND_DATAW fdw;
  wchar_t lpNameNew[MAX_PATH];

  MultiByteToWideChar(CP_ACP, 0, lpName, -1, lpNameNew, COUNTOF(lpNameNew));

  hFind = XCEFindFirstFileW(lpNameNew, &fdw);

  if(hFind != INVALID_HANDLE_VALUE)
    {
      lpfd->dwFileAttributes = fdw.dwFileAttributes;
      lpfd->ftCreationTime = fdw.ftCreationTime;
      lpfd->ftLastAccessTime = fdw.ftLastAccessTime;
      lpfd->ftLastWriteTime = fdw.ftLastWriteTime;
      lpfd->nFileSizeHigh = fdw.nFileSizeHigh;
      lpfd->nFileSizeLow = fdw.nFileSizeLow;    

      // WinCE Data...
      lpfd->dwReserved0 = fdw.dwOID;

      WideCharToMultiByte(CP_ACP, 0, 
			  fdw.cFileName, -1, 
			  lpfd->cFileName, 
			  COUNTOF(lpfd->cFileName), 
			  NULL, NULL);

      // not in wince...
      lpfd->cAlternateFileName[0] = 0;
    }

  return hFind;
}

BOOL __IMPORT
XCEFindNextFileA(HANDLE hFind, LPWIN32_FIND_DATAA lpfd)
{
  WIN32_FIND_DATAW fdw;
  BOOL res;

  // is this needed?
  fdw.dwFileAttributes = lpfd->dwFileAttributes;
  fdw.ftCreationTime = lpfd->ftCreationTime;
  fdw.ftLastAccessTime = lpfd->ftLastAccessTime;
  fdw.ftLastWriteTime = lpfd->ftLastWriteTime;
  fdw.nFileSizeHigh = lpfd->nFileSizeHigh;
  fdw.nFileSizeLow = lpfd->nFileSizeLow;    
  fdw.dwOID = lpfd->dwReserved0;

  res = FindNextFileW(hFind, &fdw);

  if(res == TRUE)
    {
      lpfd->dwFileAttributes = fdw.dwFileAttributes;
      lpfd->ftCreationTime = fdw.ftCreationTime;
      lpfd->ftLastAccessTime = fdw.ftLastAccessTime;
      lpfd->ftLastWriteTime = fdw.ftLastWriteTime;
      lpfd->nFileSizeHigh = fdw.nFileSizeHigh;
      lpfd->nFileSizeLow = fdw.nFileSizeLow;    

      // WinCE Data...
      lpfd->dwReserved0 = fdw.dwOID;

      WideCharToMultiByte(CP_ACP, 0, 
			  fdw.cFileName, -1, 
			  lpfd->cFileName, 
			  COUNTOF(lpfd->cFileName), 
			  NULL, NULL);

      lpfd->cAlternateFileName[0] = 0;
    }

  return res;
}

HANDLE __IMPORT
XCEFindFirstFileW(const wchar_t *oldpath, LPWIN32_FIND_DATAW lpfd)
{
  wchar_t newpath[MAX_PATH];

  //wprintf(L"FindFirstFileOld: %s\n", oldpath);

  if(wcscmp(oldpath, L".") == 0)
    {
      XCEGetCurrentDirectoryW(sizeof(newpath), newpath);
    }
  else if(wcscmp(oldpath, L"..") == 0)
    {
      XCEGetCurrentDirectoryW(sizeof(newpath), newpath);
      if(wcscmp(newpath, L"\\") != 0)
	{
	  wchar_t *p;

	  if((p = wcsrchr(newpath, '\\')) != NULL)
	    *p = 0;
	  if(newpath[0] == 0)
	    wcscpy(newpath, L"\\");
	}
    }
  else
    {
      XCEFixPathW(oldpath, newpath);
    }

  //wprintf(L"FindFirstFileNew: %s\n", newpath);

  return FindFirstFileW(newpath, lpfd);
}
