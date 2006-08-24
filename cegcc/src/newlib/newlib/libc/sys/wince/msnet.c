#include "sys/wcetrace.h"
#include "sys/ceshared.h"

#include <sys/types.h>
#include <sys/io.h>
#include <sys/time.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define CHECKFD(afd) \
  if (afd < 0 || afd >= MAXFDS || _fdtab[afd].fd == -1) { \
    errno = EBADF; \
    return -1; \
  } \
  if( _fdtab[afd].type != IO_FILE_TYPE_SOCKET) { \
    errno = EBADF; \
    return -1; \
  }

#define WSADESCRIPTION_LEN 256
#define WSASYS_STATUS_LEN 128

typedef struct WSAData {
  WORD                    wVersion;
  WORD                    wHighVersion;
  char                    szDescription[WSADESCRIPTION_LEN+1];
  char                    szSystemStatus[WSASYS_STATUS_LEN+1];
  unsigned short          iMaxSockets;
  unsigned short          iMaxUdpDg;
  char*                   lpVendorInfo;
} WSADATA, * LPWSADATA;

int WSAStartup(WORD version, WSADATA *wsadata);

#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)

static BOOL MSNET_initialized = FALSE;
static WSADATA MSNET_wsadata;

enum {
#undef MSNET_IMPORT
#define MSNET_IMPORT(ENUM, STR) ENUM
#include "msnet_imports.h"
#undef MSNET_IMPORT
};

static const char* _wstbl_name[] = {
#undef MSNET_IMPORT
#define MSNET_IMPORT(ENUM, STR) STR
#include "msnet_imports.h"
#undef MSNET_IMPORT
};

#define MSNET_IMPORTED_COUNT sizeof(_wstbl_name)/sizeof(_wstbl_name[0])

static void* _wstbl_fcn[MSNET_IMPORTED_COUNT];

/* 'Real' calls of WINSOCK/WS2 functions via the above function table */
int __WSAFDIsSet(SOCKET s, fd_set *fds) {
  int (*wsafdisset)(SOCKET, fd_set *) = _wstbl_fcn[OS_WSAFDISSET];
  return( (*wsafdisset)(s, fds));
}

int __MS_wsacleanup()
{
  int (*wsacleanup)(void) = _wstbl_fcn[OS_WSACLEANUP];
  return( (*wsacleanup)());
} 

int __MS_WSAStartup(WORD version, WSADATA *wsadata) {
  int (*wsastartup)(WORD, WSADATA *) = _wstbl_fcn[OS_WSASTARTUP];
  return( (*wsastartup)(version, wsadata));
}

int __MS_accept(SOCKET s, struct sockaddr *addr, int *addrlen)
{
  int (*accept)(SOCKET, struct sockaddr *, int *) = _wstbl_fcn[OS_ACCEPT];
  return( (*accept)(s, addr, addrlen));
}

int __MS_bind(SOCKET s, const struct sockaddr *addr, int addrlen)
{
  int (*bind)(SOCKET, const struct sockaddr *, int) = _wstbl_fcn[OS_BIND];
  return( (*bind)(s, addr, addrlen));
}

int __MS_closesocket(SOCKET s)
{
  int (*closesocket)(SOCKET) = _wstbl_fcn[OS_CLOSESOCKET];
  return( (*closesocket)(s));
}

int __MS_connect(SOCKET s, const struct sockaddr *addr, int addrlen)
{
  int (*connect)(SOCKET, const struct sockaddr *, int) = _wstbl_fcn[OS_CONNECT];
  return( (*connect)(s, addr, addrlen));
}

struct hostent *__MS_gethostbyaddr(const char *addr, int len, int type)
{
  struct hostent * (*gethostbyaddr)(const char *, int, int) = _wstbl_fcn[OS_GETHOSTBYADDR];
  return (*gethostbyaddr)(addr, len, type);
}

struct hostent *__MS_gethostbyname(const char *name)
{
  struct hostent * (*gethostbyname)(const char * name) = _wstbl_fcn[OS_GETHOSTBYNAME];
  return( (*gethostbyname)(name));
}

int __MS_gethostname(char *name, size_t namelen)
{
  int (*gethostname)(char *, size_t) = _wstbl_fcn[OS_GETHOSTNAME];
  return( (*gethostname)(name, namelen));
}

int __MS_getpeername(SOCKET s, struct sockaddr *addr, int *addrlen)
{
  int (*getpeername)(SOCKET, struct sockaddr *, int *) = _wstbl_fcn[OS_GETPEERNAME];
  return( (*getpeername)(s, addr, addrlen));
}

int __MS_getsockname(SOCKET s, struct sockaddr *addr, int *addrlen)
{
  int (*getsockname)(SOCKET, struct sockaddr *, int *) = _wstbl_fcn[OS_GETSOCKNAME];
  return( (*getsockname)(s, addr, addrlen));
}

