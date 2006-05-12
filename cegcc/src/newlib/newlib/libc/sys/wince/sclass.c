/* sclass.c - simple (fast) class facility without threads support
 * DESCRIPTION
 * Create, annihilate and manipulate SCLASSes and ITEMs
 *---------------------------------------------------------------------------*/
#include <stdlib.h>

#include <sys/wcebase.h>
#include <sys/sclass.h>

SCLASS
_sclass_new(int numItems, int sizeContents)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Make a new sclass and set up a list of all its free objects
 *---------------------------------------------------------------------------*/
{
  SCLASS sclass;
  ITEM   item;
  int    sizeItems;

  sizeItems = sizeof(struct item_s) + sizeContents - sizeof(int);

#ifdef dec
  /* Pad sizeItems so that objects are 64-bit aligned on Alpha */
  if (sizeItems & 0x7) sizeItems = (sizeItems & ~0x7) + 8;
#endif
  /* Allocate space for the sclass */
  if ((sclass = (SCLASS) calloc(1, sizeof(struct sclass_s))) == NULL)
    return(NULL);

  /* Allocate space for the items */
  if ((item = (ITEM) calloc(numItems, sizeItems)) == NULL)
    return(NULL);

  /* Initialize the sclass */
  sclass->newFcn = NULL;
  sclass->freeFcn = NULL;
  sclass->items = sclass->freeItems = item;
  sclass->numItems = numItems;
  /* Link the items into a free list */
  while (--numItems) {
    item->pointer.nextItem = (ITEM) ((unsigned char *) item + sizeItems);
    item = item->pointer.nextItem;
  }
  item->pointer.nextItem = NULL;

  return(sclass);
}

void
_sclass_free(SCLASS sclass)
{
  free(sclass->items);
  free(sclass);
}

ITEM
_item_new(SCLASS sclass)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Get a new item of the given sclass
 *---------------------------------------------------------------------------*/
{
  ITEM item;

  /* If a special newFcn() exists, use it */
  if (sclass->newFcn != NULL)
    return(sclass->newFcn(sclass));

  /* Otherwise, use the following generic procedure */
  if ((item = sclass->freeItems) != NULL) {
    sclass->freeItems = item->pointer.nextItem;
    item->pointer.sclass = sclass;
  }
  return(item);
}

void
_item_free(ITEM item)
/*---------------------------------------------------------------------------*
 * DESCRIPTION
 * Free up an item
 *---------------------------------------------------------------------------*/
{
  SCLASS sclass;

  /* Get the class of the item */
  sclass = item->pointer.sclass;

  /* If a special freeFcn() exists for this class, use it */
  if (sclass->freeFcn != NULL)
    sclass->freeFcn(item);

  /* Otherwise, use the following generic procedure */
  else {
    item->pointer.nextItem = sclass->freeItems;
    sclass->freeItems = item;
  }
}

