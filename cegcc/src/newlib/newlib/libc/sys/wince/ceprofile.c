#define	WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "sys/ceprofile.h"

#include <wtypes.h>

XCEPROFILEENTRY __proftab[XCE_MAXPROFILE];

void
XCEProfileDump()
{ 
  int i; 
  long long freq; 
  DWORD total; 
   
  QueryPerformanceFrequency((LARGE_INTEGER *) &freq); 
  printf("Freq: %d\n", (DWORD) freq);

  freq /= 1000LL;

  for(i = 0; i < XCE_MAXPROFILE; i++) 
    {
      if(__proftab[i].name) 
	{
	  total = __proftab[i].total / freq;

	  printf("%s: Count: %d Total: %d Average: %d\n", 
		 __proftab[i].name, __proftab[i].count, 
		 total, total / __proftab[i].count
		 ); 
	}
    } 
}