int __MS_getsockopt(SOCKET s, int level, int optname, char *optval, int *optlen)
{
  int (*getsockopt)(SOCKET, int, int, char *, int *) = _wstbl_fcn[OS_GETSOCKOPT];
  return( (*getsockopt)(s, level, optname, optval, optlen));
}

in_addr_t __MS_inet_addr(const char *cp)
{
  in_addr_t (*inet_addr)(const char *) = _wstbl_fcn[OS_INET_ADDR];
  return( (*inet_addr)(cp));
}

int __MS_ioctlsocket(SOCKET s, long cmd, unsigned long *argp)
{
  int (*ioctlsocket)(SOCKET, long, unsigned long *) = _wstbl_fcn[OS_IOCTLSOCKET];
  return( (*ioctlsocket)(s, cmd, argp));
}

int __MS_listen(SOCKET s, int backlog)
{
  int (*listen)(SOCKET, int) = _wstbl_fcn[OS_LISTEN];
  return( (*listen)(s, backlog));
}

int __MS_recv(SOCKET s, char *buf, int len, int flags)
{
  int (*recv)(SOCKET, char *, int, int) = _wstbl_fcn[OS_RECV];
  return( (*recv)(s, buf, len, flags));
}

int __MS_recvfrom(SOCKET s, char *buf, int len, int flags, struct sockaddr *addr, int *addrlen)
{
  int (*recvfrom)(SOCKET, char *, int, int, struct sockaddr *, int *) = _wstbl_fcn[OS_RECVFROM];
  return( (*recvfrom)(s, buf, len, flags, addr, addrlen));
}

int __MS_select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, const struct timeval *timeout)
{
  int (*select)(int, fd_set *, fd_set *, fd_set *, const struct timeval *) = _wstbl_fcn[OS_SELECT];
  return( (*select)(nfds, rfds, wfds, efds, timeout));
}

int __MS_send(SOCKET s, char *buf, int len, int flags)
{
  int (*send)(SOCKET, char *, int, int) = _wstbl_fcn[OS_SEND];
  return( (*send)(s, buf, len, flags));
} 

int __MS_sendto(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen)
{
  int (*sendto)(SOCKET, const char *, int, int, const struct sockaddr *, int) = _wstbl_fcn[OS_SENDTO];
  return( (*sendto)(s, buf, len, flags, to, tolen));
}

int __MS_setsockopt(SOCKET s, int level, int optname, char *optval, int optlen)
{
  int (*setsockopt)(SOCKET, int, int, char *, int) = _wstbl_fcn[OS_SETSOCKOPT];
  return( (*setsockopt)(s, level, optname, optval, optlen));
}

int __MS_shutdown(SOCKET s, int how)
{
  int (*shutdown)(SOCKET, int) = _wstbl_fcn[OS_SHUTDOWN];
  return( (*shutdown)(s, how));
}

SOCKET __MS_socket(int af, int type, int protocol)
{
  SOCKET (*socket)(int, int, int) = _wstbl_fcn[OS_SOCKET];
  return( (*socket)(af, type, protocol));
}

char* __MS_inet_ntoa(struct in_addr in)
{
  char* (*inet_ntoa)(struct in_addr) = _wstbl_fcn[OS_INET_NTOA];
  return( (*inet_ntoa)(in));
}

#define ENSURE_MSNET_INITTED() \
do { \
  if (!MSNET_initialized) \
    __msnet_init(); \
} while (0)

BOOL 
__msnet_init()
{
  wchar_t winsockName[] = { 'W', 'I', 'N', 'S', 'O', 'C', 'K', '.', 'D', 'L', 'L', '\0' };
  wchar_t ws2Name[] = { 'W', 'S', '2', '.', 'D', 'L', 'L', '\0' };
  char buf[32];
  HANDLE hnd = NULL;
  WORD version;
  int i;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);

  if (!MSNET_initialized) {
    /* We are no longer linking directly to WS2/WINSOCK.  Load symbols now */
    wcstombs(buf, winsockName, wcslen(winsockName)+1);
    WCETRACE(WCE_NETWORK, "__msnet_init: INFO loading network symbols from %s", buf);
    hnd = LoadLibraryW(winsockName);
    if (hnd == NULL) {
      wcstombs(buf, ws2Name, wcslen(ws2Name)+1);
      WCETRACE(WCE_NETWORK, "__msnet_init: INFO WINSOCK.DLL not found, trying %s", buf);
      hnd = LoadLibraryW(ws2Name);
      if (hnd == NULL) {
        WCETRACE(WCE_NETWORK, "__msnet_init: FATAL ERROR WINSOCK/WS2 not found, exiting..");
        exit(1);
      }
    }

    for (i = 0; i < MSNET_IMPORTED_COUNT; i++) {
      _wstbl_fcn[i] = GetProcAddressA(hnd, (char*)_wstbl_name[i]);
    }

    /* Windows CE 3.0 has only WINSOCK Version 1.1 */
    version = MAKEWORD(1, 1);
    int error = WSAStartup(version, &MSNET_wsadata);
    if (error == 0) {
      WCETRACE(WCE_NETWORK, "__msnet_init: winsock 1.1 initialized");
      MSNET_initialized = TRUE;
      return(TRUE);
    } else {
      WCETRACE(WCE_NETWORK, "__msnet_init: ERROR - can't initialize winsock 1.1, err = %d", error);
      return(FALSE);
    }
  }

  return(TRUE);
}

