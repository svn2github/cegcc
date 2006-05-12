#ifndef _WCETHREAD_H_
#define _WCETHREAD_H_

#include "sys/wcetypes.h"
#include "sys/wcetime.h"
#include "sys/wcemachine.h"

#include <stdint.h>

/* Thread Constants and Constructs */
#define CREATE_SUSPENDED 4

#define THREAD_PRIORITY_TIME_CRITICAL   0
#define THREAD_PRIORITY_HIGHEST         1
#define THREAD_PRIORITY_ABOVE_NORMAL    2
#define THREAD_PRIORITY_NORMAL          3
#define THREAD_PRIORITY_BELOW_NORMAL    4
#define THREAD_PRIORITY_LOWEST          5
#define THREAD_PRIORITY_ABOVE_IDLE      6
#define THREAD_PRIORITY_IDLE            7

#define THREAD_PRIORITY_ERROR_RETURN    (0x7fffffff)

typedef DWORD (*LPTHREAD_START_ROUTINE)(LPVOID pvarg);

typedef LPTHREAD_START_ROUTINE PTHREAD_START_ROUTINE;

/* TLS Constants and Constructs */
#define TLS_FUNCALLOC   0
#define TLS_FUNCFREE    1

#define TlsAlloc()  (TlsCall(TLS_FUNCALLOC, 0))
#define TlsFree(x)  (TlsCall(TLS_FUNCFREE, x))

#define TLS_OUT_OF_INDEXES ((DWORD)0xffffffff)

/* Synchronization Constants and Constructs */
#define WAIT_OBJECT_0   	0x00000000L
#define WAIT_ABANDONED  	0x00000080L
#define WAIT_ABANDONED_0	0x00000080L
#define WAIT_FAILED      0xffffffffL
#define INFINITE         0xffffffffL

#define EVENT_PULSE     1
#define EVENT_RESET     2
#define EVENT_SET       3

#define PulseEvent(x) EventModify(x, EVENT_PULSE)
#define ResetEvent(x) EventModify(x, EVENT_RESET)
#define SetEvent(x) EventModify(x, EVENT_SET)

typedef struct CRITICAL_SECTION {
  unsigned int LockCount;     /* Nesting count on critical section */
  HANDLE OwnerThread;         /* Handle of owner thread */
  HANDLE hCrit;					          /* Handle to this critical section */
  DWORD needtrap;					        /* Trap in when freeing critical section */
  DWORD dwReserved;				       /* currently unused */
} CRITICAL_SECTION, *LPCRITICAL_SECTION;

#ifdef __cplusplus
extern "C" {
#endif

/* Micro$oft Thread Creation Routines */

HANDLE CreateThread(LPSECURITY_ATTRIBUTES, DWORD, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
VOID   ExitThread(DWORD);
BOOL   TerminateThread(HANDLE, DWORD);

/* Micro$oft TLS Routines */
DWORD  TlsCall(DWORD, DWORD);
LPVOID TlsGetValue(DWORD);
BOOL   TlsSetValue(DWORD, LPVOID);

/* Micro$oft Critical Section Routines */
VOID EnterCriticalSection(LPCRITICAL_SECTION);
VOID LeaveCriticalSection(LPCRITICAL_SECTION);
VOID InitializeCriticalSection(LPCRITICAL_SECTION);
BOOL TryEnterCriticalSection(LPCRITICAL_SECTION);
VOID DeleteCriticalSection(LPCRITICAL_SECTION);

/* Micro$oft Synchronization Routines */
HANDLE CreateEvent(LPSECURITY_ATTRIBUTES, BOOL, BOOL, LPCSTR);
HANDLE CreateEventW(LPSECURITY_ATTRIBUTES, BOOL, BOOL, LPCWSTR);
HANDLE CreateMutexW(LPSECURITY_ATTRIBUTES, BOOL, LPCWSTR);
HANDLE CreateSemaphoreW(LPSECURITY_ATTRIBUTES, LONG, LONG, LPCWSTR);
BOOL   EventModify(HANDLE, DWORD);
BOOL   ReleaseMutex(HANDLE);
BOOL   ReleaseSemaphore(HANDLE, LONG, LPLONG);
DWORD  WaitForSingleObject(HANDLE, DWORD);
DWORD  WaitForMultipleObjects(DWORD,	CONST HANDLE *, BOOL, DWORD);

/* Micro$oft Thread Management Routines */
BOOL  GetExitCodeThread(HANDLE, LPDWORD);
BOOL  GetThreadContext(HANDLE, LPCONTEXT);
BOOL  SetThreadContext(HANDLE, CONST CONTEXT *);
BOOL  GetThreadTimes(HANDLE, LPFILETIME, LPFILETIME, LPFILETIME, LPFILETIME);
int   GetThreadPriority(HANDLE);
BOOL  SetThreadPriority(HANDLE, int);
DWORD SuspendThread(HANDLE);
DWORD ResumeThread(HANDLE);

uintptr_t _beginthread(void (* initialcode) (void *), unsigned stacksize, void * argument);
void _endthread (void);

uintptr_t _beginthreadex (void *security, unsigned stacksize, unsigned (*initialcode) (void *), void * argument, unsigned createflag, unsigned *thrdaddr);
void _endthreadex (unsigned retcode);

#ifdef __cplusplus
}
#endif
#endif  /* _WCETHREAD_H_ */
