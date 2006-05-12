// ceirda.h
// 
// Time-stamp: <31/05/01 07:43:05 keuchel@w2k>

#ifndef CEIRDA_H
#define CEIRDA_H

#include <_ansi.h>

typedef struct _WCE_IRDA_DEVICE_INFO
{
	unsigned char  irdaDeviceID[4];
	char	irdaDeviceName[22];
	unsigned char  dummy__reserved[2];
} WCE_IRDA_DEVICE_INFO, *PWCE_IRDA_DEVICE_INFO, IRDA_DEVICE_INFO, *PIRDA_DEVICE_INFO;

typedef struct _WCE_DEVICELIST
{
	unsigned long                numDevice;
	WCE_IRDA_DEVICE_INFO        Device[1];
} WCE_DEVICELIST, *PWCE_DEVICELIST, DEVICELIST, *PDEVICELIST;

typedef struct _SOCKADDR_IRDA
{
	unsigned short irdaAddressFamily;
	unsigned char  irdaDeviceID[4];
	char	irdaServiceName[25];
} SOCKADDR_IRDA, *PSOCKADDR_IRDA, *LPSOCKADDR_IRDA;

#define SOL_IRLMP		        0x00FF                        
#define IRLMP_ENUMDEVICES       0x00000010

// build address, does a device enumeration
__IMPORT int irda_getaddr(const char *hname, const char *sname, 
						  SOCKADDR_IRDA *ir_addr);

#endif
