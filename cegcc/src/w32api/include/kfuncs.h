/*
 * The implementation is from the WCE library
 * The description that this is supposed to be "kfuncs.h" is from pages
 * such as :
 *
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcekernl/html/_wcesdk_win32_exitthread.asp
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcesdkr/html/_wcesdk_win32_pulseevent.asp
 */
#ifndef	_W32API_KFUNCS_H_
#define	_W32API_KFUNCS_H_

#ifndef _WIN32_WCE
#error "_WIN32_WCE is not defined."
#endif

#define	PUserKData		((LPBYTE)0xFFFFC800)
#define	SYSHANDLE_OFFSET	0x004
#define	SYS_HANDLE_BASE		64
#define SH_WIN32                0
#define SH_CURTHREAD            1
#define SH_CURPROC              2

/* Process/Thread ID Methods */
static inline HANDLE GetCurrentProcess()
{
  return ((HANDLE)(SH_CURPROC+SYS_HANDLE_BASE));

}

static inline HANDLE GetCurrentThread()
{
  return ((HANDLE)(SH_CURTHREAD+SYS_HANDLE_BASE));
}

static inline DWORD GetCurrentThreadId()
{
  return ((DWORD)(((HANDLE *)(PUserKData+SYSHANDLE_OFFSET))[SH_CURTHREAD]));
}

static inline DWORD GetCurrentProcessId()
{
  return ((DWORD)(((HANDLE *)(PUserKData+SYSHANDLE_OFFSET))[SH_CURPROC]));
}

/* EventModify signature hinted on:
   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcehardware5/html/wce50lrfCeLogImportTable.asp

   Event Constants and EventModify signature in the c# example at:
   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnnetcomp/html/PISAPICF.asp  */
WINBASEAPI BOOL WINAPI EventModify(HANDLE h, DWORD e);

#define	EVENT_PULSE	1
#define	EVENT_RESET	2
#define	EVENT_SET	3

static inline BOOL PulseEvent (HANDLE x)
{
  return EventModify(x, EVENT_PULSE);
}

static inline BOOL ResetEvent (HANDLE x)
{
  return EventModify(x, EVENT_PULSE);
}

static inline BOOL SetEvent (HANDLE x)
{
  return EventModify(x, EVENT_PULSE);
}

/* TLS Constants and Constructs */
#define TLS_FUNCALLOC   0
#define TLS_FUNCFREE    1

WINBASEAPI DWORD WINAPI TlsCall(DWORD func, DWORD val);

static inline DWORD TlsAlloc (void)
{
  return (TlsCall(TLS_FUNCALLOC, 0));
}

static inline BOOL WINAPI TlsFree(DWORD x)
{
  return (TlsCall(TLS_FUNCFREE, x));
}

#endif
