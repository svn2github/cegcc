#include <unistd.h>
#include <errno.h>
#include <windef.h>

long pathconf(const char *path, int name)
{
  switch (name) {
  case _PC_LINK_MAX:
  case _PC_PIPE_BUF:
  case _PC_CHOWN_RESTRICTED:
  case _PC_NO_TRUNC:
  case _PC_VDISABLE:
    return(0);
    break;
  case _PC_MAX_CANON:
  case _PC_MAX_INPUT:
  case _PC_NAME_MAX:
    return(MAX_PATH);
    break;
  case _PC_PATH_MAX:
    return(MAX_PATH);
    break;
  }

  errno = EINVAL;
  return(-1);
}
