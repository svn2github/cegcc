#define __USE_W32_SOCKETS

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <reent.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/sysconf.h>
#include <sys/io.h>
#include <sys/fifo.h>

#include "sys/wcetrace.h"

#include <winsock2.h>

void* _fifo_alloc();

static int fdsinitialized = 0;
static CRITICAL_SECTION critsect;

/* mamaich: Used in hooking CreateFile/ReadFile/etc for transparent reading of RAR archives */
typedef HANDLE pCreateFileW(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef BOOL   pCloseHandle(HANDLE);
typedef DWORD  pSetFilePointer(HANDLE, LONG, PLONG, DWORD);
typedef BOOL   pReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL   pWriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);

extern pCreateFileW XCECreateFileW;
extern pCloseHandle XCECloseHandle;

static pCreateFileW *_CreateFileW=XCECreateFileW;
static pCloseHandle *_CloseHandle=XCECloseHandle;
static pSetFilePointer *_SetFilePointer=SetFilePointer;
static pReadFile *_ReadFile=ReadFile;
static pWriteFile *_WriteFile=WriteFile;

#if 1
typedef int (* CONSOLE_READ_FUNC)(int, unsigned char *, int);
typedef int (* CONSOLE_WRITE_FUNC)(int, const unsigned char *, int);
typedef int (* CONSOLE_IOCTL_FUNC)(int, int, void *);

HWND console_hwnd;
CONSOLE_READ_FUNC  console_read_func;
CONSOLE_WRITE_FUNC console_write_func;
CONSOLE_IOCTL_FUNC console_ioctl_func;
#endif

void SetFileFuncs(void* CF, void *SFP, void *RF, void* WF, void *CH)
{
	_CreateFileW=(pCreateFileW*)CF;
	_SetFilePointer=(pSetFilePointer*)SFP;
	_ReadFile=(pReadFile*)RF;
	_WriteFile=(pWriteFile*)WF;
	_CloseHandle=(pCloseHandle*)CH;
}

_fdent_t _fdtab[MAXFDS];

/* Prototypes from local.h that probably shouldn't be here.. */
extern int __sclose(void *);
extern _READ_WRITE_RETURN_TYPE __sread(void *, char *, int);
extern _READ_WRITE_RETURN_TYPE __swrite(void *, char const *, int);
extern fpos_t __sseek(void *, fpos_t, int);

/* Public devops for devices we support */
extern _DEVOPS _fifo_devops;

int getfiletype(int fd)
{
	FDCHECK(fd, 0);
	return _fdtab[fd].type;
}

HANDLE get_osfhandle(int fd)
{
	if(fd < 0 || fd > MAXFDS || _fdtab[fd].fd == -1)
		return INVALID_HANDLE_VALUE;
	return _fdtab[fd].hFile;
}

static void 
_initfds_p(int end)
{
  int i;

  InitializeCriticalSection(&critsect);

  EnterCriticalSection(&critsect);
  for (i = 0; i < end; i++)
  {
    memset(&_fdtab[i], 0, sizeof(_fdtab[0]));
    _fdtab[i].fd = -1;
  }
  LeaveCriticalSection(&critsect);
}

int fsync(int fd) {
  WCETRACE(WCE_IO, "syncing descriptor %d", fd);
  FDCHECK(fd, 0);
  if (FlushFileBuffers(_fdtab[fd].hnd)) {
      errno = _winerr2errno(GetLastError());
      WCETRACE(WCE_IO, "FlushFileBuffers(%d): errno=%d oserr=%d\n", fd, errno, GetLastError());
      return -1;
  }
  return 0;
}

int fdatasync(int fd) {
    return fsync(fd);
}

void
_initfds()
{
  int i;

  if (fdsinitialized)
    return;
  fdsinitialized = 1;

  _initfds_p(MAXFDS);
}

void
_initfds_std()
{
	/* stdin, stdio, stderr */
  _initfds_p(3);
}

//void
//_cleanupstdio()
//{
//	fclose(stderr);
//	fclose(stdout);
//	fclose(stdin);
//}

int __StdioInited = 0;

