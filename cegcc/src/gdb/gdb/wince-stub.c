/* wince-stub.c -- debugging stub for a Windows CE device

   Copyright 1999, 2000, 1006 Free Software Foundation, Inc.
   Contributed by Cygnus Solutions, A Red Hat Company.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without eve nthe implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

/* by Christopher Faylor (cgf@cygnus.com) */

#include <winsock.h>
#include <stdarg.h>
#include <windows.h>

wchar_t * wcschr(const wchar_t *, wchar_t);

typedef unsigned int in_addr_t;

#include "wince-stub.h"

#define MALLOC(n) (void *) LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT, (UINT)(n))
#define FREE(s) LocalFree ((HLOCAL)(s))

wchar_t* wcschr (wchar_t *s, wchar_t c);

static int skip_next_id = 0;	/* Don't read next API code from socket */

FILE* debug_f;

/* v-style interface for handling varying argument list error messages.
   Displays the error message in a dialog box and exits when user clicks
   on OK. */
static void
vstub_error (LPCWSTR fmt, va_list args)
{
  WCHAR buf[4096];
  wvsprintfW (buf, fmt, args);

  MessageBoxW (NULL, buf, L"GDB", MB_ICONERROR);
  WSACleanup ();
  ExitThread (1);
}

/* The standard way to display an error message and exit. */
static void
stub_error (LPCWSTR fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  vstub_error (fmt, args);
	va_end(args);
}

#if 0
/* v-style interface for handling varying argument list error messages.
Displays the error message in a dialog box and exits when user clicks
on OK. */
static void
vstub_log (LPCWSTR fmt, va_list args)
{
	WCHAR wbuf[4096];
	char buf[4096];
	wvsprintfW (wbuf, fmt, args);
	int len = wcslen(buf);
	wcstombs(buf, wbuf, len+1);
  send (debug_s, buf, len, 0);
}
#endif

/* v-style interface for handling varying argument list error messages.
Displays the error message in a dialog box and exits when user clicks
on OK. */
static void
vstub_log (LPCWSTR fmt, va_list args)
{
	if (!debug_f)
		return;

	WCHAR wbuf[4096];
	char buf[4096];
	wvsprintfW (wbuf, fmt, args);
	int len = wcslen(wbuf);
	wcstombs(buf, wbuf, len+1);
	fwrite (buf, 1, len, debug_f);
}

/* The standard way to display an error message and exit. */
static void
stub_log (LPWSTR fmt, ...)
{
	va_list args;
	va_start (args, fmt);
	vstub_log (fmt, args);
	va_end(args);
}



/* Allocate a limited pool of memory, reallocating over unused
   buffers.  This assumes that there will never be more than four
   "buffers" required which, so far, is a safe assumption. */
static LPVOID
mempool (unsigned int len)
{
  static int outn = -1;
  static LPWSTR outs[4] = {NULL, NULL, NULL, NULL};

  if (++outn >= (sizeof (outs) / sizeof (outs[0])))
    outn = 0;

  /* Allocate space for the converted string, reusing any previously allocated
     space, if applicable. */
  if (outs[outn])
    FREE (outs[outn]);
  outs[outn] = (LPWSTR) MALLOC (len);

  return outs[outn];
}

/* Standard "oh well" can't communicate error.  Someday this might attempt
   synchronization. */
static void
attempt_resync (LPCWSTR huh, int s)
{
  stub_error (L"lost synchronization with host attempting %s.  Error %d", huh, WSAGetLastError ());
}

/* Read arbitrary stuff from a socket. */
static int
sockread (LPCWSTR huh, int s, void *str, size_t n)
{
  for (;;)
    {
      if (recv (s, str, n, 0) == (int) n)
	return n;
      attempt_resync (huh, s);
    }
}

/* Write arbitrary stuff to a socket. */
static int
sockwrite (LPCWSTR huh, int s, const void *str, size_t n)
{
  for (;;)
    {
      if (send (s, str, n, 0) == (int) n)
	return n;
      attempt_resync (huh, s);
    }
}

/* Get an ID (possibly) and a DWORD from the host gdb.
   Don't bother with the id if the main loop has already
   read it. */
