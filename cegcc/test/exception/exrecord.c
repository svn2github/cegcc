#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "myexcpt.h"

wchar_t	msg[256];
int	i = 0;

int handler(struct _EXCEPTION_RECORD *ExceptionRecord,
		void *EstablisherFrame,
		struct _CONTEXT *ContextRecord,
		struct _DISPATCHER_CONTEXT *DispatcherContext)
{
	wsprintf(msg, L"Crash, count %d", i);
	fprintf(stderr, "Crash, count %d\nExc Code %x Addr %x Nparams %d\n",
			i,
			ExceptionRecord->ExceptionCode,
			ExceptionRecord->ExceptionAddress,
			ExceptionRecord->NumberParameters);
	MessageBoxW(0, msg, L"WinCE Exception", 0);
	i++;
#if 1
	return EXCEPTION_EXECUTE_HANDLER;
	return EXCEPTION_CONTINUE_EXECUTION;
#else
#warning This will cause a near-infinite loop
	if (i < 5)
		return EXCEPTION_CONTINUE_SEARCH;	// Don't do this, infinite loop.
	return EXCEPTION_CONTINUE_EXECUTION;
#endif
}

int main(int argc, char *argv[])
	__attribute__((__exception_handler__(handler)));

int main(int argc, char *argv[])
{
	volatile int	*i;

	i = 0;
	*i = 1;
	MessageBoxW(0, L"Main", L"Survived", 0);
	fprintf(stderr, "Main: survived\n");
	exit(0);
}