void
_initstdio()
{
  if(!__StdioInited)
  {
	  __StdioInited = 1;
	  _initstdfd(stdin,  0, (HANDLE)_fileno(_getstdfilex(0)), __SRD);
	  _initstdfd(stdout, 1, (HANDLE)_fileno(_getstdfilex(1)), __SWR);
	  _initstdfd(stderr, 2, (HANDLE)_fileno(_getstdfilex(2)), __SWR);
//	  atexit(_cleanupstdio);
  }
}

int
_getnewfd()
{
  int i;

  EnterCriticalSection(&critsect);
  for (i = 0; i < MAXFDS; i++) {
    if (_fdtab[i].fd == -1)	{
   	  _fdtab[i].flags = 0;
      LeaveCriticalSection(&critsect);
   	  return i;
   	}
  }
  LeaveCriticalSection(&critsect);

  WCETRACE(WCE_IO, "Out of file descriptors!");

  return(-1);
}

void
_setfd(int fd, int type, HANDLE hnd, int flags)
{
  _fdtab[fd].fd = fd;
  _fdtab[fd].type = type;
  _fdtab[fd].hnd = hnd;
  _fdtab[fd].flags = flags;
}

int
_assignfd(int type, HANDLE hnd, int flags)
{
  int fd;

  WCETRACE(WCE_IO, "_assignfd(%x)", hnd);

  if ((fd = _getnewfd()) >= 0)
    _setfd(fd, type, hnd, flags);

  WCETRACE(WCE_IO, "_assignfd returns %d", fd);
  return(fd);
}

void
_initstdfd(FILE *fp, int fd, HANDLE hnd, int flags)
{
  if (fd < 0 || fd > 2 || fp == NULL)
    return;

  _setfd(fd, IO_FILE_TYPE_CONSOLE, hnd, 0);

  WCETRACE(WCE_IO, "_initstdfd: fd %d hnd %x", fd, hnd);

  fp->_file = fd;
  fp->_flags = flags;
  fp->_cookie = (_PTR) fp;
  fp->_read = __sread;
  fp->_write = __swrite;
  fp->_seek = __sseek;
  fp->_close = __sclose;

#ifdef __SCLE
  if (__stextmode(fp->_file))
    fp->_flags |= __SCLE;
#endif
}

void
_initstdfifofd(FILE *fp, int fd, int flags)
{
  if (fd < 0 || fd > 2 || fp == NULL)
    return;

  _fdtab[fd].type = IO_FILE_TYPE_FIFO;
  _fdtab[fd].hnd = NULL;
  _fdtab[fd].flags = 0;

  WCETRACE(WCE_IO, "_initstdfifofd: fd %d fifofd %d", fd, _fdtab[fd].fd);

  fp->_file = fd;
  fp->_flags = flags;
  fp->_cookie = (_PTR) fp;
  fp->_read = __sread;
  fp->_write = __swrite;
  fp->_seek = __sseek;
  fp->_close = __sclose;

#ifdef __SCLE
  if (__stextmode(fp->_file))
    fp->_flags |= __SCLE;
#endif
}

void
_initecho(int stdinfd, int stdoutfd)
{
  _FIFOIOCXT fcxt = NULL;

  WCETRACE(WCE_IO, "_initecho: stdinfd %d stdoutfd %d", stdinfd, stdoutfd);

  if (stdinfd < 0 || stdoutfd < 0) {
    WCETRACE(WCE_IO, "_initecho: ERROR invalid fds (%d,%d) giving up", stdinfd, stdoutfd);
  }

  fcxt = (_FIFOIOCXT) _fdtab[stdinfd].cxt;
  WCETRACE(WCE_IO, "_initecho: fcxt %p", fcxt);
  fflush(stdout);

  if (fcxt == NULL) {
    WCETRACE(WCE_IO, "_initecho: ERROR fcxt is NULL");
    return;
  }

  if (_fdtab[stdoutfd].cxt == NULL) {
    WCETRACE(WCE_IO, "_initecho: ERROR stdout cxt is NULL");
    return;
  }

  fcxt->echofd = stdoutfd;
  fcxt->echocxt = _fdtab[stdoutfd].cxt;
}

void *
_getiocxt(int fd)
{
  if (fd < 0 || fd > MAXFDS - 1)
    return(NULL);
  return(_fdtab[fd].cxt);
}

void
_ioatexit(void)
{
  int i;

  WCETRACE(WCE_IO, "_ioatexit: STARTED");
  for (i = 0; i < MAXFDS; i++) {
    if (_fdtab[i].fd != -1)	{
      if (_fdtab[i].type == IO_FILE_TYPE_FIFO) {
        _close_r(NULL, i);
   	  }
    }
  }
}
  
