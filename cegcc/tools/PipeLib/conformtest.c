/* The plan is to evolve this file into some sort of conformance test.
   The test should be performed on 9X/NT, and the results matched to
   PipeLib.  Currently the #if 0 parts were tested manually.  They should
   be moved into threads and WaitForSingleObject calls with timeouts, or
   something.  */

#include <windows.h>
#include <stdio.h>

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
strwinerror (DWORD error)
{
  static char buf[1024];
  wchar_t *msgbuf;
  DWORD lasterr = GetLastError ();
  DWORD chars = FormatMessageW (FORMAT_MESSAGE_FROM_SYSTEM
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

      sprintf (buf, "(%ld) %ls", error, msgbuf);
      LocalFree (msgbuf);
    }
  else
    sprintf (buf, "unknown win32 error (%ld)", error);

  SetLastError (lasterr);
  return buf;
}

static DWORD WINAPI
close_handle (HANDLE arg)
{

  Sleep (3000);
  printf ("closing handle\n");
  CloseHandle (arg);

  return 0;
}

int main ()
{
  DWORD written;
  DWORD read;
  const char str[] = "hello1\n";
  char buf[512];

  HANDLE readh, writeh;
  if (!CreatePipe (&readh, &writeh, NULL, 0))
    {
      printf ("CreatePipe failed : %s\n", strwinerror (GetLastError ()));
      return 1;
    }

#if 0
  CloseHandle (writeh);

  if (!ReadFile (readh, buf, 1, &read, FALSE))
    {
      /* ERROR_BROKEN_PIPE */
      printf ("ReadFile: broken pipe? : %s\n", strwinerror (GetLastError ()));
      return 3;
    }
#endif

#if 0
  CloseHandle (readh);

  if (!WriteFile (writeh, str, sizeof (str) - 1, &written, NULL))
    {
      /* ERROR_NO_DATA */
      printf ("broken pipe? %s\n", strwinerror (GetLastError ()));
      return 2;
    }
#endif

#if 0
  HANDLE thread = CreateThread (NULL, 0, close_handle, writeh, 0, NULL);
  CloseHandle (thread);

  if (!ReadFile (readh, buf, 1, &read, FALSE))
    {
      /* ERROR_BROKEN_PIPE */
      printf ("broken pipe : %s\n", strwinerror (GetLastError ()));
      return 3;
    }
#endif

#if 1
  HANDLE thread = CreateThread (NULL, 0, close_handle, readh, 0, NULL);
  CloseHandle (thread);

  DWORD count = 0;
  while (1)
    {
      if (!WriteFile (writeh, str, sizeof (str) - 1, &written, NULL))
	{
	  /* ERROR_NO_DATA */
	  printf ("broken pipe? %s\n", strwinerror (GetLastError ()));
	  return 2;
	}
      count += written;
      /* Checking if WriteFile returns if *something* was written, or
       if it always wait for flushing the buffer.  */

      /* I see that Windows return ERROR_NO_DATA, and reports that
	 sizeof (str) - 1 was written.  I don't think we can do
	 that with WinCE.  */
      if (1 || count % 100 == 0)
	{
	  printf ("count = %d\n", count);
	  fflush (stdout);
	}
    }
#endif

  if (!ReadFile (readh, buf, sizeof (buf), &read, FALSE))
    {
      printf ("broken pipe? : %s\n", strwinerror (GetLastError ()));
      return 4;
    }

  fwrite (buf, 1, read, stdout);
  fflush (stdout);

  if (!ReadFile (readh, buf, sizeof (buf), &read, FALSE))
    {
      printf ("broken pipe? : %s\n", strwinerror (GetLastError ()));
      return 4;
    }

  fwrite (buf, 1, read, stdout);
  fflush (stdout);

  return 0;
}
