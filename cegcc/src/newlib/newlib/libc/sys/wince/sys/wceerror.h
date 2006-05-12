#ifndef _WCEERROR_H_
#define _WCEERROR_H_

#include "sys/wcetypes.h"

#define ERROR_SUCCESS              0L
#define NO_ERROR                   0L
#define ERROR_INVALID_FUNCTION     1L
#define ERROR_FILE_NOT_FOUND       2L
#define ERROR_PATH_NOT_FOUND       3L
#define ERROR_TOO_MANY_OPEN_FILES  4L
#define ERROR_ACCESS_DENIED        5L
#define ERROR_INVALID_HANDLE       6L
#define ERROR_ARENA_TRASHED        7L
#define ERROR_NOT_ENOUGH_MEMORY    8L
#define ERROR_INVALID_BLOCK        9L
#define ERROR_BAD_ENVIRONMENT      10L
#define ERROR_BAD_FORMAT           11L
#define ERROR_INVALID_ACCESS       12L
#define ERROR_INVALID_DATA         13L
#define ERROR_OUTOFMEMORY          14L
#define ERROR_INVALID_DRIVE        15L
#define ERROR_CURRENT_DIRECTORY    16L
#define ERROR_NOT_SAME_DEVICE      17L
#define ERROR_NO_MORE_FILES        18L
#define ERROR_WRITE_PROTECT        19L
#define ERROR_BAD_UNIT             20L
#define ERROR_NOT_READY            21L
#define ERROR_BAD_COMMAND          22L
#define ERROR_CRC                  23L
#define ERROR_BAD_LENGTH           24L
#define ERROR_SEEK                 25L
#define ERROR_NOT_DOS_DISK         26L
#define ERROR_SECTOR_NOT_FOUND     27L
#define ERROR_OUT_OF_PAPER         28L
#define ERROR_WRITE_FAULT          29L
#define ERROR_READ_FAULT           30L
#define ERROR_GEN_FAILURE          31L
#define ERROR_SHARING_VIOLATION    32L
#define ERROR_LOCK_VIOLATION       33L
#define ERROR_WRONG_DISK           34L
#define ERROR_SHARING_BUFFER_EXCEEDED 35L
#define ERROR_HANDLE_EOF           38L
#define ERROR_HANDLE_DISK_FULL     39L
#define ERROR_NOT_SUPPORTED        50L
#define ERROR_REM_NOT_LIST         51L
#define ERROR_DEV_NOT_EXIST        55L
#define ERROR_FILE_EXISTS          80L
#define ERROR_BROKEN_PIPE          109L
#define ERROR_OPEN_FAILED          110L
#define ERROR_DISK_FULL            112L
#define ERROR_DIR_NOT_ROOT         144L
#define ERROR_DIR_NOT_EMPTY        145L
#define ERROR_TOO_MANY_TCBS        155L
#define ERROR_ALREADY_EXISTS       183L
#define ERROR_NO_DATA              232L
#define ERROR_MORE_DATA            234L

#define ERROR_INVALID_ADDRESS      487L

#define WAIT_TIMEOUT               258L


/* Winsock Errors */

/*
 * All Windows Sockets error constants are biased by WSABASEERR from
 * the "normal"
 */
#define WSABASEERR              10000
/*
 * Windows Sockets definitions of regular Microsoft C error constants
 */
#define WSAEINTR                (WSABASEERR+4)
#define WSAEBADF                (WSABASEERR+9)
#define WSAEACCES               (WSABASEERR+13)
#define WSAEFAULT               (WSABASEERR+14)
#define WSAEINVAL               (WSABASEERR+22)
#define WSAEMFILE               (WSABASEERR+24)

/*
 * Windows Sockets definitions of regular Berkeley error constants
 */
#define WSAEWOULDBLOCK          (WSABASEERR+35)
#define WSAEINPROGRESS          (WSABASEERR+36)
#define WSAEALREADY             (WSABASEERR+37)
#define WSAENOTSOCK             (WSABASEERR+38)
#define WSAEDESTADDRREQ         (WSABASEERR+39)
#define WSAEMSGSIZE             (WSABASEERR+40)
#define WSAEPROTOTYPE           (WSABASEERR+41)
#define WSAENOPROTOOPT          (WSABASEERR+42)
#define WSAEPROTONOSUPPORT      (WSABASEERR+43)
#define WSAESOCKTNOSUPPORT      (WSABASEERR+44)
#define WSAEOPNOTSUPP           (WSABASEERR+45)
#define WSAEPFNOSUPPORT         (WSABASEERR+46)
#define WSAEAFNOSUPPORT         (WSABASEERR+47)
#define WSAEADDRINUSE           (WSABASEERR+48)
#define WSAEADDRNOTAVAIL        (WSABASEERR+49)
#define WSAENETDOWN             (WSABASEERR+50)
#define WSAENETUNREACH          (WSABASEERR+51)
#define WSAENETRESET            (WSABASEERR+52)
#define WSAECONNABORTED         (WSABASEERR+53)
#define WSAECONNRESET           (WSABASEERR+54)
#define WSAENOBUFS              (WSABASEERR+55)
#define WSAEISCONN              (WSABASEERR+56)
#define WSAENOTCONN             (WSABASEERR+57)
#define WSAESHUTDOWN            (WSABASEERR+58)
#define WSAETOOMANYREFS         (WSABASEERR+59)
#define WSAETIMEDOUT            (WSABASEERR+60)
#define WSAECONNREFUSED         (WSABASEERR+61)
#define WSAELOOP                (WSABASEERR+62)
#define WSAENAMETOOLONG         (WSABASEERR+63)
#define WSAEHOSTDOWN            (WSABASEERR+64)
#define WSAEHOSTUNREACH         (WSABASEERR+65)
#define WSAENOTEMPTY            (WSABASEERR+66)
#define WSAEPROCLIM             (WSABASEERR+67)
#define WSAEUSERS               (WSABASEERR+68)
#define WSAEDQUOT               (WSABASEERR+69)
#define WSAESTALE               (WSABASEERR+70)
#define WSAEREMOTE              (WSABASEERR+71)

#define WSAEDISCON              (WSABASEERR+101)

/* Extended Windows Sockets error constant definitions */
#define WSASYSNOTREADY          (WSABASEERR+91)
#define WSAVERNOTSUPPORTED      (WSABASEERR+92)
#define WSANOTINITIALISED       (WSABASEERR+93)

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (when using the resolver). Note that these errors are
 * retrieved via WSAGetLastError() and must therefore follow
 * the rules for avoiding clashes with error numbers from
 * specific implementations or language run-time systems.
 * For this reason the codes are based at WSABASEERR+1001.
 * Note also that [WSA]NO_ADDRESS is defined only for
 * compatibility purposes.
 */
 

/* Authoritative Answer: Host not found */
#define WSAHOST_NOT_FOUND       (WSABASEERR+1001)

/* Non-Authoritative: Host not found, or SERVERFAIL */
#define WSATRY_AGAIN            (WSABASEERR+1002)

/* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define WSANO_RECOVERY          (WSABASEERR+1003)

/* Valid name, no data record of requested type */
#define WSANO_DATA              (WSABASEERR+1004)

#ifdef __cplusplus
extern "C" {
#endif

int _winerr2errno(DWORD werror);

#ifdef __cplusplus
}
#endif
#endif  /* _WCEERROR_H_ */