int
_open_r(struct _reent *reent, const char *path, int flags, int mode)
{
  wchar_t wpath[MAX_PATH];
  char pathbuf[MAX_PATH];
  HANDLE hnd = NULL;
  DWORD fileaccess;
  DWORD fileshare; 
  DWORD filecreate;
  DWORD fileattrib;
  void *cxt;
  int fd;

  WCETRACE(WCE_IO, "open(%s, %x, %o)", path, flags, mode);

  _initfds();

  if (!strncmp("fifo", path, 4)) {
	  fd = _assignfd(IO_FILE_TYPE_FIFO, NULL, 0);

	  if (fd < 0) {
		  errno = ENMFILE;
		  return(-1);
	  }

	  _fdtab[fd].devops = _fifo_devops;
	  _fdtab[fd].cxt = cxt = _fifo_alloc();
	  if ((_fdtab[fd].fd = _fdtab[fd].devops->open_r(reent, path, flags, mode, cxt)) == -1) {
		  WCETRACE(WCE_IO, "FIFO open fails, errno %d", errno);
		  _fdtab[fd].fd = -1;
		  return(-1);
	  }
  }
  else if(!strncmp(path, "nul", 3) ||
		  !strncmp(path, "nul:", 4) ||
		  !strncmp(path, "null:", 5) ||
		  !strncmp(path, "/dev/nul", 8) ||
		  !strncmp(path, "/dev/null", 9))
  {
	  fd = _assignfd(IO_FILE_TYPE_NULL, (HANDLE) -1, 0);
	  if (fd < 0) {
		  errno = ENMFILE;
		  return(-1);
	  }
	  _fdtab[fd].devops = NULL;
	  _fdtab[fd].cxt = NULL;
  } else {
    if (strlen(path) >= MAX_PATH) {
      WCETRACE(WCE_IO, "open fails, invalid path\n");
      return(-1);
    }

    fixpath(path, pathbuf);
    mbstowcs(wpath, pathbuf, strlen(pathbuf) + 1);

    fileshare = FILE_SHARE_READ|FILE_SHARE_WRITE;
    fileattrib = FILE_ATTRIBUTE_NORMAL;

    switch (flags & (O_RDONLY | O_WRONLY | O_RDWR)) {
    case O_RDONLY:              /* read access */
      fileaccess = GENERIC_READ;
      break;
    case O_WRONLY:              /* write access */
      fileaccess = GENERIC_WRITE;
      break;
    case O_RDWR:                /* read and write access */
      fileaccess = GENERIC_READ | GENERIC_WRITE;
      break;
    default:                    /* error, bad flags */
      errno = EINVAL;
      return -1;
    }

    switch (flags & (O_CREAT | O_EXCL | O_TRUNC)) {
    case 0:
    case O_EXCL:                /* ignore EXCL w/o CREAT */
      filecreate = OPEN_EXISTING;
      break;
    case O_CREAT:
      filecreate = OPEN_ALWAYS;
      break;
    case O_CREAT | O_EXCL:
    case O_CREAT | O_TRUNC | O_EXCL:
      filecreate = CREATE_NEW;
      break;

    case O_TRUNC:
    case O_TRUNC | O_EXCL:      /* ignore EXCL w/o CREAT */
      filecreate = TRUNCATE_EXISTING;
      break;
    case O_CREAT | O_TRUNC:
      filecreate = CREATE_ALWAYS;
      break;
    default:
      /* this can't happen ... all cases are covered */
      errno = EINVAL;
      return(-1);
    }

    if ((hnd = _CreateFileW(wpath, fileaccess, fileshare, NULL, filecreate,
                           fileattrib, NULL)) == INVALID_HANDLE_VALUE) {
      errno = _winerr2errno(GetLastError());
      WCETRACE(WCE_IO, "_CreateFile(%s): errno=%d oserr=%d\n", pathbuf, errno, GetLastError());
      return(-1);
    }

    fd = _assignfd(IO_FILE_TYPE_FILE, hnd, 0);

    if (fd < 0) {
      errno = ENMFILE;
      return(-1);
    }
    _fdtab[fd].devops = NULL;
    _fdtab[fd].cxt = NULL;

    if (flags & O_APPEND) {
      _SetFilePointer(hnd, 0, NULL, FILE_END);
    }
  }

  WCETRACE(WCE_IO, "open returns %d fd %d cxt %p (hnd %x)", fd, _fdtab[fd].fd, cxt, hnd);
  return fd;
}

