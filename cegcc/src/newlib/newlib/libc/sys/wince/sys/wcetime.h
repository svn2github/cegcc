#ifndef _WCETIME_H_
#define _WCETIME_H_

#include "sys/wcetypes.h"
#include <_ansi.h>

typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _SYSTEMTIME {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME;

#define TIME_ZONE_ID_UNKNOWN  0
#define TIME_ZONE_ID_STANDARD 1
#define TIME_ZONE_ID_DAYLIGHT 2

typedef struct _TIME_ZONE_INFORMATION {
  LONG Bias;
  WCHAR StandardName[32];
  SYSTEMTIME StandardDate;
  LONG StandardBias;
  WCHAR DaylightName[32];
  SYSTEMTIME DaylightDate;
  LONG DaylightBias;
} TIME_ZONE_INFORMATION, *LPTIME_ZONE_INFORMATION;

#ifdef __cplusplus
extern "C" {
#endif

void  GetLocalTime(LPSYSTEMTIME);
void  GetSystemTime(LPSYSTEMTIME);
DWORD GetTimeZoneInformation(LPTIME_ZONE_INFORMATION);

void  SetDaylightTime(DWORD);
BOOL  SetLocalTime(CONST SYSTEMTIME *);
BOOL  SetSystemTime(CONST SYSTEMTIME *);
BOOL  SetTimeZoneInformation(CONST TIME_ZONE_INFORMATION *);

#ifdef __cplusplus
}
#endif

#endif
