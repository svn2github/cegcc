#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <sys/wcebase.h>
#include <sys/wcetrace.h>
#include <sys/wceerror.h>
#include <sys/wcefile.h>
#include <sys/wcememory.h>
#include <sys/mqueue.h>
#include <sys/fifo.h>
#include <sys/keycode.h>

#define MAPNAME    "fifotab"
#define SYSNAMELEN (32)

static int   master = FALSE;

_devops_t _devops_fifo = { "fifo", _fifo_open, _fifo_close, _fifo_read, _fifo_write, _fifo_lseek, _fifo_ioctl };
_DEVOPS _fifo_devops = &_devops_fifo;

void _fifo_setmaster()
{
  master = TRUE;
}

void _fifo_setpid(void *cxt, int pid)
{
  _FIFOIOCXT fcxt = (_FIFOIOCXT) cxt;

  WCETRACE(WCE_IO, "_fifo_setpid: cxt %p pid %d", cxt, pid);
  if (fcxt != NULL) {
    fcxt->pid = pid;
  }
}

int _fifo_getpid(void *cxt)
{
  _FIFOIOCXT fcxt = (_FIFOIOCXT) cxt;

  WCETRACE(WCE_IO, "_fifo_getpid: cxt %p", cxt);
  if (fcxt != NULL) {
    return(fcxt->pid);
  } else {
    return(-1);
  }
}

void *
_fifo_alloc()
{
  _FIFOIOCXT fcxt;
  _FIFOTAB fifotab;
  HANDLE mapHnd;
  DWORD winerr = 0;
  BOOL  new = FALSE;
  char  name[SYSNAMELEN];
  wchar_t nameW[SYSNAMELEN];
  int   i;

  fcxt = (_FIFOIOCXT) calloc(1, sizeof(_fifoiocxt_t));
  if (fcxt == NULL) {
    errno = ENOMEM;
    return(NULL);
  }
  fcxt->mqd = (mqd_t)-1;    /* Initialize our message queue descriptor */

  /* Create the per-process-group fifo table */
  WCETRACE(WCE_IO, "_fifo_alloc: CALLED pgid %d", getpgid(0));
  sprintf(name, "%d:%s", getpgid(0), MAPNAME);
  mbstowcs(nameW, name, strlen(name)+1);
  SetLastError(0);

  mapHnd = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, 
            	PAGE_READWRITE|SEC_COMMIT,	0, sizeof(_fifotab_t), nameW);

  if (mapHnd == NULL) {
    winerr = GetLastError();
    WCETRACE(WCE_IO, "_fifo_alloc: FATAL CreateFileMappingW fails winerr %d", winerr);
    exit(1);
  }

  if ((winerr = GetLastError()) != ERROR_ALREADY_EXISTS) {
    new = TRUE;
  }

  fifotab = MapViewOfFile(mapHnd, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(_fifotab_t));

  if (fifotab == NULL) {
    winerr = GetLastError();
    WCETRACE(WCE_IO, "_fifo_alloc: FATAL MapViewOfFile fails winerr %d", winerr);
    exit(1);
  }

  if (new) {
    WCETRACE(WCE_IO, "_fifo_alloc: created NEW system fifotab (%s) @%x", name, fifotab);
    memset(fifotab, 0, sizeof(_fifotab_t));
  } else {
    WCETRACE(WCE_IO, "_fifo_alloc: mapped EXISTING system fifotab @%x", fifotab);
  }

  /*  SYNC; */
  fcxt->fifotab = fifotab;
  return((void *)fcxt);
}

/* NOTE: mqd SIMPLY MUST reside in the per-process file descriptor table -
 * this is because when the descs are "copied", mq_open will have to be done */
