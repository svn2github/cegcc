#ifndef __CE_FIXINCL_H
#define __CE_FIXINCL_H

//for compatibility with MS headers

#define ARM 1
#define _ARM_ 1
#define ARMV4 1
#define ARM_WINCE 1

#ifndef UNDER_CE
#define UNDER_CE 420
#endif
#define _WIN32_WCE 420

#define NDEBUG
#define __int64 long long

/* what does this map to? __attribute__((packed)) is not the same, right? */
#define __unaligned

#undef __inline
#undef _inline
#undef inline
#define __inline static inline
#define _inline __inline

#ifndef __STDC_HOSTED__
#define __STDC_HOSTED__ 1
#define __ARM_ARCH_4__
#define __ARMEL__
#define __pe__
#define __arm__
#define __STDC__ 1
#endif

#ifndef __APCS_32__
#define __APCS_32__
#endif
#define __export __declspec(dllexport)
#define __import __declspec(dllimport)

#define _FORCENAMELESSUNION

#ifndef __cplusplus
typedef short unsigned int wchar_t;
#endif

typedef unsigned short wint_t;
//#undef ULONG

#endif
