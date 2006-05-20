#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/times.h>

#include "sys/wcebase.h"
#include "sys/wcetime.h"
#include "sys/wcetrace.h"

#include "sys/timefcns.h"

typedef union {
  QWORD ft_scalar;
  FILETIME ft_struct;
} FT;

long _dstbias = -3600L;     /* DST offset in seconds */

// 
#define LL_(X) X ## LL
#define LL(X) LL_(X)

/* note that NT Posix's TZNAME_MAX is only 10 */
static char tzstd[64] = { "PST" };
static char tzdst[64] = { "PDT" };

int _lpdays[] = {
  -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

int _days[] = {
  -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
};

/*  Day names must be three character abbreviations strung together */
const char __dnames[] = {
  "SunMonTueWedThuFriSat"
};

/*  Month names must be three character abbreviations strung together */
const char __mnames[] = {
  "JanFebMarAprMayJunJulAugSepOctNovDec"
};

static time_t elapsed_minutes_cache = 0;
static QWORD millis_at_system_start = 0;
static int   dstflag_cache = 0;
static SYSTEMTIME gmt_cache;

/* Three possible values of dstflag_cache and dstflag */
#define DAYLIGHT_TIME   1
#define STANDARD_TIME   0
#define UNKNOWN_TIME    -1

#define EPOCH_BIAS  LL(116444736000000000)

int
ftime(struct timeb *tp)
{
  FT nt_time;
  time_t t;
  SYSTEMTIME st;
  TIME_ZONE_INFORMATION tzinfo;
  DWORD tzstate;
  QWORD current_millis;

  if (millis_at_system_start == 0) {
    tzset();

    GetSystemTime(&st);
//    SystemTimeToFileTime(&st, &(nt_time.ft_struct));
	XCEGetSystemTimeAsFileTime(&st, &(nt_time.ft_struct));

	if((t = (time_t)(nt_time.ft_scalar / LL(600000000))) != elapsed_minutes_cache) {
      if ((tzstate = GetTimeZoneInformation(&tzinfo)) != 0xFFFFFFFF) {
        if ((tzstate == TIME_ZONE_ID_DAYLIGHT) &&
            (tzinfo.DaylightDate.wMonth != 0) &&
            (tzinfo.DaylightBias != 0)) {
          dstflag_cache = DAYLIGHT_TIME;
        } else {
          dstflag_cache = STANDARD_TIME;
        }
      } else {
        dstflag_cache = UNKNOWN_TIME;
      }
      elapsed_minutes_cache = t;
    }

    /* current millisecs */
	millis_at_system_start = ((nt_time.ft_scalar - EPOCH_BIAS) / LL(10000));
    /* subtract ticks */
    millis_at_system_start -= GetTickCount();
  }

  current_millis = millis_at_system_start + GetTickCount();

  tp->time = (time_t)(current_millis / LL(1000));
  tp->millitm = current_millis % LL(1000);
  tp->timezone = (short)(_timezone / 60);
  tp->dstflag = (short)dstflag_cache;

  return(0);
}

time_t 
time(time_t *timeptr)
{
  TIME_ZONE_INFORMATION tzinfo;
  SYSTEMTIME lt, gmt;
  DWORD tzstate;
  int dstflag;
  time_t tim;

  GetLocalTime(&lt);

  /*
   * Determine whether or not the local time is a Daylight Saving
   * Time. On Windows NT, the GetTimeZoneInformation API is *VERY*
   * expensive. The scheme below is intended to avoid this API call in
   * many important cases by caching the GMT value and dstflag.In a
   * subsequent call to time(), the cached value of dstflag is used
   * unless the new GMT differs from the cached value at least in the
   * minutes place.
   */
  GetSystemTime(&gmt);

  if ((gmt.wMinute == gmt_cache.wMinute) && (gmt.wHour == gmt_cache.wHour) &&
      (gmt.wDay == gmt_cache.wDay) && (gmt.wMonth == gmt_cache.wMonth) &&
      (gmt.wYear == gmt_cache.wYear)) {
    dstflag = dstflag_cache;
  } else {
    if ((tzstate = GetTimeZoneInformation(&tzinfo)) != 0xFFFFFFFF) {
      /*
       * Must be very careful in determining whether or not DST is
       * really in effect.
       */
      if ((tzstate == TIME_ZONE_ID_DAYLIGHT) && (tzinfo.DaylightDate.wMonth != 0) &&
          (tzinfo.DaylightBias != 0)) {
        dstflag = DAYLIGHT_TIME;
      } else {
        /* When in doubt, assume standard time */
        dstflag = STANDARD_TIME;
      }
    } else {
      dstflag = UNKNOWN_TIME;
    }

    dstflag_cache = dstflag;
    gmt_cache = gmt;
  }

  tim = _systotime_t(lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute,
                     lt.wSecond, dstflag);
  if (timeptr)
    *timeptr = tim;

  return tim;
 }


int
_gettimeofday_r(struct _reent *reent, struct timeval *tv, struct timezone *tzp)
{
  struct timeb tb;

  ftime(&tb);

  tv->tv_sec = tb.time;
  tv->tv_usec = tb.millitm * 1000;

  /* why not? */
  if (tzp) {
    tzp->tz_minuteswest = tb.timezone;
    tzp->tz_dsttime = tb.dstflag;
  }

  return(0);
}

clock_t 
_times_r(struct _reent *reent, struct tms *buf)
{
  FILETIME tcreat, texit, tkernel, tuser;
  clock_t timeval = 0;

  memset(buf, 0, sizeof(struct tms));
  extern HANDLE __main_thread;
  if (!__main_thread)
  {
	  WCETRACE(WCE_TIME, "_times_r: no main thread yet");
	  return -1;
  }
//  if (!GetThreadTimes(GetCurrentThread(), &tcreat, &texit, &tkernel, &tuser))
  if (!GetThreadTimes(__main_thread, &tcreat, &texit, &tkernel, &tuser))
  {
	  WCETRACE(WCE_TIME, "GetThreadTimes failed.");
	  return -1;
  }

  buf->tms_utime = (QWORD) *(QWORD *) &tuser / (QWORD) 10000;
  buf->tms_stime = (QWORD) *(QWORD *) &tkernel / (QWORD) 10000;
  buf->tms_cutime = -1;

  timeval = GetTickCount();
  WCETRACE(WCE_TIME, "utime: %d stime: %d\n", buf->tms_utime, buf->tms_stime);
  return timeval;
};

/*!! CAV someone needs to implement this but not me, not today.. */
int
_isindst(struct tm *tb)
{
  return(FALSE);
}

time_t 
_systotime_t(int yr, int mo, int dy, int hr, int mn, int sc, int dstflag)
{
  int tmpdays;
  long tmptim;
  struct tm tb;

  if ( ((long)(yr -= 1900) < _BASE_YEAR) || ((long)yr > _MAX_YEAR) )
    return (time_t)(-1);

  tmpdays = dy + _days[mo - 1];

  if ( !(yr & 3) && (mo > 2) ) {
    tmpdays++;
  }

  tmptim = (((long)yr - _BASE_YEAR) * 365L +
            (long)((yr - 1) >> 2) - _LEAP_YEAR_ADJUST + tmpdays) * 24L + hr;

  tmptim = (tmptim * 60L + mn) * 60L + sc;

  /* QUESTION: Do we care? */
  tzset();

  tmptim += _timezone;

  tb.tm_yday = tmpdays;
  tb.tm_year = yr;
  tb.tm_mon  = mo - 1;
  tb.tm_hour = hr;

  if ((dstflag == 1) || ((dstflag == -1) && _daylight && _isindst(&tb))) {
    tmptim += _dstbias;
  }

  return(tmptim);
}
