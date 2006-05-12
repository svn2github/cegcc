#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/termios.h>
#include <sys/keycode.h>

int
tcsetattr(int fd, int flags, struct termios *tio)
{
  if (tio == NULL) {
    errno = EINVAL;
    return(-1);
  }

  return(ioctl(fd, TCSETA, tio));
}

int
tcgetattr(int fd, struct termios *tio)
{
  if (tio == NULL) {
    errno = EINVAL;
    return(-1);
  }

  return(ioctl(fd, TCGETA, tio));
}

int
 tcflow(int fd, int action)
{
  return(0);
}


int
cfgetispeed(struct termios *t)
{
  /* these are normally in c_cflag */
  return B38400;
}

int
cfgetospeed(struct termios *t)
{
  return B38400;
}

int
cfsetispeed(struct termios *t, int speed)
{
  return 0;
}

int
cfsetospeed(struct termios *t, int speed)
{
  return 0;
}

int
_termios_cookout(void *dest, void *src, int *ncopy, struct termios *tptr)
{
  char c, *destcp, *srccp;
  int copied;
  int nread = 0;

  srccp = (char *) src;
  destcp = (char *) dest;
  copied = 0;

  while (copied < *ncopy) {
    c = *srccp;

    switch (c) {
    case '\n':
      if (tptr->c_oflag & ONLCR) {
       *destcp = '\r';
        destcp++;
        copied++;
      }
      break;
    case K_DEL:
      if (tptr->c_oflag & OPOST) {
       *destcp = K_BS;
        destcp++;
       *destcp = ' ';
        destcp++;
        c = K_BS;
        copied += 2;
      }
      break;
    default:
	  break;
    }

   *destcp = c;
    destcp++;
    copied++;
    srccp++;
    nread++;
  }

  if (copied > *ncopy) {
    *ncopy = copied;
  }

  return(nread);
}

int
_termios_echo(int echofd, char *buf, int len, struct termios *tptr, void *echocxt)
{
  if (tptr->c_lflag & ECHO) {
    _fifo_write(NULL, echofd, buf, len, echocxt);
  }
  return(0);
}



    