int
XCEWSAGetLastError()
{
	WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
	return GetLastError();
}

char* 
inet_ntoa(struct in_addr in)
{
  ENSURE_MSNET_INITTED();
  return __MS_inet_ntoa(in);
}

in_addr_t
inet_addr(const char *cp)
{
  ENSURE_MSNET_INITTED();
  return __MS_inet_addr(cp);
}

int 
WSAStartup(WORD version, WSADATA *wsadata) 
{
  return __MS_WSAStartup(version, wsadata);
}

int
accept(int afd, struct sockaddr *addr, socklen_t *addrlen)
{
  SOCKET s;
  SOCKET sr;
  int werr;
  int fd;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
  CHECKFD(afd);

  ENSURE_MSNET_INITTED();

  s = (SOCKET)_fdtab[afd].hnd;

  WCETRACE(WCE_IO, "calling __MS_accept, s %d\n", s);
  if ((sr = __MS_accept(s, addr, addrlen)) == INVALID_SOCKET) {
    werr = XCEWSAGetLastError();
    errno = _winerr2errno(werr);
    return(-1);
  }

  fd = _getnewfd();

  _fdtab[fd].fd = fd;
  _fdtab[fd].type = IO_FILE_TYPE_SOCKET;
  _fdtab[fd].hnd = (HANDLE) sr;

  /*!! is this correct? */
  _fdtab[fd].flags = _fdtab[afd].flags;

  WCETRACE(WCE_NETWORK, "accept: fd = %d, handle = %d", fd, sr);
  return(fd);
}

int
bind(int afd, const struct sockaddr *addr, socklen_t addrlen)
{
  SOCKET s;
  int    werr;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);

  ENSURE_MSNET_INITTED();

  CHECKFD(afd);
  s = (SOCKET)_fdtab[afd].hnd;

  if (__MS_bind(s, addr, addrlen) == SOCKET_ERROR) {
    werr = XCEWSAGetLastError();
    errno =  _winerr2errno(werr);
    return(-1);
  }

  return(0);
}

int
connect(int afd, const struct sockaddr *addr, socklen_t addrlen)
{
  SOCKET s;
  int    werr;
  time_t t;
  
  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
  ENSURE_MSNET_INITTED();

  CHECKFD(afd);
  s = (SOCKET)_fdtab[afd].hnd;

  t = time(NULL);
  WCETRACE(WCE_IO, "connect CALLED @ t=%d seconds", t);
  if (__MS_connect(s, addr, addrlen) == SOCKET_ERROR) {
    werr = XCEWSAGetLastError();
    if (werr == WSAEWOULDBLOCK)
      errno = EINPROGRESS;
    else
      errno = _winerr2errno(werr);
    t = time(NULL);
    WCETRACE(WCE_IO, "connect ERROR RETURN @ t=%d seconds errno %d werr %d", t, errno, werr);
    return(-1);
  }   
  t = time(NULL);
  WCETRACE(WCE_IO, "connect NORMAL RETURN @ t=%d seconds", t);

  return(0);
}

struct hostent *
gethostbyaddr(const void *addr, socklen_t len, int type)
{
  struct hostent *hp;
  int werr;

  ENSURE_MSNET_INITTED();

  if (type == AF_INET)
	  WCETRACE(WCE_NETWORK, "%s CALLED w/\"%s\"", __FUNCTION__, inet_ntoa(*(struct in_addr*)addr));
  else
	  WCETRACE(WCE_NETWORK, "%s CALLED w/ type %d", __FUNCTION__, type);

  if ((hp = __MS_gethostbyaddr(addr, len, type)) == NULL) {
    werr = XCEWSAGetLastError();
    errno = _winerr2errno(werr);
    WCETRACE(WCE_NETWORK, "gethostbyaddr ERROR errno %d", errno);
    return(NULL);
  }

  WCETRACE(WCE_NETWORK, "gethostbyaddr returns %p", hp);
  return(hp);
}

