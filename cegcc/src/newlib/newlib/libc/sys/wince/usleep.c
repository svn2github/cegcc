#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void
usleep(unsigned long usec)
{
  Sleep(usec/1000);
}
