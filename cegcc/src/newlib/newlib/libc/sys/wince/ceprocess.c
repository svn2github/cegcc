// ceprocess.h
//
// Time-stamp: <08/09/01 10:43:16 keuchel@netwave.de>

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys/wcetrace.h"
#include "sys/wcebase.h"
#include "sys/wcefile.h"
#include "sys/ceshared.h"



typedef struct _STARTUPINFOA {
  DWORD   cb;
  LPSTR   lpReserved;
  LPSTR   lpDesktop;
  LPSTR   lpTitle;
  DWORD   dwX;
  DWORD   dwY;
  DWORD   dwXSize;
  DWORD   dwYSize;
  DWORD   dwXCountChars;
  DWORD   dwYCountChars;
  DWORD   dwFillAttribute;
  DWORD   dwFlags;
  WORD    wShowWindow;
  WORD    cbReserved2;
  LPBYTE  lpReserved2;
  HANDLE  hStdInput;
  HANDLE  hStdOutput;
  HANDLE  hStdError;
} STARTUPINFOA, *LPSTARTUPINFOA;

#define INFINITE 0xffffffffL
#define STARTF_USESHOWWINDOW 0x00000001
#define STARTF_USESIZE       0x00000002
#define STARTF_USESTDHANDLES 0x00000100
#define SW_HIDE 0

// NOTE: This function cannot be used as the real SearchPath!!!
static int
XCESearchPathA(const char *pathlist, const char *file, char *retpath)
{
	char path[MAX_PATH];
	char *ps, *pe;
	char *pl;
	struct stat st;
	char *pend;
	char szCurrentDir[MAX_PATH];

	getcwd(szCurrentDir, sizeof(szCurrentDir));

	if(pathlist == NULL)
		return -1;

	ps = pe = pl = alloca(strlen(pathlist) + strlen(szCurrentDir) + 2);
	sprintf(ps, "%s;%s", szCurrentDir, pathlist);

	pend = pl + strlen(pl);

	while(ps < pend)
	{
		while(*pe && *pe != ';')
			pe++;
		*pe++ = 0;

		if(!strcmp(ps, "/") || !strcmp(ps, "\\"))
		{
			sprintf(path, "%s%s", ps, file);
		}
		else
		{
			sprintf(path, "%s\\%s", ps, file);
		}

		//printf("SearchPath: %s\n", path);

		if(stat(path, &st) == 0)
		{
			strcpy(retpath, path);
			return 0;
		}
		ps = pe;
	}

	return -1;
}

extern char **environ;

BOOL
XCEFreeEnvironmentStrings(LPCSTR buf)
{
  free((LPVOID) buf);
  return TRUE;
}

LPVOID
XCEGetEnvironmentStrings(VOID)
{
  LPSTR buf;
  int i;
  int size = 0;
  char *p;

  for(i = 0; environ[i] != NULL; i++)
  {
    size += strlen(environ[i]) + 1;
  }
  size += 1;

  if((buf = malloc(size)) == NULL)
    return NULL;

  for(i = 0, p = buf; environ[i] != NULL; i++)
  {
    strcpy(p, environ[i]);
    p += strlen(environ[i]) + 1;
  }
  *p = 0;

  return (LPVOID) buf;
}

