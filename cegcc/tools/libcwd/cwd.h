#ifndef __LIB_CE_CWD_H_
#define __LIB_CE_CWD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

extern char *getcwd (char *buf, int size);
extern char *_getcwd (char *buf, int size);
extern int chdir (const char *path);
extern int _chdir (const char *path);

extern char* libcwd_fixpath (char* out, const char *in);

#ifdef __cplusplus
}
#endif

#endif /* __LIB_CE_CWD_H_ */
