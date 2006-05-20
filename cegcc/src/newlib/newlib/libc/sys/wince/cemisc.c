#include <stdlib.h>

#include <sys/wcebase.h>
#include <sys/wcethread.h>
#include <sys/wcefile.h>
#include <sys/wcememory.h>
#include <sys/wcetrace.h>
#include <sys/sysconf.h>

#if 0
/* TRACE WRAPPERS The following are trace wrappers and should be removed */
LPVOID
XCEVirtualAlloc(LPVOID addr, DWORD size, DWORD type, DWORD protect)
{
  WCETRACE(WCE_VM, "va: %p %d %d %d", addr, size, type, protect);
  VirtualAlloc(addr, size, type, protect);
}

BOOL
XCEVirtualFree(LPVOID addr, DWORD size, DWORD type)
{
  WCETRACE(WCE_VM, "vf: %p %d %d", addr, size, type);
  return(VirtualFree(addr, size, type));
}

void
XCEInitializeCriticalSection(LPCRITICAL_SECTION cs)
{
  long dummy;
  int  trace;

  trace = WCETRACEGET();
  if (trace & WCE_SYNCH) {
    WCETRACE(WCE_SYNCH, "ics: %p", cs);
#if 0
    printf("ics: %p\n", cs);
    backtrace(&dummy, 20);
#endif
  }
  InitializeCriticalSection(cs);
}

void
XCEDeleteCriticalSection(LPCRITICAL_SECTION cs)
{
  WCETRACE(WCE_SYNCH, "dcs: %p", cs);
  DeleteCriticalSection(cs);
}

HANDLE
XCECreateMutexW(LPSECURITY_ATTRIBUTES sa, BOOL io, LPTSTR name)
{
  HANDLE hnd;

  hnd = CreateMutexW(sa, io, name);
  WCETRACE(WCE_SYNCH, "cmw: %p", hnd);
  return(hnd);
}

BOOL
XCEReleaseMutex(HANDLE hnd)
{
  BOOL rval;

  rval = ReleaseMutex(hnd);
  /*  WCETRACE(WCE_SYNCH, "rm: %p %d", hnd, rval); */
  return(rval);
}

HANDLE
XCECreateSemaphoreW(LPSECURITY_ATTRIBUTES sa, LONG ic, LONG mc, LPCWSTR name)
{
  HANDLE hnd;

  hnd = CreateSemaphoreW(sa, ic, mc, name);
  WCETRACE(WCE_SYNCH, "csw: %p", hnd);
  return(hnd);
}

/* END TRACE WRAPPERS */
#endif

HANDLE
XCECreateEventW(LPSECURITY_ATTRIBUTES sa, BOOL mr, BOOL is, LPTSTR name)
{
	HANDLE hnd;

	hnd = CreateEventW(sa, mr, is, name);
	WCETRACE(WCE_SYNCH, "cew: %p", hnd);
	return(hnd);
}

HANDLE
XCECreateEventA(LPSECURITY_ATTRIBUTES attr, BOOL manualReset,  
            BOOL initState, const char *name)
{
  HANDLE hnd;
  wchar_t nameW[MAX_PATH];
  int len;

  if (name) {
    len = strlen(name) + 1;
    if (len >= MAX_PATH) {
      WCETRACE(WCE_IO, "CreateEvent: FATAL ERROR strlen exceeds MAX_PATH (%d>=%d)",
               len, MAX_PATH);
      return(NULL);
    }
    mbstowcs(nameW, name, len);
    hnd = (HANDLE) XCECreateEventW(attr, manualReset, initState, nameW);
  } else {
    hnd = (HANDLE) XCECreateEventW(attr, manualReset, initState, NULL);
  }

  return(hnd);
}

void __IMPORT
XCEGetSystemTimeAsFileTime(LPFILETIME pft)
{
	SYSTEMTIME st;

	GetSystemTime(&st);
	SystemTimeToFileTime(&st, pft);
}
