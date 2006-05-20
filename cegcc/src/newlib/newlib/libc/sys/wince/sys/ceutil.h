#ifndef _WCEUTILS_H_
#define _WCEUTILS_H_

#include <_ansi.h>
#include <wchar.h>

__IMPORT void XCEShowMessageA(const char*, ...);
__IMPORT void XCEShowMessageW(const wchar_t*, ...);

__IMPORT void XCETrace(const char* fmt, ...);
__IMPORT void XCETraceA(const char* fmt, ...);
__IMPORT void XCETraceW(const wchar_t* fmt, ...);

__IMPORT char *XCEToUnixPath(char *s, int len);
__IMPORT char *XCEToDosPath(char *s, int len);

#endif /* _WCEUTILS_H_ */

