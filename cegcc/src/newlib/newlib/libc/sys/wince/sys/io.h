#ifndef _IO_H_
#define _IO_H_

#include <stdio.h>

#include <wtypes.h>
#include <sys/devops.h>

#define MAXFDS   (100)
#define MAXFILES (100)

#define IO_FILE_TYPE_FILE    1
#define IO_FILE_TYPE_SOCKET  2
#define IO_FILE_TYPE_CONSOLE 3
#define IO_FILE_TYPE_FIFO    4
#define IO_FILE_TYPE_NULL    5


typedef struct _fdent_s
{
  int fd;
  int type;
  int flags;

  // union to ease the porting from celib
  union
  {
	  HANDLE hnd;
	  HANDLE hFile;
    SOCKET sock;
  };

  void *cxt;
  _DEVOPS devops;
} _fdent_t;

#define FDCHECK(F, CS) \
	do { \
		if (F < 0 || F >= MAXFDS || _fdtab[F].fd == -1) { \
			WCETRACE(WCE_IO, "Invalid file handle: %d", F); \
			errno = EBADF; \
                        if (CS) { LeaveCriticalSection(CS); } \
			return(-1); \
		} \
	} while (0)

extern _fdent_t _fdtab[];

int  _assignfd(int type, HANDLE hnd, int flags);
void _initfds();
void _initstdfd(FILE *fp, int fd, HANDLE hnd, int flags);
void _initstdfifofd(FILE *fp, int fd, int flags);
void _initecho(int stdinfd, int stdoutfd);
void _initstdio();
void _ioatexit();
void _setfd(int fd, int type, HANDLE hnd, int flags);

#endif  /* _IO_H_ */
