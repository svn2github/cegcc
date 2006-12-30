#include <windows.h>

void bye(void)
{
	MessageBox(NULL, L"Stop", L"Halt", 0);
}

/*
 * Profiling test
 */
extern void func_a(int i);
extern int fibo(int x);
extern int func_b(int i);

main(int argc, char *argv[])
{
	int	i;

	(void)atexit(bye);

	for (i=1; i<10; i++) {
		func_a(i);
	}

	for (i=1; i<30; i++) {
		(void) func_b(i);
	}
}
