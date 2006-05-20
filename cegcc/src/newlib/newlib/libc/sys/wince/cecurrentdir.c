// currentdir.c
//
// Time-stamp: <12/02/01 14:39:45 keuchel@keuchelnt>

#include <errno.h>
#include <reent.h>
#include "getreent.h"

#include "sys/wcetrace.h"
#include <sys/param.h>

// per process as on WinNT
// ### TODO, we need a mutex here

static wchar_t _current_dirw[MAX_PATH+1] = L"\\";
static wchar_t _current_root_dirw[MAX_PATH+1] = L"";

#define _CURRENTDIRW _current_dirw
#define _CURRENTROOTDIRW _current_root_dirw

DWORD __IMPORT
XCEGetCurrentDirectoryW(DWORD dwSize, wchar_t *buf)
{
	size_t len = wcslen(_CURRENTDIRW);
	if (dwSize == 0 && buf == 0)
		return len+1;
	wcsncpy(buf, _CURRENTDIRW, dwSize-1); // room for null character
	if (dwSize > len)
		return len;
	else
		return len+1;
}

DWORD __IMPORT
XCEGetCurrentDirectoryA(DWORD dwSize, char *buf)
{
	DWORD dwLen;
	wchar_t wbuf[MAX_PATH+1];

	dwLen = XCEGetCurrentDirectoryW(dwSize, wbuf);
	if (dwSize == 0 && buf == 0)
		return dwSize;
	WideCharToMultiByte(CP_ACP, 0, wbuf, -1, buf, MIN(dwLen, dwSize), NULL, NULL);
	buf[MIN(dwLen, dwSize)] = 0;
	return dwLen;
}

BOOL __IMPORT
XCESetCurrentDirectoryW(const wchar_t *wdir)
{
	DWORD dwAttr;
	wchar_t wtmp[MAX_PATH];

	int wlen = wcslen(wdir);
	if (wlen > (MAX_PATH-1))
		return 0;
	else if (wlen>0 && !(wdir[wlen-1] == '\\' || wdir[wlen-1] == '/') && wlen > (MAX_PATH-2))
		return 0;

	XCEFixPathW(wdir, wtmp);

	if((dwAttr = XCEGetFileAttributesW(wtmp)) == 0xFFFFFFFF)
	{
		errno = _winerr2errno(GetLastError());
		WCETRACE(WCE_IO, "XCESetCurrentDirectoryW: return FALSE, errno: %d", errno);
		return FALSE;
	}

	if((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		errno = ENOTDIR;
		WCETRACE(WCE_IO, "XCESetCurrentDirectoryW: return FALSE, errno: ENOTDIR");
		return FALSE;
	}

	wcscpy(_CURRENTDIRW, wtmp);
	return TRUE;
}

BOOL __IMPORT
XCESetCurrentDirectoryA(const char *dir)
{
	wchar_t wdir[MAX_PATH];
	int len = strlen(dir);
	if (len > (MAX_PATH-1))
		return 0;
	else if (len>0 && !(dir[len-1] == '\\' || dir[len-1] == '/') && len > (MAX_PATH-2))
		return 0;
	MultiByteToWideChar(CP_ACP, 0, dir, -1, wdir, COUNTOF(wdir));
	return XCESetCurrentDirectoryW(wdir);
}

DWORD
XCEGetCurrentRootDirectoryW(DWORD dwSize, wchar_t *buf)
{
	wcscpy(buf, _CURRENTROOTDIRW);
	return wcslen(buf);
}

DWORD
XCEGetCurrentRootDirectoryA(DWORD dwSize, char *buf)
{
	DWORD dwLen;
	wchar_t wbuf[MAX_PATH];

	dwLen = XCEGetCurrentRootDirectoryW(sizeof(wbuf), wbuf);
	WideCharToMultiByte(CP_ACP, 0, wbuf, -1, buf, dwSize, NULL, NULL);
	return dwLen;
}

BOOL
XCESetCurrentRootDirectoryW(const wchar_t *wdir)
{
	DWORD dwAttr;
	wchar_t wtmp[MAX_PATH];

	XCEFixPathW(wdir, wtmp);

	if((dwAttr = XCEGetFileAttributesW(wtmp)) == 0xFFFFFFFF)
	{
		errno = ENOENT;
//		errno =_winerr2errno(GetLastError());
		return FALSE;
	}

	if((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		errno = ENOTDIR;
		return FALSE;
	}

	wcscpy(_CURRENTROOTDIRW, wtmp);

	WCETRACE(WCE_IO, "XCESetCurrentRootDirectoryW: success, new root is \"%s\"", _CURRENTROOTDIRW);

	return TRUE;
}

BOOL
XCESetCurrentRootDirectoryA(const char *dir)
{
	wchar_t wdir[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, dir, -1, wdir, COUNTOF(wdir));
	return XCESetCurrentRootDirectoryW(wdir);
}
