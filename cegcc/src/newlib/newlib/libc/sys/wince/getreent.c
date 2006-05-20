/*  get thread-specific reentrant pointer

	we should move this into a dll, to allow static linking
	to behave correctly in a multithreaded world
	where DLL_THREAD_ATTACH/DLL_THREAD_DETTACH aren't called
*/

#include <reent.h>
#include <sys/wcebase.h>
#include <sys/wcethread.h>

#include "getreent.h"

static DWORD libc_thread_index = TLS_OUT_OF_INDEXES;

BOOL
__init_ce_reent(void)
{
	if (libc_thread_index == TLS_OUT_OF_INDEXES)
		libc_thread_index = TlsAlloc();
	if (libc_thread_index == TLS_OUT_OF_INDEXES)
		return FALSE;
	return TRUE;
}

__EXPORT BOOL __handle_ce_reent_(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
	struct _cereent* reent;

	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
			if (!__init_ce_reent())
				return FALSE;
		/* fall-through */
		case DLL_THREAD_ATTACH:
			reent = (struct _cereent*)__getreent ();
			if (!reent)
				return FALSE;
		break;

		case DLL_THREAD_DETACH:
			__freereent();
		break;

		case DLL_PROCESS_DETACH:
			__freereent();
			TlsFree(libc_thread_index);
		break;
	}
	return TRUE;
}

void __freereent (void)
{
	struct _cereent* reent = (struct _cereent*) TlsGetValue(libc_thread_index);
	if (!reent)
		return;
	free(reent);
	TlsSetValue(libc_thread_index, 0);
};

__EXPORT 
struct _reent *
__getreent (void)
{
	DWORD TL_LastError;

	TL_LastError = GetLastError();

	struct _reent* reent = (struct _reent*) TlsGetValue(libc_thread_index);
	if (!reent)
	{
		reent = __create_reent();
		if (!reent || !TlsSetValue(libc_thread_index, reent))
		{
			abort();
		}
	}

	SetLastError(TL_LastError);

	if (!reent)
	{
		abort();
	}

	return reent;

	// outdated doc follows...
	/* should only reach this on static linking.
		TODO: who should free _cereent when static linking?
		We could create a simple dll to listen to DLL_THREAD_ATTACH/DETACH.
		We would register a a callback in the dll to point to a function here.
		This new dll's DllMain would call the callback, in which we would destroy the _cereent.
		it would be so simple that we are guaranteed to keep binary compatibility
		forever. :)
	*/
}

struct _reent *
__create_reent (void)
{
	struct _reent* reent = (struct _reent*) calloc(1, sizeof(struct _cereent));
	if (!reent)
		return NULL;
	_REENT_INIT_PTR((reent));
	__init_cereent((struct _cereent*)reent);
	return reent;
}
