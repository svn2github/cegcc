#ifndef _FINDPATH_H_
#define _FINDPATH_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

int rgetenv(char *name, char *value, int valuelen);
int _findexec(char *exename, char *pathbuf, int buflen);


#ifdef __cplusplus
}
#endif

#endif  /* _FINDPATH_H_ */
