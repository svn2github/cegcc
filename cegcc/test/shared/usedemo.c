#include <windows.h>

extern void doit(const TCHAR *text);

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	doit(L"Yow baby");
}
