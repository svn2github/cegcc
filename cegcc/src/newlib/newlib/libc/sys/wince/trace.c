#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <alloca.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "sys/wcetrace.h"
#include "sys/wcefile.h"

#ifndef CE_NOTRACE

static HANDLE __wcetracehnd = NULL;
static int __wcetrace = 0;
static int __wcetrace_debugger = WCE_ALL;

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

typedef struct 
{
  const char* str;
  int flag;
} trace_entry;

static const trace_entry trace_entries[] = 
{
  { "all", WCE_ALL }, 

  { "io", WCE_IO }, 
  { "network", WCE_NETWORK }, 
  { "signals", WCE_SIGNALS }, 
  { "fifos", WCE_FIFOS }, 
  { "time", WCE_TIME }, 
  { "synch", WCE_SYNCH }, 
  { "malloc", WCE_MALLOC }, 
  { "vm", WCE_VM }, 
  { "app", WCE_APP }, 
  
  { NULL, 0 }
};

static void set_from_env(const char* env, int* what)
{
  char  buf[256];
  const char *trace = getenv(env);
  if (!trace)
  {
    NKDbgPrintfW(L"%S not found in registry\n", env);
    return;
  }
  else
  {
    NKDbgPrintfW(L"parsing: %S:%S\n", env, trace);
  }

  *what = 0;
  strcpy(buf, trace);

  const trace_entry* entry;
  const char *p;

  for (p = strtok(buf, ":"); p; p = strtok(NULL, ":")) {
    NKDbgPrintfW(L"option token %S\n", p);
    int neg = 0;
    if (p[0] == '-' && p[1] != '\0') {
      NKDbgPrintfW(L"neg option\n");
      p++;
      neg = 1;
    }

    NKDbgPrintfW(L"check valid\n");

    for (entry = trace_entries; entry->str; entry++) {
      if (!strcmp(p, entry->str)) {
        NKDbgPrintfW(L"valid option.\n");
        if (neg)
          *what &= ~entry->flag;
        else
          *what |= entry->flag;
        break;
      }
    }
  }
  NKDbgPrintfW(L"no more tokens\n");
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
    len = strlen(buf) + 1;
    wchar_t *wbuf = alloca(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP,0, buf, len, wbuf, len);
    OutputDebugStringW(wbuf);
  }
}

void __WCETraceError(int trace, DWORD error, const char* func)
{
  wchar_t* wbuf;

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
