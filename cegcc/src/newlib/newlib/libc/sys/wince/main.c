#include "sys/wcebase.h"

int WinMain(HANDLE hInstance, HANDLE hPrevInstance, WCHAR* lpCmdLine, int nShowCmd);

__declspec(dllimport) WCHAR* GetCommandLineW();
__declspec(dllimport) HANDLE GetModuleHandleW (LPWSTR);

int main (int argc, char** argv)
{
	int __ShowCmd = 5; //SW_SHOW
	// Default entry is main, when it is not present -> main.c calls WinMain() function
	return WinMain(GetModuleHandleW(0), 0, GetCommandLineW(), __ShowCmd);
}
