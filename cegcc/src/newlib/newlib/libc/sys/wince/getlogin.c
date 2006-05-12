#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>

#include <sys/wcebase.h>
#include <sys/wcetrace.h>
#include <sys/wcefile.h>

static char loginbuf[126];

char *
getlogin()
{
  char *p;
  DWORD len = sizeof(loginbuf);

  if ((p = getenv("USERNAME")) != NULL)
    strcpy(loginbuf, p);
  else
    strcpy(loginbuf, "nobody");

  return loginbuf;
}