struct hostent *
gethostbyname(const char *name)
{
  struct hostent *hp;
  int werr;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
  ENSURE_MSNET_INITTED();

  if ((hp = __MS_gethostbyname(name)) == NULL) {
    werr = XCEWSAGetLastError();
    errno = _winerr2errno(werr);
    return(NULL);
  }

  return(hp);
}

int
gethostname(char *name, size_t buflen)
{
  int werr;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
  ENSURE_MSNET_INITTED();

  if (__MS_gethostname(name, buflen) == SOCKET_ERROR) {
    werr = XCEWSAGetLastError();
    errno = _winerr2errno(werr);
    return(-1);
  }

  return(0);
}

char *
localhost()
{
  char *name;
  const size_t bufsize = 256;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
  ENSURE_MSNET_INITTED();

  if ((name = malloc(bufsize)) == NULL)
    return NULL;

  gethostname(name, bufsize);
  return(name);
}

int
getpeername(int afd, struct sockaddr *addr, socklen_t *addrlen)
{
	SOCKET s;
	int werr;

	WCETRACE(WCE_NETWORK, "%s: fd %d level", __FUNCTION__, afd);

  ENSURE_MSNET_INITTED();

  CHECKFD(afd);
  s = (SOCKET)_fdtab[afd].hnd;

  if (__MS_getpeername(s, addr, addrlen) == SOCKET_ERROR) {
    werr = XCEWSAGetLastError();
    errno =  _winerr2errno(werr);
    return(-1);
  }   

  return(0);
}

int
getsockname(int afd, struct sockaddr *addr, socklen_t *addrlen)
{
  SOCKET s;
  int    werr;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
  ENSURE_MSNET_INITTED();

  CHECKFD(afd);
  s = (SOCKET)_fdtab[afd].hnd;

  if (__MS_getsockname(s, addr, addrlen) == SOCKET_ERROR) {
    werr = XCEWSAGetLastError();
    errno =  _winerr2errno(werr);
    return(-1);
  } 

  return(0);
}

int
getsockopt(int afd, int level, int optname, void *optval, socklen_t *optlen)
{
  SOCKET s;
  int r, werr;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
  ENSURE_MSNET_INITTED();

  CHECKFD(afd);
  s = (SOCKET)_fdtab[afd].hnd;

  WCETRACE(WCE_NETWORK, "getsockopt: fd %d level %d optname %d optval %p optlen %d", 
           afd, level, optname, optval, optlen);
  if ((r = __MS_getsockopt(s, level, optname, optval, optlen)) == SOCKET_ERROR) {
    werr = XCEWSAGetLastError();
    errno =  _winerr2errno(werr);
    WCETRACE(WCE_NETWORK, "getsockopt ERROR r %d errno %d werr %d", r, errno, werr);
    return(-1);
  } 
 
  WCETRACE(WCE_NETWORK, "getsockopt SUCCEEDS, r %d", r);
  return(r);
}

#define IOCPARM_MASK    0x7f            /* parameters must be < 128 bytes */
#define IOC_VOID        0x20000000      /* no parameters */
#define IOC_OUT         0x40000000      /* copy out parameters */
#define IOC_IN          0x80000000      /* copy in parameters */
#define IOC_INOUT       (IOC_IN|IOC_OUT)
/* 0x20000000 distinguishes new &
old ioctl's */
#define _IO(x,y)        (IOC_VOID|((x)<<8)|(y))

#define _IOR(x,y,t)     (IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define _IOW(x,y,t)     (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define FIONREAD    _IOR('f', 127, u_long) /* get # bytes to read */
#define FIONBIO     _IOW('f', 126, u_long) /* set/clear non-blocking i/o */
#define FIOASYNC    _IOW('f', 125, u_long) /* set/clear async i/o */

int
ioctlsocket(int afd, long cmd, u_long *argp)
{
	int r, werr;
	SOCKET s;

	WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
	ENSURE_MSNET_INITTED();

	CHECKFD(afd);
	s = (SOCKET) _fdtab[afd].hnd;

	if((r = __MS_ioctlsocket(s, cmd, argp)) == SOCKET_ERROR)
	{
		werr = XCEWSAGetLastError();
		errno =  _winerr2errno(werr);
		WCETRACE(WCE_NETWORK, "ioctlsocket ERROR r %d errno %d werr %d", r, errno, werr);
		return -1;
	}

	if(cmd == FIONBIO)
	{
		if(*argp == 1)
			_fdtab[afd].flags |= O_NONBLOCK;
		else
			_fdtab[afd].flags &= ~(O_NONBLOCK);
	}

	return r;
}

