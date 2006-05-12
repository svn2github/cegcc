/*
 * mthr_stub.c
 *
 * Implement Mingw thread-support stubs for single-threaded C++ apps.
 *
 * This file is used by if gcc is built with --enable-threads=win32 and
 * iff gcc does *NOT* use -mthreads option. 
 *
 * The -mthreads implementation is in mthr.c.
 *
 * Created by Mumit Khan  <khan@nanotech.wisc.edu>
 *
 */

/*
 * Adapted to cegcc by
 * Pedro Alves <pedro_alves@portugalmail.pt>
 *
 * By default, the key_dtors are ignored, just like -mthreads was NOT passed to gcc on mingw.
 * If mthr.dll is linked in, it's DllMain will fill the key/remove ptrs below to its own real functions.
 * Might be problematic in exceptions in c++ global constructors, but those are a problem on their own.
 * 
 */

#ifndef GNUWINCE
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
#else
#include <sys/wcebase.h>
#include <sys/wcethread.h>
#include <sys/wceerror.h>
# define DEBUG
#endif

typedef int (*__mingwthr_key_dtor_t) (DWORD key, void (*dtor) (void *));
typedef int (*__mingwthr_remove_key_dtor_t)(DWORD key);

extern __mingwthr_key_dtor_t __mingwthr_key_dtor_ptr;
extern __mingwthr_remove_key_dtor_t __mingwthr_remove_key_dtor_ptr;

/*
 * __mingwthr_register_key_dtor (DWORD key, void (*dtor) (void *))
 *
 * Public interface called by C++ exception handling mechanism in
 * libgcc (cf: __gthread_key_create).
 * No-op versions.
 */

int
__mingwthr_key_dtor (DWORD key, void (*dtor) (void *))
{
  if (__mingwthr_key_dtor_ptr)
    return __mingwthr_key_dtor_ptr(key, dtor);

#ifdef DEBUG
  NKDbgPrintfW ("%S: ignoring key: (%ld) / dtor: (%x)\n", 
          __FUNCTION__, key, dtor);
  printf ("%s: ignoring key: (%ld) / dtor: (%x)\n", 
    __FUNCTION__, key, dtor);
#endif
  return 0;
}

int
__mingwthr_remove_key_dtor (DWORD key)
{
  if (__mingwthr_remove_key_dtor_ptr)
    return __mingwthr_remove_key_dtor_ptr(key);

#ifdef DEBUG
  NKDbgPrintfW ("%S: ignoring key: (%ld)\n", __FUNCTION__, key );
  printf ("%s: ignoring key: (%ld)\n", __FUNCTION__, key );
#endif
  return 0;
}
