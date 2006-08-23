#ifndef AF_IRDA_H
#define AF_IRDA_H

#include <winsock.h>

typedef struct _SOCKADDR_IRDA {
  u_short irdaAddressFamily;
  u_char irdaDeviceID[4];
  char irdaServiceName[25];
} SOCKADDR_IRDA, *PSOCKADDR_IRDA, *LPSOCKADDR_IRDA;

typedef struct IRDA_DEVICE_INFO {
  u_char irdaDeviceID[4];
  char irdaDeviceName[22];
  u_char Reserved[2];
} _IRDA_DEVICE_INFO;
typedef _IRDA_DEVICE_INFO IRDA_DEVICE_INFO;

typedef struct DEVICELIST {
  ULONG numDevice;
  IRDA_DEVICE_INFO Device [1];
} _DEVICELIST, *PWCE_DEVICELIST;

typedef _DEVICELIST DEVICELIST;

#endif
