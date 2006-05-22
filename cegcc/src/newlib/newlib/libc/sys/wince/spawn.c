#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>

#include "sys/wcetrace.h"

#include <sys/shared.h>
#include <sys/spawn.h>
#include <sys/io.h>

#define BUFSIZE  256

extern char ** environ;

static int    Pgid = 1;
extern int    __pgid;

static DWORD LastPID=-1;
static DWORD LastExitCode=-1;

extern int __IsForkChild;
void _done_exec(pid_t pid);

int
getpgid(int pid)
{
  return(__pgid);
}

int
newpgid()
{
  return(Pgid++);
}

int
setpgid(int pid, int pgid)
{
  if (pgid == 0) {
    __pgid = Pgid++;
  } else {
    __pgid = pgid;
  }
  return(__pgid);
}

char **
_argvcopy(char* const *argv, int *argc)
{
  int i;
  char **newargv;

  /* Initial set up */
  if (argv == NULL) {
    *argc = 0;
  } else {
    for (i = 0; argv[i] != NULL; i++);
    *argc = i;
  }
  /* 2 + terminating NULL entry */
  *argc += 3;

  /* Allocate and copy */
  newargv = calloc(*argc, sizeof(char **));
  if (argv != NULL) {
    for (i = 0; argv[i] != NULL; i++) {
      newargv[i] = argv[i];
    }
  }

  return(newargv);
}

int
_spawnv(const char *command, char * const argv[], int pgid, int infd, int outfd, int errfd)
{
  int i, pid;
  int argc;
  char buf[BUFSIZE];
  char **newargv;
  _SHMBLK shmblk;

  if (argv == NULL) {
    errno == EINVAL;
    return(-1);
  }

  shmblk = _shared_init(pgid);
  if (shmblk == NULL) {
    errno = ENOMEM;
    return(-1);
  }

  WCETRACE(WCE_IO, "spawnv: _shared_init returns %x", shmblk);
  _shared_setenvblk(shmblk, environ);
  getcwd(buf, BUFSIZE);
  WCETRACE(WCE_IO, "spawnv: cwd \"%s\"", buf);
  _shared_setcwd(shmblk, buf);
  _shared_setstdinfd(shmblk, (infd >= 0) ? _fdtab[infd].fd : infd);
  _shared_setstdoutfd(shmblk, (outfd >= 0) ? _fdtab[outfd].fd : outfd);
  _shared_setstderrfd(shmblk, (errfd >= 0) ? _fdtab[errfd].fd : errfd);

  /* argvcopy assures argv is big enough for 2 more arguments + NULL */
  newargv = _argvcopy(argv, &argc);
  WCETRACE(WCE_IO, "spawnv: newargv %x argc %d", newargv, argc);
  newargv[argc - 3] = "-pgid";
  sprintf(buf, "%u", _shared_getpgid(shmblk));
  newargv[argc - 2] = buf;

  if(command[0]!='/')
  {
  	char tmp[BUFSIZE];
  	getcwd(tmp,BUFSIZE);
  	if(tmp[strlen(tmp)-1]!='/')
	  	strcat(tmp,"/");
  	strcat(tmp,command);
	pid = _spawn(tmp, newargv);
  } else
	pid = _spawn(command, newargv);

  free(newargv);
  WCETRACE(WCE_IO, "spawnv: _spawn returns %u\n", pid);
  return(pid);
}

