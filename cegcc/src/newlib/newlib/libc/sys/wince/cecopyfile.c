// copyfile.c
//
// Time-stamp: <12/02/01 14:43:26 keuchel@keuchelnt>

#include "sys/wcefile.h"
#include <wchar.h>
#include <alloca.h>

BOOL __IMPORT
XCECopyFileW(const wchar_t *oldpath1, const wchar_t *oldpath2, BOOL bFail)
{
	wchar_t newpath1[MAX_PATH];
	wchar_t newpath2[MAX_PATH];

	XCEFixPathW(oldpath1, newpath1);
	XCEFixPathW(oldpath2, newpath2);

	return CopyFileW(newpath1, newpath2, bFail);
}

BOOL __IMPORT
XCECopyFileA(const char *lpName1, const char *lpName2, BOOL bFail)
{
  wchar_t *lpNameNew1 = NULL;
  int len1 = strlen(lpName1) + 1;
  wchar_t *lpNameNew2 = NULL;
  int len2 = strlen(lpName2) + 1;

  BOOL res;

  lpNameNew1 = alloca(len1 * 2);
  lpNameNew2 = alloca(len2 * 2);

  MultiByteToWideChar(CP_ACP, 0, lpName1, -1, lpNameNew1, len1);
  MultiByteToWideChar(CP_ACP, 0, lpName2, -1, lpNameNew2, len2);

  res = XCECopyFileW(lpNameNew1, lpNameNew2, bFail);

  return res;
}
