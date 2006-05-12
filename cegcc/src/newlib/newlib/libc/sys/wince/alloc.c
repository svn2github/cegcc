#include <stdlib.h>
#include <stddef.h>

#include <sys/wcebase.h>
#include <sys/wceerror.h>
#include <sys/wcethread.h>
#include <sys/wcetrace.h>

#if defined(MALLOC_PROVIDED)
void *
_calloc_r(struct _reent *reent, size_t nitems, size_t size)
{
  void *retval = NULL;

  retval = calloc(nitems, size);
  return(retval);
}

void
_free_r(struct _reent *reent, void *ptr)
{
  free(ptr);
}

void *
_malloc_r(struct _reent *reent, size_t len)
{
  void *retval = NULL;

  retval = malloc(len);
  return(retval);
}

void *
_realloc_r(struct _reent *reent, void *ptr, size_t size)
{
  void *retval = NULL;

  retval = realloc(ptr, size);
  return(retval);
}

void *
_sbrk_r(int incr) {
  return((void *)0);
}

#else

static HANDLE mutex;
static DWORD lockid;                    /* Thread ID of lock holder */
static int   locks;                     /* # of times __malloc_lock has recursed */

void _init_dlmalloc()
{
  mutex = CreateMutexW(NULL, FALSE, NULL);
  lockid = 0;
  locks = 0;
}

void
__malloc_lock(struct _reent *reent)
{
  WaitForSingleObject(mutex, INFINITE);
}

void
__malloc_unlock(struct _reent *reent)
{
  ReleaseMutex(mutex);
}
  
void *
_sbrk_r(int incr) {
  return(wsbrk(incr));
}

#endif
