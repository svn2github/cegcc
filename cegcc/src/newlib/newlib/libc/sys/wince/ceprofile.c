#include "sys/ceprofile.h"
#include "sys/wcebase.h"

XCEPROFILEENTRY __proftab[XCE_MAXPROFILE];

typedef union _LARGE_INTEGER {
	struct {
		DWORD LowPart;
		LONG HighPart;
	};
	struct {
		DWORD LowPart;
		LONG HighPart;
	} u;
	long long QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

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
