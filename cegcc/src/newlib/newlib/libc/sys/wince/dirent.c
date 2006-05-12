#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>

#include <sys/wcebase.h>
#include <sys/wceerror.h>
#include <sys/wcefile.h>
#include <sys/wcetrace.h>
#include <sys/fixpath.h>
#include <sys/dirent.h>

#define MAXPATHLEN 255

static struct dirent dir_static;

#define IS_DIRECTORY_SEP(x) (x == '\\' || x == '/')

DIR *
opendir(const char *path)
{
  DIR *dirp;
  struct stat statbuf;
  char fpath[MAXPATHLEN];
  char *p, *d;

  WCETRACE(WCE_IO, "opendir(%s)", path);

  /* Current directory implementation - TBD */
  if (strcmp(path, ".") == 0)
  {
    char fpath2[MAXPATHLEN];
    XCEGetCurrentDirectoryA(sizeof(fpath2), fpath2);
	fixpath(fpath2, fpath);
  }
  else
	fixpath(path, fpath);

  WCETRACE(WCE_IO, "opendir2(%s)", fpath);

  if (stat(fpath, &statbuf) < 0)
    return(NULL);

  if (!(statbuf.st_mode & S_IFDIR))
    return(NULL);

  if (!(dirp = (DIR *) malloc(sizeof(DIR)))) {
    errno = ENOMEM;
    return NULL;
  }

  dirp->dd_handle = INVALID_HANDLE_VALUE;
  dirp->dd_isfat = 1;
  dirp->dd_fd = 0;
  dirp->dd_loc = 0;
  dirp->dd_size = 0;

  strcpy(dirp->dd_path, fpath);

  return(dirp);
}

int
closedir(DIR *dirp)
{
  BOOL retval = FALSE;

  if (dirp == NULL) {
    errno = EBADF;
    return(-1);
  }

  if (dirp->dd_handle != INVALID_HANDLE_VALUE) {
    retval = FindClose((HANDLE)dirp->dd_handle);
  }

  if (retval) {
    WCETRACE(WCE_IO, "closedir: FindClose succeeded");
    free((char *) dirp);
    return(0);
  } else {
    WCETRACE(WCE_IO, "closedir: FindClose FAILED");
    errno = EBADF;
    free((char *) dirp);
    return(-1);
  }
}

void
rewinddir(DIR *dirp)
{
  BOOL retval = FALSE;

  if (dirp != NULL) {
    /* To do "rewind": close the find but do not deallocate */
    if (dirp->dd_handle != INVALID_HANDLE_VALUE) {
      retval = FindClose((HANDLE)dirp->dd_handle);
    }
    dirp->dd_handle = INVALID_HANDLE_VALUE;
  }
}

struct dirent *
readdir(DIR *dirp)
{
  WIN32_FIND_DATAW fdw;
  char buf[MAX_PATH];

  if (dirp->dd_loc == 0) {
    mbstowcs(fdw.cFileName, ".", MAX_PATH);
  } else if(dirp->dd_loc == 1) {
    mbstowcs(fdw.cFileName, "..", MAX_PATH);
  } else if (dirp->dd_handle == INVALID_HANDLE_VALUE) {
    wchar_t fpathw[MAXNAMLEN];
    char fpath[MAXNAMLEN];
    int ln;

    fixpath(dirp->dd_path, fpath);

    ln = strlen(fpath) - 1;
    if (!IS_DIRECTORY_SEP(fpath[ln]))
      strcat(fpath, "\\");
    strcat(fpath, "*");

    WCETRACE(WCE_IO, "FindFirstFile: %s\n", fpath);

    mbstowcs(fpathw, fpath, MAXNAMLEN);
    memset(&fdw, 0, sizeof(WIN32_FIND_DATAW));
    dirp->dd_handle = (HANDLE)FindFirstFileW(fpathw, &fdw);

    if (dirp->dd_handle == INVALID_HANDLE_VALUE) {
      WCETRACE(WCE_IO, "readdir: FindFirstFileW failed for \"%s\"", fpath);
      return(NULL);
    }
  } else {
    if (!FindNextFileW((HANDLE)dirp->dd_handle, &fdw))
      return NULL;
  }
  
  dir_static.d_ino = 1;
  dirp->dd_loc++;

  dir_static.d_reclen = sizeof(struct dirent) - MAXNAMLEN + 3 +
    dir_static.d_namlen - dir_static.d_namlen % 4;
  
  wcstombs(buf, fdw.cFileName, MAX_PATH);
  dir_static.d_namlen = strlen(buf);
  strcpy(dir_static.d_name, buf);

  return &dir_static;
}

int
readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result)
{
  WIN32_FIND_DATAW fdw;
  char buf[MAX_PATH];

  WCETRACE(WCE_IO, "readdir_r: called 0x%p 0x%p 0x%p");

  if (dirp == NULL || entry == NULL || result == NULL) {
    errno = EINVAL;
    return(-1);
  }

  if (dirp->dd_handle == INVALID_HANDLE_VALUE) {
    wchar_t fpathw[MAXNAMLEN];
    char fpath[MAXNAMLEN];
    int ln;

    fixpath(dirp->dd_path, fpath);

    ln = strlen(fpath) - 1;
    if (!IS_DIRECTORY_SEP(fpath[ln]))
      strcat(fpath, "\\");
    strcat(fpath, "*");

    WCETRACE(WCE_IO, "readdir_r: FindFirstFile: %s", fpath);

    mbstowcs(fpathw, fpath, MAXNAMLEN);
    memset(&fdw, 0, sizeof(WIN32_FIND_DATAW));
    dirp->dd_handle = (HANDLE)FindFirstFileW(fpathw, &fdw);

    if (dirp->dd_handle == INVALID_HANDLE_VALUE) {
      WCETRACE(WCE_IO, "readdir: FindFirstFileW failed for \"%s\"", fpath);
     *result = NULL;
      return(0);
    }
  } else {
    if (!FindNextFileW((HANDLE)dirp->dd_handle, &fdw)) {
      WCETRACE(WCE_IO, "readdir_r: FindNextFileW failed");
     *result = NULL;
      return(0);  
    }
  }
  
  entry->d_ino = 1;
  dirp->dd_loc++;

  entry->d_reclen = sizeof(struct dirent) - MAXNAMLEN + 3 +
    entry->d_namlen - entry->d_namlen % 4;
  
  wcstombs(buf, fdw.cFileName, MAX_PATH);
  entry->d_namlen = strlen(buf);
  strcpy(entry->d_name, buf);
  WCETRACE(WCE_IO, "readdir_r: entry name \"%s\"", entry->d_name);

 *result = entry;
  return(0);
}
