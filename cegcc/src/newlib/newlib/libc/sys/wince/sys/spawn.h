#ifndef _SPAWN_H_
#define _SPAWN_H_

#include <stddef.h>

#define MAX_CMDLEN   (128)
#define MAX_CMDLINE  (512)

#ifdef __cplusplus
extern "C" {
#endif

int newpgid();

int _spawnv(const char *command, char* const *argv, int pgid, int infd, int outfd, int errfd);
int _spawnvp(const char *command, char* const *argv, int pgid, int infd, int outfd, int errfd);
int _spawn(const char *path, char* const * argv);
int _await(int pid, int msec);
void *_getchildhnd(int pid);

#ifdef __cplusplus
}
#endif

#endif  /* _SPAWN_H_ */