int
listen(int afd, int backlog)
{
  SOCKET s;
  int r, werr;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
  ENSURE_MSNET_INITTED();

  CHECKFD(afd);
  s = (SOCKET)_fdtab[afd].hnd;

  if ((r = __MS_listen(s, backlog)) == SOCKET_ERROR) {
    werr = XCEWSAGetLastError();
    errno =  _winerr2errno(werr);
	WCETRACE(WCE_NETWORK, "listen ERROR errno %d werr %d", errno, werr);
    return(-1);
  }

  WCETRACE(WCE_IO, "listen: r %d\n", r);
  return(r);
}

ssize_t
recv(int afd, void *buf, size_t len, int flags)
{
  SOCKET s;
  int r, werr;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
  ENSURE_MSNET_INITTED();

  CHECKFD(afd);
  s = (SOCKET)_fdtab[afd].hnd;

  if ((r = __MS_recv(s, buf, len, flags)) == SOCKET_ERROR) {
    werr = XCEWSAGetLastError();
    errno =  _winerr2errno(werr);
	WCETRACE(WCE_NETWORK, "recv ERROR r %d errno %d werr %d", r, errno, werr);
    return(-1);
  }

  return(r);
}

ssize_t
recvfrom(int afd, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
  SOCKET s;
  int r, werr;
  socklen_t slen;
  int frombufsize = *fromlen;
  char *mybuf[1500];
  struct sockaddr_in remaddr;

  ENSURE_MSNET_INITTED();

  CHECKFD(afd);
  s = (SOCKET)_fdtab[afd].hnd;

  WCETRACE(WCE_NETWORK, "%s: fd %d s %d", __FUNCTION__, afd, s);
  slen = sizeof(remaddr);
  memset(&remaddr, 0, sizeof(remaddr));
  if ((r = __MS_recvfrom(s, buf, len, 0, (struct sockaddr *)&remaddr, &slen)) == SOCKET_ERROR) {
    werr = XCEWSAGetLastError();
    errno =  _winerr2errno(werr);
	WCETRACE(WCE_NETWORK, "recvfrom: ERROR r %d errno %d werr %d", r, errno, werr);
    return(-1);
  }

  /* Winsock's recvfrom() only returns a valid 'from' when the socket
   * is connectionless.  Perl expects a valid 'from' for all types
   * of sockets, so go the extra mile.
   */

 *fromlen = slen;
  memcpy(from, &remaddr, slen);
  if (frombufsize == *fromlen)
    getpeername(s, from, fromlen);

  WCETRACE(WCE_NETWORK, "recvfrom: OK and DONE");
  return(r);
}

ssize_t
send(int afd, const void *buf, size_t len, int flags)
{
  SOCKET s;
  int r, werr;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
  ENSURE_MSNET_INITTED();

  CHECKFD(afd);
  s = (SOCKET) _fdtab[afd].hnd;

  if ((r = __MS_send(s, (char *)buf, len, flags)) == SOCKET_ERROR) {
    werr = XCEWSAGetLastError();
    WCETRACE_ERROR(WCE_NETWORK, werr);
    errno =  _winerr2errno(werr);
//    WCETRACE(WCE_NETWORK, "__MS_send: CALLED", __FUNCTION__);
    return(-1);
  }

  return(r);
}

ssize_t
sendto(int afd, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
{
  SOCKET s;
  int r, werr;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);
  ENSURE_MSNET_INITTED();

  CHECKFD(afd);
  s = (SOCKET) _fdtab[afd].hnd;

  if ((r = __MS_sendto(s, buf, len, flags, to, tolen)) == SOCKET_ERROR) {
     werr = XCEWSAGetLastError();
     errno = _winerr2errno(werr);
     return(-1);
  }

  return(r);
}

int
setsockopt(int afd, int level, int optname, const void *optval, socklen_t optlen)
{
  SOCKET s;
  int r, werr;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);

  ENSURE_MSNET_INITTED();

  CHECKFD(afd);
  s = (SOCKET)_fdtab[afd].hnd;

  WCETRACE(WCE_NETWORK, "setsockopt: fd %d level %d optname %d optval %p optlen %d", 
           afd, level, optname, optval, optlen);
  if ((r = __MS_setsockopt(s, level, optname, (char *)optval, optlen)) == SOCKET_ERROR) {
      werr = XCEWSAGetLastError();
      if (werr == WSAENOPROTOOPT) {
       	errno = EINVAL;
      } else {
        errno = _winerr2errno(werr);
      }
      WCETRACE(WCE_NETWORK, "setsockopt ERROR r %d errno %d werr %d", r, errno, werr);
      return(-1);
    }

  WCETRACE(WCE_NETWORK, "setsockopt SUCCEEDS, r %d", r);
  return(r);
}

