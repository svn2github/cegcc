#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <alloca.h>

#include <sys/wcebase.h>
#include <sys/wcetrace.h>
#include <sys/wcefile.h>

#ifndef CE_NOTRACE

extern int MultiByteToWideChar(UINT CodePage,         // code page
										DWORD dwFlags,         // character-type options
										LPCSTR lpMultiByteStr, // string to map
										int cbMultiByte,       // number of bytes in string
										LPWSTR lpWideCharStr,  // wide-character buffer
										int cchWideChar        // size of buffer
										);

extern void OutputDebugStringW(WCHAR* lpOutputString);

#define CP_ACP  0 // ANSI code page

static HANDLE __wcetracehnd = NULL;
static int   __wcetrace = 0;
static int   __wcetrace_debugger = -1;

void
WCETRACESET(int trace)
{
  __wcetrace = trace;
}

int
WCETRACEGET()
{
  return(__wcetrace);
}

void
WCETRACE_DEBUGGER_SET(int trace)
{
  __wcetrace_debugger = trace;
}

int
WCETRACE_DEBUGGER_GET()
{
  return(__wcetrace_debugger);
}

static void set_from_env(const char* env, int* what)
{
  const char *trace = getenv(env);
  char  buf[256];

  if (!trace)
    return;

  *what = 0;
  strcpy(buf, trace);
  char* l;
  for(l = buf; *l; l++)
    *l = tolower(*l);

  if (!strcmp(buf, "all")) {
    *what = -1;
    return;
  }

  char *p;
  for (p = strtok(buf, ":"); p; p = strtok(NULL, ":")) {
    if (!strcmp(p, "io"))
      *what |= WCE_IO;
    else if (!strcmp(p, "network"))
      *what |= WCE_NETWORK;
    else if (!strcmp(p, "signals"))
      *what |= WCE_SIGNALS;
    else if (!strcmp(p, "fifos"))
      *what |= WCE_FIFOS;
    else if (!strcmp(p, "time"))
      *what |= WCE_TIME;
    else if (!strcmp(p, "synch"))
      *what |= WCE_SYNCH;
    else if (!strcmp(p, "malloc"))
      *what |= WCE_MALLOC;
    else if (!strcmp(p, "vm"))
      *what |= WCE_VM;
    else if (!strcmp(p, "app"))
      *what |= WCE_APP;
  }
}

void
WCETRACEGETENV()
{
  set_from_env("WCETRACE", &__wcetrace);
  set_from_env("WCETRACE_DEBUGGER", &__wcetrace_debugger);
}

void
WCETRACECLOSE()
{
  if (__wcetracehnd != NULL && __wcetracehnd != INVALID_HANDLE_VALUE)
  {
    FlushFileBuffers(__wcetracehnd);
    XCECloseHandle(__wcetracehnd);
    __wcetracehnd = NULL;
  }
}

void
WCETRACE(int level, const char *fmt, ...)
{
  if (!(__wcetrace_debugger & level) && !(__wcetrace & level))
    return;

  char buf[1024];
  int len = sprintf(buf, "%08X:%08X: ", GetTickCount(), GetCurrentThreadId());

  va_list ap;
  va_start(ap, fmt);
  vsprintf(buf + len, fmt, ap);
  strcat(buf, "\n");

  if (__wcetrace & level) {
    if (__wcetracehnd == NULL) {
      char tracepath[256];
      wchar_t tracepathw[256];
      const char* tmppath = getenv("TMP");
      int pid = getpid();
      NKDbgPrintfW(L"pid is %d (%x)", pid, pid);
      if (!tmppath || strlen(tmppath) == 0)
        sprintf(tracepath, "/Temp/wcetrace%u.log", pid);
      else 
      {
        len = strlen(tmppath);
        if (tmppath[len-1] == '\\' || tmppath[len-1] == '/')
          sprintf(tracepath, "%swcetrace%u.log", tmppath, pid);
        else
          sprintf(tracepath, "%s/wcetrace%u.log", tmppath, pid);
      }

      mbstowcs(tracepathw, tracepath, strlen(tracepath) + 1);
      __wcetracehnd = XCECreateFileW(tracepathw, GENERIC_WRITE, 
        FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
    }

    if(__wcetracehnd != INVALID_HANDLE_VALUE) {
      int nwritten;
      WriteFile(__wcetracehnd, buf, strlen(buf), (DWORD *)&nwritten, NULL);
      FlushFileBuffers(__wcetracehnd);
    }
  }

  if (__wcetrace_debugger & level)
  {
    len = strlen(buf);
    wchar_t *wbuf = alloca(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP,0, buf, len, wbuf, len);
    OutputDebugStringW(wbuf);
  }
}

void __WCETraceError(int trace, DWORD error, const char* func)
{
  wchar_t* wbuf;

#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_FROM_SYSTEM     0x00001000
#define MAKELANGID(p, s) ((ushort)(((((ushort)(s)) << 10)|(ushort)(p))))
#define LANG_NEUTRAL 0
#define SUBLANG_DEFAULT 1

  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    error,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (wchar_t*) &wbuf,
    0, NULL );

  int len = wcslen(wbuf);
  char* buf = alloca(len);
  wcstombs(buf, wbuf, len+1);
  LocalFree(wbuf);
  WCETRACE(trace, "%s failed with error %d: %s", func, (int)error, buf); 
  printf("%s failed with error %d: %s\n", func, (int)error, buf); 
}

#endif // CE_NOTRACE
