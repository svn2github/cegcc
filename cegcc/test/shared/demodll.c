#include <windows.h>

void doit(const TCHAR *text)
{
	MessageBoxW(0, L"Message from DLL", text, 0);
}