static DWORD
getdword (LPCWSTR huh, int s, gdb_wince_id what_this)
{
  DWORD n;
  gdb_wince_id what;

  if (skip_next_id)
    skip_next_id = 0;
  else {
    do {
      if (sockread (huh, s, &what, sizeof (what)) != sizeof (what))
	stub_error (L"error getting record type from host - %s.", huh);
    } while (what_this != what);
  }

  if (sockread (huh, s, &n, sizeof (n)) != sizeof (n))
    stub_error (L"error getting %s from host.", huh);

	stub_log(L"(%02d) GET: %s : 0x%08x\n", what, huh, n);
  return n;
}

/* Get a an ID (possibly) and a WORD from the host gdb.
   Don't bother with the id if the main loop has already
   read it. */
static WORD
getword (LPCWSTR huh, int s, gdb_wince_id what_this)
{
  WORD n;
  gdb_wince_id what;

  if (skip_next_id)
    skip_next_id = 0;
  else
    do
      if (sockread (huh, s, &what, sizeof (what)) != sizeof (what))
	stub_error (L"error getting record type from host - %s.", huh);
    while (what_this != what);

  if (sockread (huh, s, &n, sizeof (n)) != sizeof (n))
    stub_error (L"error getting %s from host.", huh);

  return n;
}

/* Handy defines for getting various types of values. */
#define gethandle(huh, s, what) (HANDLE) getdword ((huh), (s), (what))
#define getpvoid(huh, s, what) (LPVOID) getdword ((huh), (s), (what))
#define puthandle(huh, s, what, h) putdword ((huh), (s), (what), (DWORD) (h))

/* Get an arbitrary block of memory from the gdb host.  This comes in
   two chunks an id/dword representing the length and the stream of memory
   itself. Returns a pointer, allocated via mempool, to a memory buffer. */
static LPWSTR
getmemory (LPCWSTR huh, int s, gdb_wince_id what, gdb_wince_len *inlen)
{
  LPVOID p;
  gdb_wince_len dummy;

  if (!inlen)
    inlen = &dummy;

  *inlen = getlen (huh, s, what);

  p = mempool ((unsigned int) *inlen);	/* FIXME: check for error */

  if ((gdb_wince_len) sockread (huh, s, p, *inlen) != *inlen)
    stub_error (L"error getting string from host.");

  return p;
}


/* Output an id/dword to the host */
static void
putdword (LPCWSTR huh, int s, gdb_wince_id what, DWORD n)
{
	stub_log(L"(%02d) PUT: %s : 0x%08x\n", what, huh, n);

  if (sockwrite (huh, s, &what, sizeof (what)) != sizeof (what))
    stub_error (L"error writing record id for %s to host.", huh);
  if (sockwrite (huh, s, &n, sizeof (n)) != sizeof (n))
    stub_error (L"error writing %s to host.", huh);
}

/* Output an id/word to the host */
static void
putword (LPCWSTR huh, int s, gdb_wince_id what, WORD n)
{
  if (sockwrite (huh, s, &what, sizeof (what)) != sizeof (what))
    stub_error (L"error writing record id for %s to host.", huh);
  if (sockwrite (huh, s, &n, sizeof (n)) != sizeof (n))
    stub_error (L"error writing %s to host.", huh);
}


/* Put an arbitrary block of memory to the gdb host.  This comes in
   two chunks an id/dword representing the length and the stream of memory
   itself. */
static void
putmemory (LPCWSTR huh, int s, gdb_wince_id what, const void *mem, gdb_wince_len len)
{
  putlen (huh, s, what, len);
  if (((long)len > 0) && (gdb_wince_len) sockwrite (huh, s, mem, len) != len)
    stub_error (L"error writing memory to host.");
}

/* Output the result of an operation to the host.  If res != 0, sends a block of
   memory starting at mem of len bytes.  If res == 0, sends -GetLastError () and
   avoids sending the mem. */
static void
putresult (LPCWSTR huh, gdb_wince_result res, int s, gdb_wince_id what, const void *mem, gdb_wince_len len)
{
  if (!res)
    len = -(int) GetLastError ();
  putmemory (huh, s, what, mem, len);
}

static HANDLE curproc;

