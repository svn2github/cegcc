#ifndef _SYS_DEVOPS_H_
#define _SYS_DEVOPS_H_

#include <stdio.h>

#if !defined(WIN32) && !defined(GNUWINCE)
typedef struct _reent {
  void *foo;
} _reent_t;
#endif

#define DEVOPS_NAMELEN  (32)

enum _devops_ids { DEVOPS_DFLT, DEVOPS_FIFO };

typedef struct _devops_s {
  char name[DEVOPS_NAMELEN];
  int (*open_r)(struct _reent *r, const char *path, int flags, int mode, void *cxt);
  int (*close_r)(struct _reent *r, int fd, void *cxt); 
  long (*read_r)(struct _reent *r, int fd, char *ptr, int len, void *cxt);
  long (*write_r)(struct _reent *r, int fd, const char *ptr, int len, void *cxt);
  off_t (*lseek_r)(struct _reent *r, int fd, off_t offset, int whence, void *cxt);
  int (*ioctl_r)(struct _reent *r, int fd, int request, void *cxt, ...);
} _devops_t, *_DEVOPS;

#endif  /* _SYS_DEVOPS_H_ */