int
_spawnvp(const char *command, char * const argv[], int pgid, int infd, int outfd, int errfd)
{
  int i, pid;
  int argc;
  char buf[BUFSIZE], pathbuf[BUFSIZE];
  char **newargv;
  _SHMBLK shmblk;

  if (command == NULL || strlen(command) == 0 || argv == NULL) {
    errno == EINVAL;
    return(-1);
  }

  WCETRACE(WCE_IO, "spawnvp \"%s\" pgid %d infd %d outfd %d errfd %d", 
           command, pgid, infd, outfd, errfd);

  /* Similar to spawnv, but incorporate PATH search */
  if (_findexec(command, pathbuf, BUFSIZE) != 0) {
    WCETRACE(WCE_IO, "spawnvp: can't locate exec \"%s\"", command);
    errno = ENOENT;
    return(-1);
  }

  shmblk = _shared_init(pgid);
  if (shmblk == NULL) {
    errno = ENOMEM;
    return(-1);
  }

  WCETRACE(WCE_IO, "spawnv: _shared_init returns %x", shmblk);
  _shared_setenvblk(shmblk, environ);
  getcwd(buf, BUFSIZE);
  WCETRACE(WCE_IO, "spawnvp: cwd \"%s\"", buf);
  _shared_setcwd(shmblk, buf);
  _shared_setstdinfd(shmblk, (infd >= 0) ? _fdtab[infd].fd : infd);
  _shared_setstdoutfd(shmblk, (outfd >= 0) ? _fdtab[outfd].fd : outfd);
  _shared_setstderrfd(shmblk, (errfd >= 0) ? _fdtab[errfd].fd : errfd);
  _shared_dump(shmblk);
  /* argvcopy assures argv is big enough for 2 more arguments + NULL */
  newargv = _argvcopy(argv, &argc);
  WCETRACE(WCE_IO, "spawnv: newargv %x argc %d", newargv, argc);
  newargv[argc - 3] = "-pgid";
  sprintf(buf, "%u", _shared_getpgid(shmblk));
  newargv[argc - 2] = buf;

  pid = _spawn(pathbuf, newargv);
  free(newargv);
  WCETRACE(WCE_IO, "spawnvp: _spawn returns %u\n", pid);
  return(pid);
}

int
_newlib_pre_spawn(int pgid, int infd, int outfd, int errfd)
{
  _SHMBLK shmblk;
  char buf[BUFSIZE];
  int newpgid;

  shmblk = _shared_init(pgid);

  if (shmblk == NULL) {
    errno = ENOMEM;
    return(-1);
  }

  _shared_setenvblk(shmblk, environ);
  getcwd(buf, BUFSIZE);
  _shared_setcwd(shmblk, buf);
  _shared_setstdinfd(shmblk, (infd >= 0) ? _fdtab[infd].fd : infd);
  _shared_setstdoutfd(shmblk, (outfd >= 0) ? _fdtab[outfd].fd : outfd);
  _shared_setstderrfd(shmblk, (errfd >= 0) ? _fdtab[errfd].fd : errfd);

  newpgid = _shared_getpgid(shmblk);

  return newpgid;
}
  
int
_spawn(const char *command, char *const argv[])
{
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  wchar_t commandW[MAX_CMDLEN];
  char    cmdline[MAX_CMDLINE];
  wchar_t cmdlineW[MAX_CMDLINE];
  char   *cp;
  int     i, winerr;

  memset(cmdline, 0, MAX_CMDLINE);
  memset(cmdlineW, 0, MAX_CMDLINE * sizeof(wchar_t));

  if (argv != NULL) {
    for (i = 0; argv[i] != NULL; i++) {
      strcat(cmdline, argv[i]);
      if (argv[i+1] != NULL)
        strcat(cmdline, " ");
    }
  }

  mbstowcs(commandW, command, strlen(command) + 1);
  WCETRACE(WCE_IO, "_spawn: \"%s\" cmdline \"%s\"", command, cmdline);
  mbstowcs(cmdlineW, cmdline, strlen(cmdline) + 1);

  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);

  if (CreateProcessW(commandW, cmdlineW, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) == FALSE) {
    errno = _winerr2errno(GetLastError());
    WCETRACE(WCE_IO, "_spawn: CreateProcess FAILED (errno %d winerr %d)", 
             errno, GetLastError());
    return(-1);
  }

  LastPID=pi.dwProcessId;

  return(pi.dwProcessId);
}

