#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "myexcpt.h"

void handler(struct _EXCEPTION_RECORD *ExceptionRecord,
		void *EstablisherFrame,
		struct _CONTEXT *ContextRecord,
		struct _DISPATCHER_CONTEXT *DispatcherContext)
{
	MessageBoxW(0, L" ? ", L"Handler : error", 0);
	exit(0);
}

int main(int argc, char *argv[])
	__attribute__((__exception_handler__(handler)));

int main(int argc, char *argv[])
{
	int	*i;

	i = 0;
	*i = 1;
	exit(0);
}