int
_close_r(struct _reent *reent, int fd)
{
  WCETRACE(WCE_IO, "close(%d)", fd);
  WCETRACE(WCE_IO, "close: fd %d type %d flags %x hnd %p cxt %p", _fdtab[fd].fd,
           _fdtab[fd].type, _fdtab[fd].flags, _fdtab[fd].hnd, _fdtab[fd].cxt);

  EnterCriticalSection(&critsect);
  FDCHECK(fd, &critsect);

  if (_fdtab[fd].devops == NULL) {
    if (_fdtab[fd].type == IO_FILE_TYPE_FILE) {
      _CloseHandle(_fdtab[fd].hnd);
    } else if (_fdtab[fd].type == IO_FILE_TYPE_SOCKET) {
      closesocket(fd);
    } else if(_fdtab[fd].type == IO_FILE_TYPE_NULL) {
    }
  } else {
    WCETRACE(WCE_IO, "close: doing device-specific close for fd %d", _fdtab[fd].fd);
    _fdtab[fd].devops->close_r(reent, _fdtab[fd].fd, _fdtab[fd].cxt);
    if (_fdtab[fd].cxt != NULL) {
      free(_fdtab[fd].cxt);
      _fdtab[fd].cxt = NULL;
    }
  }

  /* IMPORTANT - reset fd fields here */
  memset(&_fdtab[fd], 0, sizeof(_fdent_t));
  _fdtab[fd].fd = -1;
  LeaveCriticalSection(&critsect);

  return(0);
}

_ssize_t
_read_r(struct _reent *reent, int fd, void *buf, size_t count)
{
  int nread;
  int error;

  WCETRACE(WCE_IO, "read(fd = %d, count = %d, hnd %x)", fd, count, _fdtab[fd].hnd);

  if ((!__StdioInited) && (fd >= 0) && (fd <= 2))
  {
    WCETRACE(WCE_IO, "read from fd = %d with stdio uninitialized", fd);
	return count;
  }

  FDCHECK(fd, 0);

  if (_fdtab[fd].devops == NULL) {
    if (_fdtab[fd].type == IO_FILE_TYPE_FILE || _fdtab[fd].type == IO_FILE_TYPE_CONSOLE) {
      if (_ReadFile(_fdtab[fd].hnd, buf, count, (DWORD *)&nread, NULL) == FALSE) {
        WCETRACE(WCE_IO, "_ReadFile: %d", GetLastError());
        errno = EIO;
        return(-1);
      }
    } else if (_fdtab[fd].type == IO_FILE_TYPE_SOCKET) {
      if ((nread = recv(fd, buf, count, 0)) == SOCKET_ERROR) {
        /* error = WSAGetLastError(); */ 
		error = 1;
        WCETRACE(WCE_IO, "read: recv failed %d\n", error);
        if (error == WSAEWOULDBLOCK) {
          errno = EAGAIN;
          return(-1);
        }

        errno = _winerr2errno(error);
        return(-1);
      }
    } else if (_fdtab[fd].type == IO_FILE_TYPE_NULL) {
      WCETRACE(WCE_IO, "warning - read called w/IO_FILE_TYPE_NULL");
      nread = 0;
    }
  } else {
    nread = _fdtab[fd].devops->read_r(reent, _fdtab[fd].fd, buf, count, _fdtab[fd].cxt);
  }

  return(nread);
}

