// ceshared2.c - uses file mapping instead of shared dll sections
//
// Time-stamp: <23/02/02 11:36:31 keuchel@netwave.de>

#include "sys/ceshared.h"
#include "sys/wcebase.h"
#include "sys/wcememory.h"
#include "sys/wceerror.h"
#include "sys/wcetrace.h"
#include "sys/wcefile.h"

#include <errno.h>

#define min(a, b)	(a) < (b) ? a : b

HANDLE XCECreateEventA(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, const char *lpName);

#ifndef _WIN32_WCE_EMULATION
#define SYNC CacheSync(CACHE_SYNC_DISCARD|CACHE_SYNC_INSTRUCTIONS)
#else
#define SYNC
#endif

#define LOCK xceshared_lock
#define UNLOCK xceshared_release

// Synchronizing with events sometimes fails!
// I get an event, but avail count is zero!
// Seems that the compiler does not store it
// or there are problems with cache...
//
// CacheSync() seems to fix the problem...

static wchar_t *mutex_name = L"celib_shared_mutex";
static HANDLE hmutex;
static wchar_t *mapping_name = L"celib_shared_mapping";
static HANDLE hmapping;

extern CONSOLE_READ_FUNC  console_read_func;
extern CONSOLE_WRITE_FUNC console_write_func;
extern CONSOLE_IOCTL_FUNC console_ioctl_func;

struct procstart_entry xcelocal_procstart;

typedef struct shared_data {
  struct procstart_entry procstart;
  struct pipe_entry pipetab[MAXPIPES];
} SHARED_DATA;

static SHARED_DATA *psdata;

#undef exit

void
xceshared_init()
{
  DWORD dwError = 0;
  BOOL bNew = FALSE;

  SetLastError(0);

  hmapping = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, 
				PAGE_READWRITE|SEC_COMMIT,
				0, sizeof(SHARED_DATA),
				mapping_name);
  if(hmapping == NULL)
    {
      XCEShowMessageA("CreateFileMapping: %d", GetLastError());
      exit(1);
    }

  if((dwError = GetLastError()) != ERROR_ALREADY_EXISTS)
    bNew = TRUE;

  psdata = MapViewOfFile(hmapping, FILE_MAP_ALL_ACCESS, 0, 0,
			 sizeof(SHARED_DATA));

  if(psdata == NULL)
    {
      XCEShowMessageA("MapViewOfFile: %d", GetLastError());
      exit(1);
    }

  if(bNew)
    {
      memset(psdata, 0, sizeof(SHARED_DATA));
      psdata->procstart.showwindow = TRUE;
    }

  SYNC;
}

void
xceshared_reset()
{
  if(psdata)
    {
      memset(&psdata->procstart, 0, sizeof(struct procstart_entry));
      psdata->procstart.showwindow = TRUE;
    }
}

void
xceshared_setshowwindow(BOOL bShow)
{
  XCETraceA("setshowwindow %d", bShow);

  if(psdata)
    {
      psdata->procstart.showwindow = bShow;
    }
}

void
xceshared_setcwd(char *cwd)
{
  if(psdata)
    {
      strcpy(psdata->procstart.cwd, cwd);
    }
}

void
xceshared_setenvironblock(char *env)
{
  char *s;
  char *d;
  int len;
  char *endp;

  if(psdata == NULL)
    return;

  endp = psdata->procstart.environ + MAX_ENVIRONBLK;

  for(s = env, d = psdata->procstart.environ; *s;)
    {
      len = strlen(s);
      if(d + len >= endp)
	{
	  XCEShowMessageA("Out of space in environment (max %d)",
			  MAX_ENVIRONBLK);
	  exit(1);
	}
      memcpy(d, s, len + 1);
      s += len + 1;
      d += len + 1;
    }
  *d = 0;
}

BOOL
xceshared_setstdhandle(DWORD dwId, HANDLE hFile)
{
  XCETraceA("xceshared_setstdhandle(%d, %x)", dwId, hFile);

  if(psdata == NULL)
    return FALSE;

  if(hFile == INVALID_HANDLE_VALUE || hFile == NULL)
    {
    }
  else
    {
      if(!ISPIPEHANDLE(hFile))
	{
	  XCETraceA("Handle is not a pipe");
	  return FALSE;
	}

      if(!ISINHERIT(hFile))
	{
	  XCETraceA("Handle is not inheritable");
	  return TRUE;
	}
    }

  if(dwId == STD_INPUT_HANDLE)
    {
      psdata->procstart.hstdin = hFile;
    }
  else if(dwId == STD_OUTPUT_HANDLE)
    {
      psdata->procstart.hstdout = hFile;
    }
  if(dwId == STD_ERROR_HANDLE)
    {
      psdata->procstart.hstderr = hFile;
    }

  return TRUE;
}

