#ifndef _WCEFILE_H_
#define _WCEFILE_H_

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
