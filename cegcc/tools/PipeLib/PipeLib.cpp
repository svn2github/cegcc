/* Copyright (c) 2007, Pedro Alves

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  */

#include <windows.h>

#include "PipeLib.h"
#include "PipeDev.h"

#include <devload.h>
#include <stdlib.h>

#define KEY_NAME_BASE L"PipeDevice_"
#define NAME_BASE L"PD"	/* Pipe Device */
#define MAX_INSTANCES 99

#if 0

/* Doesn't work on CE < 5 ... */
static int
FindFreeInstanceIndex (void)
{
  HANDLE h;
  DEVMGR_DEVICE_INFORMATION di;
  WCHAR wzName[6];
  for (int i=0; i < MAX_INSTANCES; i++)
    {
      wsprintf (wzName, L"%s%02d:", NAME_BASE, i);
      di.dwSize = sizeof (di);
      h = FindFirstDevice (DeviceSearchByLegacyName, wzName, &di);
      if (h == INVALID_HANDLE_VALUE)
	return i;
      CloseHandle (h);
    }
  return -1;
}
#endif

static void
PrepareRegistryForInstance (DWORD dwIndex, WCHAR** wzKey)
{
  DWORD dw;
  HKEY hk;
  WCHAR szKeyName[255];
  WCHAR wzPrefix[4]; //3 letter + zero character
  DWORD dwDisp;
  swprintf (wzPrefix, L"%s%d", NAME_BASE, dwIndex / 10);

  swprintf (szKeyName, L"Drivers\\%s%d", KEY_NAME_BASE, dwIndex);
  *wzKey = wcsdup (szKeyName);

  if (ERROR_SUCCESS != RegCreateKeyEx (HKEY_LOCAL_MACHINE,
				       szKeyName, 0, NULL, 0,
				       KEY_WRITE, NULL, &hk, &dwDisp))
    {
      wprintf (L"Failed to create registry key %s, error = %d\n",
	       szKeyName, (int) GetLastError ());
      return;
    }

  RegSetValueEx (hk, L"dll", 0, REG_SZ, (BYTE *)DEVICE_DLL_NAME,
		 sizeof (DEVICE_DLL_NAME));
  RegSetValueEx (hk, L"prefix", 0, REG_SZ, (BYTE *)wzPrefix,
		 sizeof (wzPrefix));

  dw = dwIndex % 10;
  RegSetValueEx (hk, L"index", 0, REG_DWORD, (BYTE *)&dw, sizeof (dw));

  dw = DEVFLAGS_LOADLIBRARY | DEVFLAGS_NAKEDENTRIES;
  RegSetValueEx (hk, L"Flags", 0, REG_DWORD, (BYTE *)&dw, sizeof (dw));

  RegCloseKey (hk);
}

/* Undocumented, for pipedev.dll debugging purposes only.  */
PIPELIB_API BOOL
SetPipeTag (HANDLE p, const WCHAR* name)
{
  if (!DeviceIoControl (p, PIPE_IOCTL_SET_PIPE_TAG,
			(LPVOID) name, (wcslen (name) + 1) * sizeof (WCHAR),
			NULL, 0, NULL, NULL))
    return FALSE;
  return TRUE;
}

static BOOL
SetPipeName (HANDLE p, WCHAR* name)
{
  if (!DeviceIoControl (p, PIPE_IOCTL_SET_PIPE_NAME,
			(LPVOID) name, (wcslen (name) + 1) * sizeof (WCHAR),
			NULL, 0, NULL, NULL))
    return FALSE;
  return TRUE;
}

PIPELIB_API BOOL
GetPipeName (HANDLE p, WCHAR* name)
{
  DWORD actual;
  if (!DeviceIoControl (p, PIPE_IOCTL_GET_PIPE_NAME,
			NULL, 0,
			(LPVOID)name, MAX_PATH * sizeof (WCHAR),
			&actual, NULL))
    return FALSE;
  return TRUE;
}

PIPELIB_API BOOL
CreatePipe (PHANDLE hReadPipe,
	    PHANDLE hWritePipe,
	    LPSECURITY_ATTRIBUTES /* lpPipeAttributes */,
	    DWORD /* nSize */)
{
  int inst;
  WCHAR* wsKey;
  HANDLE h;
	
  *hReadPipe = NULL;
  *hWritePipe = NULL;

  for (inst = 0 ; inst < MAX_INSTANCES; inst++)
    {
      PrepareRegistryForInstance (inst, &wsKey);
      h = ActivateDevice (wsKey, 0);
      RegDeleteKey (HKEY_LOCAL_MACHINE, wsKey);
      free (wsKey);

      /* Although MSDN documents the error as INVALID_HANDLE_VALUE, I
	 see it returning NULL here.  */
      if (h != INVALID_HANDLE_VALUE && h != NULL)
	break;
    }

  if (inst == MAX_INSTANCES)
    return FALSE;

  /* name + num + ':' + '\0' */
  wchar_t device_name[(sizeof (NAME_BASE) - 1) + 2 + 1 + 1];
  wsprintf (device_name, L"%s%02d:", NAME_BASE, inst);

  *hReadPipe = CreateFile (device_name,
			   GENERIC_READ,
			   FILE_SHARE_READ | FILE_SHARE_WRITE,
			   NULL,
			   OPEN_ALWAYS,
			   FILE_ATTRIBUTE_NORMAL,
			   NULL);

  SetPipeName (*hReadPipe, device_name);

  *hWritePipe = CreateFile (device_name,
			    GENERIC_WRITE,
			    FILE_SHARE_READ | FILE_SHARE_WRITE,
			    NULL,
			    OPEN_ALWAYS,
			    FILE_ATTRIBUTE_NORMAL,
			    NULL);

  return TRUE;
}

#if 0

struct PeekStruct
{
  DWORD Size; /* for future extension */
  PVOID lpBuffer,
    DWORD nBufferSize,

    /* TODO: We need to use MapPtr for this to work */
    LPDWORD lpBytesRead,
    LPDWORD lpTotalBytesAvail,
    LPDWORD lpBytesLeftThisMessage
    };

PIPELIB_API BOOL
PeekNamedPipe (HANDLE hNamedPipe,
	       LPVOID lpBuffer,
	       DWORD nBufferSize,
	       LPDWORD lpBytesRead,
	       LPDWORD lpTotalBytesAvail,
	       LPDWORD lpBytesLeftThisMessage
	       )
{
  DWORD avail;
  DWORD actual;

  PeekStruct data;
  data.Size = sizeof (PeekStruct);
  data.lpBuffer = lpBuffer;
  data.nBufferSize = nBufferSize;
  data.lpBytesRead = lpBytesRead;
  data.lpTotalBytesAvail = lpTotalBytesAvail;
  data.lpBytesLeftThisMessage = lpBytesLeftThisMessage;

  if (!DeviceIoControl (hNamedPipe, PIPE_IOCTRL_PEEK_NAMED_PIPE,
			NULL, 0,
			(LPVOID)&data, sizeof (PeekStruct), &actual, NULL))
    return FALSE;

  /* We can detect here if we are talking to an older driver.  */ 
  if (actual != data.Size)
    return FALSE;

  return FALSE;
}

#endif
