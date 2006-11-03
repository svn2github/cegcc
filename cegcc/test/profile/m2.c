#include <windows.h>


/*
 * Profiling test with WinMain().
 */
extern void func_a(int i);
extern int fibo(int x);
extern int func_b(int i);

void gui(void)
{
	MessageBoxW(0, L"HELLO!", L"H3LLO!", 0);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	int	i;

	for (i=1; i<10; i++) {
		func_a(i);
	}

	for (i=1; i<30; i++) {
		(void) func_b(i);
	}

	gui();
}
