// cethread.c
//
// Time-stamp: <12/02/01 17:55:25 keuchel@keuchelnt>

/* adapted to cegcc <pedro_alves@portugalmail.pt> */

#define DEBUG_THREADS

#ifdef DEBUG_THREADS
# define DUMP_ERROR() do { WCETRACE_ERROR(WCE_IO, GetLastError()); } while (0)
//# define DUMP_ERROR() do { WCETRACE_ERROR(WCE_IO, GetLastError()); printf ("at line %d\n", __LINE__ ); } while (0)
#else
# define DUMP_ERROR() do; while (0)
#endif

#include "sys/wcetrace.h"
#include "getreent.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdint.h>

static DWORD _threadstart(void *);
static DWORD _threadstartex(void * ptd);

typedef void( *start_address_t )( void * );
typedef unsigned( *start_address_ex_t )( void * );

void _endthreadex (unsigned retcode);
void _endthread (void);

typedef struct thread_starter_args
{
	union
	{
		start_address_t start_address;
		start_address_ex_t start_address_ex;
	};

	void* args;
	HANDLE handle;
	DWORD tid;
} thread_starter_args;


uintptr_t
_beginthread(void (* start_address) (void *), unsigned stacksize, 
                void * arglist)
{
  struct _reent *ptd;

  struct thread_starter_args* sargs = malloc(sizeof(thread_starter_args));
  if (!sargs)
	  return((uintptr_t)-1L);

  sargs->start_address = start_address;
  sargs->args = arglist;

  if ( (sargs->handle = CreateThread( NULL,
		      stacksize,
		      _threadstart,
		      (LPVOID)sargs,
		      CREATE_SUSPENDED,
		      &(sargs->tid) )) == 0L )
    {
      DUMP_ERROR();
      goto error_return;
    }

  if ( ResumeThread ( sargs->handle ) == (DWORD)(-1L) ) {
    DUMP_ERROR();
    if (sargs->handle && (DWORD)sargs->handle != (DWORD)-1)
      CloseHandle (sargs->handle);
    goto error_return;
  }

  return (uintptr_t)sargs->handle;

error_return:
  free(sargs);
  return((uintptr_t)-1L);
}

DWORD _threadstart (void * ptd)
{
  struct thread_starter_args* pargs = (struct thread_starter_args*)ptd;

  start_address_t start_address = pargs->start_address;
  void* args = pargs->args;

  struct _cereent* reent = (struct _cereent*)__getreent ();
  reent->_thandle = pargs->handle;

  free (pargs);

  start_address (args);

  _endthread();

  // not reached
  return 0;
}


void 
_endthread (void)
{
  struct _cereent* ptd;

  if((ptd = (struct _cereent*)__getreent ()) == NULL)
    abort();

  if(ptd->_thandle != (HANDLE)(-1L))
  {
    CloseHandle (ptd->_thandle);
	ptd->_thandle = (HANDLE)(-1L);
  }

  __freereent ();

  ExitThread (0);
}

uintptr_t
_beginthreadex (void *security,
                   unsigned stacksize,
                   unsigned (*initialcode) (void *),
                   void * argument,
                   unsigned createflag,
                   unsigned *thrdaddr)
{
  unsigned long thdl;

  struct thread_starter_args* sargs = malloc(sizeof(thread_starter_args));
  if (!sargs)
	  return((unsigned long)0L);

  sargs->start_address_ex = initialcode;
  sargs->args = argument;
  sargs->handle = (HANDLE)(-1L);

  if ((thdl = (unsigned long)
       CreateThread( security,
		     stacksize,
		     _threadstartex,
		     sargs,
		     createflag,
			 &(sargs->tid) )) == 0L )
    {
      DUMP_ERROR();
	    free(sargs);
	    return((unsigned long)0L);
    }

  if (thrdaddr)
	  *thrdaddr = sargs->tid;

  return thdl;
}

DWORD
_threadstartex(void * ptd)
{
  struct thread_starter_args* pargs = (struct thread_starter_args*)ptd;

  start_address_ex_t start_address_ex = pargs->start_address_ex;
  void* args = pargs->args;
  free (pargs);

  unsigned ret = start_address_ex(args);
  _endthreadex(ret);

  // not reached
  return(0L);
}

void
_endthreadex (unsigned retcode)
{
  struct _reent* ptd;

  if((ptd = __getreent()) == NULL)
    abort();

  __freereent();

  ExitThread(retcode);
}
