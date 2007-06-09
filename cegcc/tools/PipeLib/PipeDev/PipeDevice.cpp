/* Copyright (c) 2007, Pedro Alves

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  */

#include "PipeDevice.h"

#include <windows.h>
#include <devload.h>
#include <set>
#include <string>

#ifndef min
#define min(A, B) ((A) < (B) ? (A) : (B))
#endif

typedef struct _DEVICE_PSL_NOTIFY
{
  DWORD   dwSize;
  DWORD   dwFlags;
  HANDLE  hProc;
  HANDLE  hThread;
} DEVICE_PSL_NOTIFY, *PDEVICE_PSL_NOTIFY;

#ifndef FILE_DEVICE_PSL
#define FILE_DEVICE_PSL 259
#endif

#define IOCTL_PSL_NOTIFY \
	CTL_CODE (FILE_DEVICE_PSL, 255, METHOD_NEITHER, FILE_ANY_ACCESS)

#define DLL_PROCESS_EXITING 4

#define PIPEDEV_API extern "C" __declspec (dllexport)

//#define DEBUG_MODE
//#define NOLOCKS
//#define DEBUG_LOCKS

#ifdef DEBUG_LOCKS
# define G(CS) (__LINE__, CS)
#else
# define G(CS) (CS)
#endif

class CSWrapper
{
public:
  explicit CSWrapper (CRITICAL_SECTION* cs) : cs_(*cs) {}
  ~CSWrapper () {}

  void Lock ()
  { 
#ifndef NOLOCKS
    EnterCriticalSection (&cs_); 
#endif
  }
  void Unlock ()
  {
#ifndef NOLOCKS
    LeaveCriticalSection (&cs_); 
#endif
  }
private:
  CRITICAL_SECTION& cs_;
};

class CS : public CSWrapper
{
public:
  CS () : CSWrapper (&cs_)
  {
#ifndef NOLOCKS
    InitializeCriticalSection (&cs_); 
#endif
  }
  ~CS ()
  {
#ifndef NOLOCKS
    DeleteCriticalSection (&cs_); 
#endif
  }
private:
  CRITICAL_SECTION cs_;
};

class FastCS
{
public:
  FastCS () : Count(0)
  {
#ifndef NOLOCKS
    /* auto - only release one at a time.  */
    EventHandle = CreateEvent (NULL, FALSE, FALSE, NULL); 
#endif
  }
  ~FastCS ()
  {
#ifndef NOLOCKS
    CloseHandle (EventHandle);
#endif
  }

  void Lock ()
  { 
#ifndef NOLOCKS
    if (InterlockedIncrement (&Count) == 1)
      /* first come - first serve.  */
      return;

    /* everyone else, get in line.  */
    WaitForSingleObject (EventHandle, INFINITE);
#endif
  }
  void Unlock ()
  {
#ifndef NOLOCKS
    if (InterlockedDecrement (&Count) > 0)
      /* release one pending */
      SetEvent (EventHandle);
#endif
  }
private:
  HANDLE EventHandle;
  LONG Count;
};

class RecursiveCS
{
public :
  RecursiveCS ()
    : Count(0)
    , RecursionCount(0)
    , ThreadID(0)

  {
#ifndef NOLOCKS
    /* auto - only release one at a time.  */
    EventHandle = CreateEvent (NULL, FALSE, FALSE, NULL); 
#endif
  }

  ~RecursiveCS ()
  {
#ifndef NOLOCKS
    CloseHandle (EventHandle);
#endif
  }

  void Lock ()
  {
#ifndef NOLOCKS
    if (ThreadID == GetCurrentThreadId ())
      {
	InterlockedIncrement (&RecursionCount);
	return;
      }

    if (InterlockedIncrement (&Count) == 1)
      InterlockedExchange (&RecursionCount, 0);
    else
      WaitForSingleObject (EventHandle, INFINITE);

    ThreadID = GetCurrentThreadId ();
#endif
  }
	
