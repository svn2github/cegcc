#ifndef _SHARED_H_
#define _SHARED_H_

#include "sys/fifo.h"

#define MAX_ENVIRONBLK  (2048)
#define SYSNAMELEN      (24)

/* This struct is copied to shared seg before creating a new process */
/* The child will read it */

typedef struct _pginfo_s {
  int  pgid;
  int showwindow;
  char cwd[126];
  char environ[MAX_ENVIRONBLK];
  int  stdinfd;
  int  stdoutfd;
  int  stderrfd;
} _pginfo_t, *_PGINFO;

typedef struct _shmblk_s {
  void* mutex;
  _pginfo_t pginfo;
} _shmblk_t, *_SHMBLK;

#ifdef __cplusplus
extern "C" {
#endif

_SHMBLK _shared_init(int pgid);
void    _shared_dump(_SHMBLK shmblk);
int     _shared_getshowwindow(_SHMBLK shmblk);
void    _shared_setshowwindow(_SHMBLK shmblk, int show);
void    _shared_setenvironblk(_SHMBLK shmblk, char **env);

/* Returns the number of environment variables extracted from the
   shared block.  The environment variables are written into *ENV,
   which is allocated using malloc.  */
int     _shared_getenvironblk(_SHMBLK shmblk, char **env);

void    _shared_reset(_SHMBLK shmblk);
void    _shared_getcwd(_SHMBLK shmblk, char *cwd);
void    _shared_setcwd(_SHMBLK shmblk, char *cwd);
int     _shared_getpgid(_SHMBLK shmblk);
void    _shared_setpgid(_SHMBLK shmblk, int pgid);

int     _shared_getstdinfd(_SHMBLK shmblk);
void    _shared_setstdinfd(_SHMBLK shmblk, int fd);
int     _shared_getstdoutfd(_SHMBLK shmblk);
void    _shared_setstdoutfd(_SHMBLK shmblk, int fd);
int     _shared_getstderrfd(_SHMBLK shmblk);
void    _shared_setstderrfd(_SHMBLK shmblk, int fd);
#ifdef __cplusplus
}
#endif

#endif  /* _SHARED_H_ */
