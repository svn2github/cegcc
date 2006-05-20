#include "sys/wcebase.h"
#include "sys/wceregistry.h"
#include "sys/ceutil.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

__IMPORT void XCEShowMessageW(const wchar_t *fmt, ...)
{
	va_list ap;
	wchar_t buf[512];

	va_start(ap, fmt);

#if 1
	OutputDebugStringW(L"ShowMessageW: ");
	NKvDbgPrintfW(fmt, ap);
	OutputDebugStringW(L"\n");
#endif

	_vsnwprintf(buf, COUNTOF(buf), fmt, ap);
	buf[COUNTOF(buf)-1] = 0;
	va_end(ap);

	MessageBoxW(NULL, buf, L"Message", MB_OK);
}

__IMPORT void XCEShowMessageA(const char *fmt, ...)
{
	va_list ap;
	char buf[512];
	wchar_t wbuf[512];

	va_start(ap, fmt);
	vsnprintf(buf, COUNTOF(buf), fmt, ap);
	buf[COUNTOF(buf)-1] = 0;
	va_end(ap);

	MultiByteToWideChar(CP_ACP, 0, buf, -1, wbuf, COUNTOF(wbuf));

#if 1
	OutputDebugStringW(L"ShowMessageA: ");
	OutputDebugStringW(wbuf);
	OutputDebugStringW(L"\n");
#endif

	MessageBoxW(NULL, wbuf, L"Message", MB_OK);
}

__IMPORT void XCETraceA(const char* fmt, ...)
{
	va_list ap;
	char buf[512];
	wchar_t wbuf[512];

	va_start(ap, fmt);
	vsnprintf(buf, COUNTOF(buf), fmt, ap);
	buf[COUNTOF(buf)-1] = 0;
	va_end(ap);

	MultiByteToWideChar(CP_ACP, 0, buf, -1, wbuf, COUNTOF(wbuf));
	OutputDebugStringW(wbuf);
}

__IMPORT void XCETrace(const char* fmt, ...)
{
	XCETraceA(fmt);
}

__IMPORT void XCETraceW(const wchar_t* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	NKvDbgPrintfW(fmt, ap);
}


char *
XCEToDosPath(char *s, int len)
{
	char *p;

	if (len < 0)
	{
		for(p = s; *p; p++)
			if(*p == '/')
				*p = '\\';
	}
	else
	{
		int i;
		for(i = 0; i < len ; i++)
			if(s[i] == '/')
				s[i] = '\\';
	}

	return s;
}

char *
XCEToUnixPath(char *s, int len)
{
	char *p;

	if (len < 0)
	{
		for(p = s; *p; p++)
			if(*p == '\\')
				*p = '/';
	}
	else
	{
		int i;
		for(i = 0; i < len ; i++)
			if(s[i] == '\\')
				s[i] = '/';
	}

	return s;
}

int
XCEGetEnvironmentVariableFromRegA(const char *name, char *buf, int len)
{
	int res;
	char data[1024];
	HKEY hKey;
	DWORD dwDataSize = sizeof(data);
	DWORD dwType;

	if(buf)
		buf[0] = 0;

	if((res = XCERegOpenKeyExA(HKEY_LOCAL_MACHINE, "Environment", 0,
		KEY_READ, (PHKEY)&hKey)) != 0)
	{
		return 0;
	}

	res = XCERegQueryValueExA(hKey, name, NULL, &dwType, 
		(LPBYTE)data, &dwDataSize);

	XCERegCloseKey(hKey);

	if(res != 0 || dwType != REG_SZ)
		return 0;

	if(buf == NULL)
		return strlen(data);

	strncpy(buf, data, len);

	return strlen(buf);
}

BOOL
XCESetEnvironmentVariableInRegA(const char *name, const char *value)
{
	int res;
	HKEY hKey;

	if((res = XCERegOpenKeyExA(HKEY_LOCAL_MACHINE, "Environment", 0,
		KEY_READ, &hKey)) != 0)
	{
		return FALSE;
	}

	res = XCERegSetValueExA(hKey, name, 0, REG_SZ, 
		(LPBYTE) value, strlen(value) + 1);

	XCERegCloseKey(hKey);

	return res == 0;
}

//////////////////////////////////////////////////////////////////////

int
XCEGetRegStringA(HKEY hKey, const char *keyname, const char *valname, 
				 char *buf, int len)
{
	int res;
	char data[1024];
	DWORD dwDataSize = sizeof(data);
	DWORD dwType;

	if(buf)
		buf[0] = 0;

	if((res = XCERegOpenKeyExA(hKey, keyname, 0,
		KEY_READ, &hKey)) != 0)
	{
		return 0;
	}

	res = XCERegQueryValueExA(hKey, valname, NULL, &dwType, 
		(LPBYTE)data, &dwDataSize);

	XCERegCloseKey(hKey);

	if(res != 0 || dwType != REG_SZ)
		return 0;

	if(buf == NULL)
		return strlen(data);

	strncpy(buf, data, len);

	return strlen(buf);
}

BOOL
XCESetRegStringA(HKEY hKey, const char *keyname, const char *valname, 
				 const char *value)
{
	int res;

	if((res = XCERegOpenKeyExA(hKey, keyname, 0,
		KEY_READ, &hKey)) != 0)
	{
		return FALSE;
	}

	res = XCERegSetValueExA(hKey, valname, 0, REG_SZ, 
		(LPBYTE) value, strlen(value) + 1);

	XCERegCloseKey(hKey);

	return res == 0;
}
