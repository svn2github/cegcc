/*
 * Excpt.h
 *
 * This is a header file missing from w32api but the newlib libc/sys/wince
 * requires some definition that appear to belong here.
 *
 * These are - currently - just the definitions required to compile libc/sys/wince,
 * stolen from the wcekernel.h file which came with libc/sys/wince.
 */
#ifndef	_CEGCC_INCLUDE_EXCPT_H_
#define	_CEGCC_INCLUDE_EXCPT_H_

typedef enum _EXCEPTION_DISPOSITION {
	ExceptionContinueExecution,
	ExceptionContinueSearch,
	ExceptionNestedException,
	ExceptionCollidedUnwind,
	ExceptionExecuteHandler
} EXCEPTION_DISPOSITION;

typedef struct _DISPATCHER_CONTEXT {
	ULONG ControlPc;
	struct _RUNTIME_FUNCTION *FunctionEntry;
	ULONG EstablisherFrame;
	PCONTEXT ContextRecord;
} DISPATCHER_CONTEXT, *PDISPATCHER_CONTEXT;

#endif	/* _CEGCC_INCLUDE_EXCPT_H_ */
