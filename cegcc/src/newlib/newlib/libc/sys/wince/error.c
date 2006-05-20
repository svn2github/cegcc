#include <stdlib.h>
#include <errno.h>

#include "sys/wcefile.h"
#include "sys/wceerror.h"

int
_winerr2errno(DWORD werror)
{
  switch(werror) {
  case 0:
    return 0;
  case ERROR_FILE_NOT_FOUND:
  case ERROR_PATH_NOT_FOUND:
    return ENOENT;
  case ERROR_ACCESS_DENIED:
    return EACCES;
  case ERROR_DEV_NOT_EXIST:
    return ENODEV;
  case ERROR_ALREADY_EXISTS:
    return EEXIST;
  case ERROR_DIR_NOT_EMPTY:
    return ENOTEMPTY;

  /* Winsock Errors */
  case WSAEWOULDBLOCK:
    return EWOULDBLOCK;
  case WSAEINPROGRESS:
    return EINPROGRESS;
  case WSAEALREADY:
    return EALREADY;
  case WSAENOTSOCK:
    return ENOTSOCK;
  case WSAEDESTADDRREQ:
    return EDESTADDRREQ;
  case WSAEMSGSIZE:
    return EMSGSIZE;
  case WSAEPROTOTYPE:
    return EPROTOTYPE;
  case WSAENOPROTOOPT:
    return ENOPROTOOPT;
  case WSAEPROTONOSUPPORT:
    return EPROTONOSUPPORT;
  case WSAESOCKTNOSUPPORT:
    return ESOCKTNOSUPPORT;
  case WSAEOPNOTSUPP:
    return WSAEOPNOTSUPP;
  case WSAEPFNOSUPPORT:
    return EPFNOSUPPORT;
  case WSAEAFNOSUPPORT:
    return EAFNOSUPPORT;
  case WSAEADDRINUSE:
    return EADDRINUSE;
  case WSAEADDRNOTAVAIL:
    return EADDRNOTAVAIL;
  case WSAENETDOWN:
    return ENETDOWN;
  case WSAENETUNREACH:
    return ENETUNREACH;
  case WSAENETRESET:
    return ENETRESET;
  case WSAECONNABORTED:
    return ECONNABORTED;
  case WSAECONNRESET:
    return ECONNRESET;
  case WSAENOBUFS:
    return ENOBUFS;
  case WSAEISCONN:
    return EISCONN;
  case WSAENOTCONN:
    return ENOTCONN;
  case WSAESHUTDOWN:
    return ESHUTDOWN;
  case WSAETOOMANYREFS:
    return ETOOMANYREFS;
  case WSAETIMEDOUT:
    return ETIMEDOUT;
  case WSAECONNREFUSED:
    return ECONNREFUSED;
  case WSAELOOP:
    return ELOOP;
  case WSAENAMETOOLONG:
    return ENAMETOOLONG;
  case WSAEHOSTDOWN:
    return EHOSTDOWN;
  case WSAEHOSTUNREACH:
    return EHOSTUNREACH;
  case WSAENOTEMPTY:
    return ENOTEMPTY;
  case WSAEPROCLIM:
    return EPROCLIM;
  case WSAEUSERS:
    return EUSERS;
  case WSAEDQUOT:
    return EDQUOT;
  case WSAESTALE:
    return ESTALE;
  case WSAEREMOTE:
    return EREMOTE;

  case WSAEDISCON:
    return ENOTSUP;

  default:
    return ENOTSUP;
  }
  return ENOTSUP;
}
