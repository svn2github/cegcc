/* Target-vector operations for controlling Windows CE child processes, for GDB.

   Copyright 1999, 2000, 2001, 2004, 2006 Free Software Foundation, Inc.
   Contributed by Cygnus Solutions, A Red Hat Company.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

/* by Christopher Faylor (cgf@cygnus.com) */
/* merge back with native win32 debugger by Pedro Alves (pedro_alves@portugalmail.pt) */

/* We assume we're being built with and will be used for cygwin.  */

#ifdef SHx
#undef SH4
#define SH4		/* Just to get all of the CONTEXT defines.  */
#endif

#include "defs.h"
#include "frame.h"	/* required by inferior.h */
#include "inferior.h"
#include "target.h"
#include "gdbcore.h"
#include <fcntl.h>
#include <stdlib.h>

#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <basetyps.h>	/* required for <rapi.h> on linux + w32api.  */
#include <rapi.h>
#include <netdb.h>
#ifdef	CYGWIN
#include <cygwin/in.h>
#include <cygwin/socket.h>
#else
#include <netinet/tcp.h>
#endif
#include <arpa/inet.h>

#include "buildsym.h"
#include "symfile.h"
#include "objfiles.h"
#include "gdb_string.h"
#include "gdbthread.h"
#include "gdbcmd.h"
#include <sys/param.h>
#include "wince-stub.h"
#include <time.h>
#include "regcache.h"

#include "win32.h"

#include "gdb_assert.h"

#ifndef __CYGWIN__
static int MyWcslen (const wchar_t *);
static int MyMbstowcs(wchar_t *dest, const char *src, int n);
#endif

size_t wcslen (const wchar_t *);

static const char* werror (DWORD err);

/* Should go to w32api. 
   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/apisp/html/sp_rapi_mdcy.asp
*/
STDAPI_(BOOL) CeSetFileTime (HANDLE, LPFILETIME, LPFILETIME, LPFILETIME); 

static int connection_initialized = 0;	/* True if we've initialized a
					   RAPI session.  */

/* The directory where the stub and executable files are uploaded.  */

#define DEFAULT_UPLOAD_DIR "\\gdb"

static char *remote_directory = DEFAULT_UPLOAD_DIR;

/* The types automatic upload available.  */
static enum
  {
    UPLOAD_ALWAYS = 0,
    UPLOAD_NEWER = 1,
    UPLOAD_NEVER = 2
  }
upload_when = UPLOAD_NEWER;

/* Valid options for 'set remoteupload'.  Note that options
   must track upload_when enum.  */
static struct opts
  {
    const char *name;
    int abbrev;
  }
upload_options[] =
{
  { "always", 1 },
  { "newer", 3 },
  { "never", 3 }
};

static char *remote_upload = NULL;	/* Set by 'set remoteupload'.  */
static int remote_add_host = 0;
static int debug_communication = 0;
static int debug_wince = 0;

#define DEBUG_COM(x)	do { if (debug_communication)	printf_unfiltered x; } while (0)
#define DEBUG_WINCE(x)	do { if (debug_wince)	printf_unfiltered x; } while (0)


/******************** Beginning of stub interface ********************/

/* Stub interface description:

   The Windows CE stub implements a crude RPC.  The hand-held device
   connects to gdb using port 7000.  gdb and the stub then communicate
   using packets where:

   byte 0:              command id (e.g. Create Process)

   byte 1-4:    DWORD

   byte 1-2:    WORD

   byte 1-2:    length
   byte 3-n:    arbitrary memory.

   The interface is deterministic, i.e., if the stub expects a DWORD
   then the gdb server should send a DWORD.
 */

/* Note: In the functions below, the `huh' parameter is a string
   passed from the function containing a descriptive string concerning
   the current operation.  This is used for error reporting.

   The 'what' parameter is a command id as found in wince-stub.h.

   Hopefully, the rest of the parameters are self-explanatory.
 */

static int s = -1; /* communication socket */

/* v-style interface for handling varying argument list error messages.
   Displays the error message in a dialog box and exits when user clicks
   on OK.  */
static ATTR_NORETURN void
vstub_error (LPCSTR fmt, va_list args)
{
  char buf[4096];
  vsprintf (buf, fmt, args);
  s = -1;
  error (("%s"), buf);
}

/* The standard way to display an error message and exit.  */
static ATTR_NORETURN void
stub_error (LPCSTR fmt,...)
{
  va_list args;
  va_start (args, fmt);
  vstub_error (fmt, args);
}

/* Standard "oh well" can't communicate error.  Someday this might
   attempt synchronization.  */
static void
attempt_resync (LPCSTR huh, int s)
{
  stub_error ("lost synchronization with target attempting %s", huh);
}

/* Read arbitrary stuff from a socket.  */
static int
sockread (LPCSTR huh, int s, void *str, size_t n)
{
  for (;;)
    {
      if (recv (s, str, n, 0) == n)
        return 1;
      attempt_resync (huh, s);
    }
}

/* Write arbitrary stuff to a socket.  */
static int
sockwrite (LPCSTR huh, const void *str, size_t n)
{
  for (;;)
    {
      if (send (s, str, n, 0) == n)
	return 1;
      attempt_resync (huh, s);
    }
}

/* Output an id/dword to the host.  */
static void
putdword (LPCSTR huh, gdb_wince_id what, DWORD n)
{
	DEBUG_COM (("%s %lu (0x%lx)\n", huh, n, n));

  if (!sockwrite (huh, &what, sizeof (what)))
    stub_error ("error writing record id to host for %s", huh);
  if (!sockwrite (huh, &n, sizeof (n)))
    stub_error ("error writing %s to host.", huh);
}

/* Put an arbitrary block of memory to the gdb host.  This comes in
   two chunks an id/dword representing the length and the stream of
   memory itself.  */
static void
putmemory (LPCSTR huh, gdb_wince_id what, 
	   const void *mem, gdb_wince_len len)
{
  putlen (huh, what, len);
  if (((long)len > 0) && !sockwrite (huh, mem, len))
    stub_error ("error writing %s to host.", huh);
}

static DWORD
getdword (LPCSTR huh, gdb_wince_id what_this)
{
  DWORD n;
  gdb_wince_id what;
  do
    if (!sockread (huh, s, &what, sizeof (what)))
      stub_error ("error getting record type from host - %s.", huh);
  while (what_this != what);

  if (!sockread (huh, s, &n, sizeof (n)))
    stub_error ("error getting %s from host.", huh);

  return n;
}

/* Handy defines for getting/putting various types of values.  */
#define gethandle(huh, what) ((HANDLE) getdword ((huh), (what)))
#define getpvoid(huh, what) ((LPVOID) getdword ((huh), (what)))

#define puthandle(huh, what, h) putdword ((huh), (what), (DWORD) (h))
#define putpvoid(huh, what, p) putdword ((huh), (what), (DWORD) (p))

/* Retrieve the result of an operation from the stub.  If nbytes < 0)
   then nbytes is actually an error and nothing else follows.  Use
   SetLastError to remember this.  if nbytes > 0, retrieve a block of
   *nbytes into buf.
 */
int
getresult (LPCSTR huh, gdb_wince_id what, LPVOID buf, 
	   gdb_wince_len * nbytes)
{
  gdb_wince_len dummy;
  if (nbytes == NULL)
    nbytes = &dummy;

  *nbytes = getlen (huh, what);

	DEBUG_COM (("getresult reading %ld (0x%lx) bytes\n", (long)*nbytes, (long)*nbytes));

  if ((long) *nbytes < 0)
    {
      DWORD err = -(long) *nbytes;
      SetLastError (err);
      DEBUG_COM (("getresult: %s failed with error %d: %s", huh, (int)err, werror (err))); 
      return 0;
    }

  if (!sockread (huh, s, buf, *nbytes))
    stub_error ("couldn't read information from wince stub - %s", huh);

  return 1;
}

/* Convert "narrow" string to "wide".  Manipulates a buffer ring of 8
   buffers which hold the translated string.  This is an arbitrary limit
   but it is approximately double the current needs of this module.
 */
LPWSTR
towide (const char *s, gdb_wince_len * out_len)
{
  static int n = -1;
  static LPWSTR outs[8] =
  {NULL /*, NULL, etc.  */ };
  gdb_wince_len dummy;

  if (!out_len)
    out_len = &dummy;

  /* First determine the length required to hold the converted string.  */
#ifdef	__CYGWIN__
  *out_len = sizeof (WCHAR) * MultiByteToWideChar (CP_ACP, 0, s, -1, NULL, 0);
#else
  *out_len = 2 * MyMbstowcs(NULL, s, 0);
#endif
  if (!*out_len)
    return NULL;		/* The conversion failed.  */

  if (++n >= (sizeof (outs) / sizeof (outs[0])))
    n = 0;			/* wrap */

  /* Allocate space for the converted string, reusing any previously
     allocated space, if applicable. Note that if outs[n] is NULL,
     xrealloc will act as a malloc (under cygwin, at least).
   */
  outs[n] = (LPWSTR) xrealloc (outs[n], *out_len);
  memset (outs[n], 0, *out_len);
#ifdef	__CYGWIN__
  (void) MultiByteToWideChar (CP_ACP, 0, s, -1, outs[n], *out_len);
#else
  (void) MyMbstowcs(outs[n], s, *out_len);
#endif
  return outs[n];
}

/******************** Emulation routines start here. ********************

  The functions below are modeled after their Win32 counterparts.
  They are named similarly to Win32 and take exactly the same
  arguments except where otherwise noted.  They communicate with the
  stub on the hand-held device by sending their arguments over the
  socket and waiting for results from the socket.
*/


/* TODO: ### Check the missing parameters. */
static BOOL
create_process2 (LPCSTR exec_file, LPCSTR args, DWORD flags, 
		PROCESS_INFORMATION * pi)
{
  gdb_wince_len len;
  LPWSTR buf;

  buf = towide (exec_file, &len);
  putmemory ("CreateProcess exec_file", GDB_CREATEPROCESS, buf, len);
  buf = towide (args, &len);
  putmemory ("CreateProcess args", GDB_CREATEPROCESS, buf, len);
  putdword ("CreateProcess flags", GDB_CREATEPROCESS, flags);
  return getresult ("CreateProcess result", GDB_CREATEPROCESS, pi, NULL);
}

BOOL WINAPI
REMOTE_CreateProcessA (LPCSTR pszImageName, LPSTR pszCmdLine,
                     LPSECURITY_ATTRIBUTES psaProcess, LPSECURITY_ATTRIBUTES psaThread,
                     BOOL fInheritHandles, DWORD fdwCreate,
                     PVOID pvEnvironment, LPCSTR pszCurDir,
                     LPSTARTUPINFOA psiStartInfo, LPPROCESS_INFORMATION pProcInfo )
{
  return create_process2 (pszImageName,
                          pszCmdLine,
                          fdwCreate,
                          pProcInfo);
}

HANDLE WINAPI
REMOTE_OpenProcess ( DWORD fdwAccess, BOOL fInherit, DWORD pid )
{
  if (s < 0)
    return 0;
	gdb_wince_result res;

	putdword ("OpenProcess fdwAccess", GDB_OPENPROCESS, fdwAccess);
	putdword ("OpenProcess inherit", GDB_OPENPROCESS, fInherit);
	putdword ("OpenProcess pid", GDB_OPENPROCESS, pid);

	return gethandle ("OpenProcess result", GDB_OPENPROCESS);
}

/* Emulate TerminateProcess.  
   Don't bother with the second argument since CE ignores it.
 */
BOOL WINAPI
REMOTE_TerminateProcess ( HANDLE h, UINT exitcode )
{
  (void)exitcode;

  gdb_wince_result res;
  if (s < 0)
    return 0;
  puthandle ("TerminateProcess handle", GDB_TERMINATEPROCESS, h);

  return getresult ("TerminateProcess result", 
		    GDB_TERMINATEPROCESS, &res, NULL);
}

BOOL WINAPI
REMOTE_WaitForDebugEvent (DEBUG_EVENT * ev, DWORD ms)
{
  if (s < 0)
    return 0;
  putdword ("WaitForDebugEvent ms", GDB_WAITFORDEBUGEVENT, ms);

  return getresult ("WaitForDebugEvent event", 
		    GDB_WAITFORDEBUGEVENT, ev, NULL);
}

BOOL WINAPI
REMOTE_GetThreadContext (HANDLE h, CONTEXT * c)
{
  if (s < 0)
    return 0;
  puthandle ("GetThreadContext handle", GDB_GETTHREADCONTEXT, h);
  putdword ("GetThreadContext flags", GDB_GETTHREADCONTEXT, 
	    c->ContextFlags);

  gdb_wince_len len = 0;
  BOOL res = getresult ("GetThreadContext context", 
		    GDB_GETTHREADCONTEXT, c, &len);
  gdb_assert((long)len < 0 || len == sizeof (CONTEXT));
  return res;
}

BOOL WINAPI
REMOTE_SetThreadContext (HANDLE h, const CONTEXT *c)
{
  gdb_wince_result res;
  if (s < 0)
    return 0;
  puthandle ("SetThreadContext handle", GDB_SETTHREADCONTEXT, h);
  putmemory ("SetThreadContext context", GDB_SETTHREADCONTEXT, 
	     c, sizeof (*c));

  return getresult ("SetThreadContext context", 
		    GDB_SETTHREADCONTEXT, &res, NULL);
}

BOOL WINAPI
REMOTE_ReadProcessMemory (HANDLE h, LPCVOID where, 
		     LPVOID buf, DWORD len, 
		     LPDWORD nbytes)
{
  if (s < 0)
    return 0;
  puthandle ("ReadProcessMemory handle", GDB_READPROCESSMEMORY, h);
  putpvoid ("ReadProcessMemory location", GDB_READPROCESSMEMORY, where);
  putlen ("ReadProcessMemory size", GDB_READPROCESSMEMORY, len);

  return getresult ("ReadProcessMemory buf", 
		    GDB_READPROCESSMEMORY, buf, nbytes);
}

BOOL WINAPI
REMOTE_WriteProcessMemory ( HANDLE h, LPVOID where, 
                           LPCVOID buf, DWORD len, LPDWORD nbytes)
{
  if (s < 0)
    return 0;
  puthandle ("WriteProcessMemory handle", GDB_WRITEPROCESSMEMORY, h);
  putpvoid ("WriteProcessMemory location", GDB_WRITEPROCESSMEMORY, where);
  putmemory ("WriteProcProcessMemory buf", GDB_WRITEPROCESSMEMORY, buf, len);

  return getresult ("WriteProcessMemory result", 
		    GDB_WRITEPROCESSMEMORY, nbytes, NULL);
}

BOOL WINAPI
REMOTE_FlushInstructionCache ( HANDLE h, LPCVOID baseaddress, DWORD size )
{
  gdb_wince_result res;
  if (s < 0)
    return 0;
  puthandle ("FlushInstructionCache handle", GDB_FLUSHINSTRUCTIONCACHE, h);
  putpvoid ("FlushInstructionCache base_address", GDB_FLUSHINSTRUCTIONCACHE, baseaddress);
  putdword ("FlushInstructionCache size", GDB_FLUSHINSTRUCTIONCACHE, size);
  return getresult ("FlushInstructionCache result", GDB_FLUSHINSTRUCTIONCACHE, &res, NULL);
}

