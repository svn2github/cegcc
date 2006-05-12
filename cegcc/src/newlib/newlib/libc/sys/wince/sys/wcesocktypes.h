#ifndef _WCESOCKTYPES_H_
#define _WCESOCKTYPES_H_

#include <sys/types.h>

typedef unsigned int  SOCKET;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FD_SETSIZE
# define FD_SETSIZE 64
#endif

#ifndef _WINSOCKAPI_

typedef struct fd_set {
	unsigned int fd_count;
	SOCKET  fd_array[FD_SETSIZE];
} fd_set;

__declspec(dllimport) int __WSAFDIsSet(SOCKET, fd_set *);

#define FD_CLR(fd, set) do { \
	unsigned int __i; \
	for (__i = 0; __i < (set)->fd_count ; __i++) { \
		if ((set)->fd_array[__i] == fd) { \
			while (__i < (set)->fd_count-1) { \
				(set)->fd_array[__i] = \
				(set)->fd_array[__i+1]; \
				__i++; \
			} \
			(set)->fd_count--; \
			break; \
		} \
	} \
} while(0)

#define FD_SET(fd, set) do { \
	unsigned int __i; \
	for (__i = 0; __i < (set)->fd_count; __i++) { \
		if (((set))->fd_array[__i] == (fd)) \
			break; \
	} \
	if ((__i == (set)->fd_count) && ((set)->fd_count < FD_SETSIZE)) { \
		(set)->fd_array[__i] = (fd); \
		(set)->fd_count++; \
	} \
} while(0)

#define FD_ZERO(set) do { (set)->fd_count=0; } while (0)
#define FD_ISSET(fd, set) __WSAFDIsSet(fd, set)

#endif

#ifdef __cplusplus
}
#endif

#endif  /* _WCESOCKTYPES_H_ */
