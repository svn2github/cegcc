#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// #include "myexcpt.h"

struct _DISPATCHER_CONTEXT;

FILE	*f = NULL;

int mhandler(struct _EXCEPTION_RECORD *ExceptionRecord,
		void *EstablisherFrame,
		struct _CONTEXT *ContextRecord,
		struct _DISPATCHER_CONTEXT *DispatcherContext)
{

	MessageBoxW(0, L"Crash", L"Handler-main", 0);
//	fprintf(stderr, "Crash Handler-main\n");
	return EXCEPTION_EXECUTE_HANDLER;
}

int chandler(struct _EXCEPTION_RECORD *ExceptionRecord,
		void *EstablisherFrame,
		struct _CONTEXT *ContextRecord,
		struct _DISPATCHER_CONTEXT *DispatcherContext)
{

	MessageBoxW(0, L"Crash", L"Handler-crash", 0);
//	fprintf(stderr, "Crash Handler-crash\n");
	return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[])
	__attribute__((__exception_handler__(mhandler)));
void crash(void)
	__attribute__((__exception_handler__(chandler)));

void crash(void)
{
	int	*i;

	i = 0;
//	fprintf(stderr, "Crash : started\n");
	*i = 1;
//	fprintf(stderr, "Crash : end\n");
}

int main(int argc, char *argv[])
{
	int	*i;

//	f = fopen(DATAPATH "/ex1log.txt", "w");
//	if (f == NULL)
//		fprintf(stderr, "NULL file\n");

//	fprintf(stderr, "Main : start\n");
	crash();
//	fprintf(stderr, "Main : survived\n");
	exit(0);
}
