/*
 * mthr.h
 *
 * Created by Pedro Alves <pedro_alves@portugalmail.pt>
 *
 */

/*
* These are defined in libc/sys/wince/mthr_thunk.c
*/
typedef int (*__mingwthr_key_dtor_t) (DWORD key, void (*dtor) (void *));
typedef int (*__mingwthr_remove_key_dtor_t)(DWORD key);
extern __mingwthr_key_dtor_t __mingwthr_key_dtor_ptr;
extern __mingwthr_remove_key_dtor_t __mingwthr_remove_key_dtor_ptr;

#define DLLEXPORT

#define __mingwthr_key_dtor __ce__mingwthr_key_dtor
#define __mingwthr_remove_key_dtor __ce__mingwthr_remove_key_dtor
int __mingwthr_key_dtor (DWORD key, void (*dtor) (void *));
int __mingwthr_remove_key_dtor (DWORD key );
