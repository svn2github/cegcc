#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sys/wcetrace.h"

#include "sys/ceutil.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

    int alloced = 0;

  // realize glibc behavior to allocate buffer if the buf is 0.
  // This is also a POSIX.1-2001 extension

  if (!buf) {
    if (!size) {
        size = XCEGetCurrentDirectoryW(0, 0); // fastest way to get size
        if (!size) {
            WCETRACE(WCE_IO, "getcwd WARNING: curr dir is unset");
            // ENOENT The current working directory has been unlinked.
            errno = ENOENT;
            return NULL;
        }
    }

    buf = malloc(size);
    if (!buf) {
        errno = ENOMEM;
        return NULL;
    }
    alloced = 1;

  } else if (!size) {
    // EINVAL The size argument is zero and buf is not a null pointer.
    errno = EINVAL;
    return(NULL);
  }

  size_t len = XCEGetCurrentDirectoryA((DWORD)size, buf);

  if (len > size)
  {
    // XCEToUnixPath(buf, size-1);
    // ^^ commented out, why do we care? the spec says:
    // the contents of the array pointed to by buf is undefined on error.
    errno = ERANGE;
    return(NULL);
  }
  else if (!len)
  {
    WCETRACE(WCE_IO, "getcwd WARNING: curr dir is unset");
    // ENOENT The current working directory has been unlinked.
    // well, that's the next best thing
    errno = ENOENT;
    return NULL;
  }
  else
  {
    struct stat st;
    XCEToUnixPath(buf, -1);
    if (lstat(buf, &st)) {
        // ENOENT The current working directory has been unlinked.
        if (alloced) {
            free(buf);
        }
        errno = ENOENT;
        return NULL;
    }
  }
  return buf;
}
