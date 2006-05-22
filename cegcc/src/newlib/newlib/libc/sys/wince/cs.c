#include <windows.h>

#include <stdlib.h>
#include <sys/cs.h>
#include <sys/llst.h>

#include "sys/wcetrace.h"

#define CS_SCLASS_SIZE     (32768)
#define CS_CHUNKLIST_SIZE  (64)
#define CS_INITIAL_ALLOC   (1024)
#define CS_ALLOC_INCREMENT (1024)

static UCHAR *chunklist[CS_CHUNKLIST_SIZE];
static int  chunkidx = 0;
static int  listsize = 0;
static LLST firstcs = NULL;
static LLST lastcs = NULL;
static int  initialized = FALSE;

static CRITICAL_SECTION alloccs;
static CRITICAL_SECTION freecs;

static void _cs_extend(int count);

static void (*gcfcn)(void) = NULL;
static int gcthresh = 10240;
static int alloccnt = 0;

void
cs_setparam(void (*fcn)(void), int thresh)
{
  gcfcn = fcn;
  gcthresh = thresh;
}
  
CS
cs_alloc()
{
  LLST cs;

  if (!initialized) {
    /* Initial optimism */
    initialized = TRUE;
    InitializeCriticalSection(&alloccs);
    InitializeCriticalSection(&freecs);

    EnterCriticalSection(&alloccs);
    _cs_extend(CS_ALLOC_INCREMENT);
    LeaveCriticalSection(&alloccs);
  }

  /* When we are down to the last cs, invoke GC when available */
  if (firstcs == lastcs && gcfcn != NULL) {
    if (alloccnt > gcthresh) {
      WCETRACE(WCE_SYNCH, "cs_alloc: doing GC after %d allocs", alloccnt);
      (*gcfcn)();
      alloccnt = 0;
    }
  }

  EnterCriticalSection(&alloccs);
  /* Extend if there is no cs available now */
  if (firstcs == lastcs) {
     _cs_extend(CS_ALLOC_INCREMENT);
  }

  if (firstcs == NULL) {
    cs = NULL;
  } else {
    cs = firstcs;
    firstcs = _llst_remove(firstcs);
  }
  LeaveCriticalSection(&alloccs);

  WCETRACE(WCE_SYNCH, "cs_alloc: %p", cs);
  alloccnt++;
  return((CS)cs);
}

static void
_cs_extend(int count)
{
  LLST cs;
  int i, itemsize;
  UCHAR *ptr;

  WCETRACE(WCE_SYNCH, "_cs_extend: %d:%d", count, listsize);
  listsize += count;

  itemsize = sizeof(llst_t) - sizeof(int) + sizeof(CRITICAL_SECTION);

  EnterCriticalSection(&alloccs);
  EnterCriticalSection(&freecs);

  ptr = chunklist[chunkidx] = (UCHAR *) calloc(count, itemsize);
  if (ptr == NULL) {
    WCETRACE(WCE_SYNCH, "_cs_extend: FATAL ERROR malloc failed");
    firstcs = lastcs = NULL;
    LeaveCriticalSection(&freecs);
    LeaveCriticalSection(&alloccs);
    return;
  }

  for (i = 0; i < count; i++) {
    cs = (LLST) ptr;
    InitializeCriticalSection((LPCRITICAL_SECTION)&cs->contents);
    if (firstcs == NULL) {
      firstcs = lastcs = _llst_append(firstcs, cs);
    } else {
      lastcs = _llst_append(lastcs, cs);
    }
    ptr += itemsize;
  }

  chunkidx++;
  LeaveCriticalSection(&freecs);
  LeaveCriticalSection(&alloccs);
}
  
void
cs_free(CS cs)
{
  WCETRACE(WCE_SYNCH, "cs_free: %p", cs);
  if (lastcs == NULL) {
    WCETRACE(WCE_SYNCH, "cs_free: FATAL ERROR last cs is NULL");
    return;
  }

  EnterCriticalSection(&freecs);
  lastcs = _llst_append(lastcs, (LLST)cs);
  LeaveCriticalSection(&freecs);
}

/* For now this does nothing */
void
cs_destroy()
{
  return;
}

#if 0
void
cs_destroy()
{
  LLST cs;

  WCETRACE(WCE_SYNCH, "cs_destroy: CALLED");
  EnterCriticalSection(&critsect);
  if (firstcs) {
    do {
      cs = firstcs;
      firstcs = _llst_remove(firstcs);
      DeleteCriticalSection((LPCRITICAL_SECTION)&cs->contents);
    } while (firstcs != NULL);
  }
  LeaveCriticalSection(&critsect);

  DeleteCriticalSection(&critsect);
  initialized = FALSE;
  WCETRACE(WCE_SYNCH, "cs_destroy: DONE");
}
#endif

/* Synchronization Methods as provided by M$ CRITICAL_SECTION */

void
cs_enter(CS cs)
{
  EnterCriticalSection((LPCRITICAL_SECTION)&(((LLST)cs)->contents));
}

void
cs_leave(CS cs)
{
  LeaveCriticalSection((LPCRITICAL_SECTION)&(((LLST)cs)->contents));
}

BOOL
cs_tryenter(CS cs)
{
  return(TryEnterCriticalSection((LPCRITICAL_SECTION)&(((LLST)cs)->contents)));
}
    
    


