/* Threads compatibility routines for libgcc2 and libobjc.  */
/* Compile this one with gcc.  */

/* Copyright (C) 1999, 2000, 2002, 2003, 2004, 2005
   Free Software Foundation, Inc.
   Contributed by Mumit Khan <khan@xraylith.wisc.edu>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

/* As a special exception, if you link this library with other files,
   some of which are compiled with GCC, to produce an executable,
   this library does not by itself cause the resulting executable
   to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */

/*
 * Just enough declarations to enable gthr-win32.h to compile for wince targets
 * This is a hack, when we have mingw headers working correctly, 
 * this file can be dumped.
 *
 * Put this in $(PREFIX)/arm-wince-pe/sys-include when building gcc
 *
 * Pedro Alves 2006 <pedro_alves@portugalmail.pt>
*/

#ifndef GCC_GTHR_WINCE_H
#define GCC_GTHR_WINCE_H

#ifdef __cplusplus
extern "C" {
#endif

long InterlockedIncrement(long* lpAddend);
typedef int BOOL;
typedef unsigned long DWORD;
typedef void* HANDLE;
typedef void* LPVOID;
typedef long LONG;
typedef LONG* LPLONG;
typedef unsigned short USHORT;
typedef const USHORT* LPCWSTR;
typedef void* PVOID;
typedef char CHAR;
typedef CHAR* LPBYTE;

#define TRUE 1
#define FALSE 0

void Sleep(DWORD dwMilliseconds);

DWORD  TlsCall(DWORD, DWORD);
#define TLS_FUNCALLOC   0
#define TLS_FUNCFREE    1
#define TlsAlloc()  (TlsCall(TLS_FUNCALLOC, 0))
#define TlsFree(x)  (TlsCall(TLS_FUNCFREE, x))

void* TlsGetValue(DWORD);
BOOL  TlsSetValue(DWORD, void*);

typedef struct _SECURITY_ATTRIBUTES {
  DWORD nLength;
  LPVOID lpSecurityDescriptor;
  BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

HANDLE CreateSemaphoreW(LPSECURITY_ATTRIBUTES, LONG, LONG, LPCWSTR);
#define CreateSemaphore CreateSemaphoreW
BOOL ReleaseSemaphore(HANDLE, LONG, LPLONG);

DWORD GetLastError ( void );
void SetLastError ( DWORD dwErrCode );

#define WAIT_OBJECT_0 0x00000000L
#define INFINITE 0xffffffffL
DWORD WaitForSingleObject (HANDLE, DWORD);

LONG InterlockedDecrement(LPLONG);
LONG InterlockedCompareExchange(LPLONG Destination, LONG Exchange, LONG Comperand);

#define GetCurrentThreadId() ((DWORD)(((HANDLE *)(PUserKData+SYSHANDLE_OFFSET))[SH_CURTHREAD]))

#if defined(SARM) || defined(__arm__)
#define PUserKData ((LPBYTE)0xFFFFC800)
#else
#define PUserKData ((LPBYTE)0x00005800)
#endif
#define SYSHANDLE_OFFSET 0x004
#define SH_CURTHREAD            1

#ifdef __cplusplus
}
#endif

#endif /* ! GCC_GTHR_WINCE_H */
