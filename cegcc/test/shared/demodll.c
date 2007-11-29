#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef	__CEGCC__
#include <errno.h>
#endif
#include <fcntl.h>

static int init = 0;

static void show_errmsg_dll(const char *func)
{
	char msg[128];
	wchar_t *w_msg;
#ifdef	__CEGCC__
	snprintf(msg, sizeof(msg), "%s: errno = %i", func, errno);
#else
	int e = GetLastError();
	snprintf(msg, sizeof(msg), "%s: last error = %i", func, e);
#endif
	w_msg = (wchar_t *) malloc(sizeof(wchar_t) * strlen(msg));
	mbstowcs(w_msg, msg, strlen(msg));
	MessageBoxW(0, w_msg, NULL, 0);
	free(msg);
	free(w_msg);
}

void doit(const TCHAR *text)
{
	int	fp;
#if 1
	TCHAR	buf[32];
	swprintf(buf, L"Message from DLL (%d)", init);
	MessageBoxW(0, buf, text, 0);
#else
	MessageBoxW(0, L"Message from DLL", text, 0);
#endif

	/* Code from Erik */
	fp = open("/gdb/testfile2.txt", O_CREAT | O_TRUNC | O_WRONLY);
	if (fp < 0) {
		show_errmsg_dll("open");
	} else {
		if (write(fp, "test\n", 5) != 5) {
			show_errmsg_dll("write");
		}
		close(fp);
	}
}

BOOL DllMain(HANDLE h, DWORD d, LPVOID foo)
{
	init = 1;
	return TRUE;
}
