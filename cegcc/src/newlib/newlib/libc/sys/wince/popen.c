#include <stdio.h>
#include <errno.h>
#include <sys/fifo.h>
#include <sys/spawn.h>

#include "sys/wcetrace.h"

#define MAXARGS   (32)

extern void _parse_tokens(char * string, char * tokens[], int * length);
extern void *_getiocxt(int fd);

FILE *
_popen_read(const char *cmd, const char *mode)
{
  FILE *fp;
  char *argv[MAXARGS];
  void *cxt;
  int argc, pid;
  int stdoutfd;

  WCETRACE(WCE_IO, "_popen_read(\"%s\", \"%s\")", cmd, mode);
  if (cmd == NULL || strlen(cmd) == 0) {
    errno = EINVAL;
    return(NULL);
  }

  stdoutfd = open("fifo", O_CREAT | O_EXCL | O_RDWR, 0660);
  WCETRACE(WCE_IO, "_popen_read: open fifo, stoutfd %d", stdoutfd);

  if (stdoutfd < 0) {
    errno = EMFILE;
    WCETRACE(WCE_IO, "_popen_read: ERROR stdoutfd < 0 (%d)", stdoutfd);
    return(NULL);
  }

  cxt = _getiocxt(stdoutfd);
  if (cxt == NULL) {
    errno = EBADF;
    WCETRACE(WCE_IO, "_popen_read: ERROR cxt is null");
    return(NULL);
  }

  argc = MAXARGS;
  memset(argv, 0, MAXARGS * sizeof(char *));
  _parse_tokens((char *)cmd, argv, &argc);

  /* If cmd is absolute do not do path search */
  WCETRACE(WCE_IO, "_popen_read: cmd is \"%s\"\n", argv[0]);
  if (*argv[0] == '/' || *argv[0] == '\\') {
    pid = _spawnv(argv[0], &argv[1], getpgid(0), -1, stdoutfd, stdoutfd);
  } else {
    pid = _spawnvp(argv[0], &argv[1], getpgid(0), -1, stdoutfd, stdoutfd);
  }
  WCETRACE(WCE_IO, "_popen_read: spawn returns, pid %d", pid);

  if (pid == -1) {
    WCETRACE(WCE_IO, "_popen_read: ERROR spawn failed, errno %d", errno);
    return(NULL);
  }

  _fifo_setpid(cxt, pid);

  /* Finally do fdopen to make a FILE * for the reader */
  fp = fdopen(stdoutfd, "r");
  WCETRACE(WCE_IO, "_popen_read: fdopen returned fp %p", fp);

  return(fp);
}

FILE *
popen(const char *cmd, const char *mode)
{
  FILE *fp = NULL;

  if (cmd == NULL || mode == NULL) {
    errno = EINVAL;
    return(NULL);
  }
   
  if (mode[0] == 'r') {
    fp = _popen_read(cmd, mode);
  } else {
    errno = EINVAL;
  }

  return(fp);
}

int
pclose(FILE *fp)
{
  int fd, pid;
  int rval;
  void *cxt;

  WCETRACE(WCE_IO, "pclose: CALLED, fp %p", fp);

  if (fp == NULL) {
    errno = EINVAL;
    return(-1);
  }

  fd = fp->_file;
  cxt = _getiocxt(fd);
  pid = _fifo_getpid(cxt);
  WCETRACE(WCE_IO, "pclose: fd %d pid %d cxt %p", fd, pid, cxt);

  rval = _await(pid, 0);
  WCETRACE(WCE_IO, "pclose: await returns rval %d", rval);

  fclose(fp);
}      

    
int
pipe(int fds[2])  
{
	fds[0]=fds[1]=open("fifo", O_CREAT | O_EXCL | O_RDWR, 0660);
	return fds[0]!=-1 && fds[1]!=-1;
}        

