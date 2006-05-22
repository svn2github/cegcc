#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int main (int argc, char** argv)
{
	int __ShowCmd = 5; //SW_SHOW
	// Default entry is main, when it is not present -> main.c calls WinMain() function
	return WinMain(GetModuleHandleW(0), 0, GetCommandLineW(), __ShowCmd);
}
