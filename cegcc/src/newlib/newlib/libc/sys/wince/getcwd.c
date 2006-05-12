#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/wcebase.h>
#include <sys/wcefile.h>
#include <sys/wcetrace.h>

#include "sys/ceutil.h"

int chroot(const char *path)
{
	if (path == NULL || strlen(path) == 0) {
		errno = EFAULT;
		return -1;
	}

	WCETRACE(WCE_IO, "chroot: path \"%s\"", path);

	if (!XCESetCurrentRootDirectoryA(path))
		return -1;
	return 0;
}

int
chdir(const char *path)
{
#if 0
  struct stat st;
  char  *cp = NULL;
#endif
  char   fpath[MAX_PATH+1];

  if (path == NULL || strlen(path) == 0) {
    errno = EFAULT;
    return(-1);
  }

  WCETRACE(WCE_IO, "chdir: path \"%s\"", path);
#if 0
  fixpath(path, fpath);

  if (stat(fpath, &st) < 0) {
    errno = ENOENT;
    WCETRACE(WCE_IO, "chdir: return ENOENT");
    return(-1);
  }

  if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    WCETRACE(WCE_IO, "chdir: return ENOTDIR");
    return(-1);
  }
#endif

  if (!XCESetCurrentDirectoryA(path))
	  return -1;

  XCEGetCurrentDirectoryA(MAX_PATH+1, fpath);
  XCEToUnixPath(fpath, -1);
  WCETRACE(WCE_IO, "chdir: success, cwd is \"%s\"", fpath);
  return(0);
}

char *
getcwd(char *buf, size_t size)
{
  /* This is NOT compliant with what Linux does.. */
  if (buf == NULL || size == 0) {
	/* ### TODO malloc a buffer here. Be careful with size-1 below */
    return(NULL);
  }

  size_t len = XCEGetCurrentDirectoryA((DWORD)size, buf);

  if (len > size)
  {
	  XCEToUnixPath(buf, size-1);
	  errno = ERANGE;
	  return(NULL);
  }
  else if (!len)
  {
	  WCETRACE(WCE_IO, "getcwd WARNING: curr dir is unset");
	  return NULL;
  }
  else
  {
	  XCEToUnixPath(buf, -1);
  }
  return buf;
}