int
shutdown(int fd, int how)
{
  SOCKET s;
  int r, werr;

  ENSURE_MSNET_INITTED();

  CHECKFD(fd);
  s = (SOCKET)_fdtab[fd].hnd;

  WCETRACE(WCE_IO, "shutdown: fd = %d s = %d", fd, s);
  if ((r = __MS_shutdown(s, how)) < 0) {
    werr = XCEWSAGetLastError();
    errno =  _winerr2errno(werr);
  }
  WCETRACE(WCE_IO, "__MS_shutdown returns %d errno %d werr %d", r, errno, werr);
  return(r);
}

#if 0
int
select(int n, fd_set *rfds, fd_set *wfds, fd_set *xfds, struct timeval *timeout)
{
	int i;
	int status;
	fd_set r, w, x;
	int werr, sockmap[MAXFDS];
	SOCKET s;

	WCETRACE(WCE_IO, "select(%d, %p, %p, %p, %p)", n, rfds, wfds, xfds, timeout);
	if (timeout != NULL) {
		WCETRACE(WCE_IO, "select: timeout {%d,%d}", timeout->tv_sec, timeout->tv_usec);
	}

	/* Initialze sockmap - this is the SOCKET->fd mapping for use later */
	for (i = 0; i < MAXFDS; i++)
		sockmap[i] = -1;

	if (rfds != NULL) {
		if (_fd_to_socket(rfds, sockmap) < 0) {
			WCETRACE(WCE_IO, "select: bad rfds");
			return(-1);
		}
	}

	if (wfds != NULL) {
		if (_fd_to_socket(wfds, sockmap) < 0) {
			WCETRACE(WCE_IO, "select: bad wfds");
			return(-1);
		}
	}

	if (xfds != NULL) {
		if (_fd_to_socket(xfds, sockmap) < 0) {
			WCETRACE(WCE_IO, "select: bad xfds");
			return(-1);
		}
	}

	status = __MS_select(n, rfds, wfds, xfds, timeout);
	WCETRACE(WCE_IO, "select: returns %d errno %d", status, errno);

	/* Give up here if there is an select error */
	if (status == SOCKET_ERROR) {
		int werr = XCEWSAGetLastError();
		errno = _winerr2errno(werr);
		return(-1);
	}

	/* Finally we must translate socket descriptors back to fds */
	if (rfds != NULL)
		_socket_to_fd(rfds, sockmap);

	if (wfds != NULL) 
		_socket_to_fd(wfds, sockmap);

	if (xfds != NULL) 
		_socket_to_fd(xfds, sockmap);   

	return(status);
}
#endif


// exfds are currently not used!

