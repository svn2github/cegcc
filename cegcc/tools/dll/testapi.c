/*
 * Test whether a DLL offers one or more APIs mentioned on the command line.
 * This version of the program is meant to be used from the command line,
 * e.g. via a remote shell connection.
 *
 * Success or failure are reported via the standard output.
 *
 * Copyright (c) 2009 by Danny Backx.
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

void (WINAPI *fun)(void);

int main(int argc, char *argv[])
{
	int	i;
	HINSTANCE		dll;
	wchar_t			dllname[64], wapi[64];


	if (argc < 3) {
		printf("Usage : %s dll-name api-name [api-name ..]\n", argv[0]);
		exit(0);
	}

	mbstowcs(dllname, argv[1], 64);
	dll = LoadLibrary(dllname);
	if (! dll) {
		DWORD	e = GetLastError();
		printf("LoadLibrary(%s) : cannot load DLL -> error %d\n", argv[1], e);
		exit(1);
	}
	for (i=2; i<argc; i++) {
		mbstowcs(wapi, argv[i], 64);
		*(FARPROC *)&fun = GetProcAddress(dll, wapi);
		if (fun) {
			printf("\t%s implements %s (0x%08X)\n", argv[1], argv[i], fun);
		} else {
			DWORD	e = GetLastError();
			printf("%s doesn't know about %s\n", argv[1], argv[i]);
		}
	}

	exit(0);
}