_ssize_t
_write_r(struct _reent *reent, int fd, const void *buf, size_t count){
  int nwritten = 0;
  int werr;

  WCETRACE(WCE_IO, "write(%d, %d, %x)", fd, count, _fdtab[fd].hnd);
  EnterCriticalSection(&critsect);

#if 1
  if (fd == 2 || fd == 1)
  {
	  const char* out = fd == 2?"stderr: ":"stdout: ";
    WCETRACE(WCE_IO, "%s : %s", out, buf);
  }
#endif

  /* until we can call console stuff inside the PE loader */
  if ((!__StdioInited) && (fd >= 0) && (fd <= 2))
  {
    WCETRACE(WCE_IO, "write to fd = %d with stdio uninitialized", fd);
	LeaveCriticalSection(&critsect);    
	return count;
  }

  if (_fdtab[fd].devops == NULL) {
    if (_fdtab[fd].type == IO_FILE_TYPE_FILE || _fdtab[fd].type == IO_FILE_TYPE_CONSOLE) {
      if (_WriteFile(_fdtab[fd].hnd, buf, count, (DWORD *)&nwritten, NULL) == FALSE) {
		if ((fd == 1 || fd == 2) && (_fdtab[fd].hnd == (HANDLE)-1))
		{
			/* ignore writting errors to stdout and stderr. happens when we don't have a console installed */ 
			/* ### TODO replace this with something better */
			nwritten = count;
		}
		else
		{
			WCETRACE(WCE_IO, "_WriteFile: hnd %x error %d\n", _fdtab[fd].hnd, GetLastError());
			errno = EIO;
        LeaveCriticalSection(&critsect);
	        return(-1);
		}
      }
    } else if (_fdtab[fd].type == IO_FILE_TYPE_SOCKET) {
      if ((nwritten = (int)send(fd, buf, count, 0)) == SOCKET_ERROR) {
        /* werr = WSAGetLastError(); */ 
        werr = 1;
        WCETRACE(WCE_IO, "send: sock %d error %d",  _fdtab[fd].hnd, werr);
        errno = _winerr2errno(werr);
        LeaveCriticalSection(&critsect);
        return(-1);
      }
    } else if (_fdtab[fd].type == IO_FILE_TYPE_NULL) {
		// pretty normal, no?
//      WCETRACE(WCE_IO, "warning - write called w/IO_FILE_TYPE_NULL");
      nwritten = count;
    }
  } else {
    nwritten = _fdtab[fd].devops->write_r(reent, _fdtab[fd].fd, buf, count, _fdtab[fd].cxt);
  }

  LeaveCriticalSection(&critsect);
  return nwritten;
}

off_t
_lseek_r(struct _reent *reent, int fd, off_t offset, int whence) {
  off_t newpos;
  int method;
  WCETRACE(WCE_IO, "lseek(%d, %d, %d)", fd, offset, whence);

  FDCHECK(fd, 0);

  if (_fdtab[fd].devops == NULL) {
    switch (whence) {
    case SEEK_SET:
      method = FILE_BEGIN;
      break;
    case SEEK_CUR:
      method = FILE_CURRENT;
      break;
    case SEEK_END:
      method = FILE_END;
      break;
    default:
      method = FILE_BEGIN;
    }

    if (_fdtab[fd].type == IO_FILE_TYPE_FILE) {
      if ((newpos = _SetFilePointer(_fdtab[fd].hnd, (LONG)offset, NULL, (DWORD)method)) == -1) {
        WCETRACE(WCE_IO, "_SetFilePointer(%x): error %d", _fdtab[fd].hnd, GetLastError());
        errno = EIO;
        newpos = -1;
      } 
    } else {
      errno = EINVAL;
      newpos = -1;
    }
  } else {
    newpos = _fdtab[fd].devops->lseek_r(reent, _fdtab[fd].fd, offset, whence, _fdtab[fd].cxt);
  }

  WCETRACE(WCE_IO, "lseek returns %d", newpos);
  return(newpos);
}

BOOL XCECopyFileW(
  LPCWSTR lpExistingFileName, 
  LPCWSTR lpNewFileName, 
  BOOL bFailIfExists 
); 

int
_link_r(struct _reent *reent, const char *old, const char *new)
{
  wchar_t wpathOld[MAX_PATH];
  wchar_t wpathNew[MAX_PATH];
  char pathOld[MAX_PATH];
  char pathNew[MAX_PATH];
  fixpath(old, pathOld);
  fixpath(new, pathNew);
  mbstowcs(wpathOld, pathOld, MAX_PATH);
  mbstowcs(wpathNew, pathNew, MAX_PATH);
  if(0==XCECopyFileW(wpathOld,wpathNew,FALSE))
  {
//    printf("failed rename '%s' to '%s'\n",pathOld,pathNew);
  	errno = _winerr2errno(GetLastError());
  	return(-1);
  }
  return 0;
}

