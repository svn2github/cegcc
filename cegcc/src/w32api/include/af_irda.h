#ifndef AF_IRDA_H
#define AF_IRDA_H

#include <winsock.h>

typedef struct _SOCKADDR_IRDA {
  u_short irdaAddressFamily;
  u_char irdaDeviceID[4];
  char irdaServiceName[25];
} SOCKADDR_IRDA, 
 *PSOCKADDR_IRDA, 
 *LPSOCKADDR_IRDA;

#endif