  void Unlock ()
  {
#ifndef NOLOCKS
    if (RecursionCount == 0)
      {
	if (InterlockedDecrement (&Count) > 0)
	  {
	    /* release one thread */
	    SetEvent (EventHandle);
	  }
      }
    else
      {
	InterlockedDecrement (&RecursionCount);
      }
#endif
  }

private:
  HANDLE EventHandle;
  LONG Count;
  LONG RecursionCount;
  DWORD ThreadID;
};


template <class T>
class LockGuard
{
public:
#ifdef DEBUG_LOCKS
  LockGuard (int line, T &cs) : cs_(cs), lineno(line)
  {
    WCHAR buf[100];
    wsprintf (buf, L"line : %d", line);
    MessageBoxW(0, buf, L"lock", 0);

    cs_.Lock (); 
  }
  ~LockGuard ()
  {
    WCHAR buf[100];
    wsprintf (buf, L"line : %d", lineno);
    MessageBoxW(0, buf, L"unlock", 0);

    cs_.Unlock (); 
  }
#else
  explicit LockGuard (T &cs) : cs_(cs) { cs_.Lock (); }
  ~LockGuard () { cs_.Unlock (); }
#endif

private:
  T &cs_;

#ifdef DEBUG_LOCKS
  int lineno;
#endif
};

class PipeOpenContext;

class PipeDeviceContext
{
public:
  typedef ::LockGuard<PipeDeviceContext> LockGuard;

  explicit PipeDeviceContext (LPCWSTR activepath)
    : OpenCount(0)
    , WROpenCount(0)
    , DeviceName(NULL)
    , Aborting(FALSE)
    , head(0)
    , count(0)
  {
    ActivePath = wcsdup (activepath);
    ReadEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    WriteEvent = CreateEvent (NULL, FALSE, FALSE, NULL);

    /* This should wake all threads, so it is manual reset.  */
    AbortEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
  }

  ~PipeDeviceContext ()
  {
    if (DeviceName)
      free (DeviceName);
    if (ActivePath)
      free (ActivePath);
    CloseHandle (ReadEvent);
    CloseHandle (WriteEvent);
    CloseHandle (AbortEvent);
  }

  DWORD size ()
  {
    return count;
  }

  DWORD tail ()
  {
    return (head + count) & (sizeof (buffer) - 1);
  }

  void Lock ()
  {
    cs.Lock ();
  }

  void Unlock ()
  {
    cs.Unlock ();
  }

  DWORD writeBytes (const void* data_, DWORD dsize)
  {
    DWORD fit = sizeof (buffer) - size ();
    fit = min (dsize, fit);
    const BYTE* data = (const BYTE*)data_;
    BYTE* b = buffer + tail ();
    for (DWORD i = 0; i < fit; i++)
      b[i & (sizeof (buffer) - 1)] = data[i];
    count += fit;
    return fit;
  }

  DWORD readBytes (void* buf_, DWORD bsize)
  {
    BYTE* buf = (BYTE*)buf_;
    DWORD fit = min (bsize, size ());

    const BYTE* h = buffer + head;
    for (DWORD i = 0; i < fit; i++)
      buf[i] = h[i & (sizeof (buffer) - 1)];
    count -= fit;
    head += fit;
    head &= (sizeof (buffer) - 1);
    return fit;
  }

public:
  DWORD OpenCount;
  DWORD WROpenCount;

  WCHAR* ActivePath;

  HANDLE ReadEvent;
  HANDLE WriteEvent;
  HANDLE AbortEvent;

  WCHAR* DeviceName;
  BOOL Aborting;

private:
  BYTE buffer[0x1000];
  DWORD head;
  DWORD count;

  RecursiveCS cs;
};

class PipeOpenContext
{
public:
  PipeOpenContext (PipeDeviceContext* devctx,
		   DWORD accessCode, DWORD shareMode)
    : DeviceContext (devctx)
    , dwAccessCode (accessCode)
    , dwShareMode (shareMode)
  {}

