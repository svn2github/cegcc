#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <excpt.h>

HANDLE	h = 0;

void handler(struct _EXCEPTION_RECORD *ExceptionRecord,
		void *EstablisherFrame,
		struct _CONTEXT *ContextRecord,
		struct _DISPATCHER_CONTEXT *DispatcherContext)
{
	DWORD	w;
	WriteFile(h, "Handler\r\n", 9, &w, NULL);
	(void)CloseHandle(h);
	exit(0);
}

int main(int argc, char *argv[])
	__attribute__((__exception_handler__(handler)));

int main(int argc, char *argv[])
{
	int	*i;
	DWORD	w;

	h = CreateFile(L"" DATAPATH "/ex8log.txt", GENERIC_WRITE,
			0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	i = 0;
	WriteFile(h, "Main : start\r\n", 14, &w, NULL);
	*i = 1;
	WriteFile(h, "Main : end\r\n", 12, &w, NULL);
	(void)CloseHandle(h);
	exit(0);
}
