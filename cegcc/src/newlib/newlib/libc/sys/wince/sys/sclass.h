#ifndef _SCLASS_H_
#define _SCLASS_H_

typedef struct sclass_s *SCLASS;    /* Class */
typedef struct item_s *ITEM;        /* ITEM */

struct sclass_s {
  ITEM (*newFcn)(SCLASS);
  void (*freeFcn)(ITEM);
  ITEM freeItems;
  ITEM items;
  int  numItems;
};

struct item_s {
  union {
    SCLASS sclass;
    ITEM nextItem;
  } pointer;
  int contents[1];
};

SCLASS _sclass_new(int numItems, int sizeContents);
void   _sclass_free(SCLASS sclass);
ITEM   _item_new(SCLASS sclass);
void   _item_free(ITEM item);

#define _item_size(I)            ((I)->pointer.class->objectSize)
#define _item_contents(I)        ((I)->contents)
#define _item_offset(T,F)        ((UINT)&(((T)0)->F)

#endif /* _SCLASS_H_ */
