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
    LOG ("LockGuard: line : %d\n", line);
    cs_.Lock (); 
  }
  ~LockGuard ()
  {
    LOG ("~LockGuard: line : %d\n", lineno);
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

#ifdef DEBUG_MODE
static void
LogMessage2 (const char* msg)
{
  FILE* log = fopen ("pipelog.txt", "a+");
  fprintf (log, "%s", msg);
  fclose (log);
}

static void
LogMessage (const char *file, int line, const char* msg, ...)
{
  va_list ap;
  char buf[1024];
  char *b = buf;
  va_start (ap, msg);
  sprintf (b, "%08x: %s (%d): ", (unsigned) GetCurrentThreadId (), file, line);
  b += strlen (b);
  vsprintf (b, msg, ap);
  va_end (ap);

  LogMessage2 (buf);
}

struct LogScope
{
  explicit LogScope (const char *func, const char* msg)
    : func_(func)
    , msg_(msg)
  {
    char buf[512];
    sprintf (buf, "> %s", msg_);
    LogMessage (func_, -1, buf);
  }
  ~LogScope ()
  {
    char buf[512];
    sprintf (buf, "< %s", msg_);
    LogMessage (func_, -1, buf);
  }

  const char* func_;
  const char* msg_;
};

# define LOG(MSG, ...) LogMessage (__FUNCTION__, __LINE__, MSG, ## __VA_ARGS__)
# define LOGSCOPE(MSG) LogScope scope ## __LINE__(__FUNCTION__, MSG)
#else
# define LOG(MSG, ...) do; while (0)
# define LOGSCOPE(MSG) do; while (0)
#endif

/* This is needed for MSVC, but it makes no harm in gcc.  */
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
  LOGSCOPE ("\n");
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
  LOGSCOPE ("\n");
  CSWrapper cs (&open_pipes_cs);
  LockGuard<CSWrapper> guard G(cs);

  LOG ("%ls\n", pContext);

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
  LOGSCOPE ("\n");
  /* All file handles must to be closed before deinitialising
     the driver.  */

#ifdef DEBUG_MODE
  LOG ("oc %lu, wc: %lu, close_calls: %d\n",
       dev->OpenCount,
       dev->WROpenCount, close_calls);
#endif

  if (pDeviceContext == NULL)
    return FALSE;

  {
    PipeDeviceContext::LockGuard guard G(*pDeviceContext);

    if (pDeviceContext->OpenCount != 0)
      return FALSE;
  }

  LOG ("deactivate success\n");

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
  LOGSCOPE ("\n");

  LOG ("oc %lu, wc: %lu, AccessCode %x, ShareMode %x\n",
       pDeviceContext->OpenCount, pDeviceContext->WROpenCount,
       AccessCode, ShareMode);

  PipeOpenContext* pOpenContext =
    new PipeOpenContext (pDeviceContext, AccessCode, ShareMode);

  LOG ("going to lock\n");

  PipeDeviceContext::LockGuard guard G(*pOpenContext->DeviceContext);

  LOG ("locked\n");

  pDeviceContext->OpenCount++;

  if (AccessCode & GENERIC_WRITE)
    pDeviceContext->WROpenCount++;

  LOG ("OpenCount: %lu, WROpenCount: %lu\n",
       pDeviceContext->OpenCount,
       pDeviceContext->WROpenCount);

  return (DWORD)pOpenContext;
}

static DWORD WINAPI
Deactivator (HANDLE dev)
{
  LOGSCOPE ("\n");

  if (!DeactivateDevice (dev))
    LOG("deactivate failed\n");
  else
    LOG("deactivate success\n");

  return 0;
}

static void
DeactivatePipeDevice (PipeDeviceContext* dev)
{
  LOGSCOPE ("\n");
  HANDLE hdev = GetDeviceHandle (dev);
  HANDLE h = CreateThread (NULL, 0, Deactivator, hdev, 0, NULL);
  CloseHandle (h);
}

PIPEDEV_API BOOL
Close (PipeOpenContext* pOpenContext)
{
  LOGSCOPE ("\n");
#ifdef DEBUG_MODE
  close_calls++;
#endif

  if (pOpenContext)
    LOG ("oc %lu, wc: %lu\n",
	 pOpenContext->DeviceContext->OpenCount,
	 pOpenContext->DeviceContext->WROpenCount);
  else
    LOG ("openctx %p\n", pOpenContext);

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

  LOG ("oc %lu, wc: %lu\n",
       dev->OpenCount, dev->WROpenCount);

  if (dev->OpenCount == 0)
    {
      /* Deactivating the device here seems to
	 corrupt the Device.exe, and sometimes hangs
	 the device.  Do it in an auxilary thread.  */
      DeactivatePipeDevice (dev);
    }

  return TRUE;
}