DWORD WINAPI
REMOTE_WaitForSingleObject (HANDLE h, DWORD millis)
{
  gdb_wince_result res;
  if (s < 0)
    return 1;
  puthandle ("WaitForSingleObject handle", GDB_WAITFORSINGLEOBJECT, h);
  putdword ("WaitForSingleObject millis", GDB_WAITFORSINGLEOBJECT, millis);
  return getdword ("WaitForSingleObject result", GDB_WAITFORSINGLEOBJECT);
}

DWORD WINAPI
REMOTE_SuspendThread (HANDLE h)
{
  if (s < 0)
    return 1;
  puthandle ("SuspendThread handle", GDB_SUSPENDTHREAD, h);
  return getdword ("SuspendThread result", GDB_SUSPENDTHREAD);
}

DWORD WINAPI
REMOTE_ResumeThread (HANDLE h)
{
  if (s < 0)
    return 1;
  puthandle ("ResumeThread handle", GDB_RESUMETHREAD, h);
  return getdword ("SuspendThread result", GDB_RESUMETHREAD);
}

BOOL WINAPI
REMOTE_ContinueDebugEvent (DWORD pid, DWORD tid, DWORD status)
{
  gdb_wince_result res;
  if (s < 0)
    return 0;
  putdword ("ContinueDebugEvent pid", GDB_CONTINUEDEBUGEVENT, pid);
  putdword ("ContinueDebugEvent tid", GDB_CONTINUEDEBUGEVENT, tid);
  putdword ("ContinueDebugEvent status", GDB_CONTINUEDEBUGEVENT, status);
  return getresult ("ContinueDebugEvent result", 
		    GDB_CONTINUEDEBUGEVENT, &res, NULL);
}

BOOL WINAPI
REMOTE_CloseHandle (HANDLE h)
{
  gdb_wince_result res;
  if (s < 0)
    return 0;

  /* Short circuit */
  if (!h)
  {
    SetLastError(6);
    return 0;
  }

  puthandle ("CloseHandle handle", GDB_CLOSEHANDLE, h);
  return (BOOL) getresult ("CloseHandle result", 
			  GDB_CLOSEHANDLE, &res, NULL);
}

BOOL WINAPI
REMOTE_DebugActiveProcess ( DWORD pid )
{
  gdb_wince_result res;
  if (s < 0)
    return 0;
  putdword ("DebugActiveProcess processid", GDB_DEBUGACTIVEPROCESS, pid);
  return getresult ("DebugActiveProcess result", 
    GDB_DEBUGACTIVEPROCESS, &res, NULL);
}

/* This is not a standard Win32 interface.  This function tells the
   stub to terminate.
 */
static void
stop_stub (void)
{
  if (s < 0)
    return;
  (void) putdword ("Stopping gdb stub", GDB_STOPSTUB, 0);
  s = -1;
}

/* This is not a standard Win32 interface.  This function tells the
   stub to perform an 8bit checksum on a given file.
*/
static BOOL
REMOTE_FileChecksum (LPCWSTR file, unsigned char* checksum)
{
  int len;

  if (s < 0)
    return 0;

  len = sizeof (WCHAR) * (wcslen(file)+ 1);
  putmemory ("FileChecksum filename", GDB_FILECHECKSUM, file, len);
  return getresult ("FileChecksum result", 
    GDB_FILECHECKSUM, checksum, NULL);
}


void _init_win32_wce_iface()
{
#define LOAD(FUNC) \
  win32_target_iface. FUNC = REMOTE_ ## FUNC

  LOAD(CloseHandle);
  LOAD(ContinueDebugEvent);
  LOAD(CreateProcessA);
  LOAD(DebugActiveProcess);
  LOAD(GetThreadContext);
  LOAD(FlushInstructionCache);
  LOAD(OpenProcess);
  LOAD(ReadProcessMemory);
  LOAD(ResumeThread);
  LOAD(SetThreadContext);
  LOAD(SuspendThread);
  LOAD(TerminateProcess);
  LOAD(WaitForDebugEvent);
  LOAD(WaitForSingleObject);
  LOAD(WriteProcessMemory);

#undef LOAD
}


/******************** End of emulation routines. ********************/
/******************** End of stub interface ********************/

#ifdef MIPS

CORE_ADDR mips_get_next_pc (CORE_ADDR pc);
#define wince_get_next_pc mips_get_next_pc

#endif /* MIPS */

#ifdef SHx

/* Renesas SH architecture instruction encoding masks */

#define COND_BR_MASK   0xff00
#define UCOND_DBR_MASK 0xe000
#define UCOND_RBR_MASK 0xf0df
#define TRAPA_MASK     0xff00

#define COND_DISP      0x00ff
#define UCOND_DISP     0x0fff
#define UCOND_REG      0x0f00

/* Renesas SH instruction opcodes */

#define BF_INSTR       0x8b00
#define BT_INSTR       0x8900
#define BRA_INSTR      0xa000
#define BSR_INSTR      0xb000
#define JMP_INSTR      0x402b
#define JSR_INSTR      0x400b
#define RTS_INSTR      0x000b
#define RTE_INSTR      0x002b
#define TRAPA_INSTR    0xc300
#define SSTEP_INSTR    0xc3ff

#define T_BIT_MASK     0x0001

/* Return a pointer into a CONTEXT field indexed by gdb register number.
Return a pointer to an address pointing to zero if there is no
corresponding CONTEXT field for the given register number.
*/
static ULONG *
regptr (LPCONTEXT c, int r)
{
  static ULONG zero = 0;
  ULONG *p;
  if (mappings[r] < 0)
    p = &zero;
  else
    p = (ULONG *) (((char *) c) + mappings[r]);
  return p;
}

static CORE_ADDR
sh_get_next_pc (CONTEXT *c)
{
  short *instrMem;
  int displacement;
  int reg;
  unsigned short opcode;

  instrMem = (short *) c->Fir;

  opcode = read_memory_integer ((CORE_ADDR) c->Fir, sizeof (opcode));

  if ((opcode & COND_BR_MASK) == BT_INSTR)
    {
      if (c->Psr & T_BIT_MASK)
	{
	  displacement = (opcode & COND_DISP) << 1;
	  if (displacement & 0x80)
	    displacement |= 0xffffff00;
	  /*
	     * Remember PC points to second instr.
	     * after PC of branch ... so add 4
	   */
	  instrMem = (short *) (c->Fir + displacement + 4);
	}
      else
	instrMem += 1;
    }
  else if ((opcode & COND_BR_MASK) == BF_INSTR)
    {
      if (c->Psr & T_BIT_MASK)
	instrMem += 1;
      else
	{
	  displacement = (opcode & COND_DISP) << 1;
	  if (displacement & 0x80)
	    displacement |= 0xffffff00;
	  /*
	     * Remember PC points to second instr.
	     * after PC of branch ... so add 4
	   */
	  instrMem = (short *) (c->Fir + displacement + 4);
	}
    }
  else if ((opcode & UCOND_DBR_MASK) == BRA_INSTR)
    {
      displacement = (opcode & UCOND_DISP) << 1;
      if (displacement & 0x0800)
	displacement |= 0xfffff000;

      /*
	 * Remember PC points to second instr.
	 * after PC of branch ... so add 4
       */
      instrMem = (short *) (c->Fir + displacement + 4);
    }
  else if ((opcode & UCOND_RBR_MASK) == JSR_INSTR)
    {
      reg = (char) ((opcode & UCOND_REG) >> 8);

      instrMem = (short *) *regptr (c, reg);
    }
  else if (opcode == RTS_INSTR)
    instrMem = (short *) c->PR;
  else if (opcode == RTE_INSTR)
    instrMem = (short *) *regptr (c, 15);
  else if ((opcode & TRAPA_MASK) == TRAPA_INSTR)
    instrMem = (short *) ((opcode & ~TRAPA_MASK) << 2);
  else
    instrMem += 1;

  return (CORE_ADDR) instrMem;
}

CORE_ADDR sh_get_next_pc (CORE_ADDR pc);
#define wince_get_next_pc sh_get_next_pc

#endif /* SHx */

#ifdef ARM

CORE_ADDR arm_get_next_pc (CORE_ADDR pc);
#define wince_get_next_pc arm_get_next_pc

enum target_signal
check_for_step (DEBUG_EVENT *ev, enum target_signal x)
{
  thread_info *th = thread_rec (ev->dwThreadId, 1);

  if (th->stepped &&
      th->step_pc == (CORE_ADDR) ev->u.Exception.ExceptionRecord.ExceptionAddress)
    return TARGET_SIGNAL_TRAP;
  else
    return x;
}

#endif /* ARM */

static void
undoSStep (thread_info * th)
{
  if (th->stepped)
  {
    memory_remove_breakpoint (th->step_pc, (void *) &th->step_prev);
    th->stepped = 0;
  }
}

void
wince_insert_breakpoint (thread_info *th, CORE_ADDR where)
{
  th->stepped = 1;
  th->step_pc = where;
  th->step_prev = 0;
  memory_insert_breakpoint (th->step_pc, (void *) &th->step_prev);
  return;
}

void
wince_software_single_step (enum target_signal ignore,
                            int insert_breakpoints_p)
{
  unsigned long pc;
  /* Info on currently selected thread.  */
  thread_info *th = current_thread;

  if (!insert_breakpoints_p)
  {
    undoSStep (th);
    return;
  }

  pc = read_register (PC_REGNUM);
  wince_insert_breakpoint(th, wince_get_next_pc (pc));
  return;
}

#define FACTOR (0x19db1ded53e8000LL)
#define NSPERSEC 10000000LL

void
time_t_to_filetime (time_t time_in, FILETIME *out)
{
  long long x = time_in * NSPERSEC + FACTOR;
  out->dwHighDateTime = x >> 32;
  out->dwLowDateTime = x;
}

long
to_time_t (FILETIME *ptr)
{
  /* A file time is the number of 100ns since jan 1 1601
  stuffed into two long words.
  A time_t is the number of seconds since jan 1 1970.  */

  long long x = ((long long) ptr->dwHighDateTime << 32) + ((unsigned)ptr->dwLowDateTime);

  /* pass "no time" as epoch */
  if (x == 0)
    return 0;

  x -= FACTOR;			/* number of 100ns between 1601 and 1970 */
  x /= (long long) NSPERSEC;		/* number of 100ns in a second */
  return x;
}

/* Upload a file to the remote device depending on the user's
   'set remoteupload' specification.  */
