#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define	FN	"/storage card/devel/fibo.txt"

int fibo(int n)
{
	if (n < 3)
		return 1;
	return fibo(n-1) + fibo(n-2);
}

int APIENTRY WinMain(HINSTANCE a,HINSTANCE b,LPWSTR c,int d)
{
	FILE	*f;
	int	i, r, len;
	wchar_t	*ws, buf[MAX_PATH];
	char	*s;

	f = fopen(FN, "w");
	len = GetModuleFileNameW(NULL, buf, MAX_PATH);
	s = malloc(len+1);
	wcstombs(s, buf, len+1);
	fprintf(f, "Command [%s] ", s);
	ws = GetCommandLineW();
	len = wcslen(ws);
	s = malloc(len+1);
	wcstombs(s, ws, len+1);
	fprintf(f, "param [%s]\r\n", s);

	for (i=1; i<20; i++) {
		r = fibo(i);
		fprintf(f, "fibo(%d) = %d\r\n", i, r);
	}
	fclose(f);
	exit(0);
}
