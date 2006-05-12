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

typedef void          *HINSTANCE;
typedef void          *HMODULE;

#define IN
#define FAR
#define OUT
#define OPTIONAL

#ifndef __cdecl
#define __cdecl
#endif

#ifndef __export
#define __export
#endif

#ifndef __stdcall
#define __stdcall
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
typedef USHORT         WORD;
typedef char           CHAR;
typedef CHAR          *PCHAR;
typedef float          FLOAT;
typedef FLOAT         *PFLOAT;
typedef BOOL          *PBOOL;
typedef BOOL          *LPBOOL;
typedef CHAR          *PBYTE;
typedef CHAR          *LPBYTE;
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
typedef void          *HKEY;
typedef HKEY          *PHKEY;
typedef void           VOID;
typedef void          *LPVOID;
typedef const void    *LPCVOID;

typedef long long      QWORD;
typedef long long     *PQWORD;

typedef USHORT         WCHAR;
typedef USHORT         TCHAR;
typedef USHORT        *PWCHAR;
typedef CHAR          *PSTR;
typedef USHORT        *PWSTR;
typedef USHORT        *LPWSTR;
typedef const USHORT  *PCWSTR;
typedef const USHORT  *LPCWSTR;
typedef const USHORT  *LPTSTR;
typedef const CHAR    *LPCSTR, *PCSTR;
typedef const CHAR   **LPPCSTR, *PPCSTR;
typedef PCHAR          LPSTORAGE;
typedef PCHAR          LPSTR;

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

/* The HWND typedef */
typedef void  *HWND;
typedef void  *HDC;

typedef void        *RECT;
typedef const void  *LPCRECT;

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))

#ifdef __cplusplus
}
#endif
#endif /* _WCETYPES_H_ */

