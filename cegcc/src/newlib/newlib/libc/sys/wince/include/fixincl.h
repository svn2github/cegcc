#ifndef __CE_FIXINCL_H
#define __CE_FIXINCL_H

//for compatibility with MS headers

#define WINCEMACRO
#define ARM 1
#define _ARM_ 1
#define ARMV4 1
#define ARM_WINCE 1
// in some cases #defining _WIN32 may help, but when compiling GCC it causes problems
//#define _WIN32
//#define WIN32

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

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#define _MT

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

//size_t and wchar_t are defined in many different places in all SDKs.
//let's put them here (just to be sure)

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
//typedef long unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#include <wchar.h>

#if 0
#include <wchar.h>
#ifndef _WCHAR_T_DEFINED
# define _WCHAR_T_DEFINED
#endif

typedef wchar_t WCHAR;
typedef WCHAR TCHAR;

typedef void* HANDLE;
#define DECLARE_HANDLE(name) typedef HANDLE name

#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif

#include <stddef.h>
#include <basetsd.h>
#include <winnt.h>
#include <windef.h>
#include <basetsd.h>
#include <winbase.h>
#include <wtypes.h>

#if 0
//ptrdiff_t is defined in stddef.h from the sdk
#ifndef _PTRDIFF_T_DEFINED
typedef int ptrdiff_t;
#define _PTRDIFF_T_DEFINED
#endif

#ifndef _WCTYPE_T_DEFINED
//typedef wchar_t wint_t;
//typedef wchar_t wctype_t;
#define _WCTYPE_T_DEFINED
#endif
#endif

#endif

#endif
