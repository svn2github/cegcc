// removedir.c
//
// Time-stamp: <12/02/01 14:43:12 keuchel@keuchelnt>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "sys/ceshared.h"
#include "sys/wcefile.h"
#include <alloca.h>
#include <wchar.h>

BOOL __IMPORT
XCERemoveDirectoryW(const wchar_t *oldpath)
{
  wchar_t newpath[MAX_PATH];
  XCEFixPathW(oldpath, newpath);
  return RemoveDirectoryW(newpath);
}

BOOL __IMPORT
XCERemoveDirectoryA(const char *lpName)
{
	wchar_t *lpNameNew = NULL;
	int len = strlen(lpName) + 1;
	BOOL res;

	lpNameNew = alloca(len * 2);
	MultiByteToWideChar(CP_ACP, 0, lpName, -1, lpNameNew, len);
	res = XCERemoveDirectoryW(lpNameNew);

	return res;
}
