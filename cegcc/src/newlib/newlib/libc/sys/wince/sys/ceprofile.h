#ifndef CEPROFILE_H
#define CEPROFILE_H 1

#define XCE_MAXPROFILE 20

typedef struct _xceprofentry 
{
  char *name;
  int count;
  long long start;
  long long stop;
  long long total;
} XCEPROFILEENTRY;

extern XCEPROFILEENTRY __proftab[XCE_MAXPROFILE];

#ifdef _PROFILE
# define XCE_PROFILE_START(X) \
	do { \
  __proftab[X].name = #X; \
  __proftab[X].count++; \
  QueryPerformanceCounter((LARGE_INTEGER *) &__proftab[X].start); \
	} while (0)

# define XCE_PROFILE_STOP(X) \
	do { \
  QueryPerformanceCounter((LARGE_INTEGER *) &__proftab[X].stop); \
  __proftab[X].total += (__proftab[X].stop - __proftab[X].start); \
	} while (0)

#else
# define XCE_PROFILE_START(X) do ; while (0)
# define XCE_PROFILE_STOP(X)  do ; while (0)
#endif

void XCEDumpProfiles();

#endif
