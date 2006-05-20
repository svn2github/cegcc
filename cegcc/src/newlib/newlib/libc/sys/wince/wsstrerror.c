// wsstrerror.c
//
// Time-stamp: <03/01/02 22:22:54 keuchel@netwave.de>

#include "sys/wcebase.h"
#include "sys/wceerror.h"

#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#define WSAGetLastError GetLastError

#define _E(x,y) {x, y}

static struct {
  int code;
  char *txt;
} errtab[] = {
  /* windows errors */
  _E(ERROR_SUCCESS,        "No error"),
  _E(ERROR_INVALID_FUNCTION, "Invalid function"),
  _E(ERROR_FILE_NOT_FOUND, "File not found"),
  _E(ERROR_PATH_NOT_FOUND, "Path not found"),
  _E(ERROR_TOO_MANY_OPEN_FILES, "Too many open files"),
  _E(ERROR_ACCESS_DENIED,  "Access denied"),
  _E(ERROR_INVALID_HANDLE, "Invalid handle"),
  _E(ERROR_INVALID_ACCESS, "Invalid access"),
  _E(ERROR_INVALID_DATA,   "Invalid data"),
  _E(ERROR_NOT_ENOUGH_MEMORY, "Out of memory"),
  _E(ERROR_WRITE_PROTECT,  "Write protected"),
  _E(ERROR_HANDLE_EOF,     "Handle EOF"),
  _E(ERROR_NOT_SUPPORTED,  "Operation not supported"),
  _E(ERROR_FILE_EXISTS,    "File exists"),
  _E(ERROR_BROKEN_PIPE,    "Broken pipe"),
  _E(ERROR_OPEN_FAILED,    "Open failed"),
  _E(ERROR_DISK_FULL,      "No space left on device"),
  _E(ERROR_DIR_NOT_EMPTY,  "Directory not empty"),
  _E(ERROR_NO_DATA,        "No data available"),
  _E(ERROR_MORE_DATA,      "More data available"),
  /* winsock */
  _E(WSAEINTR,             "Interupted system call"),
  _E(WSAEBADF,             "Bad file number"),
  _E(WSAEACCES,            "Access denied"),
  _E(WSAEFAULT,            "EFAULT returned"),
  _E(WSAEINVAL,            "Invalid argument"),
  _E(WSAEMFILE,            "EMFILE returned"),
  _E(WSAEWOULDBLOCK,       "Operation would block"),
  _E(WSAEINPROGRESS,       "Operation now in progress"),
  _E(WSAEALREADY,          "Operation already in progress"),
  _E(WSAENOTSOCK,          "Socket operation on non-socket"),
  _E(WSAEDESTADDRREQ,      "Destination address required"),
  _E(WSAEMSGSIZE,          "Message too long"),
  _E(WSAEPROTOTYPE,        "EPROTOTYPE returned"),
  _E(WSAENOPROTOOPT,       "Bad protocol option"),
  _E(WSAEPROTONOSUPPORT,   "Protocol not supported"),
  _E(WSAESOCKTNOSUPPORT,   "Socket type not supported"),
  _E(WSAEOPNOTSUPP,        "Bad option"),
  _E(WSAEPFNOSUPPORT,      "Protocol family not supported"),
  _E(WSAEAFNOSUPPORT,      "Address family not supported by protocol family"),
  _E(WSAEADDRINUSE,        "Address already in use"),
  _E(WSAEADDRNOTAVAIL,     "Can't assign requested address"),
  _E(WSAENETDOWN,          "Network is down"),
  _E(WSAENETUNREACH,       "Network is unreachable"),
  _E(WSAENETRESET,         "Network was reset"),
  _E(WSAECONNABORTED,      "Software caused connection abort"),
  _E(WSAECONNRESET,        "Connection reset by peer"),
  _E(WSAENOBUFS,           "No buffer space is supported"),
  _E(WSAEISCONN,           "Socket is already connected"),
  _E(WSAENOTCONN,          "Socket is not connected"),
  _E(WSAESHUTDOWN,         "Connection shut down"),
  _E(WSAETOOMANYREFS,      "Too many references"),
  _E(WSAETIMEDOUT,         "Connection timed out"),
  _E(WSAECONNREFUSED,      "Connection refused"),
  _E(WSAELOOP,             "Loop detected"),
  _E(WSAENAMETOOLONG,      "Name too long"),
  _E(WSAEHOSTDOWN,         "Host down"),
  _E(WSAEHOSTUNREACH,      "Host unreachable"),
  _E(WSAENOTEMPTY,         "Directory not empty"),
  _E(WSAEPROCLIM,          "EPROCLIM returned"),
  _E(WSAEUSERS,            "EUSERS returned"),
  _E(WSAEDQUOT,            "Disk quota exceeded"),
  _E(WSAESTALE,            "ESTALE returned"),
  _E(WSAEREMOTE,           "Object is remote"),
  _E(WSAEDISCON,           "Disconnected"),
  _E(WSASYSNOTREADY,       "System not ready"),
  _E(WSAVERNOTSUPPORTED,   "Version is not supported"),
  _E(WSANOTINITIALISED,    "Sockets library not initialized"),
  /* + 1001 */
  _E(WSAHOST_NOT_FOUND,    "Host not found"),
  _E(WSATRY_AGAIN,         "Host not found"),
  _E(WSANO_DATA,           "Host not found"),
  -1, NULL
};

__IMPORT char *
wsstrerror(int code)
{
  int i;
  static char buf[100];

  for(i = 0; errtab[i].code != -1; i++)
    if(errtab[i].code == code)
      return errtab[i].txt;
  sprintf(buf, "Error %d\n", code);
  return buf;
}

__IMPORT void
wserror(char *fmt, ...)
{
  int code = WSAGetLastError();
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, ": %s\n", wsstrerror(code));
  va_end(ap);
}

__IMPORT char *
winstrerror(int code)
{
  int i;
  static char buf[100];

  for(i = 0; errtab[i].code != -1; i++)
    {
      if(errtab[i].code == code)
	return errtab[i].txt;
    }
  sprintf(buf, "Error %d\n", code);
  return buf;
}

__IMPORT void
winperror(char *fmt, ...)
{
  int code = GetLastError();
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, ": %s\n", winstrerror(code));
  va_end(ap);
}