  ~PipeOpenContext ()
  {}

  PipeDeviceContext* DeviceContext;
  DWORD dwAccessCode;
  DWORD dwShareMode;
};

static CRITICAL_SECTION open_pipes_cs;

extern "C" BOOL WINAPI
DllMain (HANDLE /*hinstDLL*/, DWORD dwReason, LPVOID /*lpvReserved*/)
{
  switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
      InitializeCriticalSection (&open_pipes_cs);
      break;
    case DLL_PROCESS_DETACH:
      DeleteCriticalSection (&open_pipes_cs);
      break;
    }

  return TRUE;
}

static void
LogMessage (const char* msg, ...)
{
#if 0
  va_list ap;
  va_start (ap, msg);
  FILE* log = fopen ("log.txt", "a+");
  vfprintf (log, msg, ap);
  fclose (log);
  va_end (ap);
#else
  (void)msg;
#endif
}

struct LogScope
{
  explicit LogScope (const char* func)
    : func_(func)
  {
    LogMessage ("entering %s\n", func_);
  }
  ~LogScope ()
  {
    LogMessage ("leaving %s\n", func_);
  }

  const char* func_;
};

#define LOGSCOPE(MSG) LogScope scope ## __LINE__(#MSG)

/* This is needed for MSVC.  */
struct ltwstr
{
  bool operator () (const std::wstring& s1, const std::wstring& s2) const
  {
    return wcscmp (s1.c_str (), s2.c_str ()) < 0;
  }
};

typedef std::set<std::wstring, ltwstr> vwstring;
static vwstring open_pipes;

PIPEDEV_API BOOL Deinit (PipeDeviceContext* pDeviceContext);

static HANDLE
GetDeviceHandle (PipeDeviceContext* pDeviceContext)
{
  LOGSCOPE (GetDeviceHandle);
  HKEY hActive;
  DWORD Type;
  HANDLE hDev = INVALID_HANDLE_VALUE;

  DWORD status = RegOpenKeyEx (HKEY_LOCAL_MACHINE, pDeviceContext->ActivePath,
			       0, 0, &hActive);
  if (status != ERROR_SUCCESS)
    return INVALID_HANDLE_VALUE;

  DWORD Len = sizeof(hDev);
  status = RegQueryValueEx (hActive, DEVLOAD_HANDLE_VALNAME, NULL, &Type,
			    (PUCHAR)&hDev, &Len);
  if (status != ERROR_SUCCESS)
    {
      /* weird */
    }

  RegCloseKey (hActive);

  return hDev;
}

PIPEDEV_API DWORD
Init (LPCTSTR pContext)
{
  LOGSCOPE ("Init");
  CSWrapper cs (&open_pipes_cs);
  LockGuard<CSWrapper> guard G(cs);

#ifdef DEBUG_MODE
  MessageBoxW (0, pContext, L"Init", 0);
#endif

  /* TODO: The key here isn't exactly the best.  Maybe we should use
     the device name instead, and get that from the registry - that would
     also get rid of the IOCTL calls.  */
  if (open_pipes.find (pContext) != open_pipes.end ())
    /* already open, PipeLib will try another one.  */
    return 0;

  PipeDeviceContext* pDeviceContext = new PipeDeviceContext (pContext);
  if (pDeviceContext == NULL)
    return 0;

  open_pipes.insert (pContext);

  return (DWORD)pDeviceContext;
}

#ifdef DEBUG_MODE
static int close_calls;
#endif

