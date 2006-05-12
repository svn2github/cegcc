#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <setjmp.h>

#include <sys/wcetypes.h>
#include <sys/wcefile.h>
#include <sys/wceerror.h>
#include <sys/wcetrace.h>

unsigned int
sleep(unsigned int secs)
{
  Sleep(1000 * secs);
  return(0);
}