int
_unlink_r(struct _reent *reent, const char *path)
{
  wchar_t pathw[MAX_PATH];
  char pathbuf[MAX_PATH];
  BOOL res;

  if (path == NULL)
    return(-1);

  fixpath(path, pathbuf);
  mbstowcs(pathw, pathbuf, MAX_PATH);
  res = DeleteFileW(pathw);

  if (res == FALSE) {
    errno = _winerr2errno(GetLastError());
    return(-1);
  }

  return(0);
}

static int
_fd_to_socket(struct fd_set *set, int *hndmap)
{
  int i;

  if (set == NULL)
    return(0);

  for (i = 0; i < set->fd_count; i++) {
    int fd = (int)set->fd_array[i];
    FDCHECK(fd, 0);

    /* On WINCE, only IO_FILE_TYPE_SOCKET is handled */
    if (_fdtab[fd].type == IO_FILE_TYPE_SOCKET) {
      SOCKET s = _fdtab[fd].sock;
      hndmap[fd] = s;
      WCETRACE(WCE_IO, "_fd_to_socket: fd = %d, handle = %d", fd, s);
      set->fd_array[i] = s;
    } else {
      WCETRACE(WCE_IO, "_fd_to_socket: fd = %d is not a socket", fd);
    }
  }
  return(0);
}

static void
_socket_to_fd(fd_set *set, int *hndmap)
{
  int i, j, fd;

  if (set == NULL)
    return;

  if (set->fd_count > MAXFDS) {
    WCETRACE(WCE_IO, "select (_socket_to_fd) ERROR fd_count > MAXFDS (%d > %d)",
             set->fd_count, MAXFDS);
  }

  for (i = 0; i < set->fd_count; i++) {
    fd = -1;
    /* Hunt for matching hnd in the hndmap */    
    for (j = 0; j < MAXFDS; j++) {
      if (hndmap[j] == set->fd_array[i]) {
        fd = j;
        break;
      }
    }

    if (fd < 0 || fd > MAXFDS) {
      WCETRACE(WCE_IO, "_socket_to_fd: ERROR weird fd %d", fd);
    } else {
      set->fd_array[i] = fd;
    }
  }
}

#if 0
int
select(int n, fd_set *rfds, fd_set* wfds, fd_set* xfds, struct timeval *timeout)
{
  int i;
  int status;
  fd_set r, w, x;
  int werr, hndmap[MAXFDS];
  SOCKET s;

  WCETRACE(WCE_IO, "select(%d, %p, %p, %p, %p)", n, rfds, wfds, xfds, timeout);
  if (timeout != NULL) {
    WCETRACE(WCE_IO, "select: timeout {%d,%d}", timeout->tv_sec, timeout->tv_usec);
  }

  /* Initialze hndmap - this is the SOCKET->fd mapping for use later */
  for (i = 0; i < MAXFDS; i++)
    hndmap[i] = -1;

  if (rfds != NULL) {
    if (_fd_to_socket(rfds, hndmap) < 0) {
      WCETRACE(WCE_IO, "select: bad rfds");
      return(-1);
    }
  }

  if (wfds != NULL) {
    if (_fd_to_socket(wfds, hndmap) < 0) {
      WCETRACE(WCE_IO, "select: bad wfds");
      return(-1);
    }
  }

  if (xfds != NULL) {
    if (_fd_to_socket(xfds, hndmap) < 0) {
      WCETRACE(WCE_IO, "select: bad xfds");
      return(-1);
    }
  }

  status = __MS_select(n, rfds, wfds, xfds, timeout);
  WCETRACE(WCE_IO, "select: returns %d errno %d", status, errno);

  /* Give up here if there is an select error */
  if (status == SOCKET_ERROR) {
    int werr = XCEWSAGetLastError();
    errno = _winerr2errno(werr);
    return(-1);
  }

  /* Finally we must translate socket descriptors back to fds */
  if (rfds != NULL)
    _socket_to_fd(rfds, hndmap);

  if (wfds != NULL) 
    _socket_to_fd(wfds, hndmap);

  if (xfds != NULL) 
    _socket_to_fd(xfds, hndmap);

  return(status);
}
#endif

