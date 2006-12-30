#include <windows.h>
#include <aygshell.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	wchar_t	s[MAX_PATH];

	(void)SHGetAutoRunPath(s);
	MessageBoxW(0, s, L"SHGetAutoRunPath", 0);
	exit(0);
}