BOOL
XCECreateProcessA(LPCSTR appname, 
                  LPCSTR commandline,
                  LPVOID lpsaproc, 
                  LPVOID lpsathread, 
                  BOOL bInheritHandles,
                  DWORD dwCreationFlags,
                  LPVOID lpEnv,
                  LPCSTR lpCurDir,
                  LPSTARTUPINFOA lpsi,
                  LPPROCESS_INFORMATION lppi
                  )
{
  wchar_t wszAppName[126];
  char szAppName[126];
  wchar_t *wszCommandLine = NULL;
  char *szCommandLine = NULL;
  char *p;
  DWORD dwExitCode = 0;
  char path[MAX_PATH];
  char *d;
  char szCurrentDir[MAX_PATH];
  BOOL bRes;
  int commandlen;
  char buf[126];

  WCETRACE(WCE_IO, "XCECreateProcess(%s,%s)", appname, commandline);

  if(appname != NULL)
  {
    int len;
    char *p;

    len = strlen(appname) + strlen(commandline) + 4;
    p = alloca(len);

    if(appname[0] == '"')
      sprintf(p, "%s %s", appname, commandline);
    else
      sprintf(p, "\"%s\" %s", appname, commandline);

    commandline = p;
  }

  if(commandline[0] == '"')
  {
    for(p = (char *)commandline + 1, d = szAppName; *p && *p != '"';)
    {
      *d++ = *p++;
    }

    if(*p != 0)
      p++;
  }
  else
  {
    for(p = (char *)commandline, d = szAppName; *p && *p != ' ';)
    {
      *d++ = *p++;
    }
  }

  *d++ = 0;

  while(*p == ' ')
    p++;

  commandlen = strlen(p);
  szCommandLine = malloc(commandlen+1);
  wszCommandLine = malloc((commandlen+1)*2);
  strcpy(szCommandLine, p);

  for(p = szAppName; *p; p++)
  {
    if(*p == '/')
      *p = '\\';
  }

  if(szAppName[0] != '\\')
  {
    if(strstr(szAppName, ".exe") == NULL)
      strcat(szAppName, ".exe");

    if(XCESearchPathA(getenv("PATH"), szAppName, path) == 0)
      strcpy(szAppName, path);

    // fix path, in case in contains forward slashes...
    for(p = szAppName; *p; p++)
    {
      if(*p == '/')
        *p = '\\';
    }
  }

  //printf("App: %s Command: %s\n", szAppName, szCommandLine);

  MultiByteToWideChar(CP_ACP, 0, szAppName, -1,
    wszAppName, sizeof(wszAppName)/2);
  MultiByteToWideChar(CP_ACP, 0, szCommandLine, -1,
    wszCommandLine, commandlen + 1);

  // pass path to child...
  XCEGetCurrentDirectoryA(sizeof(szCurrentDir), szCurrentDir);
  // PWD is better, as this is checked by some unix programs...
  // XCESetEnvironmentVariableInRegA("PWD", szCurrentDir);
  // Only set it locally, passed via shared environment
  setenv("PWD", szCurrentDir, 1);
  sprintf(buf, "%x", GetCurrentProcessId());
  setenv("PPID", buf, 1);

  xceshared_lock();

  // initialize shared section for child
  if(lpEnv != NULL)
  {
    xceshared_setenvironblock(lpEnv);
  }
  else
  {
    char *lpCurrentEnv = XCEGetEnvironmentStrings();
    xceshared_setenvironblock(lpCurrentEnv);
    XCEFreeEnvironmentStrings(lpCurrentEnv);
  }

  if(lpCurDir != NULL)
    xceshared_setcwd(lpCurDir);
  else
    xceshared_setcwd(szCurrentDir);

  if(lpsi->dwFlags & STARTF_USESTDHANDLES)
  {
    xceshared_setstdhandle(STD_INPUT_HANDLE,
      MAKEINHERIT(lpsi->hStdInput));
    xceshared_setstdhandle(STD_OUTPUT_HANDLE, 
      MAKEINHERIT(lpsi->hStdOutput));
    xceshared_setstdhandle(STD_ERROR_HANDLE, 
      MAKEINHERIT(lpsi->hStdError));

    // must increment open count because the
    // call normally closes the pipes. emacs
    // does no duplicate them...
    pipe_increment_opencount(MAKEINHERIT(lpsi->hStdInput));
    pipe_increment_opencount(MAKEINHERIT(lpsi->hStdOutput));
    pipe_increment_opencount(MAKEINHERIT(lpsi->hStdError));
  }
  else
  {
#if 0
    // this can interfere with the parents own in/out!
    // showed up when calling gnuplot from maxima (system())
    // commented out Thu Sep 06 2001
    xceshared_setstdhandle(STD_INPUT_HANDLE, 
      XCEGetStdHandle(STD_INPUT_HANDLE));
    xceshared_setstdhandle(STD_OUTPUT_HANDLE, 
      XCEGetStdHandle(STD_OUTPUT_HANDLE));
    xceshared_setstdhandle(STD_ERROR_HANDLE, 
      XCEGetStdHandle(STD_ERROR_HANDLE));
#endif
  }

  WCETRACE(WCE_IO, "shared output set to %x", 
    xceshared_getstdhandle(STD_OUTPUT_HANDLE));

  xceshared_setshowwindow(TRUE);
  if(lpsi->dwFlags & STARTF_USESHOWWINDOW)
  {
    if(lpsi->wShowWindow == SW_HIDE)
      xceshared_setshowwindow(FALSE);
  }

  // TODO: released too early, but the child must be able to access it
  xceshared_release();

  XCETraceW(L"CreateProcess(%s, %.50s)", wszAppName, wszCommandLine);

  //SYNC;

  STARTUPINFOW wsi;

  /* same sizes */
  memcpy(&wsi, lpsi, sizeof(STARTUPINFOW));

  int len = strlen(lpsi->lpDesktop);
  wchar_t* wDesktop = alloca((len+1)*sizeof(wchar_t));
  MultiByteToWideChar(CP_ACP, 0, lpsi->lpDesktop, -1,
    wDesktop, len + 1);
  wsi.lpDesktop = wDesktop;

  len = strlen(lpsi->lpTitle);
  wchar_t* wTitle = alloca((len+1)*sizeof(wchar_t));
  MultiByteToWideChar(CP_ACP, 0, lpsi->lpTitle, -1,
    wTitle, len + 1);
  wsi.lpTitle = wTitle;

  if (lpsi->lpReserved)
  {
    len = strlen(lpsi->lpReserved);
    wchar_t* wReserved = alloca((len+1)*sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, lpsi->lpReserved, -1,
      wReserved, len + 1);
    wsi.lpReserved = wReserved;
  }

  bRes = CreateProcessW(wszAppName, wszCommandLine, NULL, NULL,
    FALSE, 0, NULL, NULL, &wsi, lppi);

  // TODO: Wait for process ready (event)
  // TODO: Remove shared stuff, otherwise a non-child may get
  // confused!
  if(bRes == FALSE)
  {
    xceshared_reset();
  }

  free(szCommandLine);
  free(wszCommandLine);

  return bRes;
}