int _fifo_open(struct _reent *r, const char *path, int flags, int mode, void *cxt)
{
  struct mq_attr mqattr = { 0, MQ_MAXMSG, MQ_MSGSIZE, 0 };
  char *cp;
  int i, fd;
  char fifopath[SYSNAMELEN];
  char buf[SYSNAMELEN];
  _fifoent_t *fifoents;
  _FIFOIOCXT fcxt = (_FIFOIOCXT) cxt;

  CXTCHECK(fd, fcxt);  
  fifoents = fcxt->fifotab->fifotab;

  /* No fd specified, use next available fd */
  if (strncmp(path, "fifo", 4) == 0) {
    if (strlen(path) == 4) {
      fd = -1;
      for (i = 0; i < MAXFIFOS; i++) {
        if (fifoents[i].refcnt == 0) {
          fd = i;
          sprintf(buf, "fifo%d", fd);
          break;
        }
      }
      if (fd == -1) {
        errno = ENMFILE;
        return(-1);
      }
    } else {
      memset(buf, 0, SYSNAMELEN);
      for (i = 0, cp = (char *) &path[4]; *cp != '\0'; cp++, i++) {
        if (!isdigit(*cp)) {
          errno = EINVAL;
          return(-1);
        }
        buf[i] = *cp;
      }
      fd = atoi(buf);
      WCETRACE(WCE_IO, "_fifo_open: parsed fd %d from \"%s\"", fd, path);
      if (fd < 0 || fd >= MAXFIFOS) {
        errno = EINVAL;
        return(-1);
      }
      strcpy(buf, path);
    }
  
    /* Prepend the mq name with our pgid */
    sprintf(fifopath, "%d:%s", getpgid(0), buf);
    if (fcxt->mqd == (mqd_t) -1) {
      if ((fcxt->mqd = mq_open(fifopath, flags, mode, &mqattr)) ==
           (mqd_t) - 1) {
        printf("_fifo_open: Unable to open message queue");
        return(-1);
      }
      fcxt->refcnt = 1;
    } else {
      fcxt->refcnt++;
    }
    WCETRACE(WCE_IO, "_fifo_open: fifopath \"%s\" fcxt->mqd %d fcxt->refcnt %d\n",
             fifopath, fcxt->mqd, fcxt->refcnt);

    fifoents[fd].fd = fd;
    fifoents[fd].refcnt++;

    /* Clear the termios settings */
    memset(&fcxt->termios, 0, sizeof(struct termios));

    fcxt->readbuf.len = -1;
    WCETRACE(WCE_IO, "_fifo_open returns %d", fd);
    return(fd);
  } else {
    WCETRACE(WCE_IO, "_fifo_open: ERROR bogus path \"%s\"", path);
    errno = EINVAL;
    return(-1);
  }
}

int
_fifo_close(struct _reent *r, int fd, void *cxt)
{
  int retval;
  _fifoent_t *fifoents;
  _FIFOIOCXT fcxt = (_FIFOIOCXT) cxt;

  WCETRACE(WCE_IO, "_fifo_close %d cxt %p", fd, cxt);

  CXTCHECK(fd, fcxt);  
  fifoents = fcxt->fifotab->fifotab;
  FIFOCHECK(fd);

  /* Signal the reader if we are the only writer */
  if (fifoents[fd].refcnt <= 2 || master) {
    retval = mq_signal(fcxt->mqd, TRUE);
  }

  /* Adjust system reference count */
  if (fifoents[fd].refcnt > 0)
    fifoents[fd].refcnt--;
  if (master)
    fifoents[fd].refcnt = 0;
  WCETRACE(WCE_IO, "_fifo_close: refcnt %d AFTER", fifoents[fd].refcnt);

  if (fcxt->refcnt > 0) {
    retval = 0;
    fcxt->refcnt--;
    WCETRACE(WCE_IO, "_fifo_close: fcxt refcnt %d->%d", fcxt->refcnt+1, fcxt->refcnt);
  }

  /* Close only if process refcnt (mqent) is zero */
  if (fcxt->refcnt == 0) {
    if (fcxt->mqd != (mqd_t) -1) {
      retval = mq_close(fcxt->mqd);
      WCETRACE(WCE_IO, "_fifo_close: mq_close returns %d errno %d\n", retval, errno);
      fcxt->mqd = (mqd_t) -1;
    }

    /* Clear the termios settings */
    memset(&fcxt->termios, 0, sizeof(struct termios));
  }

  return(retval);
}

