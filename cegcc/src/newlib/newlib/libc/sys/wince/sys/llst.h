#ifndef _LLST_H_
#define _LLST_H_

#include <sys/wcetypes.h>

#ifdef __cplusplus
extern "C" { 
#endif

/* the linked list */
typedef struct llst_s * LLST;

typedef struct llst_s {
  LLST  next;
  LLST  previous;
  int   contents[1];
} llst_t;

/* No public methods */
extern LLST  _llst_create(int size);
extern void  _llst_destroy(LLST llist, void (* fcn)(PCHAR contents, PCHAR env), PCHAR env);
extern LLST  _llst_append(LLST tail, LLST llst);
extern LLST  _llst_prepend(LLST head, LLST llst);
extern LLST  _llst_first(LLST llist);
extern LLST  _llst_last(LLST llist);
extern LLST  _llst_next(LLST llist);
extern LLST  _llst_previous(LLST llist);
extern LLST  _llst_remove(LLST llist);
extern PCHAR _llst_contents(LLST llist);
extern PCHAR _llst_replaceContents(LLST llist, PCHAR contents);
extern LLST  _llst_find(LLST llist, PCHAR contents);
extern UINT  _llst_length(LLST llist);

#ifdef DEBUG
extern void  _llst_dump(LLST llist);
#endif

#ifdef __cplusplus
}
#endif
#endif /* _LLST_H_ */
