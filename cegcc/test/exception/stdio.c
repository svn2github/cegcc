#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "myexcpt.h"

int	i = 0;
FILE	*f = NULL;

int handler(struct _EXCEPTION_RECORD *ExceptionRecord,
		void *EstablisherFrame,
		struct _CONTEXT *ContextRecord,
		struct _DISPATCHER_CONTEXT *DispatcherContext)
{
	fprintf(stderr, "Crash, count %d\nExc Code %x Addr %x Nparams %d\n",
			i,
			ExceptionRecord->ExceptionCode,
			ExceptionRecord->ExceptionAddress,
			ExceptionRecord->NumberParameters);
	i++;
	return EXCEPTION_EXECUTE_HANDLER;
}

#define	FN	DATAPATH "/exc-log.txt"

int main(int argc, char *argv[])
	__attribute__((__exception_handler__(handler)));

int main(int argc, char *argv[])
{
	volatile int	*i;

	f = fopen(FN, "w");
	if (f == NULL) {
		fprintf(stderr, "File " FN " not open\n");
		exit(0);
	}

	i = 0;
	fprintf(stderr, "Start\n");
	*i = 1;
	fprintf(stderr, "Main: survived\n");
	fclose(f);
	exit(0);
}
