#ifndef _WCEIPFCNS_H_
#define _WCEIPFCNS_H_

#include <sys/wcebase.h>
#include <sys/wcenetwork.h>

#define MIB_IF_TYPE_OTHER               1
#define MIB_IF_TYPE_ETHERNET            6
#define MIB_IF_TYPE_TOKENRING           9
#define MIB_IF_TYPE_FDDI                15
#define MIB_IF_TYPE_PPP                 23
#define MIB_IF_TYPE_LOOPBACK            24
#define MIB_IF_TYPE_SLIP                28

#define MIB_IF_ADMIN_STATUS_UP          1
#define MIB_IF_ADMIN_STATUS_DOWN        2
#define MIB_IF_ADMIN_STATUS_TESTING     3

#ifdef __cplusplus
extern "C" {
#endif

int AdapterInfoByName(char *name, IP_ADAPTER_INFO *outbuf);
int AdapterInfoIPIsValid(IP_ADAPTER_INFO *ai);
int AdapterTypeString(unsigned int type, char *buf, int buflen);
void PrintAdapterInfo(IP_ADAPTER_INFO *ai);
int PrintAllAdaptersInfo();

#ifdef __cplusplus
}
#endif
#endif  /* _WCEIPFCNS_H_ */
