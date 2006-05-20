// ceirda.c
//
// Time-stamp: <03/01/02 22:16:02 keuchel@netwave.de>

//#include <af_irda.h>
#include "sys/ceirda.h"
#include "sys/wcenetwork.h"

#include <malloc.h>


//#include "alias.h"

int
irda_getaddr(const char *hostname, const char *servicename,
		SOCKADDR_IRDA *addr)
{
  unsigned int len;
  int retry = 0;
  SOCKET s;
  int i;
  DEVICELIST *pirda_devlist = NULL;
  int status;

  len = 1024;
  pirda_devlist = malloc(len);
  pirda_devlist->numDevice = 0;

  if((s = __MS_socket(AF_IRDA, SOCK_STREAM, 0)) == SOCKET_ERROR)
    return -1;

  while(retry++ < 5)
    {
      len = 1024;

      if(__MS_getsockopt(s, SOL_IRLMP, IRLMP_ENUMDEVICES,
		    (char *) pirda_devlist, &len) == SOCKET_ERROR)
	{
	  goto bad;
	}

      if(pirda_devlist->numDevice != 0)
	{
	  break;
	}

      Sleep(200);
    }

  if(pirda_devlist->numDevice == 0)
    {
      goto bad;
    }

  for(i = 0; i < pirda_devlist->numDevice; i++)
    {
      if(strcasecmp(pirda_devlist->Device[i].irdaDeviceName, hostname) == 0)
	{
	  break;
	}
    }

  if(i == pirda_devlist->numDevice)
    {
      goto bad;
    }

  addr->irdaAddressFamily = AF_IRDA;
  memcpy(addr->irdaDeviceID, 
	 pirda_devlist->Device[i].irdaDeviceID, 4);
  strcpy(addr->irdaServiceName, servicename);

  status = 0;
  goto done;

 bad:
  status = -1;

 done:
  free(pirda_devlist);
  __MS_closesocket(s);

  return status;
}
