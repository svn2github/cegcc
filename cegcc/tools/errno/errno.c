/* Pedro Alves */

#include "errno.h"
#include <windows.h>

/* There are more WinErr codes than errno codes.
   To prevent having to do stuff like:

-   if (errno == ENOENT)
+   if (errno == ERROR_PATH_NOT_FOUND
+       || errno == ERROR_FILE_NOT_FOUND)

   ... we colapse the multiple WinErr mappings into one.  */

int
__liberrno_errno (void)
{
  DWORD winerr = GetLastError ();
  switch (winerr)
    {
#include "errno_tab.c"
    default:
      return (int) winerr;
    }
}

#ifndef COUNTOF
#define COUNTOF(STR) (sizeof (STR) / sizeof ((STR)[0]))
#endif

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

static char *
strwinerror (char* buf, DWORD error)
{
  wchar_t *msgbuf;
  DWORD lasterr = GetLastError ();
  DWORD chars = FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM
			       | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			       NULL,
			       error,
			       0, /* Default language */
			       (LPVOID)&msgbuf,
			       0,
			       NULL);
  if (chars != 0)
    {
      /* If there is an \r\n appended, zap it.  */
      if (chars >= 2
	  && msgbuf[chars - 2] == '\r'
	  && msgbuf[chars - 1] == '\n')
	{
	  chars -= 2;
	  msgbuf[chars] = 0;
	}

      if (chars > ((COUNTOF (buf)) - 1))
	{
	  chars = COUNTOF (buf) - 1;
	  msgbuf [chars] = 0;
	}

      wcstombs (buf, msgbuf, chars + 1);
      LocalFree (msgbuf);
    }
  else
    sprintf (buf, "unknown win32 error (%ld)", error);

  SetLastError (lasterr);
  return buf;
}

__inline__ static const char*
get_errstr (DWORD winerr)
{
  switch (winerr)
    {
#include "errno_str.c"
    }
  return NULL;
}

char *
strerror (int error)
{
  static char buf[1024];
  DWORD winerr = (DWORD) error;
  const char *str = get_errstr (winerr);

  if (str != NULL)
    strcpy (buf, str);
  else
    strwinerror (buf, winerr);
  return buf;
}

void
perror (const char *s)
{
  if (s && *s)
    fprintf (stderr, "%s: %s\n", s, strerror (errno));
  else
    fprintf (stderr, "%s\n", strerror (errno));
}
