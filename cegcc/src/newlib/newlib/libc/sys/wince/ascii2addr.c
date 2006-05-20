// ascii2addr.c
//
// Time-stamp: <12/02/01 16:56:51 keuchel@keuchelnt>

#include "sys/wcebase.h"
#include "sys/wcenetwork.h"
#include "sys/wceerror.h"

#include <errno.h>

#define WSASetLastError(X) SetLastError(X)

__IMPORT int ascii2addr(int af, const char *ascii, void *result)
{
	struct in_addr *ina;
	char strbuf[4*sizeof("123")]; /* long enough for V4 only */

	switch(af) 
	{
	case AF_INET:
		ina = (struct in_addr *) result;
		strbuf[0] = '\0';
    strbuf[(sizeof strbuf)-1] = '\0';
		strncat(strbuf, ascii, (sizeof strbuf)-1);
		if (inet_aton(strbuf, ina))
			return sizeof(struct in_addr);
		WSASetLastError(WSAEINVAL);
		errno = EINVAL;
		break;
	default:
		WSASetLastError(WSAEPROTONOSUPPORT);
		errno = EPROTONOSUPPORT;
		break;
	}

	return -1;
}
