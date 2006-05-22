#define WIN32_LEAN_AND_MEAN
#include <windows.h>

unsigned int
sleep(unsigned int secs)
{
  Sleep(1000 * secs);
  return(0);
}