int
_fifo_read(struct _reent *r, int fd, char *ptr, int len, void *cxt)
{
  int retval, nread, ncopy;
  char buf[MQ_MSGSIZE];
  unsigned int prio;
  int  eol = FALSE;
  char *savptr;
  char dbgbuf[2048];
  int  avail, i, bufcnt, savpos, savlen, foo;
  _fifoent_t *fifoents;
  _FIFOIOCXT fcxt = (_FIFOIOCXT) cxt;

  WCETRACE(WCE_IO, "_fifo_read %d %p %d %p", fd, ptr, len, cxt);
  if (ptr == NULL) {
    errno = EINVAL;
    return(-1);
  }

  CXTCHECK(fd, fcxt);  
  fifoents = fcxt->fifotab->fifotab;
  FIFOCHECK(fd);

  WCETRACE(WCE_IO, "_fifo_read %d %d c_oflag %d", fd, len, fcxt->termios.c_oflag);

  savptr = ptr;
  savlen = len;
  nread = 0;
  while (len > 0 && !eol) {
    /* A buffer is needed */
    if (fcxt->readbuf.len <= 0 || fcxt->readpos > (fcxt->readbuf.len-1)) {
      if (nread > 0) {
        /* Some data has already been read: return it instead of blocking */
        if (mq_msgcnt(fcxt->mqd) == 0) {
          break;
        }
      }

      /* This is the blocking read */
      retval = mq_receive(fcxt->mqd, (char *)&fcxt->readbuf, MQ_MSGSIZE, &prio);
  
      WCETRACE(WCE_IO, "fifo_read: mqrecv %d", retval);
      if (retval == 0) {
        WCETRACE(WCE_IO, "fifo_read: EOF");
        break;
      }

      if (retval == -1)
        return(retval);
      fcxt->readpos = 0;
      foo =  fcxt->readbuf.len;
      /*      printf("fifo_read: got msg len %d\n", foo); */
      WCETRACE(WCE_IO, "fifo_read: got msg len %d", foo);
    }

    savpos = fcxt->readpos;

    /* If ICANON is set in lflag: handle DEL and assemble input line until \n */
    if (fcxt->termios.c_lflag & ICANON) {
      ncopy = 1;

      WCETRACE(WCE_IO, "_fifo_read (ICANON) INCHAR %c", fcxt->readbuf.buf[fcxt->readpos]);
      switch (fcxt->readbuf.buf[fcxt->readpos]) {
      case K_DEL:
        WCETRACE(WCE_IO, "K_DEL: nread %d readpos %d ptr %p", nread, fcxt->readpos, ptr);
        ncopy = 0;
        fcxt->readpos++;
        if (nread > 0) {
          len++;
          len = (len > savlen) ? savlen : len;
          nread--;
         *ptr = '\0';
          ptr--;
        } else {
          continue;
        }
        break;
      case '\n':
      case '\r':
        eol = TRUE;
        fcxt->readbuf.buf[fcxt->readpos] = '\n';
        WCETRACE(WCE_IO, "_fifo_read (ICANON) eol %d", eol);
        break;
      }
    } else {
      avail = fcxt->readbuf.len - fcxt->readpos;
      ncopy = (len > avail) ? avail : len;
    }

    if (fcxt->termios.c_oflag) {
      bufcnt = _termios_cookout(ptr, &(fcxt->readbuf.buf[fcxt->readpos]), &ncopy, &fcxt->termios);
      len -= ncopy;
      fcxt->readpos += bufcnt;
      ptr += ncopy;
      nread += ncopy;
    } else {
      if (ncopy > 0) {
        bufcnt = ncopy;
        memcpy(ptr, &(fcxt->readbuf.buf[fcxt->readpos]), ncopy);
        len -= ncopy;
        fcxt->readpos += ncopy;
        ptr += ncopy;
        nread += ncopy;
      }
    }

    if (fcxt->termios.c_lflag & ECHO) {
      /* Echo only if our io context has been set up for it (startup.c) */
      if (fcxt->echocxt != NULL) {
        _termios_echo(fcxt->echofd, &(fcxt->readbuf.buf[savpos]), bufcnt, &fcxt->termios, fcxt->echocxt);
      }
    }
  }

  if (fcxt->termios.c_lflag & ICANON && nread > 0) {
    dbgbuf[0] = 0;
    sprintf(dbgbuf, "%d ", ptr[0]);
    for (i = 1; i < nread; i++) {
      sprintf(buf, "%d ", ptr[i]);
      strcat(dbgbuf, buf);
    }
    WCETRACE(WCE_IO, "_fifo_read: (ICANON) INBUF %s", dbgbuf);
  }
  WCETRACE(WCE_IO, "_fifo_read: return w/nread %d\n", nread);
  return(nread);
}