PIPEDEV_API BOOL
Deinit (PipeDeviceContext* pDeviceContext)
{
  LOGSCOPE ("Deinit");
  /* All file handles must to be closed before deinitialising
     the driver.  */

#ifdef DEBUG_MODE
  WCHAR buf[100];
  wsprintf (buf, L"opencount %d : calls %d", pDeviceContext->OpenCount,
	    close_calls);
  MessageBoxW (0, buf, L"Deinit", 0);
#endif

  if (pDeviceContext == NULL)
    return FALSE;

  {
    PipeDeviceContext::LockGuard guard G(*pDeviceContext);

    if (pDeviceContext->OpenCount != 0)
      return FALSE;
  }

#ifdef DEBUG_MODE
  MessageBoxW (0, L"deactivate success", L"Deinit", 0);
#endif

  /* Allow reuse.  */
  open_pipes.erase (pDeviceContext->ActivePath);

  /* Race?  Is it possible that there can be another process
     calling any function on the device while we are
     Dinitializing it?  Doesn't CE take care of that?  */
  delete pDeviceContext;
  return TRUE;
}

PIPEDEV_API DWORD
Open (PipeDeviceContext* pDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
  LOGSCOPE ("Open");

#ifdef DEBUG_MODE
  wchar_t buf[100];
  wsprintf (buf, L"opencount %d", pDeviceContext->OpenCount);
  MessageBoxW (0, buf, L"open 1", 0);
#endif

  PipeOpenContext* pOpenContext =
    new PipeOpenContext (pDeviceContext, AccessCode, ShareMode);

#ifdef DEBUG_MODE
  MessageBoxW (0, L"going to lock", L"open 2", 0);
#endif

  PipeDeviceContext::LockGuard guard G(*pOpenContext->DeviceContext);

#ifdef DEBUG_MODE
  MessageBoxW (0, L"locked", L"open 3", 0);
#endif

  pDeviceContext->OpenCount++;

  if (AccessCode & GENERIC_WRITE)
    pDeviceContext->WROpenCount++;

#ifdef DEBUG_MODE
  wsprintf (buf, L"opencount %d", pDeviceContext->OpenCount);
  MessageBoxW (0, buf, L"open", 0);
#endif

  return (DWORD)pOpenContext;
}

struct DeactivatorData
{
  HANDLE th;
  HANDLE dev;
};

static DWORD WINAPI
Deactivator (void* arg)
{
  LOGSCOPE ("Deactivator");

  DeactivatorData* data = (DeactivatorData*)arg;

#ifdef DEBUG_MODE
  wchar_t buf[100];
  wsprintf (buf, L"%x", data->dev);
  MessageBoxW(0, buf, L"close: dev handle", 0);
  if (!DeactivateDevice (data->dev))
    MessageBoxW(0, buf, L"deactivate failed", 0);
  else
    MessageBoxW(0, buf, L"after deactivate", 0);
#else
  DeactivateDevice (data->dev);
#endif

  CloseHandle (data->th);
  delete data;

  return 0;
}

static void
DeactivatePipeDevice (PipeDeviceContext* dev)
{
  LOGSCOPE ("DeactivatePipeDevice");

  HANDLE hdev = GetDeviceHandle (dev);
  DeactivatorData* data = new DeactivatorData ();
  data->dev = hdev;
  data->th = CreateThread (NULL, 0, Deactivator, data,
			   CREATE_SUSPENDED, NULL);
  ResumeThread (data->th);
}

PIPEDEV_API BOOL
Close (PipeOpenContext* pOpenContext)
{
  LOGSCOPE ("Close");
#ifdef DEBUG_MODE
  close_calls++;

  wchar_t buf[100];
  if (pOpenContext)
    {
      wsprintf (buf, L"opencount %d : %p",
		pOpenContext->DeviceContext->OpenCount,
		pOpenContext);
      MessageBoxW (0, buf, L"close", 0);
    }
  else
    {
      wsprintf (buf, L"openctx %p", pOpenContext);
      MessageBoxW (0, buf, L"close", 0);
    }
#endif

  if (pOpenContext == NULL)
    return FALSE;

  PipeDeviceContext* dev = pOpenContext->DeviceContext;

  PipeDeviceContext::LockGuard guard G(*dev);

  if (dev->OpenCount == 0)
    return FALSE;

  dev->OpenCount--;

  if (pOpenContext->dwAccessCode & GENERIC_WRITE)
    {
      dev->WROpenCount--;
      if (dev->WROpenCount == 0)
	/* Wake up the reading side so it can see
	   the broken pipe.  */
	SetEvent (dev->AbortEvent);
    }

  delete pOpenContext;

#ifdef DEBUG_MODE
  wsprintf (buf, L"opencount %d", dev->OpenCount);
  MessageBoxW (0, buf, L"close 2", 0);
#endif

  if (dev->OpenCount == 0)
    {
#if 0
      HANDLE hdev = GetDeviceHandle (dev);
      DeactivateDevice (hdev);
#else
      DeactivatePipeDevice (dev);
#endif
    }

  return TRUE;
}

