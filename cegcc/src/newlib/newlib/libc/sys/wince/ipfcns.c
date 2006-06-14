#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iptypes.h>
#include <ipifcons.h>

#include <stdio.h>
#include <errno.h>

#include "sys/wcetrace.h"

#include <malloc.h>

#define ADAPTER_BUFSIZ  (4096)
#define MAXADAPTERS 20
#define ADAPTERNUMBER 0

int AdapterTypeString(unsigned int type, char *buf, int buflen);

int
AdapterInfoByName(char *name, IP_ADAPTER_INFO *outbuf)
{
  IP_ADAPTER_INFO *aiPtr = NULL;
  unsigned char *aiBuf, *saveBuf;
  unsigned char *cp;
  unsigned long  aiLen, bufLen;
  int    i, status, retval;

  if (name == NULL || outbuf == NULL || strlen(name) == 0) {
    errno = EINVAL;
    return(-1);
  }
    
  aiLen = ADAPTER_BUFSIZ;
  saveBuf = aiBuf = calloc(1, aiLen);
  if (aiBuf == NULL) {
    errno = ENOMEM;
    return(-1);
  }

  aiPtr = (IP_ADAPTER_INFO *)aiBuf;

  status = GetAdaptersInfo(aiPtr, &aiLen);
  if (status) {
    WCETRACE(WCE_NETWORK, "AdapterInfoByName: ERROR - GetAdaptersInfo failed status %d\n", status);
    errno = ENOMEM;
    retval = -1;
    goto cleanup;
  }

  /* Try to match the specified adapter name */
  do {
    errno = ENOENT;
    retval = -1;  /* initial pessimism */
    WCETRACE(WCE_NETWORK, "AdapterInfoByName: current adapter \"%s\" Type %d", aiPtr->AdapterName, aiPtr->Type);
    if (strncasecmp(name, aiPtr->AdapterName, strlen(name)) == 0) {
      memcpy(outbuf, aiPtr, sizeof(IP_ADAPTER_INFO));
      retval = 0;
      WCETRACE(WCE_NETWORK, "AdapterInfoByName: success for \"%s\"", name);
      break;
    }
  } while ((aiPtr = aiPtr->Next) != NULL);

 cleanup:
  free(saveBuf);
  return(retval);
}

int
AdapterTypeString(unsigned int type, char *buf, int buflen)
{
  if (buf == NULL || buflen < 12) {
    errno = EINVAL;
    return(-1);
  }

  switch (type) {
  case MIB_IF_TYPE_ETHERNET:
    strcpy(buf, "ETHERNET");
    break;
  case MIB_IF_TYPE_TOKENRING:
    strcpy(buf, "TOKENRING");
    break;
  case MIB_IF_TYPE_PPP:
    strcpy(buf, "PPP");
    break;
  case MIB_IF_TYPE_LOOPBACK:
    strcpy(buf, "LOOPBACK");
    break;
  case MIB_IF_TYPE_SLIP:
    strcpy(buf, "SLIP");
    break;
  case MIB_IF_TYPE_OTHER:
  default:
    strcpy(buf, "OTHER");
  }

  return(0);
}

#define BUFLEN  (64)

void
PrintAdapterInfo(IP_ADAPTER_INFO *ai)
{
  char buf1[BUFLEN];
  char buf2[BUFLEN];
  unsigned char *cp;

  if (ai == NULL)
    return;

  AdapterTypeString(ai->Type, buf1, BUFLEN);
  cp = ai->Address;
  sprintf(buf2, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x", cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]);
  printf("%-10.10sLink encap:%s  HWaddr %s\n", ai->AdapterName, buf1, buf2);
  if (ai->CurrentIpAddress != NULL) {
    memset(buf1, 0, BUFLEN);
    memset(buf2, 0, BUFLEN);
    memcpy(buf1, ai->CurrentIpAddress->IpAddress.String, 16);
    memcpy(buf2, ai->CurrentIpAddress->IpMask.String, 16);
    printf("          inet addr:%s  Mask:%s\n", buf1, buf2);
  } else {
    printf("          inet addr:(null)  Mask:(null)\n", buf1, buf2);
  }
}

int
AdapterInfoIPIsValid(IP_ADAPTER_INFO *ai)
{
  char buf1[BUFLEN];
  unsigned char *cp;
  int rval = 0;

  if (ai == NULL)
    return rval;

  if (ai->CurrentIpAddress != NULL) {
    memset(buf1, 0, BUFLEN);
    memcpy(buf1, ai->CurrentIpAddress->IpAddress.String, 16);
    rval = (strncmp(buf1, "0.0", 3) == 0) ? 0 : 1;

    printf("inet addr:%s %s\n", buf1, (rval == 1) ? "valid" : "invalid");
  } else {
    printf("AdapterInfoIPIsValid: ERROR NULL ai pointer encountered!\n");
  }

  return(rval);
}

int
PrintAllAdaptersInfo()
{
  IP_ADAPTER_INFO *aiPtr = NULL;
  unsigned char *aiBuf, *saveBuf;
  unsigned char *cp;
  unsigned long  aiLen, bufLen;
  int    i, status, retval;
    
  aiLen = ADAPTER_BUFSIZ;
  saveBuf = aiBuf = calloc(1, aiLen);
  if (aiBuf == NULL) {
    errno = ENOMEM;
    return(-1);
  }

  aiPtr = (IP_ADAPTER_INFO *)aiBuf;

  status = GetAdaptersInfo(aiPtr, &aiLen);
  if (status) {
    WCETRACE(WCE_NETWORK, "PrintAllAdaptersInfo: ERROR - GetAdaptersInfo failed status %d\n", status);
    errno = ENOMEM;
    retval =(-1);
    goto cleanup;
  }

  /* Print out info for all adapters */
  do {
    errno = ENOENT;
    retval = -1;  /* initial pessimism */
    WCETRACE(WCE_NETWORK, "PrintAllAdaptersInfo: current adapter \"%s\" Type %d", aiPtr->AdapterName, aiPtr->Type);
    PrintAdapterInfo(aiPtr);
  } while ((aiPtr = aiPtr->Next) != NULL);

 cleanup:
  free(saveBuf);
  return(0);
}

