// findfile.c
//
// Time-stamp: <12/02/01 14:42:40 keuchel@keuchelnt>

#include <wchar.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define COUNTOF(X) (sizeof (X)/sizeof ((X)[0]))

HANDLE WINAPI
FindFirstFileA (const char *lpName, LPWIN32_FIND_DATAA lpfd)
{
  HANDLE hFind;
  WIN32_FIND_DATAW fdw;
  wchar_t lpNameNew[MAX_PATH];

  MultiByteToWideChar(CP_ACP, 0, lpName, -1, lpNameNew, COUNTOF (lpNameNew));

  hFind = FindFirstFileW (lpNameNew, &fdw);

  if(hFind != INVALID_HANDLE_VALUE)
    {
      lpfd->dwFileAttributes = fdw.dwFileAttributes;
      lpfd->ftCreationTime = fdw.ftCreationTime;
      lpfd->ftLastAccessTime = fdw.ftLastAccessTime;
      lpfd->ftLastWriteTime = fdw.ftLastWriteTime;
      lpfd->nFileSizeHigh = fdw.nFileSizeHigh;
      lpfd->nFileSizeLow = fdw.nFileSizeLow;

#ifdef __COREDLL__
      lpfd->dwReserved0 = fdw.dwOID;
#endif

      WideCharToMultiByte (CP_ACP, 0,
			  fdw.cFileName, -1,
			  lpfd->cFileName,
			  COUNTOF (lpfd->cFileName),
			  NULL, NULL);

      // not in wince...
      lpfd->cAlternateFileName[0] = 0;
    }

  return hFind;
}

BOOL WINAPI
FindNextFileA (HANDLE hFind, LPWIN32_FIND_DATAA lpfd)
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
#ifdef __COREDLL__
  fdw.dwOID = lpfd->dwReserved0;
#endif

  res = FindNextFileW(hFind, &fdw);

  if(res == TRUE)
    {
      lpfd->dwFileAttributes = fdw.dwFileAttributes;
      lpfd->ftCreationTime = fdw.ftCreationTime;
      lpfd->ftLastAccessTime = fdw.ftLastAccessTime;
      lpfd->ftLastWriteTime = fdw.ftLastWriteTime;
      lpfd->nFileSizeHigh = fdw.nFileSizeHigh;
      lpfd->nFileSizeLow = fdw.nFileSizeLow;

#ifdef __COREDLL__
      lpfd->dwReserved0 = fdw.dwOID;
#endif

      WideCharToMultiByte(CP_ACP, 0,
			  fdw.cFileName, -1,
			  lpfd->cFileName,
			  COUNTOF(lpfd->cFileName),
			  NULL, NULL);

      lpfd->cAlternateFileName[0] = 0;
    }

  return res;
}
