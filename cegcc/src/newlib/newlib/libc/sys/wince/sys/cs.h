#ifndef _CS_H_
#define _CS_H_

#include <sys/wcetypes.h>

#ifdef __cplusplus
extern "C" { 
#endif

typedef void *CS;

void cs_setparam(void (*fcn)(void), int thresh);
CS cs_alloc();
void cs_free(CS cs);
void cs_destroy();
void cs_enter(CS cs);
void cs_leave(CS cs);
BOOL cs_tryenter(CS cs);

#ifdef __cplusplus
}
#endif
#endif /* _CS_H_ */