int
ioctl(int fd, unsigned int request, void *arg)
{
  DWORD high, low;
  int length, pos, avail, *iptr;
  int werr;
  SOCKET s;

  WCETRACE(WCE_IO, "ioctl(%d, %p %p)", fd, request, arg);

  FDCHECK(fd, 0);

  if (_fdtab[fd].devops == NULL) {
    if (_fdtab[fd].type == IO_FILE_TYPE_FILE) {
      switch (request) {
      case FIONREAD:
        if (arg == NULL) {
          errno = EINVAL;
          return(-1);
        }
        low = GetFileSize(_fdtab[fd].hnd, &high);

        /* FIXME: Error checking */
        length = (int)(((long long)high) << 32L) | (long long)low;
        WCETRACE(WCE_IO, "ioctl(%d (FIONREAD) length %d", fd, length);

        low = _SetFilePointer(_fdtab[fd].hnd, 0, (PLONG)&high, FILE_CURRENT);
        if ((low == 0xffffffff) && (GetLastError() != NO_ERROR)) {
          errno = EBADF;
          return(-1);
        }

        pos = (int)(((long long)high) << 32L) | (long long)low;
        avail = length - pos;
        WCETRACE(WCE_IO, "ioctl(%d (FIONREAD) pos %d avail %d", fd, pos, avail);
        iptr = (int *)arg;
       *iptr = (avail > 0) ? avail : 0;
        return(0);
      default:
        errno = ENOSYS;
        return(-1);
      }
    } else if (_fdtab[fd].type == IO_FILE_TYPE_SOCKET) {
      s = _fdtab[fd].sock;
      WCETRACE(WCE_IO, "ioctl: doing ioctlsocket w/0x%x (fd %d hnd %x)", request, fd, s);
      if (ioctlsocket(fd, request, arg) == SOCKET_ERROR) {
        /* werr = WSAGetLastError(); */ 
		werr = 1;
        errno =  _winerr2errno(werr);
        return(-1);
      }
    } else if(_fdtab[fd].type == IO_FILE_TYPE_NULL) {
      errno = ENOSYS;
      return(-1);
    }
  } else {
    return(_fdtab[fd].devops->ioctl_r(NULL, _fdtab[fd].fd, request, _fdtab[fd].cxt, arg));
  }

  return(0);
}

int
_getpid_r(struct _reent *reent)
{
  int pid;

  pid = GetCurrentProcessId();
//  return pid & 0x7FFFFFFF; // pedro: Reiner, what is the rationale for this in celib?
  return pid;
}

int
isatty(int fd)
{
  WCETRACE(WCE_IO, "isatty(%d)", fd);

  if (!fdsinitialized)
    return(FALSE);

  if (_fdtab[fd].type == IO_FILE_TYPE_CONSOLE ||
      _fdtab[fd].type == IO_FILE_TYPE_FIFO) {
    WCETRACE(WCE_IO, "isatty(%d): yes", fd);
    return(TRUE);
  }

  WCETRACE(WCE_IO, "isatty(%d): no", fd);
  return(FALSE);
}

int
ftruncate(int fd, off_t size)
{
  DWORD newpos;

  FDCHECK(fd, 0);

  if (_fdtab[fd].type != IO_FILE_TYPE_FILE) {
    errno = EBADF;
    return -1;
  }

  if ((newpos = _SetFilePointer(_fdtab[fd].hnd, size, NULL, 
			       FILE_BEGIN)) == -1)
    {
      errno = _winerr2errno(GetLastError());
      return -1;
    }

  if(!SetEndOfFile(_fdtab[fd].hnd))
    {
      errno = _winerr2errno(GetLastError());
      return -1;
    }

  return 0;
}

// TODO: The fd entry is simply copied. When one of the files
// is closed, the handle is also closed! There should be a 
// refcount on the handles! That would require another table.

// TODO: Consider using DuplicateHandle here.

int
dup(int fd)
{
  int newfd;


  newfd = _getnewfd();

  if(newfd >= 0)
    memcpy(&_fdtab[newfd], &_fdtab[fd], sizeof(_fdent_t));

  return newfd;
}

int
dup2(int fd1, int fd2)
{
  if(fd2 < 0 || fd2 >= MAXFDS);
    {
      errno = EBADF;
      return -1;
    }

  if(_fdtab[fd2].fd != -1)
    close(fd2);

  memcpy(&_fdtab[fd2], &_fdtab[fd1], sizeof(_fdent_t));

  return fd2;
}
