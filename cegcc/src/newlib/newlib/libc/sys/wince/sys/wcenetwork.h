#ifndef _WCENETWORK_H_
#define _WCENETWORK_H_

#ifndef _WINSOCK2API_
#define _WINSOCK2API_

#include "wcesocktypes.h"

#define _WINSOCKAPI_

// should go to global define
#define __USE_W32_SOCKETS

#include <sys/wcetypes.h>
#include <sys/wcebase.h>
#include <sys/wcefile.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR    (-1)

#define WSADESCRIPTION_LEN   (256)
#define WSASYS_STATUS_LEN    (128)

typedef struct WSAData {
  WORD      wVersion;
  WORD      wHighVersion;
  char      szDescription[WSADESCRIPTION_LEN+1];
  char      szSystemStatus[WSASYS_STATUS_LEN+1];
  USHORT    iMaxSockets;
  USHORT    iMaxUdpDg;
  char     *lpVendorInfo;
} WSADATA;

#ifdef __cplusplus
extern "C" {
#endif

int __MS_accept(SOCKET, struct sockaddr *, int *);
int __MS_bind(SOCKET, const struct sockaddr *, int);
int __MS_connect(SOCKET, const struct sockaddr *, int);
struct hostent *__MS_gethostbyaddr(const char *, int, int);
struct hostent *__MS_gethostbyname(const char *);
int __MS_gethostname(char *, size_t);
int __MS_getpeername(SOCKET, struct sockaddr *, int *);
int __MS_getsockname(SOCKET, struct sockaddr *, int *);
int __MS_getsockopt(SOCKET, int, int, char *, int *);
int __MS_ioctlsocket(SOCKET, long, unsigned long*);
int __MS_listen(SOCKET, int);
int __MS_recv(SOCKET, char *, int, int);
int __MS_recvfrom(SOCKET, char *, int, int, struct sockaddr *, int*);
int __MS_select(int, fd_set *, fd_set *, fd_set *, const struct timeval *); 
int __MS_send(SOCKET, char *, int, int);
int __MS_sendto(SOCKET, const char *, int , int , const struct sockaddr *, int );
int __MS_setsockopt(SOCKET, int, int, char *, int);
int __MS_shutdown(SOCKET, int);
SOCKET __MS_socket(int, int, int);

/* MS ping replacement API */
typedef struct ip_option_information_s {
  unsigned char  Ttl;             /* Time To Live */
  unsigned char  Tos;             /* Type Of Service */
  unsigned char  Flags;           /* IP header flags */
  unsigned char  OptionsSize;     /* Size in bytes of options data */
  unsigned char *OptionsData;     /* Pointer to options data */
} ip_option_information_t;
typedef ip_option_information_t IP_OPTION_INFORMATION, *PIP_OPTION_INFORMATION;

typedef struct icmp_echo_reply_s {
  unsigned long  Address;         /* Replying address */
  unsigned long  Status;          /* Reply IP_STATUS */
  unsigned long  RoundTripTime;   /* RTT in milliseconds */
  unsigned short DataSize;        /* Reply data size in bytes */
  unsigned short Reserved;        /* Reserved for system use */
  void           *Data;           /* Pointer to the reply data */
  IP_OPTION_INFORMATION Options;  /* Reply options */
} icmp_echo_reply_t;
typedef icmp_echo_reply_t ICMP_ECHO_REPLY, *PICMP_ECHO_REPLY;

/* Stuff from iptypes.h */

#define MAX_ADAPTER_DESCRIPTION_LENGTH  (128)
#define MAX_ADAPTER_NAME_LENGTH         (256)
#define MAX_ADAPTER_ADDRESS_LENGTH      (8)
#define DEFAULT_MINIMUM_ENTITIES        (32)
#define MAX_HOSTNAME_LEN                (128)
#define MAX_DOMAIN_NAME_LEN             (128)
#define MAX_SCOPE_ID_LEN                (256)

/* IP_ADDRESS_STRING - for decimal "dot-quad" IP address strings */
typedef struct ip_address_string_s {
  char String[4 * 4];
} ip_address_string_t, IP_ADDRESS_STRING, *PIP_ADDRESS_STRING, IP_MASK_STRING, *PIP_MASK_STRING;

/* IP_ADDR_STRING - store an IP address with its associated netmask */
typedef struct _IP_ADDR_STRING {
  struct _IP_ADDR_STRING* Next;
  IP_ADDRESS_STRING IpAddress;
  IP_MASK_STRING IpMask;
  DWORD Context;
} IP_ADDR_STRING, *PIP_ADDR_STRING;

typedef struct ip_adapter_info_s {
  struct ip_adapter_info_s *Next;
  DWORD ComboIndex;
  char  AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];
  char  Description[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
  UINT  AddressLength;
  BYTE  Address[MAX_ADAPTER_ADDRESS_LENGTH];
  DWORD Index;
  UINT  Type;
  UINT  DhcpEnabled;
  PIP_ADDR_STRING CurrentIpAddress;
  IP_ADDR_STRING IpAddressList;
  IP_ADDR_STRING GatewayList;
  IP_ADDR_STRING DhcpServer;
  BOOL  HaveWins;
  IP_ADDR_STRING PrimaryWinsServer;
  IP_ADDR_STRING SecondaryWinsServer;
  unsigned long leaseObtained;
  unsigned long leaseExpired;
} ip_adapter_info_t, IP_ADAPTER_INFO, *PIP_ADAPTER_INFO;


DWORD  GetAdaptersInfo(PIP_ADAPTER_INFO, PULONG);
BOOL   IcmpCloseHandle(HANDLE);
HANDLE IcmpCreateFile(VOID);
DWORD  IcmpSendEcho(HANDLE, unsigned long, LPVOID, WORD, PIP_OPTION_INFORMATION, LPVOID, DWORD, DWORD);

int WSAGetLastError();
int WSAStartup(WORD, WSADATA *);

#ifdef __cplusplus
}
#endif

#endif /* _WINSOCKAPI_ */

#endif  /* _WCENETWORK_H_ */

