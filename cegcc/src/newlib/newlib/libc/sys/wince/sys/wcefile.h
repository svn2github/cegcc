#ifndef _WCEFILE_H_
#define _WCEFILE_H_

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#ifndef FILE_TYPE_UNKNOWN
/* WinCE SDK doesn't define these, 
   but let's match the desktop windows numbers,
   in case we are building with w32api headers.  */
#define FILE_TYPE_UNKNOWN          0
#define FILE_TYPE_DISK	           1
#define FILE_TYPE_CHAR	           2
#define FILE_TYPE_PIPE             3
#endif

#ifndef STD_INPUT_HANDLE
/* WinCE SDK doesn't define these, 
   but let's match the desktop windows numbers,
   in case we are building with w32api headers.  */
#define STD_INPUT_HANDLE (DWORD)(0xfffffff6)
#define STD_OUTPUT_HANDLE (DWORD)(0xfffffff5)
#define STD_ERROR_HANDLE (DWORD)(0xfffffff4)
#endif

#ifdef __cplusplus
extern "C" {
#endif

BOOL   XCECloseHandle(HANDLE);
LONG   XCECompareFileTime(const FILETIME *, const FILETIME *);
BOOL   XCECreateDirectoryW(LPCWSTR, LPSECURITY_ATTRIBUTES);
HANDLE XCECreateFileW(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
BOOL   XCEDeleteFileW(LPCWSTR);
BOOL   XCEMoveFileW(LPCWSTR, LPCWSTR);
BOOL   XCEReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
BOOL   XCERemoveDirectoryW(LPCWSTR);
BOOL   XCEWriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);

typedef int (* CONSOLE_READ_FUNC)(int, unsigned char *, int);
typedef int (* CONSOLE_WRITE_FUNC)(int, const unsigned char *, int);
typedef int (* CONSOLE_IOCTL_FUNC)(int, int, void *);

#ifdef __cplusplus
}
#endif
#endif  /* _WCEFILE_H_ */
