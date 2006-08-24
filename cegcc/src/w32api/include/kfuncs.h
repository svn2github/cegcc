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

#define	PUserKData		((LPBYTE)0xFFFFC800)
#define	SYSHANDLE_OFFSET	0x004
#define	SYS_HANDLE_BASE		64
#define SH_WIN32                0
#define SH_CURTHREAD            1
#define SH_CURPROC              2

/* Process/Thread ID Methods */
#define	GetCurrentThread()	((HANDLE)(SH_CURTHREAD+SYS_HANDLE_BASE))
#define	GetCurrentProcess()	((HANDLE)(SH_CURPROC+SYS_HANDLE_BASE))
#define	GetCurrentThreadId()	((DWORD)(((HANDLE *)(PUserKData+SYSHANDLE_OFFSET))[SH_CURTHREAD]))
#define	GetCurrentProcessId()	((DWORD)(((HANDLE *)(PUserKData+SYSHANDLE_OFFSET))[SH_CURPROC]))



/* Stolen from sys/oldsys/wcethread.h */
#define	EVENT_PULSE	1
#define	EVENT_RESET	2
#define	EVENT_SET	3
#define	PulseEvent(x)	EventModify(x, EVENT_PULSE)
#define	ResetEvent(x)	EventModify(x, EVENT_RESET)
#define	SetEvent(x)	EventModify(x, EVENT_SET)

#endif