/* Emulate CreateProcess.  Returns &pi if no error. */
static void
create_process (int s)
{
  LPWSTR exec_file = getmemory (L"CreateProcess exec_file", s, GDB_CREATEPROCESS, NULL);
  LPWSTR args = getmemory (L"CreateProcess args", s, GDB_CREATEPROCESS, NULL);
  DWORD flags = getdword (L"CreateProcess flags", s, GDB_CREATEPROCESS);
  PROCESS_INFORMATION pi;
  gdb_wince_result res;

	if (!exec_file || exec_file[0] == '\0')
	{
		exec_file = args;
	}

  res = CreateProcessW (exec_file,
			args,	/* command line */
			NULL,	/* Security */
			NULL,	/* thread */
			FALSE,	/* inherit handles */
			flags,	/* start flags */
			NULL,
			NULL,	/* current directory */
			NULL,
			&pi);

#if 0
  WCHAR buf[512];
  wsprintf(buf, L"res = %d", res);
  MessageBoxW(0, buf, L"CreateProcessW", 0);
#endif
  putresult (L"CreateProcess", res, s, GDB_CREATEPROCESS, &pi, sizeof (pi));

#if 0
  if (!res)
  {
	  wchar_t buf[100];
	  wsprintf(buf, L"Error creating process. Error: %d", (int)GetLastError());
	  MessageBoxW (NULL, buf, L"GDB", MB_ICONERROR);
  }
#endif

  if (curproc)
  {
	  // which conditions trigger this?
	  MessageBoxW (NULL, L"There was already a curproc", L"GDB", MB_ICONERROR);
  }
  else
	  curproc = pi.hProcess;
}

/* Emulate TerminateProcess.  Returns return value of TerminateProcess if
   no error.
   *** NOTE:  For some unknown reason, TerminateProcess seems to always return
   an ACCESS_DENIED (on Windows CE???) error.  So, force a TRUE value for now. */
static void
terminate_process (int s)
{
  gdb_wince_result res;
  HANDLE h = gethandle (L"TerminateProcess handle", s, GDB_TERMINATEPROCESS);

//  res = TerminateProcess (h, 0) || 1;	/* Doesn't seem to work on SH so default to TRUE */
  res = TerminateProcess (h, 0);
  putresult (L"Terminate process result", res, s, GDB_TERMINATEPROCESS,
	     &res, sizeof (res));
  /* So, we just bye-bye */
  WSACleanup ();
  ExitThread (1);
}

static void debug_active_process (int s)
{
  gdb_wince_result res;
  DWORD pid = getdword (L"DebugActiveProcess pid", s, GDB_DEBUGACTIVEPROCESS);
  res = DebugActiveProcess (pid);
  putresult (L"SetThreadContext result", res, s, GDB_DEBUGACTIVEPROCESS,
    &res, sizeof (res));
}

static int stepped = 0;

/* Handle single step instruction.  FIXME: unneded? */
static void
flag_single_step (int s)
{
  stepped = 1;
  skip_next_id = 0;
}

struct skipper
{
  wchar_t *s;
  int nskip;
} skippy[] =
{
  {L"Undefined Instruction:", 1},
  {L"Data Abort:", 2},
  {NULL, 0}
};

static int
skip_message (DEBUG_EVENT *ev)
{
  char s[80];
  DWORD nread;
  struct skipper *skp;
  int nbytes = ev->u.DebugString.nDebugStringLength;

  if (nbytes > sizeof(s))
    nbytes = sizeof(s);

  memset (s, 0, sizeof (s));
  if (!ReadProcessMemory (curproc, ev->u.DebugString.lpDebugStringData,
			  s, nbytes, &nread))
    return 0;

  for (skp = skippy; skp->s != NULL; skp++)
    if (wcsncmp ((wchar_t *) s, skp->s, wcslen (skp->s)) == 0)
      return skp->nskip;

  return 0;
}