PIPEDEV_API DWORD
Read (PipeOpenContext* pOpenContext, LPVOID pBuffer_, DWORD dwCount)
{
  LOGSCOPE ("Read");

  if (IsBadReadPtr (pBuffer_, dwCount))
    {
      SetLastError (ERROR_INVALID_PARAMETER);
      return -1;
    }
	
  BYTE* pBuffer = (BYTE*)pBuffer_;
  DWORD needed = dwCount;

  if (pOpenContext == NULL
      || (pOpenContext->dwAccessCode & GENERIC_READ) == 0)
    return (DWORD)-1;

  PipeDeviceContext* dev = pOpenContext->DeviceContext;

  BOOL breaknext = FALSE;
  HANDLE Events[2];

  do
    {
      {
	PipeDeviceContext::LockGuard guard G(*dev);

	Events[0] = dev->WriteEvent;
	Events[1] = dev->AbortEvent;

	if (dev->Aborting)
	  /* this device is long gone */
	  return (DWORD)-1;

	if (dev->WROpenCount == 0)
	  /* broken pipe */
	  return (DWORD)-1;

	if (dev->OpenCount == 0)
	  /* weird */
	  return (DWORD)-1;

	DWORD read = dev->readBytes (pBuffer, dwCount);
	pBuffer += read;
	dwCount -= read;

	if (read)
	  SetEvent (dev->ReadEvent);

	if (dwCount == 0)
	  break;

	if (breaknext)
	  break;
      }

      switch (WaitForMultipleObjects (2, Events, FALSE, INFINITE))
	{
	case WAIT_OBJECT_0:
	  breaknext = TRUE;
	  break;
	default:
	  //			MessageBoxW (0, L"pipe aborted", L"Read", 0);
	  /* With either wait error or AbortEvent
	     signaled, return with error.  */
	  return (DWORD)-1;
	}
    }
  while (dwCount);

  return needed - dwCount;
}

PIPEDEV_API DWORD
Write (PipeOpenContext* pOpenContext, LPCVOID pBuffer_, DWORD dwCount)
{
  LOGSCOPE ("Write");

  if (IsBadReadPtr (pBuffer_, dwCount))
    {
      SetLastError (ERROR_INVALID_PARAMETER);
      return -1;
    }

  const BYTE* pBuffer = (const BYTE*)pBuffer_;
  DWORD needed = dwCount;

#ifdef DEBUG_MODE
  wchar_t buf[100];
  wsprintf (buf, L"opencount %d", pOpenContext->DeviceContext->OpenCount);
  MessageBoxW (0, buf, L"write", 0);
#endif

  if (pOpenContext == NULL
      || (pOpenContext->dwAccessCode & GENERIC_WRITE) == 0)
    return (DWORD)-1;

  PipeDeviceContext* dev = pOpenContext->DeviceContext;

  BOOL breaknext = FALSE;
  HANDLE Events[2];

  do
    {
      {
	PipeDeviceContext::LockGuard guard G(*dev);

#ifdef DEBUG_MODE
	MessageBoxW (0, L"lock acquired", L"write", 0);
#endif

	Events[0] = dev->ReadEvent;
	Events[1] = dev->AbortEvent;

	if (dev->Aborting)
	  /* this device is long gone */
	  return (DWORD)-1;

	if (dev->OpenCount == 0)
	  /* weird */
	  return (DWORD)-1;

	/* According to MSDN, attempting to read from a pipe without writers,
	   generates a broken pipe error, but the opposite isn't forbidden, so we
	   allow writing to a pipe without a reader.  */

	DWORD wrote = dev->writeBytes (pBuffer, dwCount);
	pBuffer += wrote;
	dwCount -= wrote;

	/* According to MSDN, a write of 0, also wakes
	   the reading end of the pipe.  */
	PulseEvent (dev->WriteEvent);

	if (dwCount == 0)
	  break;

	if (breaknext)
	  break;
      }

      switch (WaitForMultipleObjects (2, Events, FALSE, INFINITE))
	{
	case WAIT_OBJECT_0:
	  breaknext = TRUE;
	  break;
	default:
	  //			MessageBoxW (0, L"pipe aborted", L"Write", 0);
	  /* With either wait error or AbortEvent
	     signaled, return with error.  */
	  return (DWORD) -1;
	}
    }
  while (dwCount);

  return needed - dwCount;
}