int
select(int nfds, fd_set *rdfds, fd_set *wrfds, fd_set *exfds, 
		  const struct timeval* timeout)
{
	int i;
	fd_set r2, w2, x2;
	fd_set rout, wout, xout;
	fd_set *pr2, *pw2, *px2;
	int res = 0;
	int ftype;
	struct timeval t2;
	int terminal_ready = 0;
	int terminal_write = 0;
  int terminal_error = 0;
	int pipe_readable = 0;
	int pipe_writeable = 0;
  int pipe_error = 0;
	int cnt;
	MSG msg;
	int non_sockets_ready;

	pr2 = pw2 = px2 = NULL;

again:
	pipe_readable = pipe_writeable = 0;
	non_sockets_ready = 0;

#if 1
// ### TODO: Are these really needed on SDK headers?
//#define PM_NOREMOVE 0x0000
//#define WM_KEYDOWN 0x0100
	if(PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE) != FALSE)
	{
		if(msg.message != WM_KEYDOWN)
		{
			GetMessageW(&msg, NULL, 0, 0);
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
#endif

	if(rdfds) 
	{
		pr2 = &r2;

		FD_ZERO(pr2);

		for(i = 0; i < rdfds->fd_count; i++)
		{
			int s = rdfds->fd_array[i];

			ftype = getfiletype(s);

			if(ISPIPEHANDLE(get_osfhandle(s)))
			{
				char c;
				DWORD dwRead, dwAvail, dwMsg;

				if(XCEPeekNamedPipe(get_osfhandle(s), &c, 1, 
					&dwRead, &dwAvail, &dwMsg) == FALSE)
				{
					// we must exit the loop...
					if(GetLastError() == ERROR_BROKEN_PIPE)
					{
						pipe_readable = 1;
						non_sockets_ready++;
					}
				}
				else if(dwAvail != 0)
				{
					pipe_readable = 1;
					non_sockets_ready++;
				}
			}
			else if(ftype == IO_FILE_TYPE_SOCKET)
			{
				//WCETRACE(WCE_NETWORK, "adding sock %d to read set", get_osfhandle(s));

				FD_SET((SOCKET) get_osfhandle(s), pr2);
			}
			else if(ftype == IO_FILE_TYPE_CONSOLE)
			{
				//WCETRACE(WCE_NETWORK, "adding console %d to read set", s);

				if(PeekMessageW(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN,
					PM_NOREMOVE) != FALSE)
				{
					WCETRACE(WCE_NETWORK, "Terminal read ready");
					terminal_ready = 1;
					non_sockets_ready++;
				}
			}
			else
			{
				XCEShowMessageA("Bad fd in select");
				abort();

				errno = EBADF;
				return -1;
			}
		}
	}

	if(wrfds) 
	{
		pw2 = &w2;

		FD_ZERO(pw2);

		for(i = 0; i < wrfds->fd_count; i++)
		{
			int s = wrfds->fd_array[i];

			ftype = getfiletype(s);

			if(ISPIPEHANDLE(get_osfhandle(s)))
			{
				pipe_writeable = 1;
				non_sockets_ready++;
			}
			else if(ftype == IO_FILE_TYPE_SOCKET)
			{
				WCETRACE(WCE_NETWORK, "adding sock %d to write set", get_osfhandle(s));
				FD_SET((SOCKET) get_osfhandle(s), pw2);
        WCETRACE(WCE_NETWORK, "pw2->fd_count == %d", pw2->fd_count);
			}
			else if(ftype == IO_FILE_TYPE_CONSOLE)
			{
				//WCETRACE(WCE_NETWORK, "adding console %d to write set", s);
				terminal_write = 1;
				non_sockets_ready++;
			}
			else
			{
				XCEShowMessageA("Bad fd in xceselect");
//				abort();
				errno = EBADF;
				return -1;
			}
		}
	}

  if(exfds)
  {
    px2 = &x2;

    FD_ZERO(px2);

    for(i = 0; i < exfds->fd_count; i++)
    {
      int s = exfds->fd_array[i];

      ftype = getfiletype(s);

      if(ISPIPEHANDLE(get_osfhandle(s)))
      {
        pipe_error = 1;
        non_sockets_ready++;
      }
      else if(ftype == IO_FILE_TYPE_SOCKET)
      {
        WCETRACE(WCE_NETWORK, "adding sock %d to error set", get_osfhandle(s));
        FD_SET((SOCKET) get_osfhandle(s), px2);
        WCETRACE(WCE_NETWORK, "px2->fd_count == %d", px2->fd_count);
      }
      else if(ftype == IO_FILE_TYPE_CONSOLE)
      {
        //WCETRACE(WCE_NETWORK, "adding console %d to error set", s);
        terminal_error = 1;
        non_sockets_ready++;
      }
      else
      {
        XCEShowMessageA("Bad fd in xceselect");
        errno = EBADF;
        return -1;
      }
    }
  }

	if(timeout == NULL)
	{
		// must have timeout when waiting for keyboard
		t2.tv_sec = 0;
		t2.tv_usec = 100;

		if(non_sockets_ready)
			t2.tv_usec = 0;

		res = __MS_select(nfds, pr2, pw2, px2, &t2);

		if(res == 0 && non_sockets_ready == 0)
		{
			// using sleep is much better for sftp
			Sleep(10);
			goto again;
		}
	}
	else if(non_sockets_ready)
	{
		t2.tv_sec = 0;
		t2.tv_usec = 0;

		res = __MS_select(nfds, pr2, pw2, px2, &t2);
	}
	else
	{
		WCETRACE(WCE_NETWORK, "WARNING: select() with timeout");
		res = __MS_select(nfds, pr2, pw2, px2, timeout);
	}

	if(res == SOCKET_ERROR)
	{
		return -1;
	}

	res = 0;

	if(pr2)
	{
		cnt = rdfds->fd_count;

		FD_ZERO(&rout);

		WCETRACE(WCE_NETWORK, "Checking %d original fds for read", cnt);

		for(i = 0; i < cnt; i++)
		{
			int s = rdfds->fd_array[i];

			ftype = getfiletype(s);

			WCETRACE(WCE_NETWORK, "Checking read fd %d, os handle %x",
				s, get_osfhandle(s));

			if(ISPIPEHANDLE(get_osfhandle(s)))
			{
				char c;
				DWORD dwRead, dwAvail, dwMsg;

				if(XCEPeekNamedPipe(get_osfhandle(s), &c, 1, 
					&dwRead, &dwAvail, &dwMsg) == FALSE)
				{
					// reader needs not get aware of this...
//					if(GetLastError() == ERROR_BROKEN_PIPE)
	//					FD_SET(s, &rout);
				}
				else if(dwAvail == 0)
				{
					WCETRACE(WCE_NETWORK, "pipe %d not readable", s);
				}
				else
				{
					WCETRACE(WCE_NETWORK, "pipe %d readable", s);
					FD_SET(s, &rout);
				}
			}
			else if(ftype == IO_FILE_TYPE_SOCKET)
			{
				if(!FD_ISSET(get_osfhandle(s), pr2))
				{
					WCETRACE(WCE_NETWORK, "socket %d not readable", s);
				}
				else
				{
					WCETRACE(WCE_NETWORK, "socket %d readable", s);
					FD_SET(s, &rout);
				}
			}
			else if(ftype == IO_FILE_TYPE_CONSOLE)
			{
				if(terminal_ready)
				{
					WCETRACE(WCE_NETWORK, "console %d readable", s);
					FD_SET(s, &rout);
				}
				else
				{
					WCETRACE(WCE_NETWORK, "console %d not readable", s);
				}
			}
		}

		res += rout.fd_count;
		memcpy(rdfds, &rout, sizeof(fd_set));
	}

	if(pw2)
	{
		cnt = wrfds->fd_count;

		FD_ZERO(&wout);

		WCETRACE(WCE_NETWORK, "Checking %d original fds for write", cnt);

		for(i = 0; i < cnt; i++)
		{
			int s = wrfds->fd_array[i];

			WCETRACE(WCE_NETWORK, "Checking write fd %d, os handle %x",
				s, get_osfhandle(s));

			ftype = getfiletype(s);

			if(ISPIPEHANDLE(get_osfhandle(s)))
			{
				WCETRACE(WCE_NETWORK, "pipe %d writeable", s);
				FD_SET(s, &wout);
			}
			else if(ftype == IO_FILE_TYPE_SOCKET)
			{
				if(!FD_ISSET(get_osfhandle(s), pw2))
				{
					WCETRACE(WCE_NETWORK, "socket %d not writeable", s);
				}
				else
				{
					FD_SET(s, &wout);
					WCETRACE(WCE_NETWORK, "socket %d writeable", s);
				}
			}
			else if(ftype == IO_FILE_TYPE_CONSOLE)
			{
				WCETRACE(WCE_NETWORK, "console %d writeable", s);
				FD_SET(s, &wout);
			}
			else
			{
				XCEShowMessageA("Illegal fd in select");
				DebugBreak();
			}
		}

		res += wout.fd_count;
		memcpy(wrfds, &wout, sizeof(fd_set));
	}

  if(px2)
  {
    cnt = exfds->fd_count;

    FD_ZERO(&xout);

    WCETRACE(WCE_NETWORK, "Checking %d original fds for errors", cnt);

    for(i = 0; i < cnt; i++)
    {
      int s = exfds->fd_array[i];

      ftype = getfiletype(s);

      WCETRACE(WCE_NETWORK, "Checking error fd %d, os handle %x",
        s, get_osfhandle(s));

      if(ISPIPEHANDLE(get_osfhandle(s)))
      {
        char c;
        DWORD dwRead, dwAvail, dwMsg;

        if(XCEPeekNamedPipe(get_osfhandle(s), &c, 1, 
          &dwRead, &dwAvail, &dwMsg) == FALSE)
        {
          // reader needs not get aware of this...
          if(GetLastError() == ERROR_BROKEN_PIPE)
            FD_SET(s, &xout);
        }
      }
      else if(ftype == IO_FILE_TYPE_SOCKET)
      {
        if(!FD_ISSET(get_osfhandle(s), px2))
        {
          WCETRACE(WCE_NETWORK, "socket %d has no errors", s);
        }
        else
        {
          WCETRACE(WCE_NETWORK, "socket %d has errors", s);
          FD_SET(s, &xout);
        }
      }
      else if(ftype == IO_FILE_TYPE_CONSOLE)
      {
      }
    }

    res += xout.fd_count;
    memcpy(exfds, &xout, sizeof(fd_set));
  }


	WCETRACE(WCE_NETWORK, "select finally returns %d", res);
	return res;
}

int
socket(int af, int type, int protocol)
{
  SOCKET s;
  int fd, werr;

  WCETRACE(WCE_NETWORK, "%s: CALLED", __FUNCTION__);

  ENSURE_MSNET_INITTED();

  if ((s = __MS_socket(af, type, protocol)) == INVALID_SOCKET) {
    werr = XCEWSAGetLastError();
    errno =  _winerr2errno(werr);
    WCETRACE(WCE_NETWORK, "socket: ERROR errno %d", errno);
    return(-1);
  }

  fd = _getnewfd();

  _fdtab[fd].fd = fd;
  _fdtab[fd].type = IO_FILE_TYPE_SOCKET;
  _fdtab[fd].sock = s;
  _fdtab[fd].flags = 0;

  WCETRACE(WCE_NETWORK, "socket: fd = %d, handle = %d", fd, s);

  return(fd);
}

int
closesocket(int fd)
{
	int ftype = getfiletype(fd);
	if (ftype != IO_FILE_TYPE_SOCKET)
		return -1;
	SOCKET s = get_osfhandle(fd);
	return __MS_closesocket(s);
}
