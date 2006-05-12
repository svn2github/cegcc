#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/llst.h>

LLST
_llst_create(int size)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Creates and returns a single-element list containing contents.
 *---------------------------------------------------------------------------*/
{
  LLST  llst = (LLST) calloc(1, sizeof(llst_t) + size - sizeof(int));

  return(llst);
}


void
_llst_destroy(LLST llst, void (* fcn)(PCHAR contents, PCHAR env), PCHAR env)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Annihilates the list containing llst, de-allocating its memory.  If fcn
 * is not NULL, the method applies it to the contents of each element before
 * annihilating the element.  Typically, fcn would destroy the contents.  The
 * env argument is an arbitrary environment pointer for fcn.  If fcn is NULL,
 * nothing is done to the contents; in particular, they are NOT destroyed.
 *---------------------------------------------------------------------------*/
{
  LLST  next;

  llst = _llst_first(llst);

  while (llst) {
    if (fcn) fcn((PCHAR)llst->contents, env);
    next = llst->next;
    free(llst);
    llst = next;
  }
}


LLST
_llst_append(LLST tail, LLST llst)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Append the given element llst to the tail.  Return the new tail.
 * WARNING: tail REALLY DOES have to be the tail!!
 *---------------------------------------------------------------------------*/
{
  if (llst) {
    llst->previous = tail;
    if (tail) {
      tail->next = llst;
    }
    llst->next = NULL;
  }

  return(llst);
}


LLST
_llst_prepend(LLST head, LLST llst)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Prepend the given element llst to the head.  Return the new head.
 * WARNING: head REALLY DOES have to be the head!!
 *---------------------------------------------------------------------------*/
{
  if (llst) {
    llst->next = head;
    if (head) {
      head->previous = llst;
    }
    llst->previous = NULL;
  }

  return(llst);
}


LLST
_llst_first(LLST llst)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Returns the first element of the list, the only one with no predecessor
 *---------------------------------------------------------------------------*/
{
  if (llst)
    while (llst->previous)
      llst = llst->previous;

  return(llst);
}


LLST
_llst_last(LLST llst)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Returns the last element of the list, the only one with no successor
 *---------------------------------------------------------------------------*/
{
  if (llst)
    while (llst->next)
      llst = llst->next;

  return(llst);
}


LLST
_llst_next(LLST llst)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Returns the next element of the list, if any
 *---------------------------------------------------------------------------*/
{
  return(llst ? llst->next : NULL);
}


LLST
_llst_previous(LLST llst)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Returns the previous element of the list, if any
 *---------------------------------------------------------------------------*/
{
  return(llst ? llst->previous : NULL);
}


LLST
_llst_remove(LLST llst)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Removes the element at llst from the list, returning the next element.
 * This method DOES NOT DESTROY the element at llst; it just extracts it
 * from the list as a single-element list in its own right.  To destroy the
 * extracted element, call _llst_destroy() on it.
 *---------------------------------------------------------------------------*/
{
  LLST  next;

  if (llst) {
    if (llst->previous)
      llst->previous->next = llst->next;
    if (llst->next)
      llst->next->previous = llst->previous;
    next = llst->next;
    llst->previous = NULL;
    llst->next = NULL;
  }

  return(next);
}

UINT
_llst_length(LLST llst)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Returns the number of elements in llst
 *---------------------------------------------------------------------------*/
{
  UINT  count = 0;

  llst = _llst_first(llst);
  while (llst) {
    llst = llst->next;
    count++;
  }

  return(count);
}