int
_await(int pid, int msec)
{
  HANDLE hnd;
  int    retval, winerr;

  hnd = OpenProcess(0, FALSE, (unsigned int)pid);
  if (hnd == NULL) {
    errno = _winerr2errno(GetLastError());
    WCETRACE(WCE_IO, "_await: ERROR can't open target process (errno %d winerr %d", errno, winerr);
    return(-1);
  }

  if (msec == 0)
    msec = INFINITE;
  if ((retval = WaitForSingleObject(hnd, msec)) == WAIT_FAILED) {
    CloseHandle(hnd);
    errno = _winerr2errno(GetLastError());
    WCETRACE(WCE_IO, "_await: ERROR WFSO failed (errno %d, winerr %d",
             errno, winerr);
    return(-1);
  }

  WCETRACE(WCE_IO, "_await: WFSO joined pid %u", pid);
  retval = GetExitCodeProcess(hnd,&LastExitCode);
  WCETRACE(WCE_IO, "_await: GECP returns %d, exit code %d", retval, LastExitCode);
  CloseHandle(hnd);
  return(0);
}

void *
_getchildhnd(int pid)
{
  HANDLE hnd;
  int    retval, winerr;

  hnd = OpenProcess(0, FALSE, (unsigned int)pid);
  if (hnd == NULL) {
    errno = _winerr2errno(GetLastError());
    WCETRACE(WCE_IO, "_getchildhnd: ERROR can't open target process (errno %d winerr %d", errno, winerr);
    return(NULL);
  }

  return((void *)hnd);
}

int execv(const char *command, char * const argv[])
{
  WCETRACE(WCE_IO, "execv: ");
	int pid=_spawnv(command, argv+1, getpgid(0), -1,-1,-1); /* eat process name from argv, get it from command */

	if(__IsForkChild)
		_done_exec(pid);

	if(pid!=-1)
	{
		_await(pid,0);
		_exit(0);
	}
	return -1;
}

int
execvp(const char *command, char * const argv[])
{
    WCETRACE(WCE_IO, "execvp: ");
	int pid=_spawnvp(command, argv+1, getpgid(0), -1,-1,-1);

	if(__IsForkChild)
		_done_exec(pid);

	if(pid!=-1)
	{
		_await(pid,0);
		_exit(0);
	}
	return -1;
}

int
_exec(const char *command, char * const argv[])
{
	return execv(command, argv);		
}

pid_t
_wait(int *status)
{
	_await(LastPID,0);
	*status=LastExitCode;
    WCETRACE(WCE_IO, "wait: pid=%d, exit code=%d",LastPID,LastExitCode);
	return LastPID;
}
pid_t
_wait_r(void* dum,int *status)
{
	return _wait(status);
}

#define _P_WAIT         1

int spawnv(int mode, const char *command, char * const argv[])
{
    WCETRACE(WCE_IO, "spawnv: ");
	if(mode&_P_WAIT)
	{
		int pid=_spawnv(command, argv+1, getpgid(0), -1,-1,-1);

		if(__IsForkChild)
			_done_exec(pid);

		if(pid!=-1)
		{
			_await(pid,0);
			return LastExitCode;
		} else
		  return -1;
	}
	else
		return _spawnv(command, argv+1, getpgid(0), -1,-1,-1);
}

int
spawnvp(int mode, const char *command, char * const argv[])
{
    WCETRACE(WCE_IO, "spawnvp: ");
	if(mode&_P_WAIT)
	{
		int pid=_spawnvp(command, argv+1, getpgid(0), -1,-1,-1);

		if(__IsForkChild)
			_done_exec(pid);

		if(pid!=-1)
		{
			_await(pid,0);
			return LastExitCode;
		} else
		  return -1;
	}
	else
		return _spawnvp(command, argv+1, getpgid(0), -1,-1,-1);
}

/* FIXME: passed environment is ignored. Current is used instead */
int _execve_r(void *dum, const char *command, char * const argv[], char * const env[])
{
    WCETRACE(WCE_IO, "execve: ");
	int pid=_spawnv(command, argv+1, getpgid(0), -1,-1,-1);

	if(__IsForkChild)
		_done_exec(pid);

	if(pid!=-1)
	{
		_await(pid,0);
		_exit(0);
	}
	return -1;
}

size_t getpagesize(void)
{
	return 65536;
}
