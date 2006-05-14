#ifndef _WCETYPES_H_
#define _WCETYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRUE
enum    bools {FALSE, TRUE};
#endif

typedef long           LONG;
typedef unsigned long  ULONG;
typedef ULONG         *PULONG;
typedef unsigned short USHORT;
typedef USHORT        *PUSHORT;
typedef unsigned char  UCHAR;
typedef UCHAR         *PUCHAR;
typedef char          *PSZ;
typedef void          *PVOID;

typedef void          *HANDLE;
typedef HANDLE*       PHANDLE;

#ifndef _WINDEF_

typedef void           VOID;

typedef void*         HINSTANCE;
typedef void*         HMODULE;
typedef void*         HKEY;
typedef HKEY*         PHKEY;

/* The HWND typedef */
typedef void  *HWND;
typedef void  *HDC;

typedef void        *RECT;
#endif

typedef const RECT* LPCRECT;

#define IN
#define FAR
#define OUT
#define OPTIONAL

#if 0

#ifndef __cdecl
#define __cdecl
#endif

#ifndef __export
#define __export
#endif

#ifndef __stdcall
#define __stdcall
#endif

#endif

#undef  far
#undef  near
#undef  pascal

#define far
#define near
#define CONST          const

typedef unsigned long  DWORD;
#ifndef XWINDOWS
typedef int            BOOL;
#else
#ifndef XMD_H
typedef int            BOOL;
#endif
#endif
typedef UCHAR          BYTE;
typedef BYTE*         PBYTE;
typedef USHORT         WORD;
typedef char           CHAR;
typedef CHAR          *PCHAR;
typedef float          FLOAT;
typedef FLOAT         *PFLOAT;
typedef BOOL          *PBOOL;
typedef BOOL          *LPBOOL;
typedef BYTE          *LPBYTE;
typedef int            INT;
typedef unsigned int   UINT;
typedef UINT          *PUINT;
typedef INT           *PINT;
typedef INT           *LPINT;
typedef USHORT        *PWORD;
typedef USHORT        *LPWORD;
typedef LONG          *LPLONG;
typedef LONG          *PLONG;
typedef ULONG         *PDWORD;
typedef ULONG         *LPDWORD;
typedef void          *LPVOID;
typedef const void    *LPCVOID;

typedef long long      QWORD;
typedef long long     *PQWORD;

#ifdef __cplusplus
typedef wchar_t         WCHAR;
#else
typedef USHORT         WCHAR;
#endif

typedef CHAR*         PSTR;
typedef WCHAR         TCHAR;
typedef WCHAR*        PWCHAR;
typedef WCHAR*        PWSTR;
typedef WCHAR*        LPWSTR;
typedef WCHAR*        LPTSTR;
typedef const WCHAR*  PCWSTR;
typedef const WCHAR*  LPCWSTR;
typedef const CHAR    *LPCSTR, *PCSTR;
typedef const CHAR   **LPPCSTR, *PPCSTR;
typedef PCHAR          LPSTR;

struct IStorage;
typedef struct IStorage* LPSTORAGE;

/* Function pointer types used in various interfaces */
typedef void (*LPFNDESTROYED) (void);

/* Types for passing & returning polymorphic values */
typedef UINT   WPARAM;
typedef LONG   HRESULT;
typedef LONG   SCODE;
typedef LONG  *PSCODE;
typedef LONG   LPARAM;
typedef LONG   LRESULT;
typedef USHORT LANGID;

/* Registry Types */
typedef DWORD  ACCESS_MASK;

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))

#ifdef __cplusplus
}
#endif
#endif /* _WCETYPES_H_ */
