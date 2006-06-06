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

#include <windows.h>
#include <rapi.h>
#include <netdb.h>
#include <cygwin/in.h>
#include <cygwin/socket.h>
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

void ShowError(const char* lpszFunction);

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

#define DEBUG_COM(x)	do { if (debug_communication)	printf_unfiltered x; } while (0)


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

static int s;			/* communication socket */

/* v-style interface for handling varying argument list error messages.
   Displays the error message in a dialog box and exits when user clicks
   on OK.  */
static void
vstub_error (LPCSTR fmt, va_list args)
{
  char buf[4096];
  vsprintf (buf, fmt, args);
  s = -1;
  error (("%s"), buf);
}

/* The standard way to display an error message and exit.  */
static void
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

#if 0
static WORD
getword (LPCSTR huh, gdb_wince_id what_this)
{
  WORD n;
  gdb_wince_id what;
  do
    if (!sockread (huh, s, &what, sizeof (what)))
      stub_error ("error getting record type from host - %s.", huh);
  while (what_this != what);

  if (!sockread (huh, s, &n, sizeof (n)))
    stub_error ("error getting %s from host.", huh);

  return n;
}
#endif

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
      SetLastError (-(long) *nbytes);
      if (debug_communication)
      {
			  printf_unfiltered("getresult: ");
			  ShowError(huh);
      }
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
  *out_len = sizeof (WCHAR) * MultiByteToWideChar (CP_ACP, 0, s, 
						   -1, NULL, 0);
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
  (void) MultiByteToWideChar (CP_ACP, 0, s, -1, outs[n], *out_len);
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
    return 1;

  /* Short circuit */
  if (!h)
  {
    SetLastError(6);
    return FALSE;
  }

  puthandle ("CloseHandle handle", GDB_CLOSEHANDLE, h);
  return (int) getresult ("CloseHandle result", 
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

#define FACTOR (0x19db1ded53ea710LL)
#define NSPERSEC 10000000

/* Convert a Win32 time to "UNIX" format.  */
long
to_time_t (FILETIME * ptr)
{
  /* A file time is the number of 100ns since jan 1 1601
     stuffed into two long words.
     A time_t is the number of seconds since jan 1 1970.  */

  long rem;
  long long x = ((long long) ptr->dwHighDateTime << 32) + ((unsigned) ptr->dwLowDateTime);
  x -= FACTOR;			/* Number of 100ns between 1601 and 1970.  */
  rem = x % ((long long) NSPERSEC);
  rem += (NSPERSEC / 2);
  x /= (long long) NSPERSEC;	/* Number of 100ns in a second.  */
  x += (long long) (rem / NSPERSEC);
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
  time_t utime;
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
    return remotefile;		/* Don't bother uploading.  */

  /* Open the source.  */
  if ((fd = openp (getenv ("PATH"), OPF_TRY_CWD_FIRST, (char *) from,
		   O_RDONLY, 0, NULL)) < 0)
    error (_("couldn't open %s"), from);

  /* Get the time for later comparison.  */
  if (fstat (fd, &st))
    st.st_mtime = (time_t) - 1;

  /* Always attempt to create the directory on the remote system.  */
  wstr = towide (dir, NULL);
  (void) CeCreateDirectory (wstr, NULL);

  /* Attempt to open the remote file, creating it if it doesn't exist.  */
  wstr = towide (remotefile, NULL);
  h = CeCreateFile (wstr, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		    OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  /* Some kind of problem?  */
  err = CeGetLastError ();
  if (h == NULL || h == INVALID_HANDLE_VALUE)
    error (_("error opening file \"%s\".  Windows error %d."),
	   remotefile, (int)err);

  CeGetFileTime (h, &crtime, &actime, &wrtime);
  utime = to_time_t (&wrtime);
#if 0
  if (utime < st.st_mtime)
    {
      char buf[80];
      strcpy (buf, ctime(&utime));
      printf ("%s < %s\n", buf, ctime(&st.st_mtime));
    }
#endif
  /* See if we need to upload the file.  */
  if (upload_when == UPLOAD_ALWAYS ||
      err != ERROR_ALREADY_EXISTS ||
      !CeGetFileTime (h, &crtime, &actime, &wrtime) ||
      to_time_t (&wrtime) < st.st_mtime)
    {
      DWORD nbytes;
      char buf[4096];
      int n;

      /* Upload the file.  */
	  printf_unfiltered("Uploading to remote device: %s\n", remotefile);
      while ((n = read (fd, buf, sizeof (buf))) > 0)
	if (!CeWriteFile (h, buf, (DWORD) n, &nbytes, NULL))
	  error (_("error writing to remote device - %d."),
		 (int)CeGetLastError ());
    }

  close (fd);
  if (!CeCloseHandle (h))
    error (_("error closing remote file - %d."), (int)CeGetLastError ());

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
			towide (args, NULL),
			NULL, NULL, 0, 0, 
			NULL, NULL, NULL, &pi))
    error (_("Unable to start remote stub '%s'.  Windows CE error %d."),
	   stub_file_name, (int)CeGetLastError ());
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

void ShowError(const char* lpszFunction)
{
	char* lpMsgBuf;
	DWORD dw = GetLastError(); 

	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*) &lpMsgBuf,
		0, NULL );

	printf_unfiltered("%s failed with error %d: %s", lpszFunction, (int)dw, lpMsgBuf); 
	LocalFree(lpMsgBuf);
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

#if 0
  add_info
    ("dll", info_dll_command, _("Status of loaded DLLs."));
  add_info_alias ("sharedlibrary", "dll", 1);

  c = add_com ("upload", class_files, dll_symbol_command,
	  _("Upload file to device."));
  set_cmd_completer (c, filename_completer);
#endif
}