char *
upload_to_device (const char *to, const char *from)
{
  HANDLE h;
  const char *dir = remote_directory ?: DEFAULT_UPLOAD_DIR;
  int len;
  static char *remotefile = NULL;
  LPWSTR wstr;
  char *p;
  DWORD err;
  const char *in_to = to;
  FILETIME crtime, actime, wrtime;
  struct stat st;
  int fd;

  /* Look for a path separator and only use trailing part.  */
  while ((p = strpbrk (to, "/\\")) != NULL)
    to = p + 1;

  if (!*to)
    error (_("no filename found to upload - %s."), in_to);

  len = strlen (dir) + strlen (to) + 2;
  remotefile = (char *) xrealloc (remotefile, len);
  strcpy (remotefile, dir);
  strcat (remotefile, "\\");
  strcat (remotefile, to);

  if (upload_when == UPLOAD_NEVER)
  {
    DEBUG_WINCE(("Ignoring upload of '%s' (upload_when == UPLOAD_NEVER)\n", remotefile));
    return remotefile;		/* Don't bother uploading.  */
  }

  /* Open the source.  */
  if ((fd = openp (getenv ("PATH"), OPF_TRY_CWD_FIRST, (char *) from,
		   O_RDONLY, 0, NULL)) < 0)
    error (_("couldn't open %s"), from);

  /* Get the time for later comparison.  */
  if (fstat (fd, &st))
    st.st_mtime = (time_t) -1;

  /* Always attempt to create the directory on the remote system.  */
  wstr = towide (dir, NULL);
  (void) CeCreateDirectory (wstr, NULL);

  /* Attempt to open the remote file, creating it if it doesn't exist.  */
  wstr = towide (remotefile, NULL);

  h = CeCreateFile(wstr, GENERIC_READ, FILE_SHARE_READ, NULL, 
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  int got_time = 0;
  if (h && h != INVALID_HANDLE_VALUE)
  {
    got_time = CeGetFileTime (h, &crtime, &actime, &wrtime);
    if (!CeCloseHandle (h)) {
      DWORD err = CeGetLastError ();
      error (_("error closing remote file - %d. (%s)"), (int)err, werror (err));
    }
  }

  /* See if we need to upload the file.  */
  int need_upload = h == NULL || 
                    upload_when == UPLOAD_ALWAYS ||
                    !got_time ||
                    to_time_t (&wrtime) != st.st_mtime;

  h = NULL;

  DEBUG_WINCE(("to_time_t : %ld - st_mtime : %ld\n", 
                      to_time_t (&wrtime), (long)st.st_mtime));

  /* We won't have a socket when we are uploading the stub. 
     In this case, rely only on the time stamps.  */
  if (!need_upload && s != -1)
  {
    int n;
    unsigned char buf[4096];
    unsigned char chksum = 0;
    unsigned char rem_checkum = 0;

    /* checksum compare, to be really sure we don't need to upload */
    lseek (fd, 0, SEEK_SET);

    while ((n = read (fd, (char*)buf, sizeof (buf))) > 0)
    {
      int i = 0;
      for (i = 0 ; i< n; i++)
        chksum^=buf[i];
    }

    BOOL checksum_ok = REMOTE_FileChecksum (towide(remotefile, NULL), &rem_checkum);
    if (!checksum_ok || rem_checkum != chksum)
      need_upload = 1;

    DEBUG_WINCE(("checksum_ok = %d, checksum = %d, remote checksum = %d\n", 
      checksum_ok, chksum, rem_checkum));
  }

  if (need_upload)
    {
      DWORD nbytes;
      char buf[4096];
      int n;
      off_t size;
      size_t r = 0;
      int dots_written = 0;

      h = CeCreateFile (wstr, GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

      /* Some kind of problem?  */
      err = CeGetLastError ();
      if (h == NULL || h == INVALID_HANDLE_VALUE)
        error (_("error opening file\"%s\" for writing. Windows error %d. (%s)"),
        remotefile, (int)err, werror (err));

      /* Upload the file.  */
	  printf_unfiltered ("Uploading to remote device: %s\n", remotefile);

      size = lseek (fd, 0, SEEK_END);
      lseek (fd, 0, SEEK_SET);

      while ((n = read (fd, buf, sizeof (buf))) > 0)
      {
        const int tot_dots = 20;
        size_t dot;

        if (!CeWriteFile (h, buf, (DWORD) n, &nbytes, NULL))
        {
          DWORD err = CeGetLastError ();
          error (_("error writing to remote device - %d. (%s)"),
          (int)err, werror (err));
        }
        r+=n;
        dot = (size_t) ( ( (float)r / size ) * tot_dots);
        for (; dots_written < dot; ++dots_written)
        {
          printf_unfiltered (".");
          gdb_flush (gdb_stdout);
        }
      }
      printf_unfiltered ("\n");

      time_t_to_filetime(st.st_ctime, &crtime);
      time_t_to_filetime(st.st_atime, &actime);
      time_t_to_filetime(st.st_mtime, &wrtime);

      CeSetFileTime(h, &crtime, &actime, &wrtime);

      if (!CeCloseHandle (h)) {
        DWORD err = CeGetLastError ();
        error (_("error closing remote file - %d. (%s)"), (int)err, werror (err));
      }

    }

  close (fd);

  return remotefile;
}

/* Initialize the connection to the remote device.  */
void
wince_initialize (void)
{
  int tmp;
  char args[256];
  char *hostname;
  struct sockaddr_in sin;
  char *stub_file_name;
  int s0;
  PROCESS_INFORMATION pi;

  if (upload_when != UPLOAD_NEVER) {
  if (!connection_initialized)
    switch (CeRapiInit ())
      {
      case 0:
	connection_initialized = 1;
	break;
      default:
	CeRapiUninit ();
	error (_("Can't initialize connection to remote device."));
	break;
      }
  }

  /* Upload the stub to the handheld device.  */
  stub_file_name = upload_to_device ("wince-stub.exe", WINCE_STUB);
  strcpy (args, stub_file_name);

  if (remote_add_host)
    {
      strcat (args, " ");
      hostname = strchr (args, '\0');
      if (gethostname (hostname, sizeof (args) - strlen (args)))
	error (_("couldn't get hostname of this system."));
    }

  /* Get a socket.  */
  if ((s0 = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    stub_error ("Couldn't connect to host system.");

  /* Allow rapid reuse of the port.  */
  tmp = 1;
  (void) setsockopt (s0, SOL_SOCKET, SO_REUSEADDR, 
		     (char *) &tmp, sizeof (tmp));
  tmp = 1;
  (void) setsockopt (s, IPPROTO_TCP, TCP_NODELAY, 
		     (char *) &tmp, sizeof (tmp));

  /* Set up the information for connecting to the host gdb process.  */
  memset (&sin, 0, sizeof (sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons (7000);	/* FIXME: This should be configurable.  */

  if (bind (s0, (struct sockaddr *) &sin, sizeof (sin)))
    error (_("couldn't bind socket"));

  if (listen (s0, 1))
    error (_("Couldn't open socket for listening."));

  /* Start up the stub on the remote device.  */
printf_unfiltered("Starting on device: \"%s %s\"\n", stub_file_name, args);
  if (upload_when != UPLOAD_NEVER) {
    /* If we don't upload, don't bother to start via RAPI either */
  if (!CeCreateProcess (towide (stub_file_name, NULL), 
			NULL,
			NULL, NULL, 0, 0, 
			NULL, NULL, NULL, &pi))
    {
      DWORD err = CeGetLastError ();
      error (_("Unable to start remote stub '%s'.  Windows CE error %d. (%s)"),
	     stub_file_name, (int)err, werror (err));
    }
  } else {
    printf_unfiltered("Please start stub with correct argument on the device\n");
  }

  /* Wait for a connection */

printf_unfiltered("Waiting for connection...\n");
  if ((s = accept (s0, NULL, NULL)) < 0)
    error (_("couldn't set up server for connection."));
  else
	printf_unfiltered("Connected\n");

  close (s0);
}

struct _my_wce_error_message_t {
	DWORD	e;
	char	*msg;
};
static const struct _my_wce_error_message_t _my_wce_error_messages[];

static const char* werror (DWORD err)
{
  static char msgbuf[1024];

#ifdef	__CYGWIN__
  FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    err,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    msgbuf,
    sizeof (msgbuf), 
    NULL);
#else
  int i;
  for (i=0; _my_wce_error_messages[i].msg; i++)
	  if (_my_wce_error_messages[i].e == err) {
		  sprintf(msgbuf, "%s", _my_wce_error_messages[i].msg);
		  return msgbuf;
	  }
  sprintf(msgbuf, "FormatMessageA-Clone: unknown error %d\n", (int)err);
#endif
  return msgbuf;
}

void win32_stop_stub_comm()
{
  stop_stub ();
  if (connection_initialized)
    CeRapiUninit ();
  connection_initialized = 0;
}

/* Explicitly upload file to remotedir */

void
win32_load (char *file, int from_tty)
{
  if (s < 0)
    wince_initialize ();
  upload_to_device (file, file);
}

/* Handle 'set remoteupload' parameter.  */

#define replace_upload(what) \
  do { \
    upload_when = what; \
    remote_upload = xrealloc (remote_upload, \
			      strlen (upload_options[upload_when].name) + 1); \
    strcpy (remote_upload, upload_options[upload_when].name); \
  } while (0)

static void
set_upload_type (char *ignore, int from_tty, struct cmd_list_element *c)
{
  int i, len;
  char *bad_option;

  if (!remote_upload || !remote_upload[0])
    {
      replace_upload (UPLOAD_NEWER);
      if (from_tty)
	printf_unfiltered ("Upload upload_options are: always, newer, never.\n");
      return;
    }

  len = strlen (remote_upload);
  for (i = 0; 
       i < (sizeof (upload_options) / sizeof (upload_options[0])); 
       i++)
    if (len >= upload_options[i].abbrev &&
	strncasecmp (remote_upload, upload_options[i].name, len) == 0)
      {
	replace_upload (i);
	return;
      }

  bad_option = remote_upload;
  replace_upload (UPLOAD_NEWER);
  error (_("Unknown upload type: %s."), bad_option);
}

#if 0
/* List currently loaded DLLs. */
void
info_dll_command (char *ignore, int from_tty)
{
  struct so_list *so = &solib_start;

  if (!so->next)
    return;

  printf_filtered ("%*s  Load Address\n", -max_dll_name_len, "DLL Name");
  while ((so = so->next) != NULL)
    printf_filtered ("%*s  %08lx\n", -max_dll_name_len, so->so_name, so->);

  return;
}
#endif

void
_initialize_wince (void)
{
  struct cmd_list_element *c;

  add_setshow_string_noescape_cmd ("remotedirectory", no_class,
				   &remote_directory, _("\
Set directory for remote upload."), _("\
Show directory for remote upload."), 
				   NULL, /* FIXME: i18n: */
				   NULL, NULL,
				   &setlist, &showlist);
  remote_directory = xstrdup (remote_directory);

  add_setshow_string_noescape_cmd ("remoteupload", no_class,
				   &remote_upload, _("\
Set how to upload executables to remote device."), _("\
Show how to upload executables to remote device."),
				   NULL, /* FIXME: i18n: */
				   set_upload_type, NULL,
				   &setlist, &showlist);
  set_upload_type (NULL, 0, 0);

  add_setshow_boolean_cmd ("remoteaddhost", class_support,
			   &remote_add_host, _("\
Set whether to add this host to remote stub arguments for\n\
debugging over a network."), _("\
Show whether to add this host to remote stub arguments for\n\
debugging over a network."), NULL,
			   NULL,
			   NULL, /* FIXME: i18n: */
			   &setlist, &showlist);

  add_setshow_boolean_cmd ("debugcommunication", class_support,
          &debug_communication, _("\
Set whether to display stub communication debugging."), _("\
Show whether to display stub communication debugging."), NULL,
          NULL,
          NULL, /* FIXME: i18n: */
          &setlist, &showlist);

  add_setshow_boolean_cmd ("debugwince", class_support,
        &debug_wince, _("\
Set whether to display Windows CE support specific debugging."), _("\
Show whether to display Windows CE support specific debugging."), NULL,
          NULL,
          NULL, /* FIXME: i18n: */
          &setlist, &showlist);
}


#ifndef	__CYGWIN__
/*
 * This is a list of Windows CE error values and their explanations, obtained from
 *
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcecoreos5/html/wce50grfErrorValues.asp
 *
 * we're using this to make our own version of FormatMessageA print sensible messages.
 */
static const struct _my_wce_error_message_t _my_wce_error_messages[] = {
{ ERROR_SUCCESS, "The operation completed successfully. " /* 0  */ },
{ ERROR_INVALID_FUNCTION, "Incorrect function. " /* 1  */ },
{ ERROR_FILE_NOT_FOUND, "The system cannot find the file specified. " /* 2  */ },
{ ERROR_PATH_NOT_FOUND, "The system cannot find the path specified. " /* 3  */ },
{ ERROR_TOO_MANY_OPEN_FILES, "The system cannot open the file. " /* 4  */ },
{ ERROR_ACCESS_DENIED, "Access is denied. " /* 5  */ },
{ ERROR_INVALID_HANDLE, "The handle is invalid. " /* 6  */ },
{ ERROR_ARENA_TRASHED, "The storage control blocks were destroyed. " /* 7  */ },
{ ERROR_NOT_ENOUGH_MEMORY, "Not enough storage is available to process this command. " /* 8  */ },
{ ERROR_INVALID_BLOCK, "The storage control block address is invalid. " /* 9  */ },
{ ERROR_BAD_ENVIRONMENT, "The environment is incorrect. " /* 10  */ },
{ ERROR_BAD_FORMAT, "An attempt was made to load a program with an incorrect format. " /* 11  */ },
{ ERROR_INVALID_ACCESS, "The access code is invalid. " /* 12  */ },
{ ERROR_INVALID_DATA, "The data is invalid. " /* 13  */ },
{ ERROR_OUTOFMEMORY, "Not enough storage is available to complete this operation. " /* 14  */ },
{ ERROR_INVALID_DRIVE, "The system cannot find the drive specified. " /* 15  */ },
{ ERROR_CURRENT_DIRECTORY, "The directory cannot be removed. " /* 16  */ },
{ ERROR_NOT_SAME_DEVICE, "The system cannot move the file to a different disk drive. " /* 17  */ },
{ ERROR_NO_MORE_FILES, "There are no more files. " /* 18  */ },
{ ERROR_WRITE_PROTECT, "The media is write protected. " /* 19  */ },
{ ERROR_BAD_UNIT, "The system cannot find the specified device. " /* 20  */ },
{ ERROR_NOT_READY, "The device is not ready. " /* 21  */ },
{ ERROR_BAD_COMMAND, "The device does not recognize the command. " /* 22  */ },
{ ERROR_CRC, "Data error (cyclic redundancy check). " /* 23  */ },
{ ERROR_BAD_LENGTH, "The program issued a command but the command length is incorrect. " /* 24  */ },
{ ERROR_SEEK, "The drive cannot locate a specific area or track on the disk. " /* 25  */ },
{ ERROR_NOT_DOS_DISK, "The specified disk or diskette cannot be accessed. " /* 26  */ },
{ ERROR_SECTOR_NOT_FOUND, "The drive cannot find the sector requested. " /* 27  */ },
{ ERROR_OUT_OF_PAPER, "The printer is out of paper. " /* 28  */ },
{ ERROR_WRITE_FAULT, "The system cannot write to the specified device. " /* 29  */ },
{ ERROR_READ_FAULT, "The system cannot read from the specified device. " /* 30  */ },
{ ERROR_GEN_FAILURE, "A device attached to the system is not functioning. " /* 31  */ },
{ ERROR_SHARING_VIOLATION, "The process cannot access the file because it is being used by another process. " /* 32  */ },
{ ERROR_LOCK_VIOLATION, "The process cannot access the file because another process has locked a portion of the file. " /* 33  */ },
{ ERROR_WRONG_DISK, "The wrong diskette is in the drive. Insert %2 (Volume Serial Number: %3) into drive %1. " /* 34  */ },
{ ERROR_SHARING_BUFFER_EXCEEDED, "Too many files opened for sharing. " /* 36  */ },
{ ERROR_HANDLE_EOF, "Reached the end of the file. " /* 38  */ },
{ ERROR_HANDLE_DISK_FULL, "The disk is full. " /* 39  */ },
{ ERROR_NOT_SUPPORTED, "The network request is not supported. " /* 50  */ },
{ ERROR_REM_NOT_LIST, "The remote computer is not available. " /* 51  */ },
{ ERROR_DUP_NAME, "A duplicate name exists on the network. " /* 52  */ },
{ ERROR_BAD_NETPATH, "The network path was not found. " /* 53  */ },
{ ERROR_NETWORK_BUSY, "The network is busy. " /* 54  */ },
{ ERROR_DEV_NOT_EXIST, "The specified network resource or device is no longer available. " /* 55  */ },
{ ERROR_TOO_MANY_CMDS, "The network BIOS command limit has been reached. " /* 56  */ },
{ ERROR_ADAP_HDW_ERR, "A network adapter hardware error occurred. " /* 57  */ },
{ ERROR_BAD_NET_RESP, "The specified server cannot perform the requested operation. " /* 58  */ },
{ ERROR_UNEXP_NET_ERR, "An unexpected network error occurred. " /* 59  */ },
{ ERROR_BAD_REM_ADAP, "The remote adapter is not compatible. " /* 60  */ },
{ ERROR_PRINTQ_FULL, "The printer queue is full. " /* 61  */ },
{ ERROR_NO_SPOOL_SPACE, "Space to store the file waiting to be printed is not available on the server. " /* 62  */ },
{ ERROR_PRINT_CANCELLED, "Your file waiting to be printed was deleted. " /* 63  */ },
{ ERROR_NETNAME_DELETED, "The specified network name is no longer available. " /* 64  */ },
{ ERROR_NETWORK_ACCESS_DENIED, "Network access is denied. " /* 65  */ },
{ ERROR_BAD_DEV_TYPE, "The network resource type is not correct. " /* 66  */ },
{ ERROR_BAD_NET_NAME, "The network name cannot be found. " /* 67  */ },
{ ERROR_TOO_MANY_NAMES, "The name limit for the local computer network adapter card was exceeded. " /* 68  */ },
{ ERROR_TOO_MANY_SESS, "The network BIOS session limit was exceeded. " /* 69  */ },
{ ERROR_SHARING_PAUSED, "The remote server has been paused or is in the process of being started. " /* 70  */ },
{ ERROR_REQ_NOT_ACCEP, "No more connections can be made to this remote computer at this time because there are already as many connections as the computer can accept. " /* 71  */ },
{ ERROR_REDIR_PAUSED, "The specified printer or disk device has been paused. " /* 72  */ },
{ ERROR_FILE_EXISTS, "The file exists. " /* 80  */ },
{ ERROR_CANNOT_MAKE, "The directory or file cannot be created. " /* 82  */ },
{ ERROR_FAIL_I24, "Fail on interrupt 24 handler. " /* 83  */ },
{ ERROR_OUT_OF_STRUCTURES, "Storage to process this request is not available. " /* 84  */ },
{ ERROR_ALREADY_ASSIGNED, "The local device name is already in use. " /* 85  */ },
{ ERROR_INVALID_PASSWORD, "The specified network password is not correct. " /* 86  */ },
{ ERROR_INVALID_PARAMETER, "The parameter is incorrect. " /* 87  */ },
{ ERROR_NET_WRITE_FAULT, "A write fault occurred on the network. " /* 88  */ },
{ ERROR_NO_PROC_SLOTS, "The system cannot start another process at this time. " /* 89  */ },
{ ERROR_TOO_MANY_SEMAPHORES, "Cannot create another system semaphore. " /* 100  */ },
{ ERROR_EXCL_SEM_ALREADY_OWNED, "The exclusive semaphore is owned by another process. " /* 101  */ },
{ ERROR_SEM_IS_SET, "The semaphore is set and cannot be closed. " /* 102  */ },
{ ERROR_TOO_MANY_SEM_REQUESTS, "The semaphore cannot be set again. " /* 103  */ },
{ ERROR_INVALID_AT_INTERRUPT_TIME, "Cannot request exclusive semaphores at interrupt time. " /* 104  */ },
{ ERROR_SEM_OWNER_DIED, "The previous ownership of this semaphore has ended. " /* 105  */ },
{ ERROR_SEM_USER_LIMIT, "Insert the diskette for drive %1. " /* 106  */ },
{ ERROR_DISK_CHANGE, "The program stopped because an alternate diskette was not inserted. " /* 107  */ },
{ ERROR_DRIVE_LOCKED, "The disk is in use or locked by another process. " /* 108  */ },
{ ERROR_BROKEN_PIPE, "The pipe has been ended. " /* 109  */ },
{ ERROR_OPEN_FAILED, "The system cannot open the device or file specified. " /* 110  */ },
{ ERROR_BUFFER_OVERFLOW, "The file name is too long. " /* 111  */ },
{ ERROR_DISK_FULL, "There is not enough space on the disk. " /* 112  */ },
{ ERROR_NO_MORE_SEARCH_HANDLES, "No more internal file identifiers available. " /* 113  */ },
{ ERROR_INVALID_TARGET_HANDLE, "The target internal file identifier is incorrect. " /* 114  */ },
{ ERROR_INVALID_CATEGORY, "The IOCTL call made by the application program is not correct. " /* 117  */ },
{ ERROR_INVALID_VERIFY_SWITCH, "The verify-on-write switch parameter value is not correct. " /* 118  */ },
{ ERROR_BAD_DRIVER_LEVEL, "The system does not support the command requested. " /* 119  */ },
{ ERROR_CALL_NOT_IMPLEMENTED, "This function is not valid on this platform. " /* 120  */ },
{ ERROR_SEM_TIMEOUT, "The semaphore time-out period has expired. " /* 121  */ },
{ ERROR_INSUFFICIENT_BUFFER, "The data area passed to a system call is too small. " /* 122  */ },
{ ERROR_INVALID_NAME, "The file name, directory name, or volume label syntax is incorrect. " /* 123  */ },
{ ERROR_INVALID_LEVEL, "The system call level is not correct. " /* 124  */ },
{ ERROR_NO_VOLUME_LABEL, "The disk has no volume label. " /* 125  */ },
{ ERROR_MOD_NOT_FOUND, "The specified module could not be found. " /* 126  */ },
{ ERROR_PROC_NOT_FOUND, "The specified procedure could not be found. " /* 127  */ },
{ ERROR_WAIT_NO_CHILDREN, "There are no child processes to wait for. " /* 128  */ },
{ ERROR_CHILD_NOT_COMPLETE, "The %1 application cannot be run in Windows NT mode. " /* 129  */ },
{ ERROR_DIRECT_ACCESS_HANDLE, "Attempt to use a file handle to an open disk partition for an operation other than raw disk I/O. " /* 130  */ },
{ ERROR_NEGATIVE_SEEK, "An attempt was made to move the file pointer before the beginning of the file. " /* 131  */ },
{ ERROR_SEEK_ON_DEVICE, "The file pointer cannot be set on the specified device or file. " /* 132  */ },
{ ERROR_IS_JOIN_TARGET, "A JOIN or SUBST command cannot be used for a drive that contains previously joined drives. " /* 133  */ },
{ ERROR_IS_JOINED, "An attempt was made to use a JOIN or SUBST command on a drive that has already been joined. " /* 134  */ },
{ ERROR_IS_SUBSTED, "An attempt was made to use a JOIN or SUBST command on a drive that has already been substituted. " /* 135  */ },
{ ERROR_NOT_JOINED, "The system tried to delete the JOIN of a drive that is not joined. " /* 136  */ },
{ ERROR_NOT_SUBSTED, "The system tried to delete the substitution of a drive that is not substituted. " /* 137  */ },
{ ERROR_JOIN_TO_JOIN, "The system tried to join a drive to a directory on a joined drive. " /* 138  */ },
{ ERROR_SUBST_TO_SUBST, "The system tried to substitute a drive to a directory on a substituted drive. " /* 139  */ },
{ ERROR_JOIN_TO_SUBST, "The system tried to join a drive to a directory on a substituted drive. " /* 140  */ },
{ ERROR_SUBST_TO_JOIN, "The system tried to SUBST a drive to a directory on a joined drive. " /* 141  */ },
{ ERROR_BUSY_DRIVE, "The system cannot perform a JOIN or SUBST at this time. " /* 142  */ },
{ ERROR_SAME_DRIVE, "The system cannot join or substitute a drive to or for a directory on the same drive. " /* 143  */ },
{ ERROR_DIR_NOT_ROOT, "The directory is not a subdirectory of the root directory. " /* 144  */ },
{ ERROR_DIR_NOT_EMPTY, "The directory is not empty. " /* 145  */ },
{ ERROR_IS_SUBST_PATH, "The path specified is being used in a substitute. " /* 146  */ },
{ ERROR_IS_JOIN_PATH, "Not enough resources are available to process this command. " /* 147  */ },
{ ERROR_PATH_BUSY, "The path specified cannot be used at this time. " /* 148  */ },
{ ERROR_IS_SUBST_TARGET, "An attempt was made to join or substitute a drive for which a directory on the drive is the target of a previous substitute. " /* 149  */ },
{ ERROR_SYSTEM_TRACE, "System trace information was not specified in your Config.sys file, or tracing is disallowed. " /* 150  */ },
{ ERROR_INVALID_EVENT_COUNT, "The number of specified semaphore events for DosMuxSemWait is not correct. " /* 151  */ },
{ ERROR_TOO_MANY_MUXWAITERS, "DosMuxSemWait did not execute; too many semaphores are already set. " /* 152  */ },
{ ERROR_INVALID_LIST_FORMAT, "The DosMuxSemWait list is not correct. " /* 153  */ },
{ ERROR_LABEL_TOO_LONG, "The volume label you entered exceeds the label character limit of the target file system. " /* 154  */ },
{ ERROR_TOO_MANY_TCBS, "Cannot create another thread. " /* 155  */ },
{ ERROR_SIGNAL_REFUSED, "The recipient process has refused the signal. " /* 156  */ },
{ ERROR_DISCARDED, "The segment is already discarded and cannot be locked. " /* 157  */ },
{ ERROR_NOT_LOCKED, "The segment is already unlocked. " /* 158  */ },
{ ERROR_BAD_THREADID_ADDR, "The address for the thread identifier is not correct. " /* 159  */ },
{ ERROR_BAD_ARGUMENTS, "The argument string passed to DosExecPgm is not correct. " /* 160  */ },
{ ERROR_BAD_PATHNAME, "The specified path is invalid. " /* 161  */ },
{ ERROR_SIGNAL_PENDING, "A signal is already pending. " /* 162  */ },
{ ERROR_MAX_THRDS_REACHED, "No more threads can be created in the system. " /* 164  */ },
{ ERROR_LOCK_FAILED, "Unable to lock a region of a file. " /* 167  */ },
{ ERROR_BUSY, "The requested resource is in use. " /* 170  */ },
{ ERROR_CANCEL_VIOLATION, "A lock request was not outstanding for the supplied cancel region. " /* 173  */ },
{ ERROR_ATOMIC_LOCKS_NOT_SUPPORTED, "The file system does not support atomic changes to the lock type. " /* 174  */ },
{ ERROR_INVALID_SEGMENT_NUMBER, "The system detected a segment number that was not correct. " /* 180  */ },
{ ERROR_INVALID_ORDINAL, "The operating system cannot run %1. " /* 182  */ },
{ ERROR_ALREADY_EXISTS, "Cannot create a file when that file already exists. " /* 183  */ },
{ ERROR_INVALID_FLAG_NUMBER, "The flag passed is not correct. " /* 186  */ },
{ ERROR_SEM_NOT_FOUND, "The specified system semaphore name was not found. " /* 187  */ },
{ ERROR_INVALID_STARTING_CODESEG, "The operating system cannot run %1. " /* 188  */ },
{ ERROR_INVALID_STACKSEG, "The operating system cannot run %1. " /* 189  */ },
{ ERROR_INVALID_MODULETYPE, "The operating system cannot run %1. " /* 190  */ },
{ ERROR_INVALID_EXE_SIGNATURE, "Cannot run %1 in Windows NT mode. " /* 191  */ },
{ ERROR_EXE_MARKED_INVALID, "The operating system cannot run %1. " /* 192  */ },
{ ERROR_BAD_EXE_FORMAT, "Is not a valid application. " /* 193  */ },
{ ERROR_ITERATED_DATA_EXCEEDS_64k, "The operating system cannot run %1. " /* 194  */ },
{ ERROR_INVALID_MINALLOCSIZE, "The operating system cannot run %1. " /* 195  */ },
{ ERROR_DYNLINK_FROM_INVALID_RING, "The operating system cannot run this application program. " /* 196  */ },
{ ERROR_IOPL_NOT_ENABLED, "The operating system is not presently configured to run this application. " /* 197  */ },
{ ERROR_INVALID_SEGDPL, "The operating system cannot run %1. " /* 198  */ },
{ ERROR_AUTODATASEG_EXCEEDS_64k, "The operating system cannot run this application program. " /* 199  */ },
{ ERROR_RING2SEG_MUST_BE_MOVABLE, "The code segment cannot be greater than or equal to 64 KB. " /* 200  */ },
{ ERROR_RELOC_CHAIN_XEEDS_SEGLIM, "The operating system cannot run %1. " /* 201  */ },
{ ERROR_INFLOOP_IN_RELOC_CHAIN, "The operating system cannot run %1. " /* 202  */ },
{ ERROR_ENVVAR_NOT_FOUND, "The system could not find the environment option that was entered. " /* 203  */ },
{ ERROR_NO_SIGNAL_SENT, "No process in the command subtree has a signal handler. " /* 205  */ },
{ ERROR_FILENAME_EXCED_RANGE, "The file name or extension is too long. " /* 206  */ },
{ ERROR_RING2_STACK_IN_USE, "The ring 2 stack is in use. " /* 207  */ },
{ ERROR_META_EXPANSION_TOO_LONG, "The global file name characters, '*' or '?,' are entered incorrectly or too many global file name characters are specified. " /* 208  */ },
{ ERROR_INVALID_SIGNAL_NUMBER, "The signal being posted is not correct. " /* 209  */ },
{ ERROR_THREAD_1_INACTIVE, "The signal handler cannot be set. " /* 210  */ },
{ ERROR_LOCKED, "The segment is locked and cannot be reallocated. " /* 212  */ },
{ ERROR_TOO_MANY_MODULES, "Too many dynamic-link modules are attached to this program or dynamic-link module. " /* 214  */ },
{ ERROR_NESTING_NOT_ALLOWED, "Cannot nest calls to the LoadModule function. " /* 215  */ },
{ ERROR_EXE_MACHINE_TYPE_MISMATCH, "The image file %1 is valid, but is for a machine type other than the current machine. " /* 216  */ },
{ ERROR_BAD_PIPE, "The pipe state is invalid. " /* 230  */ },
{ ERROR_PIPE_BUSY, "All pipe instances are busy. " /* 231  */ },
{ ERROR_NO_DATA, "The pipe is being closed. " /* 232  */ },
{ ERROR_PIPE_NOT_CONNECTED, "No process is on the other end of the pipe. " /* 233  */ },
{ ERROR_MORE_DATA, "More data is available. " /* 234  */ },
{ ERROR_VC_DISCONNECTED, "The session was canceled. " /* 240  */ },
{ ERROR_INVALID_EA_NAME, "The specified extended attribute name was invalid. " /* 254  */ },
{ ERROR_EA_LIST_INCONSISTENT, "The extended attributes are inconsistent. " /* 255  */ },
{ ERROR_NO_MORE_ITEMS, "No more data is available. " /* 259  */ },
{ ERROR_CANNOT_COPY, "The copy functions cannot be used. " /* 266  */ },
{ ERROR_DIRECTORY, "The directory name is invalid. " /* 267  */ },
{ ERROR_EAS_DIDNT_FIT, "The extended attributes did not fit in the buffer. " /* 275  */ },
{ ERROR_EA_FILE_CORRUPT, "The extended attribute file on the mounted file system is corrupt. " /* 276  */ },
{ ERROR_EA_TABLE_FULL, "The extended attribute table file is full. " /* 277  */ },
{ ERROR_INVALID_EA_HANDLE, "The specified extended attribute handle is invalid. " /* 278  */ },
{ ERROR_EAS_NOT_SUPPORTED, "The mounted file system does not support extended attributes. " /* 282  */ },
{ ERROR_NOT_OWNER, "Attempt to release mutex not owned by caller. " /* 288  */ },
{ ERROR_TOO_MANY_POSTS, "Too many posts were made to a semaphore. " /* 298  */ },
{ ERROR_PARTIAL_COPY, "Only part of a ReadProcessMemory or WriteProcessMemory request was completed. " /* 299  */ },
{ ERROR_MR_MID_NOT_FOUND, "The system cannot find message text for message number 0x%1 in the message file for %2. " /* 317  */ },
{ ERROR_INVALID_ADDRESS, "Attempt to access invalid address. " /* 487  */ },
{ ERROR_ARITHMETIC_OVERFLOW, "Arithmetic result exceeded 32 bits. " /* 534  */ },
{ ERROR_PIPE_CONNECTED, "There is a process on other end of the pipe. " /* 535  */ },
{ ERROR_PIPE_LISTENING, "Waiting for a process to open the other end of the pipe. " /* 536  */ },
{ ERROR_EA_ACCESS_DENIED, "Access to the extended attribute was denied. " /* 994  */ },
{ ERROR_OPERATION_ABORTED, "The I/O operation has been aborted because of either a thread exit or an application request. " /* 995  */ },
{ ERROR_IO_INCOMPLETE, "Overlapped I/O event is not in a signaled state. " /* 996  */ },
{ ERROR_IO_PENDING, "Overlapped I/O operation is in progress. " /* 997  */ },
{ ERROR_NOACCESS, "Invalid access to memory location. " /* 998  */ },
{ ERROR_SWAPERROR, "Error performing inpage operation. " /* 999  */ },
{ ERROR_STACK_OVERFLOW, "Recursion too deep; the stack overflowed. " /* 1001  */ },
{ ERROR_INVALID_MESSAGE, "The window cannot act on the sent message. " /* 1002  */ },
{ ERROR_CAN_NOT_COMPLETE, "Cannot complete this function. " /* 1003  */ },
{ ERROR_INVALID_FLAGS, "Invalid flags. " /* 1004  */ },
{ ERROR_UNRECOGNIZED_VOLUME, "The volume does not contain a recognized file system. Verify that all required file system drivers are loaded and that the volume is not corrupted. " /* 1005  */ },
{ ERROR_FILE_INVALID, "The volume for a file has been externally altered so that the opened file is no longer valid. " /* 1006  */ },
{ ERROR_FULLSCREEN_MODE, "The requested operation cannot be performed in full-screen mode. " /* 1007  */ },
{ ERROR_NO_TOKEN, "An attempt was made to reference a token that does not exist. " /* 1008  */ },
{ ERROR_BADDB, "The configuration registry database is corrupt. " /* 1009  */ },
{ ERROR_BADKEY, "The configuration registry key is invalid. " /* 1010  */ },
{ ERROR_CANTOPEN, "The configuration registry key could not be opened. " /* 1011  */ },
{ ERROR_CANTREAD, "The configuration registry key could not be read. " /* 1012  */ },
{ ERROR_CANTWRITE, "The configuration registry key could not be written. " /* 1013  */ },
{ ERROR_REGISTRY_RECOVERED, "One of the files in the registry database had to be recovered by use of a log or alternate copy. The recovery was successful. " /* 1014  */ },
{ ERROR_REGISTRY_CORRUPT, "The registry is corrupted. The structure of one of the files that contains registry data is corrupted, or the system's image of the file in memory is corrupted, or the file could not be recovered because the alternate copy or log was absent or corrupted. " /* 1015  */ },
{ ERROR_REGISTRY_IO_FAILED, "An I/O operation initiated by the registry failed unrecoverably. The registry could not read in, or write out, or flush, one of the files that contain the system's image of the registry. " /* 1016  */ },
{ ERROR_NOT_REGISTRY_FILE, "The system has attempted to load or restore a file into the registry, but the specified file is not in a registry file format. " /* 1017  */ },
{ ERROR_KEY_DELETED, "Illegal operation attempted on a registry key that has been marked for deletion. " /* 1018  */ },
{ ERROR_NO_LOG_SPACE, "System could not allocate the required space in a registry log. " /* 1019  */ },
{ ERROR_KEY_HAS_CHILDREN, "Cannot create a symbolic link in a registry key that already has subkeys or values. " /* 1020  */ },
{ ERROR_CHILD_MUST_BE_VOLATILE, "Cannot create a stable subkey under a volatile parent key. " /* 1021  */ },
{ ERROR_NOTIFY_ENUM_DIR, "A notify change request is being completed and the information is not being returned in the caller's buffer. The caller now needs to enumerate the files to find the changes. " /* 1022  */ },
{ ERROR_DEPENDENT_SERVICES_RUNNING, "A stop control has been sent to a service that other running services are dependent on. " /* 1051  */ },
{ ERROR_INVALID_SERVICE_CONTROL, "The requested control is not valid for this service. " /* 1052  */ },
{ ERROR_SERVICE_REQUEST_TIMEOUT, "The service did not respond to the start or control request in a timely fashion. " /* 1053  */ },
{ ERROR_SERVICE_NO_THREAD, "A thread could not be created for the service. " /* 1054  */ },
{ ERROR_SERVICE_DATABASE_LOCKED, "The service database is locked. " /* 1055  */ },
{ ERROR_SERVICE_ALREADY_RUNNING, "An instance of the service is already running. " /* 1056  */ },
{ ERROR_INVALID_SERVICE_ACCOUNT, "The account name is invalid or does not exist. " /* 1057  */ },
{ ERROR_SERVICE_DISABLED, "The specified service is disabled and cannot be started. " /* 1058  */ },
{ ERROR_CIRCULAR_DEPENDENCY, "Circular service dependency was specified. " /* 1059  */ },
{ ERROR_SERVICE_DOES_NOT_EXIST, "The specified service does not exist as an installed service. " /* 1060  */ },
{ ERROR_SERVICE_CANNOT_ACCEPT_CTRL, "The service cannot accept control messages at this time. " /* 1061  */ },
{ ERROR_SERVICE_NOT_ACTIVE, "The service has not been started. " /* 1062  */ },
{ ERROR_FAILED_SERVICE_CONTROLLER_CONNECT, "The service process could not connect to the service controller. " /* 1063  */ },
{ ERROR_EXCEPTION_IN_SERVICE, "An exception occurred in the service when handling the control request. " /* 1064  */ },
{ ERROR_DATABASE_DOES_NOT_EXIST, "The database specified does not exist. " /* 1065  */ },
{ ERROR_SERVICE_SPECIFIC_ERROR, "The service has returned a service-specific error code. " /* 1066  */ },
{ ERROR_PROCESS_ABORTED, "The process terminated unexpectedly. " /* 1067  */ },
{ ERROR_SERVICE_DEPENDENCY_FAIL, "The dependency service or group failed to start. " /* 1068  */ },
{ ERROR_SERVICE_LOGON_FAILED, "The service did not start due to a logon failure. " /* 1069  */ },
{ ERROR_SERVICE_START_HANG, "After starting, the service stopped responding (hung) in a start-pending state. " /* 1070  */ },
{ ERROR_INVALID_SERVICE_LOCK, "The specified service database lock is invalid. " /* 1071  */ },
{ ERROR_SERVICE_MARKED_FOR_DELETE, "The specified service has been marked for deletion. " /* 1072  */ },
{ ERROR_SERVICE_EXISTS, "The specified service already exists. " /* 1073  */ },
{ ERROR_ALREADY_RUNNING_LKG, "The system is currently running with the last-known-good configuration. " /* 1074  */ },
{ ERROR_SERVICE_DEPENDENCY_DELETED, "The dependency service does not exist or has been marked for deletion. " /* 1075  */ },
{ ERROR_BOOT_ALREADY_ACCEPTED, "The current boot has already been accepted for use as the last-known-good control set. " /* 1076  */ },
{ ERROR_SERVICE_NEVER_STARTED, "No attempts to start the service have been made since the last boot. " /* 1077  */ },
{ ERROR_DUPLICATE_SERVICE_NAME, "The name is already in use as either a service name or a service display name. " /* 1078  */ },
{ ERROR_DIFFERENT_SERVICE_ACCOUNT, "The account specified for this service is different from the account specified for other services running in the same process. " /* 1079  */ },
{ ERROR_END_OF_MEDIA, "The physical end of the tape has been reached. " /* 1100  */ },
{ ERROR_FILEMARK_DETECTED, "A tape access reached a filemark. " /* 1101  */ },
{ ERROR_BEGINNING_OF_MEDIA, "The beginning of the tape or partition was encountered. " /* 1102  */ },
{ ERROR_SETMARK_DETECTED, "A tape access reached the end of a set of files. " /* 1103  */ },
{ ERROR_NO_DATA_DETECTED, "No more data is on the tape. " /* 1104  */ },
{ ERROR_PARTITION_FAILURE, "Tape could not be partitioned. " /* 1105  */ },
{ ERROR_INVALID_BLOCK_LENGTH, "When accessing a new tape of a multivolume partition, the current block size is incorrect. " /* 1106  */ },
{ ERROR_DEVICE_NOT_PARTITIONED, "Tape partition information could not be found when loading a tape. " /* 1107  */ },
{ ERROR_UNABLE_TO_LOCK_MEDIA, "Unable to lock the media eject mechanism. " /* 1108  */ },
{ ERROR_UNABLE_TO_UNLOAD_MEDIA, "Unable to unload the media. " /* 1109  */ },
{ ERROR_MEDIA_CHANGED, "The media in the drive may have changed. " /* 1110  */ },
{ ERROR_BUS_RESET, "The I/O bus was reset. " /* 1111  */ },
{ ERROR_NO_MEDIA_IN_DRIVE, "No media in drive. " /* 1112  */ },
{ ERROR_NO_UNICODE_TRANSLATION, "No mapping for the Unicode character exists in the target multibyte code page. " /* 1113  */ },
{ ERROR_DLL_INIT_FAILED, "A dynamic link library (DLL) initialization routine failed. " /* 1114  */ },
{ ERROR_SHUTDOWN_IN_PROGRESS, "A system shutdown is in progress. " /* 1115  */ },
{ ERROR_NO_SHUTDOWN_IN_PROGRESS, "Unable to abort the system shutdown because no shutdown was in progress. " /* 1116  */ },
{ ERROR_IO_DEVICE, "The request could not be performed because of an I/O device error. " /* 1117  */ },
{ ERROR_SERIAL_NO_DEVICE, "No serial device was successfully initialized. The serial driver will unload. " /* 1118  */ },
{ ERROR_IRQ_BUSY, "Unable to open a device that was sharing an interrupt request (IRQ) with other devices. At least one other device that uses that IRQ was already opened. " /* 1119  */ },
{ ERROR_MORE_WRITES, "A serial I/O operation was completed by another write to the serial port. The IOCTL_SERIAL_XOFF_COUNTER reached zero.) " /* 1120  */ },
{ ERROR_COUNTER_TIMEOUT, "A serial I/O operation completed because the time-out period expired. In other words, the IOCTL_SERIAL_XOFF_COUNTER did not reach zero. " /* 1121  */ },
{ ERROR_FLOPPY_ID_MARK_NOT_FOUND, "No identifier address mark was found on the floppy disk. " /* 1122  */ },
{ ERROR_FLOPPY_WRONG_CYLINDER, "Mismatch between the floppy disk sector identifier field and the floppy disk controller track address. " /* 1123  */ },
{ ERROR_FLOPPY_UNKNOWN_ERROR, "The floppy disk controller reported an error that is not recognized by the floppy disk driver. " /* 1124  */ },
{ ERROR_FLOPPY_BAD_REGISTERS, "The floppy disk controller returned inconsistent results in its registers. " /* 1125  */ },
{ ERROR_DISK_RECALIBRATE_FAILED, "While accessing the hard disk, a recalibrate operation failed, even after retries. " /* 1126  */ },
{ ERROR_DISK_OPERATION_FAILED, "While accessing the hard disk, a disk operation failed even after retries. " /* 1127  */ },
{ ERROR_DISK_RESET_FAILED, "While accessing the hard disk, a disk controller reset was needed, but even that failed. " /* 1128  */ },
{ ERROR_EOM_OVERFLOW, "Physical end of tape encountered. " /* 1129  */ },
{ ERROR_NOT_ENOUGH_SERVER_MEMORY, "Not enough server storage is available to process this command. " /* 1130  */ },
{ ERROR_POSSIBLE_DEADLOCK, "A potential deadlock condition has been detected. " /* 1131  */ },
{ ERROR_MAPPED_ALIGNMENT, "The base address or the file offset specified does not have the proper alignment. " /* 1132  */ },
{ ERROR_SET_POWER_STATE_VETOED, "An attempt to change the system power state was vetoed by another application or driver. " /* 1140  */ },
{ ERROR_SET_POWER_STATE_FAILED, "The basic input/output system (BIOS) failed an attempt to change the system power state. " /* 1141  */ },
{ ERROR_TOO_MANY_LINKS, "An attempt was made to create more links on a file than the file system supports. " /* 1142  */ },
{ ERROR_OLD_WIN_VERSION, "The specified program requires a newer version of Windows. " /* 1150  */ },
{ ERROR_APP_WRONG_OS, "The specified program is not a Windows or MS-DOS program. " /* 1151  */ },
{ ERROR_SINGLE_INSTANCE_APP, "Cannot start more than one instance of the specified program. " /* 1152  */ },
{ ERROR_RMODE_APP, "The specified program was written for an earlier version of Windows. " /* 1153  */ },
{ ERROR_INVALID_DLL, "One of the library files needed to run this application is damaged. " /* 1154  */ },
{ ERROR_NO_ASSOCIATION, "No application is associated with the specified file for this operation. " /* 1155  */ },
{ ERROR_DDE_FAIL, "An error occurred in sending the command to the application. " /* 1156  */ },
{ ERROR_DLL_NOT_FOUND, "One of the library files needed to run this application cannot be found. " /* 1157  */ },
{ ERROR_BAD_DEVICE, "The specified device name is invalid. " /* 1200  */ },
{ ERROR_CONNECTION_UNAVAIL, "The device is not currently connected but it is a remembered connection. " /* 1201  */ },
{ ERROR_DEVICE_ALREADY_REMEMBERED, "An attempt was made to remember a device that had previously been remembered. " /* 1202  */ },
{ ERROR_NO_NET_OR_BAD_PATH, "No network provider accepted the given network path. " /* 1203  */ },
{ ERROR_BAD_PROVIDER, "The specified network provider name is invalid. " /* 1204  */ },
{ ERROR_CANNOT_OPEN_PROFILE, "Unable to open the network connection profile. " /* 1205  */ },
{ ERROR_BAD_PROFILE, "The network connection profile is corrupt. " /* 1206  */ },
{ ERROR_NOT_CONTAINER, "Cannot enumerate a noncontainer. " /* 1207  */ },
{ ERROR_EXTENDED_ERROR, "An extended error has occurred. " /* 1208  */ },
{ ERROR_INVALID_GROUPNAME, "The format of the specified group name is invalid. " /* 1209  */ },
{ ERROR_INVALID_COMPUTERNAME, "The format of the specified computer name is invalid. " /* 1210  */ },
{ ERROR_INVALID_EVENTNAME, "The format of the specified event name is invalid. " /* 1211  */ },
{ ERROR_INVALID_DOMAINNAME, "The format of the specified domain name is invalid. " /* 1212  */ },
{ ERROR_INVALID_SERVICENAME, "The format of the specified service name is invalid. " /* 1213  */ },
{ ERROR_INVALID_NETNAME, "The format of the specified network name is invalid. " /* 1214  */ },
{ ERROR_INVALID_SHARENAME, "The format of the specified share name is invalid. " /* 1215  */ },
{ ERROR_INVALID_PASSWORDNAME, "The format of the specified password is invalid. " /* 1216  */ },
{ ERROR_INVALID_MESSAGENAME, "The format of the specified message name is invalid. " /* 1217  */ },
{ ERROR_INVALID_MESSAGEDEST, "The format of the specified message destination is invalid. " /* 1218  */ },
{ ERROR_SESSION_CREDENTIAL_CONFLICT, "The credentials supplied conflict with an existing set of credentials. " /* 1219  */ },
{ ERROR_REMOTE_SESSION_LIMIT_EXCEEDED, "An attempt was made to establish a session to a network server, but there are already too many sessions established to that server. " /* 1220  */ },
{ ERROR_DUP_DOMAINNAME, "The workgroup or domain name is already in use by another computer on the network. " /* 1221  */ },
{ ERROR_NO_NETWORK, "The network is not present or not started. " /* 1222  */ },
{ ERROR_CANCELLED, "The operation was canceled by the user. " /* 1223  */ },
{ ERROR_USER_MAPPED_FILE, "The requested operation cannot be performed on a file with a user-mapped section open. " /* 1224  */ },
{ ERROR_CONNECTION_REFUSED, "The remote system refused the network connection. " /* 1225  */ },
{ ERROR_GRACEFUL_DISCONNECT, "The network connection was gracefully closed. " /* 1226  */ },
{ ERROR_ADDRESS_ALREADY_ASSOCIATED, "The network transport endpoint already has an address associated with it. " /* 1227  */ },
{ ERROR_ADDRESS_NOT_ASSOCIATED, "An address has not yet been associated with the network endpoint. " /* 1228  */ },
{ ERROR_CONNECTION_INVALID, "An operation was attempted on a nonexistent network connection. " /* 1229  */ },
{ ERROR_CONNECTION_ACTIVE, "An invalid operation was attempted on an active network connection. " /* 1230  */ },
{ ERROR_NETWORK_UNREACHABLE, "The remote network is not reachable by the transport. " /* 1231  */ },
{ ERROR_HOST_UNREACHABLE, "The remote system is not reachable by the transport. " /* 1232  */ },
{ ERROR_PROTOCOL_UNREACHABLE, "The remote system does not support the transport protocol. " /* 1233  */ },
{ ERROR_PORT_UNREACHABLE, "No service is operating at the destination network endpoint on the remote system. " /* 1234  */ },
{ ERROR_REQUEST_ABORTED, "The request was aborted. " /* 1235  */ },
{ ERROR_CONNECTION_ABORTED, "The network connection was aborted by the local system. " /* 1236  */ },
{ ERROR_RETRY, "The operation could not be completed. A retry should be performed. " /* 1237  */ },
{ ERROR_CONNECTION_COUNT_LIMIT, "A connection to the server could not be made because the limit on the number of concurrent connections for this account has been reached. " /* 1238  */ },
{ ERROR_LOGIN_TIME_RESTRICTION, "Attempting to log in during an unauthorized time of day for this account. " /* 1239  */ },
{ ERROR_LOGIN_WKSTA_RESTRICTION, "The account is not authorized to log in from this station. " /* 1240  */ },
{ ERROR_INCORRECT_ADDRESS, "The network address could not be used for the operation requested. " /* 1241  */ },
{ ERROR_ALREADY_REGISTERED, "The service is already registered. " /* 1242  */ },
{ ERROR_SERVICE_NOT_FOUND, "The specified service does not exist. " /* 1243  */ },
{ ERROR_NOT_AUTHENTICATED, "The operation being requested was not performed because the user has not been authenticated. " /* 1244  */ },
{ ERROR_NOT_LOGGED_ON, "The operation being requested was not performed because the user has not logged on to the network. The specified service does not exist. " /* 1245  */ },
{ ERROR_CONTINUE, "Caller to continue with work in progress. " /* 1246  */ },
{ ERROR_ALREADY_INITIALIZED, "An attempt was made to perform an initialization operation when initialization has already been completed. " /* 1247  */ },
{ ERROR_NO_MORE_DEVICES, "No more local devices. " /* 1248  */ },
{ ERROR_NOT_ALL_ASSIGNED, "Not all privileges referenced are assigned to the caller. " /* 1300  */ },
{ ERROR_SOME_NOT_MAPPED, "Some mapping between account names and security IDs was not done. " /* 1301  */ },
{ ERROR_NO_QUOTAS_FOR_ACCOUNT, "No system quota limits are specifically set for this account. " /* 1302  */ },
{ ERROR_LOCAL_USER_SESSION_KEY, "No encryption key is available. A well-known encryption key was returned. " /* 1303  */ },
{ ERROR_NULL_LM_PASSWORD, "The password is too complex to be converted to a LAN Manager password. The LAN Manager password returned is a null string. " /* 1304  */ },
{ ERROR_UNKNOWN_REVISION, "The revision level is unknown. " /* 1305  */ },
{ ERROR_REVISION_MISMATCH, "Indicates two revision levels are incompatible. " /* 1306  */ },
{ ERROR_INVALID_OWNER, "This security identifier may not be assigned as the owner of this object. " /* 1307  */ },
{ ERROR_INVALID_PRIMARY_GROUP, "This security identifier may not be assigned as the primary group of an object. " /* 1308  */ },
{ ERROR_NO_IMPERSONATION_TOKEN, "An attempt has been made to operate on an impersonation token by a thread that is not currently impersonating a client. " /* 1309  */ },
{ ERROR_CANT_DISABLE_MANDATORY, "The group cannot be disabled. " /* 1310  */ },
{ ERROR_NO_LOGON_SERVERS, "There are currently no logon servers available to service the logon request. " /* 1311  */ },
{ ERROR_NO_SUCH_LOGON_SESSION, "A specified logon session does not exist. It may already have been terminated. " /* 1312  */ },
{ ERROR_NO_SUCH_PRIVILEGE, "A specified privilege does not exist. " /* 1313  */ },
{ ERROR_PRIVILEGE_NOT_HELD, "A required privilege is not held by the client. " /* 1314  */ },
{ ERROR_INVALID_ACCOUNT_NAME, "The name provided is not a properly formed account name. " /* 1315  */ },
{ ERROR_USER_EXISTS, "The specified user already exists. " /* 1316  */ },
{ ERROR_NO_SUCH_USER, "The specified user does not exist. " /* 1317  */ },
{ ERROR_GROUP_EXISTS, "The specified group already exists. " /* 1318  */ },
{ ERROR_NO_SUCH_GROUP, "The specified group does not exist. " /* 1319  */ },
{ ERROR_MEMBER_IN_GROUP, "Either the specified user account is already a member of the specified group, or the specified group cannot be deleted because it contains a member. " /* 1320  */ },
{ ERROR_MEMBER_NOT_IN_GROUP, "The specified user account is not a member of the specified group account. " /* 1321  */ },
{ ERROR_LAST_ADMIN, "The last remaining administration account cannot be disabled or deleted. " /* 1322  */ },
{ ERROR_WRONG_PASSWORD, "Unable to update the password. The value provided as the current password is incorrect. " /* 1323  */ },
{ ERROR_ILL_FORMED_PASSWORD, "Unable to update the password. The value provided for the new password contains values that are not allowed in passwords. " /* 1324  */ },
{ ERROR_PASSWORD_RESTRICTION, "Unable to update the password because a password update rule has been violated. " /* 1325  */ },
{ ERROR_LOGON_FAILURE, "Logon failure \u2014 unknown user name or bad password. " /* 1326  */ },
{ ERROR_ACCOUNT_RESTRICTION, "Logon failure \u2014 user account restriction. " /* 1327  */ },
{ ERROR_INVALID_LOGON_HOURS, "Logon failure \u2014 account logon time restriction violation. " /* 1328  */ },
{ ERROR_INVALID_WORKSTATION, "Logon failure \u2014 user not allowed to log on to this computer. " /* 1329  */ },
{ ERROR_PASSWORD_EXPIRED, "Logon failure \u2014 the specified account password has expired. " /* 1330  */ },
{ ERROR_ACCOUNT_DISABLED, "Logon failure \u2014 account currently disabled. " /* 1331  */ },
{ ERROR_NONE_MAPPED, "No mapping between account names and security IDs was done. " /* 1332  */ },
{ ERROR_TOO_MANY_LUIDS_REQUESTED, "Too many LUIDs were requested at one time. " /* 1333  */ },
{ ERROR_LUIDS_EXHAUSTED, "No more LUIDs are available. " /* 1334  */ },
{ ERROR_INVALID_SUB_AUTHORITY, "The subauthority part of a security identifier is invalid for this particular use. " /* 1335  */ },
{ ERROR_INVALID_ACL, "The access control list (ACL) structure is invalid. " /* 1336  */ },
{ ERROR_INVALID_SID, "The security identifier structure is invalid. " /* 1337  */ },
{ ERROR_INVALID_SECURITY_DESCR, "The security descriptor structure is invalid. " /* 1338  */ },
{ ERROR_BAD_INHERITANCE_ACL, "The inherited access control list (ACL) or access control entry (ACE) could not be built. " /* 1340  */ },
{ ERROR_SERVER_DISABLED, "The server is currently disabled. " /* 1341  */ },
{ ERROR_SERVER_NOT_DISABLED, "The server is currently enabled. " /* 1342  */ },
{ ERROR_INVALID_ID_AUTHORITY, "The value provided was an invalid value for an identifier authority. " /* 1343  */ },
{ ERROR_ALLOTTED_SPACE_EXCEEDED, "No more memory is available for security information updates. " /* 1344  */ },
{ ERROR_INVALID_GROUP_ATTRIBUTES, "The specified attributes are invalid, or incompatible with the attributes for the group as a whole. " /* 1345  */ },
{ ERROR_BAD_IMPERSONATION_LEVEL, "Either a required impersonation level was not provided, or the provided impersonation level is invalid. " /* 1346  */ },
{ ERROR_CANT_OPEN_ANONYMOUS, "Cannot open an anonymous level security token. " /* 1347  */ },
{ ERROR_BAD_VALIDATION_CLASS, "The validation information class requested was invalid. " /* 1348  */ },
{ ERROR_BAD_TOKEN_TYPE, "The type of the token is inappropriate for its attempted use. " /* 1349  */ },
{ ERROR_NO_SECURITY_ON_OBJECT, "Unable to perform a security operation on an object that has no associated security. " /* 1350  */ },
{ ERROR_CANT_ACCESS_DOMAIN_INFO, "Indicates that a Windows NT Server could not be contacted or that objects within the domain are protected such that necessary information could not be retrieved. " /* 1351  */ },
{ ERROR_INVALID_SERVER_STATE, "The security account manager (SAM) or local security authority (LSA) server was in the wrong state to perform the security operation. " /* 1352  */ },
{ ERROR_INVALID_DOMAIN_STATE, "The domain was in the wrong state to perform the security operation. " /* 1353  */ },
{ ERROR_INVALID_DOMAIN_ROLE, "This operation is only allowed for the Primary Domain Controller (PDC) of the domain. " /* 1354  */ },
{ ERROR_NO_SUCH_DOMAIN, "The specified domain did not exist. " /* 1355  */ },
{ ERROR_DOMAIN_EXISTS, "The specified domain already exists. " /* 1356  */ },
{ ERROR_DOMAIN_LIMIT_EXCEEDED, "An attempt was made to exceed the limit on the number of domains per server. " /* 1357  */ },
{ ERROR_INTERNAL_DB_CORRUPTION, "Unable to complete the requested operation because of either a catastrophic media failure or a data structure corruption on the disk. " /* 1358  */ },
{ ERROR_INTERNAL_ERROR, "The security account database contains an internal inconsistency. " /* 1359  */ },
{ ERROR_GENERIC_NOT_MAPPED, "Generic access types were contained in an access mask that should already be mapped to nongeneric types. " /* 1360  */ },
{ ERROR_BAD_DESCRIPTOR_FORMAT, "A security descriptor is not in the right format (absolute or self-relative). " /* 1361  */ },
{ ERROR_NOT_LOGON_PROCESS, "The requested action is restricted for use by logon processes only. The calling process has not registered as a logon process. " /* 1362  */ },
{ ERROR_LOGON_SESSION_EXISTS, "Cannot start a new logon session with an identifier that is already in use. " /* 1363  */ },
{ ERROR_NO_SUCH_PACKAGE, "A specified authentication package is unknown. " /* 1364  */ },
{ ERROR_BAD_LOGON_SESSION_STATE, "The logon session is not in a state that is consistent with the requested operation. " /* 1365  */ },
{ ERROR_LOGON_SESSION_COLLISION, "The logon session identifier is already in use. " /* 1366  */ },
{ ERROR_INVALID_LOGON_TYPE, "A logon request contained an invalid logon type value. " /* 1367  */ },
{ ERROR_RXACT_INVALID_STATE, "The transaction state of a registry subtree is incompatible with the requested operation. " /* 1369  */ },
{ ERROR_RXACT_COMMIT_FAILURE, "An internal security database corruption has been encountered. " /* 1370  */ },
{ ERROR_SPECIAL_ACCOUNT, "Cannot perform this operation on built-in accounts. " /* 1371  */ },
{ ERROR_SPECIAL_GROUP, "Cannot perform this operation on this built-in special group. " /* 1372  */ },
{ ERROR_SPECIAL_USER, "Cannot perform this operation on this built-in special user. " /* 1373  */ },
{ ERROR_MEMBERS_PRIMARY_GROUP, "The user cannot be removed from a group because the group is currently the user's primary group. " /* 1374  */ },
{ ERROR_TOKEN_ALREADY_IN_USE, "The token is already in use as a primary token. " /* 1375  */ },
{ ERROR_NO_SUCH_ALIAS, "The specified local group does not exist. " /* 1376  */ },
{ ERROR_MEMBER_NOT_IN_ALIAS, "The specified account name is not a member of the local group. " /* 1377  */ },
{ ERROR_MEMBER_IN_ALIAS, "The specified account name is already a member of the local group. " /* 1378  */ },
{ ERROR_ALIAS_EXISTS, "The specified local group already exists. " /* 1379  */ },
{ ERROR_LOGON_NOT_GRANTED, "Logon failure \u2014 the user has not been granted the requested logon type at this computer. " /* 1380  */ },
{ ERROR_TOO_MANY_SECRETS, "The maximum number of secrets that may be stored in a single system has been exceeded. " /* 1381  */ },
{ ERROR_SECRET_TOO_LONG, "The length of a secret exceeds the maximum length allowed. " /* 1382  */ },
{ ERROR_INTERNAL_DB_ERROR, "The local security authority database contains an internal inconsistency. " /* 1383  */ },
{ ERROR_TOO_MANY_CONTEXT_IDS, "During a logon attempt, the user's security context accumulated too many security IDs. " /* 1384  */ },
{ ERROR_LOGON_TYPE_NOT_GRANTED, "Logon failure \u2014 the user has not been granted the requested logon type at this computer. " /* 1385  */ },
{ ERROR_NT_CROSS_ENCRYPTION_REQUIRED, "A cross-encrypted password is necessary to change a user password. " /* 1386  */ },
{ ERROR_NO_SUCH_MEMBER, "A new member could not be added to a local group because the member does not exist. " /* 1387  */ },
{ ERROR_INVALID_MEMBER, "A new member could not be added to a local group because the member has the wrong account type. " /* 1388  */ },
{ ERROR_TOO_MANY_SIDS, "Too many security IDs have been specified. " /* 1389  */ },
{ ERROR_LM_CROSS_ENCRYPTION_REQUIRED, "A cross-encrypted password is necessary to change this user password. " /* 1390  */ },
{ ERROR_NO_INHERITANCE, "Indicates an ACL contains no inheritable components. " /* 1391  */ },
{ ERROR_FILE_CORRUPT, "The file or directory is corrupted and non-readable. " /* 1392  */ },
{ ERROR_DISK_CORRUPT, "The disk structure is corrupted and non-readable. " /* 1393  */ },
{ ERROR_NO_USER_SESSION_KEY, "There is no user session key for the specified logon session. " /* 1394  */ },
{ ERROR_LICENSE_QUOTA_EXCEEDED, "The service being accessed is licensed for a particular number of connections. No more connections can be made to the service at this time because there are already as many connections as the service can accept. " /* 1395  */ },
{ ERROR_INVALID_WINDOW_HANDLE, "Invalid window handle. " /* 1400  */ },
{ ERROR_INVALID_MENU_HANDLE, "Invalid menu handle. " /* 1401  */ },
{ ERROR_INVALID_CURSOR_HANDLE, "Invalid cursor handle. " /* 1402  */ },
{ ERROR_INVALID_ACCEL_HANDLE, "Invalid accelerator table handle. " /* 1403  */ },
{ ERROR_INVALID_HOOK_HANDLE, "Invalid hook handle. " /* 1404  */ },
{ ERROR_INVALID_DWP_HANDLE, "Invalid handle to a multiple-window position structure. " /* 1405  */ },
{ ERROR_TLW_WITH_WSCHILD, "Cannot create a top-level child window. " /* 1406  */ },
{ ERROR_CANNOT_FIND_WND_CLASS, "Cannot find window class. " /* 1407  */ },
{ ERROR_WINDOW_OF_OTHER_THREAD, "Invalid window, it belongs to another thread. " /* 1408  */ },
{ ERROR_HOTKEY_ALREADY_REGISTERED, "Hot key is already registered. " /* 1409  */ },
{ ERROR_CLASS_ALREADY_EXISTS, "Class already exists. " /* 1410  */ },
{ ERROR_CLASS_DOES_NOT_EXIST, "Class does not exist. " /* 1411  */ },
{ ERROR_CLASS_HAS_WINDOWS, "Class still has open windows. " /* 1412  */ },
{ ERROR_INVALID_INDEX, "Invalid index. " /* 1413  */ },
{ ERROR_INVALID_ICON_HANDLE, "Invalid icon handle. " /* 1414  */ },
{ ERROR_PRIVATE_DIALOG_INDEX, "Using private DIALOG window words. " /* 1415  */ },
{ ERROR_LISTBOX_ID_NOT_FOUND, "The list box identifier was not found. " /* 1416  */ },
{ ERROR_NO_WILDCARD_CHARACTERS, "No wildcards were found. " /* 1417  */ },
{ ERROR_CLIPBOARD_NOT_OPEN, "Thread does not have a clipboard open. " /* 1418  */ },
{ ERROR_HOTKEY_NOT_REGISTERED, "Hot key is not registered. " /* 1419  */ },
{ ERROR_WINDOW_NOT_DIALOG, "The window is not a valid dialog window. " /* 1420  */ },
{ ERROR_CONTROL_ID_NOT_FOUND, "Control identifier not found. " /* 1421  */ },
{ ERROR_INVALID_COMBOBOX_MESSAGE, "Invalid message for a combo box because it does not have an edit control. " /* 1422  */ },
{ ERROR_WINDOW_NOT_COMBOBOX, "The window is not a combo box. " /* 1423  */ },
{ ERROR_INVALID_EDIT_HEIGHT, "Height must be less than 256. " /* 1424  */ },
{ ERROR_DC_NOT_FOUND, "Invalid device context (DC) handle. " /* 1425  */ },
{ ERROR_INVALID_HOOK_FILTER, "Invalid hook procedure type. " /* 1426  */ },
{ ERROR_INVALID_FILTER_PROC, "Invalid hook procedure. " /* 1427  */ },
{ ERROR_HOOK_NEEDS_HMOD, "Cannot set nonlocal hook without a module handle. " /* 1428  */ },
{ ERROR_GLOBAL_ONLY_HOOK, "This hook procedure can only be set globally. " /* 1429  */ },
{ ERROR_JOURNAL_HOOK_SET, "The journal hook procedure is already installed. " /* 1430  */ },
{ ERROR_HOOK_NOT_INSTALLED, "The hook procedure is not installed. " /* 1431  */ },
{ ERROR_INVALID_LB_MESSAGE, "Invalid message for single-selection list box. " /* 1432  */ },
{ ERROR_LB_WITHOUT_TABSTOPS, "This list box does not support tab stops. " /* 1434  */ },
{ ERROR_DESTROY_OBJECT_OF_OTHER_THREAD, "Cannot destroy object created by another thread. " /* 1435  */ },
{ ERROR_CHILD_WINDOW_MENU, "Child windows cannot have menus. " /* 1436  */ },
{ ERROR_NO_SYSTEM_MENU, "The window does not have a system menu. " /* 1437  */ },
{ ERROR_INVALID_MSGBOX_STYLE, "Invalid message box style. " /* 1438  */ },
{ ERROR_INVALID_SPI_VALUE, "Invalid system-wide (SPI_*) parameter. " /* 1439  */ },
{ ERROR_SCREEN_ALREADY_LOCKED, "Screen already locked. " /* 1440  */ },
{ ERROR_HWNDS_HAVE_DIFF_PARENT, "All handles to windows in a multiple-window position structure must have the same parent. " /* 1441  */ },
{ ERROR_NOT_CHILD_WINDOW, "The window is not a child window. " /* 1442  */ },
{ ERROR_INVALID_GW_COMMAND, "Invalid GW_* command. " /* 1443  */ },
{ ERROR_INVALID_THREAD_ID, "Invalid thread identifier. " /* 1444  */ },
{ ERROR_NON_MDICHILD_WINDOW, "Cannot process a message from a window that is not a multiple-document interface (MDI) window. " /* 1445  */ },
{ ERROR_POPUP_ALREADY_ACTIVE, "Pop-up menu already active. " /* 1446  */ },
{ ERROR_NO_SCROLLBARS, "The window does not have scroll bars. " /* 1447  */ },
{ ERROR_INVALID_SCROLLBAR_RANGE, "Scroll bar range cannot be greater than 0x7FFF. " /* 1448  */ },
{ ERROR_INVALID_SHOWWIN_COMMAND, "Cannot show or remove the window in the way specified. " /* 1449  */ },
{ ERROR_NO_SYSTEM_RESOURCES, "Insufficient system resources exist to complete the requested service. " /* 1450  */ },
{ ERROR_NONPAGED_SYSTEM_RESOURCES, "Insufficient system resources exist to complete the requested service. " /* 1451  */ },
{ ERROR_PAGED_SYSTEM_RESOURCES, "Insufficient system resources exist to complete the requested service. " /* 1452  */ },
{ ERROR_WORKING_SET_QUOTA, "Insufficient quota to complete the requested service. " /* 1453  */ },
{ ERROR_PAGEFILE_QUOTA, "Insufficient quota to complete the requested service. " /* 1454  */ },
{ ERROR_COMMITMENT_LIMIT, "The paging file is too small for this operation to complete. " /* 1455  */ },
{ ERROR_MENU_ITEM_NOT_FOUND, "A menu item was not found. " /* 1456  */ },
{ ERROR_INVALID_KEYBOARD_HANDLE, "Invalid keyboard layout handle. " /* 1457  */ },
{ ERROR_HOOK_TYPE_NOT_ALLOWED, "Hook type not allowed. " /* 1458  */ },
{ ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION, "This operation requires an interactive window station. " /* 1459  */ },
{ ERROR_TIMEOUT, "This operation returned because the time-out period expired. " /* 1460  */ },
{ ERROR_EVENTLOG_FILE_CORRUPT, "The event tracking file is corrupted. " /* 1500  */ },
{ ERROR_EVENTLOG_CANT_START, "No event tracking file could be opened, so the event tracking service did not start. " /* 1501  */ },
{ ERROR_LOG_FILE_FULL, "The event tracking file is full. " /* 1502  */ },
{ ERROR_EVENTLOG_FILE_CHANGED, "The event tracking file has changed between read operations. " /* 1503  */ },
{ RPC_S_INVALID_STRING_BINDING, "The string binding is invalid. " /* 1700  */ },
{ RPC_S_WRONG_KIND_OF_BINDING, "The binding handle is not the correct type. " /* 1701  */ },
{ RPC_S_INVALID_BINDING, "The binding handle is invalid. " /* 1702  */ },
{ RPC_S_PROTSEQ_NOT_SUPPORTED, "The RPC protocol sequence is not supported. " /* 1703  */ },
{ RPC_S_INVALID_RPC_PROTSEQ, "The RPC protocol sequence is invalid. " /* 1704  */ },
{ RPC_S_INVALID_STRING_UUID, "The string universal unique identifier (UUID) is invalid. " /* 1705  */ },
{ RPC_S_INVALID_ENDPOINT_FORMAT, "The endpoint format is invalid. " /* 1706  */ },
{ RPC_S_INVALID_NET_ADDR, "The network address is invalid. " /* 1707  */ },
{ RPC_S_NO_ENDPOINT_FOUND, "No endpoint was found. " /* 1708  */ },
{ RPC_S_INVALID_TIMEOUT, "The time-out value is invalid. " /* 1709  */ },
{ RPC_S_OBJECT_NOT_FOUND, "The object universal unique identifier (UUID) was not found. " /* 1710  */ },
{ RPC_S_ALREADY_REGISTERED, "The object universally unique identifier (UUID) has already been registered. " /* 1711  */ },
{ RPC_S_TYPE_ALREADY_REGISTERED, "The type UUID has already been registered. " /* 1712  */ },
{ RPC_S_ALREADY_LISTENING, "The remote procedure call (RPC) server is already listening. " /* 1713  */ },
{ RPC_S_NO_PROTSEQS_REGISTERED, "No protocol sequences have been registered. " /* 1714  */ },
{ RPC_S_NOT_LISTENING, "The RPC server is not listening. " /* 1715  */ },
{ RPC_S_UNKNOWN_MGR_TYPE, "The manager type is unknown. " /* 1716  */ },
{ RPC_S_UNKNOWN_IF, "The interface is unknown. " /* 1717  */ },
{ RPC_S_NO_BINDINGS, "There are no bindings. " /* 1718  */ },
{ RPC_S_NO_PROTSEQS, "There are no protocol sequences. " /* 1719  */ },
{ RPC_S_CANT_CREATE_ENDPOINT, "The endpoint cannot be created. " /* 1720  */ },
{ RPC_S_OUT_OF_RESOURCES, "Not enough resources are available to complete this operation. " /* 1721  */ },
{ RPC_S_SERVER_UNAVAILABLE, "The RPC server is unavailable. " /* 1722  */ },
{ RPC_S_SERVER_TOO_BUSY, "The RPC server is too busy to complete this operation. " /* 1723  */ },
{ RPC_S_INVALID_NETWORK_OPTIONS, "The network options are invalid. " /* 1724  */ },
{ RPC_S_NO_CALL_ACTIVE, "There is not a remote procedure call active in this thread. " /* 1725  */ },
{ RPC_S_CALL_FAILED, "The remote procedure call failed. " /* 1726  */ },
{ RPC_S_CALL_FAILED_DNE, "The remote procedure call failed and did not execute. " /* 1727  */ },
{ RPC_S_PROTOCOL_ERROR, "A remote procedure call (RPC) protocol error occurred. " /* 1728  */ },
{ RPC_S_UNSUPPORTED_TRANS_SYN, "The transfer syntax is not supported by the RPC server. " /* 1730  */ },
{ RPC_S_UNSUPPORTED_TYPE, "The universal unique identifier (UUID) type is not supported. " /* 1732  */ },
{ RPC_S_INVALID_TAG, "The tag is invalid. " /* 1733  */ },
{ RPC_S_INVALID_BOUND, "The array bounds are invalid. " /* 1734  */ },
{ RPC_S_NO_ENTRY_NAME, "The binding does not contain an entry name. " /* 1735  */ },
{ RPC_S_INVALID_NAME_SYNTAX, "The name syntax is invalid. " /* 1736  */ },
{ RPC_S_UNSUPPORTED_NAME_SYNTAX, "The name syntax is not supported. " /* 1737  */ },
{ RPC_S_UUID_NO_ADDRESS, "No network address is available to use to construct a universal unique identifier (UUID). " /* 1739  */ },
{ RPC_S_DUPLICATE_ENDPOINT, "The endpoint is a duplicate. " /* 1740  */ },
{ RPC_S_UNKNOWN_AUTHN_TYPE, "The authentication type is unknown. " /* 1741  */ },
{ RPC_S_MAX_CALLS_TOO_SMALL, "The maximum number of calls is too small. " /* 1742  */ },
{ RPC_S_STRING_TOO_LONG, "The string is too long. " /* 1743  */ },
{ RPC_S_PROTSEQ_NOT_FOUND, "The RPC protocol sequence was not found. " /* 1744  */ },
{ RPC_S_PROCNUM_OUT_OF_RANGE, "The procedure number is out of range. " /* 1745  */ },
{ RPC_S_BINDING_HAS_NO_AUTH, "The binding does not contain any authentication information. " /* 1746  */ },
{ RPC_S_UNKNOWN_AUTHN_SERVICE, "The authentication service is unknown. " /* 1747  */ },
{ RPC_S_UNKNOWN_AUTHN_LEVEL, "The authentication level is unknown. " /* 1748  */ },
{ RPC_S_INVALID_AUTH_IDENTITY, "The security context is invalid. " /* 1749  */ },
{ RPC_S_UNKNOWN_AUTHZ_SERVICE, "The authorization service is unknown. " /* 1750  */ },
{ EPT_S_INVALID_ENTRY, "The entry is invalid. " /* 1751  */ },
{ EPT_S_CANT_PERFORM_OP, "The server endpoint cannot perform the operation. " /* 1752  */ },
{ EPT_S_NOT_REGISTERED, "There are no more endpoints available from the endpoint mapper. " /* 1753  */ },
{ RPC_S_NOTHING_TO_EXPORT, "No interfaces have been exported. " /* 1754  */ },
{ RPC_S_INCOMPLETE_NAME, "The entry name is incomplete. " /* 1755  */ },
{ RPC_S_INVALID_VERS_OPTION, "The version option is invalid. " /* 1756  */ },
{ RPC_S_NO_MORE_MEMBERS, "There are no more members. " /* 1757  */ },
{ RPC_S_NOT_ALL_OBJS_UNEXPORTED, "There is nothing to unexport. " /* 1758  */ },
{ RPC_S_INTERFACE_NOT_FOUND, "The interface was not found. " /* 1759  */ },
{ RPC_S_ENTRY_ALREADY_EXISTS, "The entry already exists. " /* 1760  */ },
{ RPC_S_ENTRY_NOT_FOUND, "The entry is not found. " /* 1761  */ },
{ RPC_S_NAME_SERVICE_UNAVAILABLE, "The name service is unavailable. " /* 1762  */ },
{ RPC_S_INVALID_NAF_ID, "The network address family is invalid. " /* 1763  */ },
{ RPC_S_CANNOT_SUPPORT, "The requested operation is not supported. " /* 1764  */ },
{ RPC_S_NO_CONTEXT_AVAILABLE, "No security context is available to allow impersonation. " /* 1765  */ },
{ RPC_S_INTERNAL_ERROR, "An internal error occurred in a remote procedure call (RPC). " /* 1766  */ },
{ RPC_S_ZERO_DIVIDE, "The RPC server attempted an integer division by zero. " /* 1767  */ },
{ RPC_S_ADDRESS_ERROR, "An addressing error occurred in the RPC server. " /* 1768  */ },
{ RPC_S_FP_DIV_ZERO, "A floating-point operation at the RPC server caused a division by zero. " /* 1769  */ },
{ RPC_S_FP_UNDERFLOW, "A floating-point underflow occurred at the RPC server. " /* 1770  */ },
{ RPC_S_FP_OVERFLOW, "A floating-point overflow occurred at the RPC server. " /* 1771  */ },
{ RPC_X_NO_MORE_ENTRIES, "The list of RPC servers available for the binding of auto handles has been exhausted. " /* 1772  */ },
{ RPC_X_SS_CHAR_TRANS_OPEN_FAIL, "Unable to open the character translation table file. " /* 1773  */ },
{ RPC_X_SS_CHAR_TRANS_SHORT_FILE, "The file containing the character translation table has fewer than 512 bytes. " /* 1774  */ },
{ RPC_X_SS_IN_NULL_CONTEXT, "A null context handle was passed from the client to the host during a remote procedure call. " /* 1775  */ },
{ RPC_X_SS_CONTEXT_DAMAGED, "The context handle changed during a remote procedure call. " /* 1777  */ },
{ RPC_X_SS_HANDLES_MISMATCH, "The binding handles passed to a remote procedure call do not match. " /* 1778  */ },
{ RPC_X_SS_CANNOT_GET_CALL_HANDLE, "The stub is unable to get the remote procedure call handle. " /* 1779  */ },
{ RPC_X_NULL_REF_POINTER, "A null reference pointer was passed to the stub. " /* 1780  */ },
{ RPC_X_ENUM_VALUE_OUT_OF_RANGE, "The enumeration value is out of range. " /* 1781  */ },
{ RPC_X_BYTE_COUNT_TOO_SMALL, "The byte count is too small. " /* 1782  */ },
{ RPC_X_BAD_STUB_DATA, "The stub received bad data. " /* 1783  */ },
{ ERROR_INVALID_USER_BUFFER, "The supplied user buffer is not valid for the requested operation. " /* 1784  */ },
{ ERROR_UNRECOGNIZED_MEDIA, "The disk media is not recognized. It may not be formatted. " /* 1785  */ },
{ ERROR_NO_TRUST_LSA_SECRET, "The workstation does not have a trust secret. " /* 1786  */ },
{ ERROR_NO_TRUST_SAM_ACCOUNT, "The SAM database on the Windows NT Server does not have a computer account for this workstation trust relationship. " /* 1787  */ },
{ ERROR_TRUSTED_DOMAIN_FAILURE, "The trust relationship between the primary domain and the trusted domain failed. " /* 1788  */ },
{ ERROR_TRUSTED_RELATIONSHIP_FAILURE, "The trust relationship between this workstation and the primary domain failed. " /* 1789  */ },
{ ERROR_TRUST_FAILURE, "The network logon failed. " /* 1790  */ },
{ RPC_S_CALL_IN_PROGRESS, "A remote procedure call is already in progress for this thread. " /* 1791  */ },
{ ERROR_NETLOGON_NOT_STARTED, "An attempt was made to logon, but the network logon service was not started. " /* 1792  */ },
{ ERROR_ACCOUNT_EXPIRED, "The user's account has expired. " /* 1793  */ },
{ ERROR_REDIRECTOR_HAS_OPEN_HANDLES, "The redirector is in use and cannot be unloaded. " /* 1794  */ },
{ ERROR_PRINTER_DRIVER_ALREADY_INSTALLED, "The specified printer driver is already installed. " /* 1795  */ },
{ ERROR_UNKNOWN_PORT, "The specified port is unknown. " /* 1796  */ },
{ ERROR_UNKNOWN_PRINTER_DRIVER, "The printer driver is unknown. " /* 1797  */ },
{ ERROR_UNKNOWN_PRINTPROCESSOR, "The print processor is unknown. " /* 1798  */ },
{ ERROR_INVALID_SEPARATOR_FILE, "The specified separator file is invalid. " /* 1799  */ },
{ ERROR_INVALID_PRIORITY, "The specified priority is invalid. " /* 1800  */ },
{ ERROR_INVALID_PRINTER_NAME, "The printer name is invalid. " /* 1801  */ },
{ ERROR_PRINTER_ALREADY_EXISTS, "The printer already exists. " /* 1802  */ },
{ ERROR_INVALID_PRINTER_COMMAND, "The printer command is invalid. " /* 1803  */ },
{ ERROR_INVALID_DATATYPE, "The specified data type is invalid. " /* 1804  */ },
{ ERROR_INVALID_ENVIRONMENT, "The environment specified is invalid. " /* 1805  */ },
{ RPC_S_NO_MORE_BINDINGS, "There are no more bindings. " /* 1806  */ },
{ ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT, "The account used is an interdomain trust account. Use your global user account or local user account to access this server. " /* 1807  */ },
{ ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT, "The account used is a computer account. Use your global user account or local user account to access this server. " /* 1808  */ },
{ ERROR_NOLOGON_SERVER_TRUST_ACCOUNT, "The account used is a server trust account. Use your global user account or local user account to access this server. " /* 1809  */ },
{ ERROR_DOMAIN_TRUST_INCONSISTENT, "The name or security identifier (SID) of the domain specified is inconsistent with the trust information for that domain. " /* 1810  */ },
{ ERROR_SERVER_HAS_OPEN_HANDLES, "The server is in use and cannot be unloaded. " /* 1811  */ },
{ ERROR_RESOURCE_DATA_NOT_FOUND, "The specified image file did not contain a resource section. " /* 1812  */ },
{ ERROR_RESOURCE_TYPE_NOT_FOUND, "The specified resource type cannot be found in the image file. " /* 1813  */ },
{ ERROR_RESOURCE_NAME_NOT_FOUND, "The specified resource name cannot be found in the image file. " /* 1814  */ },
{ ERROR_RESOURCE_LANG_NOT_FOUND, "The specified resource language identifier cannot be found in the image file. " /* 1815  */ },
{ ERROR_NOT_ENOUGH_QUOTA, "Not enough quota is available to process this command. " /* 1816  */ },
{ RPC_S_NO_INTERFACES, "No interfaces have been registered. " /* 1817  */ },
{ RPC_S_CALL_CANCELLED, "The server was altered while processing this call. " /* 1818  */ },
{ RPC_S_BINDING_INCOMPLETE, "The binding handle does not contain all required information. " /* 1819  */ },
{ RPC_S_COMM_FAILURE, "Communications failure. " /* 1820  */ },
{ RPC_S_UNSUPPORTED_AUTHN_LEVEL, "The requested authentication level is not supported. " /* 1821  */ },
{ RPC_S_NO_PRINC_NAME, "No principal name registered. " /* 1822  */ },
{ RPC_S_NOT_RPC_ERROR, "The error specified is not a valid Windows NT RPC error value. " /* 1823  */ },
{ RPC_S_UUID_LOCAL_ONLY, "A UUID that is valid only on this computer has been allocated. " /* 1824  */ },
{ RPC_S_SEC_PKG_ERROR, "A security package specific error occurred. " /* 1825  */ },
{ RPC_S_NOT_CANCELLED, "Thread is not canceled. " /* 1826  */ },
{ RPC_X_INVALID_ES_ACTION, "Invalid operation on the encoding/decoding handle. " /* 1827  */ },
{ RPC_X_WRONG_ES_VERSION, "Incompatible version of the serializing package. " /* 1828  */ },
{ RPC_X_WRONG_STUB_VERSION, "Incompatible version of the RPC stub. " /* 1829  */ },
{ RPC_X_INVALID_PIPE_OBJECT, "The idl pipe object is invalid or corrupted. " /* 1830  */ },
// { RPC_X_INVALID_PIPE_OPERATION, "The operation is invalid for a given idl pipe object. " /* 1831  */ },
{ RPC_X_WRONG_PIPE_VERSION, "The Interface Definition Language (IDL) pipe version is not supported. " /* 1832  */ },
{ RPC_S_GROUP_MEMBER_NOT_FOUND, "The group member was not found. " /* 1898  */ },
{ EPT_S_CANT_CREATE, "The endpoint mapper database could not be created. " /* 1899  */ },
{ RPC_S_INVALID_OBJECT, "The object UUID is the nil UUID. " /* 1900  */ },
{ ERROR_INVALID_TIME, "The specified time is invalid. " /* 1901  */ },
{ ERROR_INVALID_FORM_NAME, "The specified form name is invalid. " /* 1902  */ },
{ ERROR_INVALID_FORM_SIZE, "The specified form size is invalid. " /* 1903  */ },
{ ERROR_ALREADY_WAITING, "The specified printer handle is already being waited on " /* 1904  */ },
{ ERROR_PRINTER_DELETED, "The specified printer has been deleted. " /* 1905  */ },
{ ERROR_INVALID_PRINTER_STATE, "The state of the printer is invalid. " /* 1906  */ },
{ ERROR_PASSWORD_MUST_CHANGE, "The user must change his password before he logs on the first time. " /* 1907  */ },
{ ERROR_DOMAIN_CONTROLLER_NOT_FOUND, "Could not find the domain controller for this domain. " /* 1908  */ },
{ ERROR_ACCOUNT_LOCKED_OUT, "The referenced account is currently locked out and may not be logged on to. " /* 1909  */ },
{ OR_INVALID_OXID, "The object exporter specified was not found. " /* 1910  */ },
{ OR_INVALID_OID, "The object specified was not found. " /* 1911  */ },
{ OR_INVALID_SET, "The object resolver set specified was not found. " /* 1912  */ },
{ RPC_S_SEND_INCOMPLETE, "Some data remains to be sent in the request buffer. " /* 1913  */ },
{ ERROR_INVALID_PIXEL_FORMAT, "The pixel format is invalid. " /* 2000  */ },
{ ERROR_BAD_DRIVER, "The specified driver is invalid. " /* 2001  */ },
{ ERROR_INVALID_WINDOW_STYLE, "The window style or class attribute is invalid for this operation. " /* 2002  */ },
{ ERROR_METAFILE_NOT_SUPPORTED, "The requested metafile operation is not supported. " /* 2003  */ },
{ ERROR_TRANSFORM_NOT_SUPPORTED, "The requested transformation operation is not supported. " /* 2004  */ },
{ ERROR_CLIPPING_NOT_SUPPORTED, "The requested clipping operation is not supported. " /* 2005  */ },
{ ERROR_BAD_USERNAME, "The specified user name is invalid. " /* 2202  */ },
{ ERROR_NOT_CONNECTED, "This network connection does not exist. " /* 2250  */ },
{ ERROR_OPEN_FILES, "This network connection has files open or requests pending. " /* 2401  */ },
{ ERROR_ACTIVE_CONNECTIONS, "Active connections still exist. " /* 2402  */ },
{ ERROR_DEVICE_IN_USE, "The device is in use by an active process and cannot be disconnected. " /* 2404  */ },
{ ERROR_UNKNOWN_PRINT_MONITOR, "The specified print monitor is unknown. " /* 3000  */ },
{ ERROR_PRINTER_DRIVER_IN_USE, "The specified printer driver is currently in use. " /* 3001  */ },
{ ERROR_SPOOL_FILE_NOT_FOUND, "The spool file was not found. " /* 3002  */ },
{ ERROR_SPL_NO_STARTDOC, "A StartDocPrinter call was not issued. " /* 3003  */ },
{ ERROR_SPL_NO_ADDJOB, "An AddJob call was not issued. " /* 3004  */ },
{ ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED, "The specified print processor has already been installed. " /* 3005  */ },
{ ERROR_PRINT_MONITOR_ALREADY_INSTALLED, "The specified print monitor has already been installed. " /* 3006  */ },
{ ERROR_INVALID_PRINT_MONITOR, "The specified print monitor does not have the required functions. " /* 3007  */ },
{ ERROR_PRINT_MONITOR_IN_USE, "The specified print monitor is currently in use. " /* 3008  */ },
{ ERROR_PRINTER_HAS_JOBS_QUEUED, "The requested operation is not allowed when there are jobs queued to the printer. " /* 3009  */ },
{ ERROR_SUCCESS_REBOOT_REQUIRED, "The requested operation is successful. Changes will not be effective until the system is rebooted. " /* 3010  */ },
{ ERROR_SUCCESS_RESTART_REQUIRED, "The requested operation is successful. Changes will not be effective until the service is restarted. " /* 3011  */ },
{ ERROR_WINS_INTERNAL, "WINS encountered an error while processing the command. " /* 4000  */ },
{ ERROR_CAN_NOT_DEL_LOCAL_WINS, "The local WINS cannot be deleted. " /* 4001  */ },
{ ERROR_STATIC_INIT, "The importation from the file failed. " /* 4002  */ },
{ ERROR_INC_BACKUP, "The backup failed. Was a full backup done before? " /* 4003  */ },
{ ERROR_FULL_BACKUP, "The backup failed. Check the directory to which you are backing the database. " /* 4004  */ },
{ ERROR_REC_NON_EXISTENT, "The name does not exist in the WINS database. " /* 4005  */ },
{ ERROR_RPL_NOT_ALLOWED, "Replication with a nonconfigured partner is not allowed. " /* 4006  */ },
{ ERROR_NO_BROWSER_SERVERS_FOUND, "The list of servers for this workgroup is not currently available. " /* 6118  */ },
{ 0, 0 /* The end */ }
};

/*
 * Private WCS functions
 *
 * We need this because the format of a wide character string may be different
 * on the host than on the PDA.
 *
 * The PDA's WCS format on my PDA holds 16 bits per character.
 */
static int MyMbstowcs(wchar_t *dest, const char *src, int n)
{
	int	x;
	char	*d;

	if (dest == NULL) {
		/* Calculate the number of characters */
		return strlen(src) + 1;
	}

	d = (char *)dest;
	for (x=0; x < n && src[x]; x++) {
		d[2 * x] = src[x];
		d[2 * x + 1] = '\0';
		if (x < n-1) {
			d[2 * x + 2] = '\0';
			d[2 * x + 3] = '\0';
		}
	}
	return x + 1;
}

int MyWcstombs(char *dest, const wchar_t *src, int n)
{
	char	*p = (char *)src;
	int	i;

	if (dest == NULL) {
		/* Calculate the number of characters */
		for (i=0; p[2*i]; i++) ;
		return i+1;
	}
	dest[n] = '\0';
	for (i=0; i<n && p[2*i]; i++) {
		dest[i] = p[2*i];
		dest[i+1] = '\0';
	}
	return i;
}

/*
 * This function is slightly different from "wcslen" - it returns the number of bytes,
 * not characters, so no more need to multiply by the size of a character.
 */
static int MyWcslen(const wchar_t *s)
{
	int	i;
	char	*p;

	for (p=(char *)s, i=0; p[i] != '\0' || p[i+1] != '\0'; i += 2) ;
	return i + 2;
}

#endif	/* __CYGWIN__ */