PIPEDEV_API DWORD
Seek (PipeOpenContext* /*pOpenContext*/, long /*Amount*/, WORD /*wType*/)
{
  LOGSCOPE ("Seek");
  /* Pipes don't support seeking.  */
  return (DWORD)-1;
}

PIPEDEV_API BOOL
IOControl (PipeOpenContext* pOpenContext, DWORD dwCode,
	   PBYTE pBufIn, DWORD dwLenIn,
	   PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
  LOGSCOPE ("IOControl");

  /* Kill unused warnings.  */
  (void)pBufIn;
  (void)dwLenIn;
  (void)pBufOut;
  (void)dwLenOut;
  (void)pdwActualOut;

  BOOL bRet = FALSE;

  if (pOpenContext == NULL)
    return FALSE;

  PipeDeviceContext* dev = pOpenContext->DeviceContext;
  PipeDeviceContext::LockGuard guard G(*dev);

#ifdef DEBUG_MODE
  wchar_t buf[100];
  wsprintf (buf, L"%x : %d", dwCode, dev->OpenCount);
  MessageBoxW (0, buf, L"IOControl", 0);
#endif

  if (dwCode == IOCTL_PSL_NOTIFY)
    {
      PDEVICE_PSL_NOTIFY pPslPacket = (PDEVICE_PSL_NOTIFY)pBufIn;

      if (pPslPacket->dwSize == sizeof (DEVICE_PSL_NOTIFY)
	  && pPslPacket->dwFlags == DLL_PROCESS_EXITING)
	{
#ifdef DEBUG_MODE
	  WCHAR buf[100];
	  wsprintf (buf, L"%p : %p", pPslPacket->hProc, pPslPacket->hThread);
	  MessageBoxW(0, buf, L"process dying", 0);
#endif
	  dev->Aborting = TRUE;
	  /* Unlock all blocked threads.  */
	  SetEvent (dev->AbortEvent);
	}
      return TRUE;
    }

  switch (dwCode)
    {
    case PIPE_IOCTL_SET_PIPE_NAME:
      if (dev->DeviceName)
	free (dev->DeviceName);
      dev->DeviceName = wcsdup ((WCHAR*)pBufIn);
      bRet = TRUE;
      break;
    case PIPE_IOCTL_GET_PIPE_NAME:
      wcscpy ( (WCHAR*)pBufOut, dev->DeviceName);
      *pdwActualOut = (wcslen (dev->DeviceName) + 1) * sizeof (WCHAR);
      bRet = TRUE;
      break;
    }

  return bRet;
}

PIPEDEV_API void
PowerDown (PipeDeviceContext* /*pDeviceContext*/)
{
}

PIPEDEV_API void
PowerUp (PipeDeviceContext* /*pDeviceContext*/)
{
}
