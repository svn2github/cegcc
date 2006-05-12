#ifndef _TERMIOS_H_
#define _TERMIOS_H_

#include <sys/termbits.h>

/* ioctls */
#define TCGETS		0x5401
#define TCSETS		0x5402
#define TCSETSW		0x5403
#define TCSETSF		0x5404
#define TCGETA		0x5405
#define TCSETA		0x5406
#define TCSETAW		0x5407
#define TCSETAF		0x5408
#define TCSBRK		0x5409
#define TCXONC		0x540A
#define TCFLSH		0x540B

#define TIOCGWINSZ	0x5413
#define TIOCSWINSZ	0x5414

typedef int speed_t;

struct winsize {
  unsigned short ws_row;
  unsigned short ws_col;
  unsigned short ws_xpixel;
  unsigned short ws_ypixel;
};

#define NCC 8

struct termios {
  unsigned short c_iflag; /* input mode flags */
  unsigned short c_oflag; /* output mode flags */
  unsigned short c_cflag; /* control mode flags */
  unsigned short c_lflag; /* local mode flags */
  unsigned char c_line;   /* line discipline */
  unsigned char c_cc[NCC]; /* control characters */
};

#ifdef __cplusplus
extern "C" {
#endif

int tcsetattr(int fd, int flags, struct termios *t);
int tcgetattr(int fd, struct termios *t);
int cfgetispeed(struct termios *t);
int cfgetospeed(struct termios *t);
int cfsetispeed(struct termios *t, int speed);
int cfsetospeed(struct termios *t, int speed);

int _termios_cookout(void *dest, void *src, int *ncopy, struct termios *tptr);
int _termios_echo(int echofd, char *buf, int len, struct termios *tptr, void *echocxt);

#ifdef __cplusplus
}
#endif
#endif  /* _TERMIOS_H_ */
