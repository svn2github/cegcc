#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>

#include <sys/sysconf.h>
#include <sys/wcebase.h>
#include <sys/wcetrace.h>
#include <sys/wcethread.h>
#include <sys/wcekernel.h>
#include <sys/shared.h>
#include <sys/io.h>

EXCEPTION_DISPOSITION
_eh_handler_(struct _EXCEPTION_RECORD *ExceptionRecord, 
			 void *EstablisherFrame, 
			 struct _CONTEXT *ContextRecord, 
			 struct _DISPATCHER_CONTEXT *DispatcherContext);

typedef int fnmain(int argc, char** argv); 

int _startup_(fnmain *_main);

EXCEPTION_DISPOSITION 
_eh_handler(struct _EXCEPTION_RECORD *ExceptionRecord,
			      void* EstablisherFrame, 
            struct _CONTEXT* ContextRecord,
            struct _DISPATCHER_CONTEXT *DispatcherContext)
{
	return _eh_handler_(ExceptionRecord, EstablisherFrame, 
						ContextRecord, DispatcherContext);
}

void _pei386_runtime_relocator();

int _startup(fnmain *_main)
{
  _pei386_runtime_relocator();
	_startup_(_main);
}
