/*
 * This application can read a file, and test whether the indicated DLL
 * implements the APIs described in the file.
 *
 * The program has been created to work without command line : input is
 * read from a file (\temp\testapi.in.txt) and output is written to
 * another file (\temp\testapi.out.txt). Errors may be reported via
 * dialogs on the Windows CE console.
 *
 * File format : a single entry on each line.
 * First line mentions the name of the dll to be loaded (e.g. coredll).
 * All next lines mention the name of a function that the DLL will be
 * searched for (e.g. AppendMenuW).
 *
 * Copyright (c) 2009 by Danny Backx.
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

void (WINAPI *fun)(void);

char	*input = "\\temp\\testapi.in.txt",
	*output = "\\temp\\testapi.out.txt";

int main(int argc, char *argv[])
{
	int		r, count = 0;
	HINSTANCE	dll;
	wchar_t		dllname[64], wapi[64];
	char		field[64], dllnm[64];

	FILE	*infile = NULL,
		*outfile = NULL;

	infile = fopen(input, "r");
	if (infile == NULL) {
		MessageBoxW(0, L"Could not open input file", L"Error", 0);
		exit(1);
	}

	outfile = fopen(output, "w");
	if (outfile == NULL) {
		MessageBoxW(0, L"Could not open output file", L"Error", 0);
		exit(2);
	}

	fscanf(infile, "%s\n", &field);
	mbstowcs(dllname, field, 64);
	strcpy(dllnm, field);
	dll = LoadLibrary(dllname);
	if (! dll) {
		DWORD	e = GetLastError();
		fprintf(outfile, "LoadLibrary(%s) : cannot load DLL -> error %d\n", dllnm, e);
		fclose(outfile);
		exit(1);
	}
	fprintf(outfile, "Testapi2 started (%s)\n", field);

	r = fscanf(infile, "%s\n", &field);
	while (r != EOF) {
		if (count++ > 2000) {
			printf("Terminating\n");
			fprintf(outfile, "Terminating\n");
			fclose(outfile);
			exit(1);
		}
		mbstowcs(wapi, field, 64);
		*(FARPROC *)&fun = GetProcAddress(dll, wapi);
		if (fun) {
			fprintf(outfile, "\t%s implements %s (0x%08X)\n", dllnm, field, fun);
		} else {
			DWORD	e = GetLastError();
			fprintf(outfile, "%s doesn't know about %s\n", dllnm, field);
		}
		r = fscanf(infile, "%s\n", &field);
	}

	fclose(outfile);
	exit(0);
}
