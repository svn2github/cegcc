#ifndef _WCEUTILS_H_
#define _WCEUTILS_H_

#include <_ansi.h>
#include <wchar.h>

void XCEShowMessageA(const char*, ...);
void XCEShowMessageW(const wchar_t*, ...);

void XCETrace(const char* fmt, ...);
void XCETraceA(const char* fmt, ...);
void XCETraceW(const wchar_t* fmt, ...);

char *XCEToUnixPath(char *s, int len);
char *XCEToDosPath(char *s, int len);

#endif /* _WCEUTILS_H_ */

