#ifndef _SYS_FIFO_H_
#define _SYS_FIFO_H_

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <sys/termios.h>
#include <sys/devops.h>
#include <sys/mqueue.h>

#define MAXFIFOS    (8)

/* Opcodes */
#define FIFO_DATA   (0)
#define FIFO_EOF    (1)

/* Message Queue Parameters */
#define MQ_MAXMSG   (128)
#define MQ_MSGSIZE  (132)

#define MSG_BUFSIZE (MQ_MSGSIZE - 4)

typedef struct _fifomsg_s {
  short len;
  short opcode;
  char  buf[MSG_BUFSIZE];
} _fifomsg_t;

typedef struct _fifoent_s {
  int    fd;
  int    refcnt;
#if defined(WIN32)
  void* hnd;
  void* mutex;
#else
# error "Needs UNDER_CE test above"
#endif
} _fifoent_t;

typedef struct _fifotab_s {
  void* mutex;
  _fifoent_t fifotab[MAXFIFOS];
} _fifotab_t, *_FIFOTAB;

typedef struct _fifoiocxt_s {
  mqd_t mqd;
  struct termios termios;
  _FIFOTAB fifotab;
  int   echofd;
  void *echocxt;
  int   pid;
  int   refcnt;
  int   readpos;
  _fifomsg_t readbuf;
} _fifoiocxt_t, *_FIFOIOCXT;

#define CXTCHECK(F, C) \
  if (C == NULL || (C)->fifotab == NULL) { \
    printf("Bad FIFO context %p for fd %d\n", C, F); \
    errno = EINVAL; \
    return(-1); \
  } 

#define FIFOCHECK(F) \
  if (F < 0 || F >= MAXFIFOS || fifoents == NULL || fifoents[F].fd == -1) { \
    printf("Invalid file handle: %d", F); \
    errno = EBADF; \
    return(-1); \
  }

extern _fifoent_t *_fifotab;

#ifdef __cplusplus
extern "C" {
#endif

void  _fifo_settab(_fifoent_t *tab);
void  _fifo_setpid(void *cxt, int pid);
int   _fifo_getpid(void *cxt);
void *_fifo_getiocxt(int fd);

/* The device operations */
int _fifo_open(struct _reent *r, const char *path, int flags, int mode, void *cxt);
int _fifo_close(struct _reent *r, int fd, void *cxt); 
long _fifo_read(struct _reent *r, int fd, char *ptr, int len, void *cxt);
long _fifo_write(struct _reent *r, int fd, const char *ptr, int len, void *cxt);
off_t _fifo_lseek(struct _reent *r, int fd, off_t offset, int whence, void *cxt);
int _fifo_ioctl(struct _reent *r, int fd, int request, void *cxt, ...);

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_FIFO_H_ */