/* Emulate WaitForDebugEvent.  Returns the debug event on success. */
static void
wait_for_debug_event (int s)
{
  DWORD ms = getdword (L"WaitForDebugEvent ms", s, GDB_WAITFORDEBUGEVENT);
  gdb_wince_result res;
  DEBUG_EVENT ev;
  static int skip_next = 0;

  for (;;)
    {
      res = WaitForDebugEvent (&ev, ms);

#if 0
			putresult (L"WaitForDebugEvent event", res, s, GDB_WAITFORDEBUGEVENT,
				&ev, sizeof (ev));
			break;
#endif

	  if (ev.dwProcessId != (DWORD)curproc)
		  goto ignore;

      if (ev.dwDebugEventCode == OUTPUT_DEBUG_STRING_EVENT)
	{
	  if (skip_next)
	    {
	      skip_next--;
	      goto ignore;
	    }
	  if (skip_next = skip_message (&ev))
	    goto ignore;
	}
	  else if (ev.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
	  {
		  curproc = NULL;
	  }

      putresult (L"WaitForDebugEvent event", res, s, GDB_WAITFORDEBUGEVENT,
		 &ev, sizeof (ev));
      break;

    ignore:
      ContinueDebugEvent (ev.dwProcessId, ev.dwThreadId, DBG_CONTINUE);
    }

}

/* Emulate OpenProcess.  Returns CONTEXT structure on success. */
static void
open_process (int s)
{
	DWORD fdwAccess = getdword (L"OpenProcess access", s, GDB_OPENPROCESS);
	BOOL fInherit = getdword (L"OpenProcess inherit", s, GDB_OPENPROCESS);
	DWORD pid = getdword (L"OpenProcess pid", s, GDB_OPENPROCESS);

	HANDLE h = OpenProcess(fdwAccess, fInherit, pid);

	puthandle (L"OpenProcess result", s, GDB_OPENPROCESS, h);
}

/* Emulate GetThreadContext.  Returns CONTEXT structure on success. */
static void
get_thread_context (int s)
{
  CONTEXT c = {0};
  gdb_wince_result res;
  HANDLE h = gethandle (L"GetThreadContext handle", s, GDB_GETTHREADCONTEXT);
  c.ContextFlags = getdword (L"GetThreadContext flags", s, GDB_GETTHREADCONTEXT);
  res = (gdb_wince_result) GetThreadContext (h, &c);

  /* We only care for the context until the Psr register inclusive. 
     The sdk defines a bigger context structure, bug w32api headers don't have
     the rest of the context defined yet. Until we find documentation of the missing members,
     we use only what we know. Without this, we were overwriting some memory in the host.
  */
  size_t size = offsetof (CONTEXT, Psr) + sizeof (c.Psr);
  putresult (L"GetThreadContext data", res, s, GDB_GETTHREADCONTEXT,
	     &c, size);
}

/* Emulate SetThreadContext.  Returns success of SetThreadContext. */
static void
set_thread_context (int s)
{
  gdb_wince_result res;
  HANDLE h = gethandle (L"SetThreadContext handle", s, GDB_SETTHREADCONTEXT);
  LPCONTEXT pc = (LPCONTEXT) getmemory (L"SetThreadContext context", s,
					GDB_SETTHREADCONTEXT, NULL);

  /* The problem described in get_thread_context applies here, but in a different way.
     The host is sending a smaller context that possibly the target is expecting. */
  res = SetThreadContext (h, pc);
  putresult (L"SetThreadContext result", res, s, GDB_SETTHREADCONTEXT,
	     &res, sizeof (res));
}

/* Emulate ReadProcessMemory.  Returns memory read on success. */
static void
read_process_memory (int s)
{
  HANDLE h = gethandle (L"ReadProcessMemory handle", s, GDB_READPROCESSMEMORY);
  LPVOID p = getpvoid (L"ReadProcessMemory base", s, GDB_READPROCESSMEMORY);
  gdb_wince_len len = getlen (L"ReadProcessMemory size", s, GDB_READPROCESSMEMORY);
  LPVOID buf = mempool ((unsigned int) len);
  DWORD outlen;
  gdb_wince_result res;

  outlen = 0;
  res = (gdb_wince_result) ReadProcessMemory (h, p, buf, len, &outlen);
  putresult (L"ReadProcessMemory data", res, s, GDB_READPROCESSMEMORY,
	     buf, (gdb_wince_len) outlen);
}

/* Emulate WriteProcessMemory.  Returns WriteProcessMemory success. */
static void
write_process_memory (int s)
{
  HANDLE h = gethandle (L"WriteProcessMemory handle", s, GDB_WRITEPROCESSMEMORY);
  LPVOID p = getpvoid (L"WriteProcessMemory base", s, GDB_WRITEPROCESSMEMORY);
  gdb_wince_len len;
  LPVOID buf = getmemory (L"WriteProcessMemory buf", s, GDB_WRITEPROCESSMEMORY, &len);
  DWORD outlen;
  gdb_wince_result res;

  outlen = 0;
  res = WriteProcessMemory (h, p, buf, len, &outlen);
  putresult (L"WriteProcessMemory data", res, s, GDB_WRITEPROCESSMEMORY,
	     & outlen, sizeof (gdb_wince_len));
}

static void flush_instruction_cache ( int s )
{
  HANDLE h = gethandle (L"FlushInstructionCache handle", s, GDB_FLUSHINSTRUCTIONCACHE);
  LPCVOID baseaddress = getpvoid (L"FlushInstructionCache base", s, GDB_FLUSHINSTRUCTIONCACHE);
  DWORD size = getdword (L"FlushInstructionCache size", s, GDB_FLUSHINSTRUCTIONCACHE);
  gdb_wince_result res = FlushInstructionCache(h, baseaddress, size);
  putresult (L"FlushInstructionCache result", res, s, GDB_FLUSHINSTRUCTIONCACHE,
    &res, sizeof (res));
}


static void
wait_for_single_object (int s)
{
  HANDLE h = gethandle (L"WaitForSingleObject handle", s, GDB_WAITFORSINGLEOBJECT);
  DWORD millis = getdword (L"WaitForSingleObject handle", s, GDB_WAITFORSINGLEOBJECT);

	DWORD res = WaitForSingleObject (h, millis);
	putdword (L"WaitForSingleObject result", s, GDB_WAITFORSINGLEOBJECT, res);
}

/* Emulate SuspendThread.  Returns value returned from SuspendThread. */
static void
suspend_thread (int s)
{
  DWORD res;
  HANDLE h = gethandle (L"SuspendThread handle", s, GDB_SUSPENDTHREAD);
  res = SuspendThread (h);
  putdword (L"SuspendThread result", s, GDB_SUSPENDTHREAD, res);
}

/* Emulate ResumeThread.  Returns value returned from ResumeThread. */
static void
resume_thread (int s)
{
  DWORD res;
  HANDLE h = gethandle (L"ResumeThread handle", s, GDB_RESUMETHREAD);
  res = ResumeThread (h);
  putdword (L"ResumeThread result", s, GDB_RESUMETHREAD, res);
}

/* Emulate ContinueDebugEvent.  Returns ContinueDebugEvent success. */
static void
continue_debug_event (int s)
{
  gdb_wince_result res;
  DWORD pid = getdword (L"ContinueDebugEvent pid", s, GDB_CONTINUEDEBUGEVENT);
  DWORD tid = getdword (L"ContinueDebugEvent tid", s, GDB_CONTINUEDEBUGEVENT);
  DWORD status = getdword (L"ContinueDebugEvent status", s, GDB_CONTINUEDEBUGEVENT);
  res = (gdb_wince_result) ContinueDebugEvent (pid, tid, status);
  putresult (L"ContinueDebugEvent result", res, s, GDB_CONTINUEDEBUGEVENT, &res, sizeof (res));
}

/* Emulate CloseHandle.  Returns CloseHandle success. */
static void
close_handle (int s)
{
  gdb_wince_result res;
  HANDLE h = gethandle (L"CloseHandle handle", s, GDB_CLOSEHANDLE);
  res = (gdb_wince_result) CloseHandle (h);
  putresult (L"CloseHandle result", res, s, GDB_CLOSEHANDLE, &res, sizeof (res));
}

typedef struct {
  enum win_func id;
  void (*handler)(int s);
} msg_handler_map_t;

static const msg_handler_map_t msg_handler_map[] = 
{
  { GDB_CLOSEHANDLE, close_handle },
  { GDB_CONTINUEDEBUGEVENT, continue_debug_event },
  { GDB_CREATEPROCESS, create_process },
  { GDB_DEBUGACTIVEPROCESS, debug_active_process },
  { GDB_FLUSHINSTRUCTIONCACHE, flush_instruction_cache },
  { GDB_GETTHREADCONTEXT, get_thread_context },
	{ GDB_OPENPROCESS, open_process },
  { GDB_READPROCESSMEMORY, read_process_memory },
  { GDB_RESUMETHREAD, resume_thread },
  { GDB_SETTHREADCONTEXT, set_thread_context },
  { GDB_SINGLESTEP, flag_single_step },
  { GDB_STOPSTUB, terminate_process },
  { GDB_SUSPENDTHREAD, suspend_thread },
  { GDB_TERMINATEPROCESS, terminate_process },
  { GDB_WAITFORDEBUGEVENT, wait_for_debug_event },
  { GDB_WAITFORSINGLEOBJECT, wait_for_single_object },
  { GDB_WRITEPROCESSMEMORY, write_process_memory },
};

#define HANDLER_COUNT (sizeof (msg_handler_map)/sizeof (msg_handler_map[0]))

/* compile time asserts to make sure we are catching every possible msg, and only once each.*/
static char error_there_are_handlers_missing[HANDLER_COUNT-(GDB_INVALID-GDB_FIRST)];
static char error_there_are_too_many_handlers[(GDB_INVALID-GDB_FIRST)-HANDLER_COUNT];

/* Main loop for reading requests from gdb host on the socket. */
static void
dispatch (int s)
{
  gdb_wince_id id;

again:
  /* Continue reading from socket until we receive a GDB_STOPSUB. */
  while (sockread (L"Dispatch", s, &id, sizeof (id)) > 0)
    {
      skip_next_id = 1;
			const msg_handler_map_t* handler = msg_handler_map;
			int i;
			for (i = 0; i < HANDLER_COUNT; i++) {
				if (msg_handler_map[i].id == id) {
					(*msg_handler_map[i].handler)(s);
					goto again;
				}
			}
	    WCHAR buf[80];
      wsprintfW (buf, L"Invalid command id received: %d", id);
      MessageBoxW (NULL, buf, L"GDB", MB_ICONERROR);
      skip_next_id = 0;
	  }
}

/* The Windows Main entry point */
int WINAPI
WinMain (HINSTANCE hi, HINSTANCE hp, LPWSTR cmd, int show)
{
  struct hostent *h = NULL;
  in_addr_t in_a;
  int s;
  struct WSAData wd = {0};
  struct sockaddr_in sin = {0};
  int tmp;
  LPWSTR whost;
  char host[80];

	debug_f = fopen ("stub-log.txt", "w");
	if (!debug_f) {
		MessageBoxW(0, L"error opening debug log file", L"gdb stub", 0);
	}
	else
	{
		fflush(debug_f);
		setvbuf(debug_f, 0, _IONBF, 0);
		fflush(debug_f);
	}

	stub_log(L"stub loaded and running\n");

  whost = wcschr (cmd, L' ');	/* Look for argument. */

  /* If no host is specified, just use default */
  if (whost)
    {
      /* Eat any spaces. */
      while (*whost == L' ' || *whost == L'\t')
	whost++;

      wcstombs (host, whost, 80);	/* Convert from UNICODE to ascii */
    }

  /* Winsock initialization. */
  if (WSAStartup (MAKEWORD (1, 1), &wd))
    stub_error (L"Couldn't initialize WINSOCK.");

  /* If whost was specified, first try it.  If it was not specified or the
     host lookup failed, try the Windows CE magic ppp_peer lookup.  ppp_peer
     is supposed to be the Windows host sitting on the other end of the
     serial cable. */
  if (whost && *whost) {
	 // gethostbyname function cannot resolve IP address strings passed to it.
     if (*host >= '0' && *host <= '9') {
       in_a = inet_addr(host);
     }
     else {
       h = gethostbyname (host);
       if (h == NULL)
         stub_error (L"Couldn't get IP address of host system.  Error %d", WSAGetLastError ());
     }
  }
  else if ((h = gethostbyname ("ppp_peer")) == NULL)
    stub_error (L"Couldn't get IP address of host system.  Error %d", WSAGetLastError ());

  /* Get a socket. */
  if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    stub_error (L"Couldn't connect to host system. Error %d", WSAGetLastError ());

  /* Allow rapid reuse of the port. */
  tmp = 1;
  setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *) &tmp, sizeof (tmp));
  tmp = 1;
  setsockopt (s, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof (tmp));

  /* Set up the information for connecting to the host gdb process. */
  memset (&sin, 0, sizeof (sin));
  if (h) {
    sin.sin_family = h->h_addrtype;
    memcpy (&sin.sin_addr, h->h_addr, h->h_length);
  } else {
    sin.sin_family = AF_INET;
    memcpy (&sin.sin_addr, (char*)&in_a, sizeof(in_a));
       WCHAR buf[100];
	   int sin_addr;
	   memcpy(&sin_addr, &sin.sin_addr, sizeof(sin_addr));
       wsprintf(buf, L"!%08x", sin_addr);
       MessageBoxW (NULL, buf, L"GDB", MB_ICONERROR);
  }
#define DEFAULT_PORT 7000
#define DEFAULT_DEBUG_PORT 7001
  sin.sin_port = htons (DEFAULT_PORT);	/* FIXME: This should be configurable */

	stub_log(L"going to connect\n");

  /* Connect to host */
  if (connect (s, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    stub_error (L"Couldn't connect to host gdb.");

#if 0
	/* Connect to debug host */
	if (connect (debug_s, (struct sockaddr *) &sin, sizeof (sin)) < 0)
		stub_error (L"Couldn't connect to debug host.");
#endif

	stub_log(L"starting dispatch\n");

  /* Read from socket until told to exit. */
  dispatch (s);
  WSACleanup ();
  return 0;
}
