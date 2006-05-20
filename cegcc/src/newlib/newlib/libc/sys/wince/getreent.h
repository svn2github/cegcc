#ifndef CE_GETREENT_H
#define CE_GETREENT_H

#include <reent.h>
#include <sys/wcebase.h>
#include <sys/wcefile.h>
#include <wchar.h>

/* inherit from _reent, and add new members 
	works as long as nowhere in newlib is sizeof(_reent) requested
	if this doesn't work, we need to copy sys/reent.h to sys/wince/sys
	and extend it there
*/

struct _cereent
{
	struct _reent reent;

#if 0
	// currentdir is per thread, so you can write a multi-threaded ftpd.
	// this differs from normal Win32, where currentdir is per process.
	// UPDATE: on second thought disable this
	wchar_t _current_dirw[MAX_PATH];
	wchar_t _current_root_dirw[MAX_PATH];
#endif
	void* _thandle;

/*
	// other stuff to be inserted (strerror, asctime, ...)
	void * _curwin; // for console
	(VAR)->_opterr = 1; \
	(VAR)->_optind = 1; \
*/
};

#define __init_cereent(VAR) \
	do { \
		/* (VAR)->_current_dirw[0] = '\\'; (VAR)->_current_dirw[1] = L'\0'; */ \
		/* (VAR)->_current_root_dirw[0] = '\\'; (VAR)->_current_root_dirw[1] = L'\0'; */ \
		(VAR)->_thandle = (void*)-1; \
	} while(0)

#endif

void __freereent (void);
BOOL __init_ce_reent(void);
struct _reent *__create_reent (void);