//////////////////////////////////////////////////////////////////////

BOOL
xceshared_getshowwindow()
{
  // called before xce_init() in console... dont want to
  // recompile all progs...
  if(psdata == NULL)
    xceshared_init();

  return psdata->procstart.showwindow;
}

void
xceshared_getcwd(char *cwd)
{
  if(psdata == NULL)
    return;

  strcpy(cwd, psdata->procstart.cwd);
}

void
xceshared_getenvironblock(char *buf)
{
  char *s;
  char *d;
  int len;

  buf[0] = 0;

  if(psdata == NULL)
    return;

  for(s = psdata->procstart.environ, d = buf; *s;)
    {
      len = strlen(s);
      memcpy(d, s, len + 1);
      s += len + 1;
      d += len + 1;
    }
  *d = 0;
}

HANDLE
xceshared_getstdhandle(DWORD dwId)
{
  if(psdata == NULL)
    return INVALID_HANDLE_VALUE;

  if(dwId == STD_INPUT_HANDLE)
    {
      return psdata->procstart.hstdin;
    }
  else if(dwId == STD_OUTPUT_HANDLE)
    {
      return psdata->procstart.hstdout;
    }
  if(dwId == STD_ERROR_HANDLE)
    {
      return psdata->procstart.hstderr;
    }

  return INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////

BOOL
xceshared_lock()
{
  if((hmutex = CreateMutexW(NULL, FALSE, mutex_name)) == NULL)
    {
      return FALSE;
    }
  WaitForSingleObject(hmutex, INFINITE);
  return TRUE;
}

BOOL
xceshared_release()
{
  return ReleaseMutex(hmutex);
}

//////////////////////////////////////////////////////////////////////
// Needed because of cache problems? Did not fix it...

void
pipe_setavail(struct pipe_entry *pp, int avail)
{
  pp->avail = avail;
}

int
pipe_getavail(struct pipe_entry *pp)
{
  return pp->avail;
}

// we assume that less that bufsize bytes are written...

int
pipe_write(HANDLE hPipe, const unsigned char *buf, int size)
{
  HANDLE hevent_canread, hevent_canwrite;
  struct pipe_entry *pp;
  int written = 0;
  int pipeindex;
  int len = 0;

  XCETraceA("pipe_write(%x, %x, %d)", hPipe, buf, size);

  pipeindex = DECODEPIPEHANDLE(hPipe);
  pp = &psdata->pipetab[pipeindex];

  hevent_canread = 
    XCECreateEventA(NULL, FALSE, FALSE, pp->event_name_canread);
  hevent_canwrite = 
    XCECreateEventA(NULL, FALSE, FALSE, pp->event_name_canwrite);

  if(pp->used == 0 || pp->opencntwrite == 0)
    {
      XCETraceA("pipe_write on closed write handle");
      errno = EIO;
      return -1;
    }

  if(pp->opencntread == 0)
    {
      XCETraceA("pipe_write: no reader handle open");
      errno = EPIPE;
      SetLastError(ERROR_BROKEN_PIPE);
      return -1;
    }

  // Wait until reader consumed data...
  
  // Seems that there can be deadlocks where 2 processes
  // wait for the reader of the other... Ange-ftp writes
  // to ftp without reading output, and ftp writes before
  // reading input...
  
  // So its a bad idea to wait for the reader as long as the buffer
  // is not full... This has to be fixed...

  SYNC;
  
  if(pipe_getavail(pp) > 0)
    {
      XCETraceA("pipe_write: not empty, waiting for pipe reader (%d/%d)",
	       pp->opencntread, pp->opencntwrite);

      while(1)
	{
	  DWORD dwWait;
	  
	  dwWait = WaitForSingleObject(hevent_canwrite, 100);

	  SYNC;

	  if(dwWait == WAIT_OBJECT_0 && pipe_getavail(pp) == 0)
	    break;

	  if(dwWait != WAIT_TIMEOUT)
	    {
	      XCETraceA("error: pipe_write: dwWait = %d, avail = %d", 
		       dwWait, pipe_getavail(pp));
	      errno = EIO;
	      return -1;
	    }

	  if(pp->opencntread == 0)
	    {
	      XCETraceA("pipe_write: reader has closed pipe");
	      errno = EPIPE;
	      SetLastError(ERROR_BROKEN_PIPE);
	      return -1;
	    }
	}
    }
  else
    {
      // clear any pending events...
      WaitForSingleObject(hevent_canwrite, 0);
    }

  len = min(size, pp->size);

  memcpy(pp->buf, buf, len);
  pipe_setavail(pp, len);

  XCETraceA("%d bytes written to pipe, signalling reader", 
	   pipe_getavail(pp));

  SYNC;

  SetEvent(hevent_canread);
  Sleep(0);

  size -= len;
  written += len;

  CloseHandle(hevent_canread);
  CloseHandle(hevent_canwrite);

  return written;
}

int
pipe_read(HANDLE hPipe, unsigned char *buf, int size)
{
  HANDLE hevent_canread, hevent_canwrite;
  struct pipe_entry *pp;
  int read = 0;
  int len;
  int pipeindex;

  XCETraceA("pipe_read(%x, %x, %d)", hPipe, buf, size);

  pipeindex = DECODEPIPEHANDLE(hPipe);
  pp = &psdata->pipetab[pipeindex];

  hevent_canread = 
    XCECreateEventA(NULL, FALSE, FALSE, pp->event_name_canread);
  hevent_canwrite = 
    XCECreateEventA(NULL, FALSE, FALSE, pp->event_name_canwrite);

  if(pp->used == 0 || pp->opencntread == 0)
    {
      XCETraceA("pipe_read on closed read handle");
      return -1;
    }

  XCETraceA("pipe_read: waiting for pipe writer (%d/%d)",
	   pp->opencntread, pp->opencntwrite);

  while(1)
    {
      DWORD dwWait;

      dwWait = WaitForSingleObject(hevent_canread, 100);

      SYNC;

      if(dwWait == WAIT_OBJECT_0 && pipe_getavail(pp) != 0)
	break;

      if(dwWait == WAIT_OBJECT_0 && pipe_getavail(pp) == 0)
	{
	  XCETraceA("got event but no data ready");
	  errno = EIO;
	  return -1;
	}
      else if(dwWait != WAIT_TIMEOUT)
	{
	  XCETraceA("pipe_read: wait failed: dwWait = %d avail = %d err = %d", 
		   dwWait, pipe_getavail(pp), GetLastError());
	  errno = EIO;
	  return -1;
	}

      if(pp->opencntwrite == 0 && pipe_getavail(pp) == 0)
	{
	  XCETraceA("writer has closed pipe");
	  errno = EPIPE;
	  SetLastError(ERROR_BROKEN_PIPE);
	  // emacs seems to need 0, but tcl -1?
	  //return -1;
	  return 0;
	}
    }

  len = min(pipe_getavail(pp), size);
  memcpy(buf, pp->readp, len);
  pipe_setavail(pp, pipe_getavail(pp) - len);
  pp->readp += len;
  read += len;

  SYNC;

  // signal writer to write more...
  if(pipe_getavail(pp) == 0)
    {
      pp->readp = pp->buf;
      
      XCETraceA("pipe_reader: signalling writer, avail = %d",
	       pipe_getavail(pp));

      SetEvent(hevent_canwrite);
      Sleep(0);
    }
  else
    {
      // signal ourselves that we can read...
      SetEvent(hevent_canread);
      Sleep(0);
    }

  SYNC;

  XCETraceA("read_pipe returns %d", read);

  CloseHandle(hevent_canread);
  CloseHandle(hevent_canwrite);

  return read;
}

//////////////////////////////////////////////////////////////////////

BOOL
XCECreatePipe(PHANDLE hRead, PHANDLE hWrite, LPSECURITY_ATTRIBUTES lpsa,
	     DWORD size)
{
  int i;
  int pipeindex = -1;
  HANDLE hEvent;

  XCETraceA("CreatePipe()");

  xceshared_lock();

  for(i = 0; i < MAXPIPES; i++)
    {
      if(psdata->pipetab[i].used == 0)
	{
	  pipeindex = i;
	  break;
	}
    }

  if(pipeindex == -1)
    {
      xceshared_release();
      errno = ENOMEM;
      return FALSE;
    }

  memset(&psdata->pipetab[pipeindex], 0, sizeof(struct pipe_entry));
  psdata->pipetab[pipeindex].used = 1;
  psdata->pipetab[pipeindex].hread = MAKEPIPEREADHANDLE(pipeindex);
  psdata->pipetab[pipeindex].hwrite = MAKEPIPEWRITEHANDLE(pipeindex);
  psdata->pipetab[pipeindex].size = PIPEBUFSIZE;
  psdata->pipetab[pipeindex].readp = psdata->pipetab[pipeindex].buf;
  //psdata->pipetab[pipeindex].writep = psdata->pipetab[pipeindex].buf;
  psdata->pipetab[pipeindex].avail = 0;
  psdata->pipetab[pipeindex].opencntread = 1;
  psdata->pipetab[pipeindex].opencntwrite = 1;

  sprintf(psdata->pipetab[pipeindex].event_name_canread, 
	  "PIPE_READ_EVENT%d", pipeindex);
  sprintf(psdata->pipetab[pipeindex].event_name_canwrite, 
	  "PIPE_WRITE_EVENT%d", pipeindex);

  // TODO: These handles are never closed. They are needed
  // because read/write currently open/close handles. When
  // no handle is open, the event can get lost...

  hEvent = 
    XCECreateEventA(NULL, FALSE, FALSE, 
		    psdata->pipetab[pipeindex].event_name_canread);

  // There is currently a leak due to incorrect reference counting,
  // so we must clear all events on these handles... Else the next
  // write gets an EIO...
  WaitForSingleObject(hEvent, 0);

  hEvent = 
    XCECreateEventA(NULL, FALSE, FALSE, 
		    psdata->pipetab[pipeindex].event_name_canwrite);
  WaitForSingleObject(hEvent, 0);

  *hRead = psdata->pipetab[pipeindex].hread;
  *hWrite = psdata->pipetab[pipeindex].hwrite;

  XCETraceA("CreatePipe(): hRead = %x, hWrite = %x",
	   *hRead, *hWrite);

  xceshared_release();

  return TRUE;
}

BOOL
XCEReadFile(HANDLE hFile, LPVOID buf, DWORD dwSize, LPDWORD lpdwRead,
	    LPOVERLAPPED lpOverlapped)
{
  int res = TRUE;
  
  if(ISCONSREADHANDLE(hFile) && console_read_func)
    {
      int fd = 0;

      if((*lpdwRead = (*console_read_func)(fd, buf, dwSize)) < 0)
	{
	  SetLastError(ERROR_BROKEN_PIPE);

	  *lpdwRead = 0;

	  return FALSE;
	}
      return TRUE;
    }
  else if(ISPIPEHANDLE(hFile))
    {
      if((*lpdwRead = pipe_read(hFile, buf, dwSize)) == (DWORD) -1)
	{
	  *lpdwRead = 0;
	  return FALSE;
	}
      return TRUE;
    }

  res = ReadFile(hFile, buf, dwSize, lpdwRead, lpOverlapped);
  
  return res;
}

BOOL
XCEWriteFile(HANDLE hFile, LPCVOID buf, DWORD dwSize, LPDWORD lpdwWritten,
	    LPOVERLAPPED lpOverlapped)
{
  int res = TRUE;

  if(ISCONSWRITEHANDLE(hFile) && console_write_func)
    {
      int fd = DECODECONSHANDLE(hFile);

      if((*lpdwWritten = (*console_write_func)(fd, buf, dwSize)) < 0)
	{
	  *lpdwWritten = 0;
	  return FALSE;
	}
      return TRUE;
    }
  else if(ISPIPEHANDLE(hFile))
    {
      if((*lpdwWritten = pipe_write(hFile, buf, dwSize)) == (DWORD) -1)
	{
	  *lpdwWritten = 0;
	  return FALSE;
	}
      return TRUE;
    }

  res = WriteFile(hFile, buf, dwSize, lpdwWritten, lpOverlapped);
  
  return res;
}

BOOL
close_pipe(HANDLE handle)
{
  int pipeindex;

  pipeindex = DECODEPIPEHANDLE(handle);

  XCETraceA("close_pipe(0x%x): openread %d/openwrite %d",
	   handle,
	   psdata->pipetab[pipeindex].opencntread,
	   psdata->pipetab[pipeindex].opencntwrite);

  if(psdata->pipetab[pipeindex].used == 0)
    {
      xceshared_release();
      XCETraceA("WARNING: Closing pipe handle that is not in use");
      return FALSE;
    }

  if(ISPIPEREADHANDLE(handle))
    {
      if(psdata->pipetab[pipeindex].opencntread > 0)
	{
	  psdata->pipetab[pipeindex].opencntread--;
	}
      else
	{
	  xceshared_release();
	  XCETraceA("WARNING: Closing zero client pipe read handle");
	  return FALSE;
	}
    }
  else
    {
      if(psdata->pipetab[pipeindex].opencntwrite > 0)
	{
	  psdata->pipetab[pipeindex].opencntwrite--;
	}
      else
	{
	  xceshared_release();
	  XCETraceA("WARNING: Closing zero client pipe write handle");
	  return FALSE;
	}
    }

  if(psdata->pipetab[pipeindex].opencntread == 0 &&
     psdata->pipetab[pipeindex].opencntwrite == 0)
    {
      psdata->pipetab[pipeindex].used = 0;
    }

  xceshared_release();
  return TRUE;
}

int
pipe_increment_opencount(HANDLE hFile)
{
  int pipeindex;

  if(!ISPIPEHANDLE(hFile))
    return -1;

  pipeindex = DECODEPIPEHANDLE(hFile);

  XCETraceA("inc_open_count(0x%x): before: read %d/write %d",
	   hFile,
	   psdata->pipetab[pipeindex].opencntread,
	   psdata->pipetab[pipeindex].opencntwrite);

  if(ISPIPEHANDLE(hFile) && ISINHERIT(hFile))
    {
      // must add one for the inherited program, because
      // the caller normally closed duplicated handles...
      if(ISPIPEREADHANDLE(hFile))
	{
	  psdata->pipetab[pipeindex].opencntread++;
	}
      else
	{
	  psdata->pipetab[pipeindex].opencntwrite++;
	}
    }

  XCETraceA("inc_open_count(0x%x): after: read %d/ write %d",
	   hFile,
	   psdata->pipetab[pipeindex].opencntread,
	   psdata->pipetab[pipeindex].opencntwrite);

  return 0;
}

BOOL
XCECloseHandle(HANDLE handle)
{
  BOOL res = TRUE;

  if(ISPIPEHANDLE(handle))
    res = close_pipe(handle);
  else
    res = CloseHandle(handle);

  WCETRACE(WCE_SYNCH, "ch: %p %d", handle, res);

  return res;
}

BOOL
XCEDuplicateHandle(HANDLE hProc1, HANDLE hFile1, 
		  HANDLE hProc2, HANDLE *phFile2,
		  DWORD dwAccess, BOOL bInherit, 
		  DWORD dwOpts
		  )
{
  XCETraceA("DuplicateHandle(%x)", hFile1);

  if(hProc1 != hProc2)
    {
      XCETraceA("DuplicateHandle on different processes");
      return FALSE;
    }

  if(hProc1 != GetCurrentProcess())
    {
      XCETraceA("DuplicateHandle on different process");
      return FALSE;
    }

  if(!ISPIPEHANDLE(hFile1))
    {
      XCETraceA("DuplicateHandle on non-pipe");
      //return FALSE;
    }

  *phFile2 = MAKEINHERIT(hFile1);

  if(ISPIPEHANDLE(hFile1))
    pipe_increment_opencount(*phFile2);

  return TRUE;
}

BOOL
XCESetStdHandle(DWORD dwId, HANDLE hFile)
{
  XCETraceA("SetStdHandle(%d, %x)", dwId, hFile);

  if(hFile == NULL || hFile == INVALID_HANDLE_VALUE)
    {
      // Do nothing...
    }
  else if(!ISPIPEHANDLE(hFile))
    {
      // We just set it to invalid...
      XCETraceA("Warning: Handle is not a pipe");
      hFile = INVALID_HANDLE_VALUE;
      //return FALSE;
    }

  if(dwId == STD_INPUT_HANDLE)
    {
      xcelocal_procstart.hstdin = hFile;
    }
  else if(dwId == STD_OUTPUT_HANDLE)
    {
      xcelocal_procstart.hstdout = hFile;
    }
  if(dwId == STD_ERROR_HANDLE)
    {
      xcelocal_procstart.hstderr = hFile;
    }

  return TRUE;
}

HANDLE
XCEGetStdHandle(DWORD dwId)
{
  HANDLE hFile;

  XCETraceA("GetStdHandle(%d)", dwId);

  if(dwId == STD_INPUT_HANDLE)
    {
      hFile = xcelocal_procstart.hstdin;
    }
  else if(dwId == STD_OUTPUT_HANDLE)
    {
      hFile = xcelocal_procstart.hstdout;
    }
  else if(dwId == STD_ERROR_HANDLE)
    {
      hFile = xcelocal_procstart.hstderr;
    }
  else
    {
      hFile = INVALID_HANDLE_VALUE;
    }

  if(!ISPIPEHANDLE(hFile))
    {
      if(dwId == STD_INPUT_HANDLE && console_read_func)
	{
	  hFile = MAKECONSREADHANDLE(0);
	}
      else if(dwId == STD_OUTPUT_HANDLE && console_write_func)
	{
	  hFile = MAKECONSWRITEHANDLE(1);
	}
      else if(dwId == STD_ERROR_HANDLE && console_write_func)
	{
	  hFile = MAKECONSWRITEHANDLE(2);
	}
      else
	{
	  hFile = INVALID_HANDLE_VALUE;
	}
    }

  return hFile;
}

DWORD
XCEGetFileType(HANDLE hFile)
{
  XCETraceA("GetFileType(%x)", hFile);

  if(hFile == INVALID_HANDLE_VALUE)
    {
      return FILE_TYPE_UNKNOWN;
    }
  else if(ISPIPEHANDLE(hFile))
    {
      return FILE_TYPE_PIPE;
    }
  else if(ISCONSHANDLE(hFile))
    {
      return FILE_TYPE_CHAR;
    }

  return FILE_TYPE_DISK;
}

BOOL 
XCEPeekNamedPipe(HANDLE   hPipe, 
		 LPVOID   lpvBuffer,	
		 DWORD    cbBuffer,
		 LPDWORD  lpcbRead,
		 LPDWORD  lpcbAvail,
		 LPDWORD  lpcbMessage)
{
  struct pipe_entry *pp;
  int pipeindex;

  XCETraceA("PeekNamedPipe(%x)", hPipe);

  if(!ISPIPEHANDLE(hPipe))
    {
      return FALSE;
    }

  SYNC;

  pipeindex = DECODEPIPEHANDLE(hPipe);
  pp = &psdata->pipetab[pipeindex];

  if(pp->used == 0 || pp->opencntread == 0)
    {
      XCETraceA("peek on pipe with closed read handle");
      errno = EPIPE;
      SetLastError(ERROR_BROKEN_PIPE);
      return FALSE;
    }
  if(pp->used == 0 || pp->opencntwrite == 0)
    {
      XCETraceA("peek on pipe with closed write handle");
      errno = EPIPE;
      SetLastError(ERROR_BROKEN_PIPE);
      return FALSE;
    }

  // does not copy data yet...
  *lpcbAvail = pp->avail;

  XCETraceA("Avail: %d", *lpcbAvail);

  return TRUE;
}

int
xceshared_attach()
{
  return 0;
}

int
xceshared_detach()
{
  // TODO: We should also close the event handles we have
  // allocated in CreatePipe()
  //
  // We should also have a table of other pipes,
  // not only standard handles...

  if(ISPIPEHANDLE(XCEGetStdHandle(STD_INPUT_HANDLE)))
     XCECloseHandle(XCEGetStdHandle(STD_INPUT_HANDLE));
  if(ISPIPEHANDLE(XCEGetStdHandle(STD_OUTPUT_HANDLE)))
     XCECloseHandle(XCEGetStdHandle(STD_OUTPUT_HANDLE));
  if(ISPIPEHANDLE(XCEGetStdHandle(STD_ERROR_HANDLE)))
     XCECloseHandle(XCEGetStdHandle(STD_ERROR_HANDLE));

  return 0;
}
