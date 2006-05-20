#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>

//#include "local.h"

#include "sys/wcebase.h"
#include "sys/wcetime.h"

unsigned long _dstbias;


_VOID
_DEFUN (_tzset_hook_r, (reent_ptr),
struct _reent *reent_ptr)
{
	TIME_ZONE_INFORMATION tzinfo;
	int defused;

	// no race condition because this function is called wrapped in a lock. see _tzset_r
	static int alloced = 0;
	if (!alloced)
	{
		static char tzname0[64];
		static char tzname1[64];
		strcpy(tzname0, _tzname[0]);
		strcpy(tzname1, _tzname[1]);
		_tzname[0] = tzname0;
		_tzname[1] = tzname1;
		alloced = 1;
	}

	if ( GetTimeZoneInformation( &tzinfo ) == 0xFFFFFFFF )
		return;

	_timezone = tzinfo.Bias * 60L;

	if ( tzinfo.StandardDate.wMonth != 0 )
		_timezone += (tzinfo.StandardBias * 60L);

	if ( (tzinfo.DaylightDate.wMonth != 0) &&
		(tzinfo.DaylightBias != 0) )
	{
		_daylight = 1;
		_dstbias = (tzinfo.DaylightBias - tzinfo.StandardBias) * 60L;
	}
	else {
		_daylight = 0;
		_dstbias = 0;
	}

#define WC_COMPOSITECHECK         0x00000200
#define WC_SEPCHARS               0x00000020

	if ( (WideCharToMultiByte( CP_ACP,
		WC_COMPOSITECHECK | WC_SEPCHARS,
		tzinfo.StandardName, -1, _tzname[0], 63, NULL, &defused ) != 0) &&
		(!defused)
    ) {
		_tzname[0][63] = '\0';
	}
	else {
		_tzname[0][0] = '\0';
	}

	if ( (WideCharToMultiByte( CP_ACP,
		WC_COMPOSITECHECK | WC_SEPCHARS,
		tzinfo.DaylightName, -1, _tzname[1], 63, NULL, &defused ) != 0) &&
		(!defused) 
	) {
		_tzname[1][63] = '\0';
	}
	else {
		_tzname[1][0] = '\0';
	}
};
