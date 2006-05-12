#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <errno.h>

#include "sys/wcebase.h"
#include "sys/wceerror.h"
#include "sys/wcefile.h"
#include "sys/wcetrace.h"
#include "sys/io.h"

int
utime(const char *fname, struct utimbuf *times)
{
  int fd;
  int retval;
  if ((fd = open(fname, O_RDWR)) < 0)
    return (-1);
  retval = futime(fd, times);
  close(fd);
  return(retval);
}

int
futime(int fd, struct utimbuf *times)
{
  struct tm *tmb;
  SYSTEMTIME systemTime;
  FILETIME localFileTime;
  FILETIME lastWriteTime;
  FILETIME lastAccessTime;
  struct utimbuf deftimes;

  if (times == NULL) {
    time(&deftimes.modtime);
    deftimes.actime = deftimes.modtime;
    times = &deftimes;
  }

  if ((tmb = localtime(&times->modtime)) == NULL) {
    errno = EINVAL;
    return (-1);
  }

  systemTime.wYear = (WORD) (tmb->tm_year + 1900);
  systemTime.wMonth = (WORD) (tmb->tm_mon + 1);
  systemTime.wDay = (WORD) (tmb->tm_mday);
  systemTime.wHour = (WORD) (tmb->tm_hour);
  systemTime.wMinute = (WORD) (tmb->tm_min);
  systemTime.wSecond = (WORD) (tmb->tm_sec);
  systemTime.wMilliseconds = 0;

  if (!SystemTimeToFileTime(&systemTime, &localFileTime)
      || !LocalFileTimeToFileTime(&localFileTime, &lastWriteTime)) {
    errno = EINVAL;
    return(-1);
  }

  if ((tmb = localtime(&times->actime)) == NULL) {
    errno = EINVAL;
    return (-1);
  }

  systemTime.wYear = (WORD) (tmb->tm_year + 1900);
  systemTime.wMonth = (WORD) (tmb->tm_mon + 1);
  systemTime.wDay = (WORD) (tmb->tm_mday);
  systemTime.wHour = (WORD) (tmb->tm_hour);
  systemTime.wMinute = (WORD) (tmb->tm_min);
  systemTime.wSecond = (WORD) (tmb->tm_sec);
  systemTime.wMilliseconds = 0;

  if (!SystemTimeToFileTime(&systemTime, &localFileTime)
      || !LocalFileTimeToFileTime(&localFileTime, &lastAccessTime)) {
    errno = EINVAL;
    return(-1);
  }

  if (!SetFileTime(_fdtab[fd].hnd, NULL, &lastAccessTime, &lastWriteTime)) {
    errno = EINVAL;
    return(-1);
  }
  return (0);
}
