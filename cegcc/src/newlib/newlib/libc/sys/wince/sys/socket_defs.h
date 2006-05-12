#ifndef SOCKET_DEFS_H
#define SOCKET_DEFS_H

#ifndef _WINSOCKAPI_
#ifndef _COMPILING_NEWLIB
#if 0
#define accept xceaccept
#define bind xcebind
#define connect xceconnect
#define gethostbyaddr xcegethostbyaddr
#define gethostbyname xcegethostbyname
#define gethostname xcegethostname
#define localhost xcelocalhost
#define getpeername xcegetpeername
#define getsockname xcegetsockname
#define getsockopt xcegetsockopt
#define ioctlsocket xceioctlsocket
#define listen xcelisten
#define recv xcerecv
#define recvfrom xcerecvfrom
#define send xcesend
#define sendto xcesendto
#define setsockopt xcesetsockopt
#define shutdown xceshutdown
#define select xceselect
#define socket xcesocket
#define closesocket xceclosesocket
#endif

#endif
#endif

#endif
