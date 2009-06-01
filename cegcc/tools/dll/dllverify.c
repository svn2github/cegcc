/*
 * Copyright (c) 2009 by Danny Backx
 *
 * This file is part of cegcc.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING3.  If not see
 * <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <windows.h>
#include <winbase.h>

#define	LINELEN	512
#define	SYMLEN	64

BOOL (WINAPI *fun)(SYSTEM_POWER_STATUS_EX *, BOOL);
BOOL			r;
SYSTEM_POWER_STATUS_EX	st;
HINSTANCE		dll = 0;

int main(int argc, char *argv[])
{
	char			bf[LINELEN], lib[LINELEN], name[SYMLEN];
	wchar_t			*libname, wname[SYMLEN];
	int			len, rr, lineno, id;
	int			nfail = 0, nsuccess = 0, nexports = 0, nempty = 0;
	FILE			*f;

	if (argc != 2) {
		printf("Usage : %s libname.def\n", argv[0]);
		exit(1);
	}

	f = fopen(argv[1], "r");
	if (!f) {
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		exit(1);
	}

	lineno = 0;
	while (! feof(f)) {
		if (fgets(bf, LINELEN, f) == 0)
			break;
		lineno++;

		if (bf[0] == '\0' || bf[0] == ';') {
			nempty++;
		} else if (strncasecmp(bf, "library", 7) == 0) {
			rr = sscanf(bf+8, "%[^ \t\r\n]", lib);

			len = strlen(lib);
			libname = calloc(len+1, sizeof(wchar_t));
			mbstowcs(libname, lib, len);
			libname[len] = 0;

			printf("Loading DLL [%s]\n", lib);

			dll = LoadLibrary(libname);
			if (! dll) {
				DWORD	e = GetLastError();
				printf("\tLoadLibrary -> error %d\n", e);
				exit(1);
			}
		} else if (strncasecmp(bf, "exports", 7) == 0) {
			nexports++;	/* Do nothing */
		} else if (dll) {
			rr = sscanf(bf, "%[^ \t\r\n]", &name);

			if (rr == 1) {
				mbstowcs(wname, name, strlen(name)+1);

				*(FARPROC *)&fun = GetProcAddress(dll, wname);
				if (fun == NULL) {
					printf("##### %s not found\n", name);
					nfail++;
				} else {
//					printf("\tbf %s\t%s ok\n", bf, name);
					nsuccess++;
				}
			}
		}
	}

	fclose(f);

	printf("Results\n\t%d lines read\n\t%d failures\n"
		"\t%d success\n\t%d export lines\n\t%d empty lines\n",
			lineno,
			nfail,
			nsuccess, nexports, nempty);
	exit(0);
}

/*
LIBRARY AYGSHELL
EXPORTS
SHImListPopup
SHInitDialog		@56	; described on MSDN, published by name and number
SHInitExtraControls
Shell_RegAllocString
StrStrI
SubClassThisWindow
;
; These are NONAME but confirmed somehow
;
SHHandleWMSettingChange		@83     NONAME	; described as NONAME on MSDN
SHHandleWMActivate		@84     NONAME	; described as NONAME on MSDN
SHSendBackToFocusWindow		@97	NONAME  ; http://groups.google.hu/group/microsoft.public.windowsce.embedded/msg/e407143ac07929b9
;
*/
