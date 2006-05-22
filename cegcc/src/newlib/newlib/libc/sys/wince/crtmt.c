/*
 * crtmt.c
 *
 * This object file defines _CRT_MT to have a value of 1, which will
 * turn on MT support in GCC runtime. This is only linked in when
 * you specify -mthreads when linking with gcc. The Mingw support
 * library, libmingw32.a, contains the complement, crtst.o, which
 * sets this variable to 0. 
 *
 * Mumit Khan  <khan@nanotech.wisc.edu>
 * Pedro Alves <pedro_alves@portugalmail.pt>
 *
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/*
 * defined in _crt_mt.c
*/
extern int _CRT_MT;

/*
 * I changed this to be a function so -lgcc is all in cegcc.dll, 
 * crt0.o calls __set_runtime_thread_mode that depending on -mthreads, can call this or
 * the version in crtst.c.
*/
void __set_runtime_thread_mode(void)
{
  _CRT_MT = 1;

  /* force cegccthrd.dll linking by using a symbol from it */
  typedef int (*__mingwthr_remove_key_dtor_t)(DWORD key);
  extern int __ce__mingwthr_remove_key_dtor(DWORD key);
  volatile __mingwthr_remove_key_dtor_t dummy = __ce__mingwthr_remove_key_dtor;
  (void)dummy; /*shut up unused warning*/
}
