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

#ifndef __PIPEDEVICE_H__
#define __PIPEDEVICE_H__

#include <windows.h>
#include <winioctl.h>

#define PIPE_IOCTL_SET_PIPE_NAME \
	CTL_CODE(FILE_DEVICE_STREAMS, 2048, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define PIPE_IOCTL_GET_PIPE_NAME \
	CTL_CODE(FILE_DEVICE_STREAMS, 2049, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define PIPE_IOCTL_SET_PIPE_TAG \
	CTL_CODE(FILE_DEVICE_STREAMS, 2050, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define PIPE_IOCTL_PEEK_NAMED_PIPE \
	CTL_CODE(FILE_DEVICE_STREAMS, 2051, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define DEVICE_DLL_NAME L"PipeDev.dll"

struct PeekStruct
{
    DWORD dwSize; /* for future extension */
    LPVOID lpBuffer;
    DWORD nBufferSize;
    LPDWORD lpBytesRead;
    LPDWORD lpTotalBytesAvail;
    LPDWORD lpBytesLeftThisMessage;
};

#endif
