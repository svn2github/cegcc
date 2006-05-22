#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef int (*__mingwthr_key_dtor_t) (DWORD key, void (*dtor) (void *));
typedef int (*__mingwthr_remove_key_dtor_t)(DWORD key);

__mingwthr_key_dtor_t __mingwthr_key_dtor_ptr = 0;
__mingwthr_remove_key_dtor_t __mingwthr_remove_key_dtor_ptr = 0;
