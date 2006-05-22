#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <setjmp.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "sys/wcetrace.h"

int __umask;
int __IsForkChild=0;
static jmp_buf __ForkJmpBuf;
static int __SpawnedPid=-1;

#if 0
int
access(const char *fname, int mode)
{
  struct stat st;

  if (fname == NULL)
    return(-1);

  if (stat(fname, &st) < 0)
    return(-1);
 
  /* Check for writability */
  if (mode & W_OK) {
    if (!(st.st_mode & S_IWRITE)) {
      errno = EACCES;
      return(-1);
    }
  }

  return(0);
}

#endif

mode_t
umask(mode_t mask)
{
  int oldumask = __umask;

  __umask = mask;

  return(oldumask);
}

int
mkdir(const char *dirname, mode_t mode)
{
  wchar_t dirnamew[MAX_PATH];
  char    pathbuf[MAX_PATH];
  BOOL    res;

  fixpath(dirname, pathbuf);
  mbstowcs(dirnamew, pathbuf, MAX_PATH);
  res = CreateDirectoryW(dirnamew, NULL);

  if (res != TRUE) {
      errno =_winerr2errno(GetLastError());
      return(-1);
    }

  return(0);
}

int
rmdir(const char *dirname)
{
  wchar_t dirnamew[MAX_PATH];
  char    pathbuf[MAX_PATH];
  BOOL res;

  if (dirname == NULL)
    return(-1);

  fixpath(dirname, pathbuf);
  mbstowcs(dirnamew, pathbuf, MAX_PATH);
  WCETRACE(WCE_IO, "rmdir: path is \"%s\"", pathbuf);
  res = RemoveDirectoryW(dirnamew);
  WCETRACE(WCE_IO, "RemoveDirectory returns %d, oserr %d", res, GetLastError());

  if (res != TRUE) {
    errno =_winerr2errno(GetLastError());
    return(-1);
  }

  return(0);
}

int
_rename(const char *old, const char *new)
{
  struct stat st;
  wchar_t oldw[MAX_PATH], neww[MAX_PATH];
  BOOL res;

  if (old == NULL || new == NULL)
    return(-1);

  if (stat(new, &st) == 0) {
    if (remove(new) < 0) {
	     return(-1);
    }
  }

  mbstowcs(neww, new, MAX_PATH);
  mbstowcs(oldw, old, MAX_PATH);
  res = MoveFileW(oldw, neww);

  if (res != TRUE) {
    errno =_winerr2errno(GetLastError());
    return(-1);
  }

  return(0);
}

int
_remove(const char *path)
{
  wchar_t pathw[MAX_PATH];
  char    pathbuf[MAX_PATH];
  BOOL res;

  if (path == NULL)
    return(-1);

  fixpath(path, pathbuf);
  mbstowcs(pathw, pathbuf, MAX_PATH);
  res = DeleteFileW(pathw);

  if (res != TRUE) {
    errno = _winerr2errno(GetLastError());
    return(-1);
  }

  return(0);
}


int 
chown(const char *path, uid_t owner, gid_t group)
{
  return(0);
}

int
fchown(int fd, uid_t owner, gid_t group)
{
  return(0);
}

int
mknod(const char *pathname, mode_t mode, dev_t dev)
{
  errno = ENOTSUP;
  return(-1);
}

int 
symlink(const char *oldpath, const char *newpath)
{
  errno = ENOTSUP;
  return(-1);
}

int
readlink(const char *path, char *buf, size_t bufsiz)
{
  errno = ENOTSUP;
  return(-1);
}

unsigned
alarm(unsigned secs)
{
  errno = ENOTSUP;
  return(-1);
}

pid_t
_vfork_r()
{
	printf("Forking...\n");
  if(__IsForkChild)	// do not allow child to fork itself
  {
    errno = ENOTSUP;
    return(-1);
  }

  if(setjmp(__ForkJmpBuf))
  {
    puts("Returned to parent...");
    return __SpawnedPid;
  } 
  else
  {
    puts("Inside child...");
    __IsForkChild=1;
  }
  return 0;
}

void _done_exec(pid_t pid)
{
	__SpawnedPid=pid;
	__IsForkChild=0;
	longjmp(__ForkJmpBuf,1);
}