PIPEDEV_API DWORD
Read (PipeOpenContext* pOpenContext, LPVOID pBuffer_, DWORD dwCount)
{
  LOGSCOPE ("\n");

  if (IsBadReadPtr (pBuffer_, dwCount))
    {
      SetLastError (ERROR_INVALID_PARAMETER);
      LOG ("\n");
      return -1;
    }
	
  BYTE* pBuffer = (BYTE*)pBuffer_;
  DWORD needed = dwCount;

  if (pOpenContext == NULL
      || (pOpenContext->dwAccessCode & GENERIC_READ) == 0)
    {
      LOG ("Invalid access or no context\n");
      return (DWORD)-1;
    }

  PipeDeviceContext* dev = pOpenContext->DeviceContext;

  BOOL breaknext = FALSE;
  HANDLE Events[2];

  do
    {
      {
	PipeDeviceContext::LockGuard guard G(*dev);

	Events[0] = dev->WriteEvent;
	Events[1] = dev->AbortEvent;

	/* Read before checking for broken pipe, so
	   we get a chance to return valid data when that
	   happens.  */
	DWORD read = dev->readBytes (pBuffer, dwCount);
	pBuffer += read;
	dwCount -= read;

	if (read)
	  SetEvent (dev->ReadEvent);

	if (dev->Aborting            /* this device is long gone, */
	    || dev->WROpenCount == 0 /* or broken pipe */
	    || dev->OpenCount == 0   /* or, ... weird */
	    )
	  {
	    SetLastError (ERROR_BROKEN_PIPE);
	    if (needed - dwCount)
	      {
		/* I don't know a way to report error and 'valid
		   data' at the same time.  Is there a way?  Instead
		   we report 'valid' and the next Read call will error.  */
		LOG ("Pipe broken, but with data\n");
		break;
	      }
	    LOG ("Pipe broken\n");
	    return (DWORD) -1;
	  }

	if (read || dwCount == 0)
	  {
	    /* We've either read something or pBuffer_ is full.  */
	    LOG ("read || dwCount == 0\n");
	    break;
	  }

	if (breaknext)
	  break;
      }

      /* The buffer was empty, wait for data.  */
      switch (WaitForMultipleObjects (2, Events, FALSE, INFINITE))
	{
	case WAIT_OBJECT_0:
	  /* Data was written to the pipe.  Do one more iteration
	     to fetch what we can and bail out.  */
	  breaknext = TRUE;
	  break;
	default:
	  /* With either wait error or AbortEvent
	     signaled, return with error.  */
	  LOG ("WaitForMultipleObjects default case\n");
	  return (DWORD) -1;
	}
    }
  while (dwCount);

  LOG ("Read: %u\n", needed - dwCount);
  return needed - dwCount;
}

PIPEDEV_API DWORD
Write (PipeOpenContext* pOpenContext, LPCVOID pBuffer_, DWORD dwCount)
{
  LOGSCOPE ("\n");

  if (IsBadWritePtr ((void*) pBuffer_, dwCount))
    {
      SetLastError (ERROR_INVALID_PARAMETER);
      LOG ("\n");
      return -1;
    }

  const BYTE* pBuffer = (const BYTE*)pBuffer_;
  DWORD needed = dwCount;

  LOG ("oc %lu, wc: %lu\n",
       pOpenContext->DeviceContext->OpenCount,
       pOpenContext->DeviceContext->WROpenCount);

  if (pOpenContext == NULL
      || (pOpenContext->dwAccessCode & GENERIC_WRITE) == 0)
    {
      LOG ("Invalid access or no context\n");
      return (DWORD)-1;
    }

  PipeDeviceContext* dev = pOpenContext->DeviceContext;

  BOOL breaknext = FALSE;
  HANDLE Events[2];

  do
    {
      {
	PipeDeviceContext::LockGuard guard G(*dev);

	LOG ("lock acquired\n");

	Events[0] = dev->ReadEvent;
	Events[1] = dev->AbortEvent;

	if (dev->Aborting)
	  {
	    /* this device is long gone */
	    SetLastError (ERROR_NO_DATA);
	    LOG ("Device is gone\n");
	    return (DWORD)-1;
	  }

	if (dev->OpenCount == 0)
	  {
	    /* weird */
	    SetLastError (ERROR_NO_DATA);
	    LOG ("Device is not open\n");
	    return (DWORD)-1;
	  }

	if (dev->OpenCount == dev->WROpenCount)
	  {
	    /* no readers */
	    SetLastError (ERROR_NO_DATA);
	    LOG ("No readers left: oc %lu, wc: %lu\n",
		 dev->OpenCount,
		 dev->WROpenCount);
	    return (DWORD)-1;
	  }

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
	  /* With either wait error or AbortEvent
	     signaled, return with error.  */
	  LOG ("WaitForMultipleObjects default case\n");
	  return (DWORD) -1;
	}
    }
  while (dwCount);

  LOG ("%u\n", needed - dwCount);
  return needed - dwCount;
}

PIPEDEV_API DWORD
Seek (PipeOpenContext* /*pOpenContext*/, long /*Amount*/, WORD /*wType*/)
{
  LOGSCOPE ("\n");
  /* Pipes don't support seeking.  */
  return (DWORD)-1;
}

PIPEDEV_API BOOL
IOControl (PipeOpenContext* pOpenContext, DWORD dwCode,
	   PBYTE pBufIn, DWORD dwLenIn,
	   PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
  LOGSCOPE ("\n");

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
  LOG ("%x : %d\n", dwCode, dev->OpenCount);
#endif

  if (dwCode == IOCTL_PSL_NOTIFY)
    {
      PDEVICE_PSL_NOTIFY pPslPacket = (PDEVICE_PSL_NOTIFY)pBufIn;

      if (pPslPacket->dwSize == sizeof (DEVICE_PSL_NOTIFY)
	  && pPslPacket->dwFlags == DLL_PROCESS_EXITING)
	{
	  LOG ("Process dying: %p : %p\n", pPslPacket->hProc, pPslPacket->hThread);
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