int
_fifo_write(struct _reent *r, int fd, char *ptr, int len, void *cxt)
{
  
  int retval, nwrite;
  unsigned int prio = 1;
  _fifomsg_t  msg;
  _fifoent_t *fifoents;
  _FIFOIOCXT fcxt = (_FIFOIOCXT) cxt;

  WCETRACE(WCE_IO, "_fifo_write %d %d %p", fd, len, cxt);

  CXTCHECK(fd, fcxt);  
  fifoents = fcxt->fifotab->fifotab;
  FIFOCHECK(fd);

  if (ptr == NULL) {
    errno = EINVAL;
    return(-1);
  }
 
  nwrite = 0;
  while (len > 0) {
    msg.len = (len > MSG_BUFSIZE) ? MSG_BUFSIZE : len;
    memcpy(msg.buf, ptr, msg.len);
    retval= mq_send(fcxt->mqd, (char *)&msg, MQ_MSGSIZE, prio);
  
    if (retval == -1) {
      WCETRACE(WCE_IO, "mq_send returns %d errno %d\n", retval, errno);
      return(retval);
    }
    ptr += msg.len;
    nwrite += msg.len;
    len -= msg.len;
  }

  return(nwrite);
}

int
_fifo_lseek(struct _reent *r, int fd, off_t offset, int whence, void *cxt)
{
  errno = ENOSYS;
  return(-1);
}

int
_fifo_ioctl(struct _reent *r, int fd, int request, void *cxt, ...)
{
  va_list ap;
  void   *argbuf = NULL;
  _fifoent_t *fifoents;
  _FIFOIOCXT fcxt = (_FIFOIOCXT) cxt;

  WCETRACE(WCE_IO, "_fifo_ioctl %d %d %p", fd, request, cxt);

  CXTCHECK(fd, fcxt);  
  fifoents = fcxt->fifotab->fifotab;
  FIFOCHECK(fd);

  WCETRACE(WCE_IO, "_fifo_ioctl %d %d", fd, request);
      
  switch (request) {
  case TCGETA:
  case TCSETA:
//    va_start(ap, request);
	va_start(ap, cxt);
    argbuf = va_arg(ap, void *);
    va_end(ap);
    if (argbuf == NULL) {
      WCETRACE(WCE_IO, "_fifo_ioctl ERROR argbuf is NULL");
      errno = EINVAL;
      return(-1);
    }

    if (request == TCGETA) {
      memcpy(argbuf, &fcxt->termios, sizeof(struct termios)); 
    } else {
      memcpy(&fcxt->termios, argbuf, sizeof(struct termios)); 
    }
    break;
  default:
    WCETRACE(WCE_IO, "_fifo_ioctl ERROR unimp request %d", request);
    errno = ENOSYS;
    return(-1);
  }

  WCETRACE(WCE_IO, "_fifo_ioctl SUCCESS for request %d", request);
  return(0);
}
