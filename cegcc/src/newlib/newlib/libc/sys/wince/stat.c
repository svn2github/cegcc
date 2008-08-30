#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "sys/wcetrace.h"
#include "sys/io.h"

extern int __StdioInited;

int
_fstat_r(struct _reent *reent, int fd, struct stat *st) {
  BY_HANDLE_FILE_INFORMATION fi;
  SYSTEMTIME systemTime;
  FILETIME   localFileTime;
  HANDLE     hnd;
  int        permission = 0;

  WCETRACE(WCE_IO, "fstat(%d)", fd);

  FDCHECK(fd, 0);
#if 0
  static int first_time = 1;

  if ((!__StdioInited) && (fd >= 0) && (fd <= 2))
//  if (first_time)
  {
  //  first_time = 0;
    WCETRACE(WCE_IO, "fstat-ing fd = %d with stdio uninitialized", fd);
    errno = EINVAL;
    return(-1);
  }
#endif
  memset(st, 0, sizeof(struct stat));

  if (_fdtab[fd].type == IO_FILE_TYPE_CONSOLE || _fdtab[fd].type == IO_FILE_TYPE_FIFO) {
    st->st_size = 0;
    st->st_mode = S_IFCHR;
    permission = S_IREAD|S_IWRITE;

    st->st_mode |= permission | (permission >> 3) | (permission >> 6);
    return(0);
  } else if(_fdtab[fd].type == IO_FILE_TYPE_NULL) {
    st->st_size = 0;
    st->st_mode = S_IFREG;
    permission = S_IREAD|S_IWRITE;

    st->st_mode |= permission | (permission >> 3) | (permission >> 6);
    return(0);
  } else if(_fdtab[fd].type != IO_FILE_TYPE_FILE) {
    WCETRACE(WCE_IO, "fstat on invalid file type");
    errno = EINVAL;
    return(-1);
  }

  hnd = _fdtab[fd].hnd;

  memset(&fi, 0, sizeof(BY_HANDLE_FILE_INFORMATION));
  if(0==GetFileInformationByHandle(hnd, &fi))
  {
	/* mamaich: fstat on RAR archive */
	int Pos=lseek(fd,0,SEEK_CUR);
  	st->st_mode = S_IFREG;
  	st->st_size = lseek(fd,0,SEEK_END);
	lseek(fd,Pos,SEEK_SET);
    permission = S_IREAD|S_IWRITE;

    st->st_mode |= permission | (permission >> 3) | (permission >> 6);
    return 0;
  }

  st->st_size = fi.nFileSizeLow;
  st->st_mode = S_IFREG;

  if(fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    st->st_mode = S_IFDIR;

  permission |= S_IREAD;

  if(!(fi.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
    permission |= S_IWRITE;

  if(fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    permission |= S_IEXEC;

  st->st_mode |= permission | (permission >> 3) | (permission >> 6);

  FileTimeToLocalFileTime(&fi.ftLastWriteTime, &localFileTime);
  FileTimeToSystemTime(&localFileTime, &systemTime);
  
  st->st_mtime = _systotime_t(systemTime.wYear, systemTime.wMonth,
				              systemTime.wDay, systemTime.wHour, systemTime.wMinute,
				              systemTime.wSecond, -1);

  FileTimeToLocalFileTime(&fi.ftCreationTime, &localFileTime);
  FileTimeToSystemTime(&localFileTime, &systemTime);
  
  st->st_ctime = _systotime_t(systemTime.wYear, systemTime.wMonth,
				              systemTime.wDay, systemTime.wHour, systemTime.wMinute,
				              systemTime.wSecond, -1);

  FileTimeToLocalFileTime(&fi.ftLastAccessTime, &localFileTime);
  FileTimeToSystemTime(&localFileTime, &systemTime);

  st->st_atime = _systotime_t(systemTime.wYear, systemTime.wMonth,
				              systemTime.wDay, systemTime.wHour, systemTime.wMinute,
				              systemTime.wSecond, -1);

  if(st->st_atime == 0) 
    st->st_atime = st->st_mtime;
  if (st->st_ctime == 0) 
    st->st_ctime = st->st_mtime;

  if(fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    st->st_nlink = 2;
  else
    st->st_nlink = fi.nNumberOfLinks;

  /* Only low seems to be used... */
  /* printf("Low: %d High: %d\n", fi.nFileIndexLow, fi.nFileIndexHigh); */

  st->st_rdev = 1;
  st->st_ino = fi.nFileIndexLow;

  return 0;
}

int
_stat_r(struct _reent *reent, const char *path, struct stat *st)
{
  WIN32_FIND_DATAW fdw;
  HANDLE hnd;
  SYSTEMTIME systemTime;
  FILETIME localFileTime;
  int permission = 0;
  char fpath[MAX_PATH];
  wchar_t fpathw[MAX_PATH];

  WCETRACE(WCE_IO, "stat(%s)", path);

  memset(st, 0, sizeof(struct stat));

  if(stricmp(path, "con") == 0 ||
	  stricmp(path, "/dev/tty") == 0)
  {
	  st->st_size = 0;
	  st->st_mode = S_IFCHR;
	  permission = S_IREAD|S_IWRITE;

	  st->st_mode |= permission | (permission >> 3) | (permission >> 6);
	  return 0;
  }
  else if(!stricmp(path, "nul") ||
		  !stricmp(path, "nul:") ||
		  !stricmp(path, "null") ||
		  !stricmp(path, "null:") ||
		  !stricmp(path, "/dev/nul") ||
		  !stricmp(path, "/dev/null"))
  {
	  st->st_size = 0;
	  st->st_mode = S_IFREG;
	  permission = S_IREAD|S_IWRITE;

	  st->st_mode |= permission | (permission >> 3) | (permission >> 6);
	  return 0;
  }

  fixpath(path, fpath);

  /* Handle root directory as a special case */
  if (!strcmp(fpath, "\\")) {
    st->st_size = 1024;
    st->st_mode = S_IFDIR;
    permission = S_IREAD|S_IWRITE|S_IEXEC;

    st->st_mode |= permission | (permission >> 3) | (permission >> 6);
    return(0);
  }

  mbstowcs(fpathw, fpath, MAX_PATH);
  if ((hnd = FindFirstFileW(fpathw, &fdw)) == INVALID_HANDLE_VALUE) {
    DWORD dwError = GetLastError();

    if (dwError == ERROR_NO_MORE_FILES) {
      errno = ENOENT;
    } else {
     	errno = _winerr2errno(GetLastError());
    }

    WCETRACE(WCE_IO, "stat: FindFile(%s): errno = %d oserr = %d",
		           fpath, errno, GetLastError());

    return(-1);
  }

  if (fdw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    st->st_size = 1024;
  } else {
    st->st_size = fdw.nFileSizeLow;
  }

  if (fdw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    st->st_mode = S_IFDIR;
  } else {
    st->st_mode = S_IFREG;
  }

  permission |= S_IREAD;

  if (!(fdw.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
    permission |= S_IWRITE;

  if (fdw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    permission |= S_IEXEC;

  st->st_mode |= permission | (permission >> 3) | (permission >> 6);

  FileTimeToLocalFileTime(&fdw.ftLastWriteTime, &localFileTime);
  FileTimeToSystemTime(&localFileTime, &systemTime);
  
  st->st_mtime = _systotime_t(systemTime.wYear, systemTime.wMonth,
                  systemTime.wDay,	systemTime.wHour,	systemTime.wMinute,
                  systemTime.wSecond,	-1);

  FileTimeToLocalFileTime(&fdw.ftCreationTime, &localFileTime);
  FileTimeToSystemTime(&localFileTime, &systemTime);
  
  st->st_ctime = _systotime_t(systemTime.wYear, systemTime.wMonth,
				              systemTime.wDay, systemTime.wHour, systemTime.wMinute,
				              systemTime.wSecond, -1);

  FileTimeToLocalFileTime(&fdw.ftLastAccessTime, &localFileTime);
  FileTimeToSystemTime(&localFileTime, &systemTime);

  st->st_atime = _systotime_t(systemTime.wYear, systemTime.wMonth,
				              systemTime.wDay, systemTime.wHour, systemTime.wMinute,
				              systemTime.wSecond, -1);

  if(st->st_atime == 0) 
    st->st_atime = st->st_mtime;
  if (st->st_ctime == 0) 
    st->st_ctime = st->st_mtime;

  if(fdw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    st->st_nlink = 2;
  else
    st->st_nlink = 1;

  st->st_rdev = 1;
  st->st_ino = 0;

  FindClose(hnd);

  return(0);
}

int
lstat(const char *path, struct stat *st)
{
  return _stat_r(_impure_ptr, path, st);
}

int
chmod(const char *fpath, mode_t mode)
{
  int   len;
  DWORD attr;
  wchar_t fpathw[MAX_PATH];
  char  pathbuf[MAX_PATH];

  if (fpath == NULL || (len = strlen(fpath) == 0) || len > MAX_PATH-1) {
    errno = EINVAL;
    WCETRACE(WCE_IO, "chmod: called w/invalid path");
    return(-1);
  }
  WCETRACE(WCE_IO, "chmod %s %o", fpath, mode);
  fixpath(fpath, pathbuf);  

  mbstowcs(fpathw, pathbuf, MAX_PATH);
  if ((attr = GetFileAttributesW(fpathw)) == 0xFFFFFFFF)
    {
      errno =_winerr2errno(GetLastError());
      WCETRACE(WCE_IO, "chmod: GetFileAttributesW errno %d winerr %d",
               errno, GetLastError());
      return(-1);
    }

  if (mode & S_IWRITE) {
    attr &= ~(FILE_ATTRIBUTE_READONLY);
  } else {
    attr |= (FILE_ATTRIBUTE_READONLY);
  }

  WCETRACE(WCE_IO, "chmod: calling SetFileAttributesW, attr %u");
  if (SetFileAttributesW(fpathw, attr) == FALSE) {
    errno =_winerr2errno(GetLastError());
    WCETRACE(WCE_IO, "chmod: SetFileAttributesW errno %d winerr %d",
             errno, GetLastError());
    return(-1);
  }

  return(0);
}