__IMPORT int
XCEExecuteProcessA(const char *commandline, BOOL bWait, LPDWORD lpdwProcId)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	char szAppName[126];
	char *szCommandLine = NULL;
	char *p;
	DWORD dwExitCode = 0;
	char path[MAX_PATH];
	char *d;
	BOOL bRes;
	int commandlen;

	WCETRACE(WCE_IO, "ExecuteProcessA(%s)", commandline);

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	if(commandline[0] == '"')
	{
		for(p = (char *)commandline + 1, d = szAppName; *p && *p != '"';)
		{
			*d++ = *p++;
		}
		p++;
	}
	else
	{
		for(p = (char *)commandline, d = szAppName; *p && *p != ' ';)
		{
			*d++ = *p++;
		}
	}

	*d++ = 0;

	while(*p == ' ')
		p++;

	commandlen = strlen(p);
	szCommandLine = malloc(commandlen+1);
	strcpy(szCommandLine, p);

	for(p = szAppName; *p; p++)
	{
		if(*p == '/')
			*p = '\\';
	}

	if(szAppName[0] != '\\')
	{
		if(!strstr(szAppName, ".exe"))
			strcat(szAppName, ".exe");

		if(XCESearchPathA(getenv("PATH"), szAppName, path) == 0)
			strcpy(szAppName, path);

		// fix path, in case in contains forward slashes...
		for(p = szAppName; *p; p++)
		{
			if(*p == '/')
				*p = '\\';
		}
	}

	bRes = XCECreateProcessA(szAppName, szCommandLine, NULL, NULL,
		FALSE, 0, NULL, NULL, &si, &pi);

	free(szCommandLine);

	if(bRes == FALSE)
		return -1;

	if(bWait)
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
	}
	else
	{
		if(lpdwProcId)
			*lpdwProcId = pi.dwProcessId;
	}

	XCECloseHandle(pi.hThread);
	XCECloseHandle(pi.hProcess);

	return dwExitCode;
}

__IMPORT int
XCEWaitProcess(DWORD dwProcId)
{
	HANDLE hProcess;
	DWORD dwExitCode;

	if((hProcess = OpenProcess(0, FALSE, dwProcId)) == NULL)
		return -1;

	WaitForSingleObject(hProcess, INFINITE);
	GetExitCodeProcess(hProcess, &dwExitCode);

	XCECloseHandle(hProcess);

	return dwExitCode;
}

__IMPORT DWORD
XCESearchPath2A(LPCSTR  lpszPath,	// address of search path 
				LPCSTR  lpszFile,	// address of filename 
				LPCSTR  lpszExtension, // address of extension 
				DWORD   cchReturnBuffer, // size, in characters, of buffer 
				LPSTR   lpszReturnBuffer, // address of buffer found filename 
				LPSTR   *plpszFilePart // address of pointer file component 
				)
{
	char fname[256];
	char buf[256];
	wchar_t wbuf[256];
	int res;
	char *p;

	if(lpszPath == NULL)
		lpszPath = getenv("PATH");

	strcpy(fname, lpszFile);

	if(lpszExtension)
		strcat(fname, lpszExtension);

	fixpath(fname, buf);
	strcpy(fname, buf);

	if(fname[0] == '\\')
	{
		mbstowcs(wbuf, buf, strlen(buf) + 1);
		if(XCEGetFileAttributesW(wbuf) == (DWORD) -1)
			return 0;

		strcpy(lpszReturnBuffer, fname);

		if((p = strrchr(lpszReturnBuffer, '\\')) != NULL)
			*plpszFilePart = ++p;
		return strlen(lpszReturnBuffer);
	}

	if((res = XCESearchPathA(lpszPath, fname, lpszReturnBuffer)) == 0)
	{
		if((p = strrchr(lpszReturnBuffer, '\\')) != NULL)
			*plpszFilePart = ++p;
		return strlen(lpszReturnBuffer);
	}

	return 0;
}
