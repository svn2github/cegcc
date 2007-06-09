/* Copyright (c) 2007, Pedro Alves
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include <windows.h>
#include <commctrl.h>

#include <PipeLib.h>

extern "C" BOOL GetStdioPathW(int, wchar_t*, DWORD*);
extern "C" BOOL SetStdioPathW(int, const wchar_t*);

volatile BOOL stop = 0;

static HANDLE readh[3];
static HANDLE writeh[3];

static DWORD WINAPI
stdin_thread (void*)
{
  /* We don't need the reading side.  */
  CloseHandle (readh[0]);
  readh[0] = INVALID_HANDLE_VALUE;

  while (!stop)
    {
      DWORD read = 0;
      char buf[1];

      if (!ReadFile ((HANDLE) fileno (stdin), buf, sizeof (buf), &read, FALSE))
	{
	  OutputDebugStringW (L"stdin: broken pipe\n");
	  break;
	}

      if (read)
	{
	  DWORD written = 0;
	  WriteFile (writeh[0], buf, read, &written, NULL);
	}
    }

  OutputDebugStringW(L"stdin thread gone\n");
  return 0;
}

static DWORD WINAPI
stdout_thread (void*)
{
  /* We can't close the write side of the pipe until the
     child opens its version.  Since it will only be open on the
     first stdout access, we have to wait until the read side returns
     something - which means the child opened stdout.  */

  while (!stop)
    {
      DWORD read = 0;
      char buf[1];
      if (!ReadFile (readh[1], buf, sizeof (buf), &read, FALSE))
	{
	  OutputDebugStringW (L"stout: broken pipe\n");
	  break;
	}
      else if (writeh[1] != INVALID_HANDLE_VALUE)
	{
	  /* We can now close our wr side.  */
	  CloseHandle (writeh[1]);
	  writeh[1] = INVALID_HANDLE_VALUE;

	}
      if (read)
	{
	  DWORD written = 0;
	  WriteFile ((HANDLE) fileno (stdout), buf, read, &written, NULL);
	}
    }
  OutputDebugStringW(L"stdout thread gone\n");
  return 0;
}


void
create_inferior (const wchar_t *command, wchar_t** /* child_argv */)
{
  PROCESS_INFORMATION processInfo;
  wchar_t prev_path[3][MAX_PATH];
  BOOL bRet;

  for (size_t i = 0; i < 3; i++)
    {
      wchar_t devname[MAX_PATH];
      if (!CreatePipe (&readh[i], &writeh[i], NULL, 0))
	return;

#if 0
      CloseHandle (readh[i]);
      CloseHandle (writeh[i]);
      continue;
#endif

      GetPipeName (readh[i], devname);
      DWORD dwLen = MAX_PATH;
      GetStdioPathW (i, prev_path[i], &dwLen);
      SetStdioPathW (i, devname);
    }

  bRet = CreateProcess (command, L"", NULL, NULL, FALSE, 0,
			NULL, NULL, NULL, &processInfo);

  if (!bRet)
    return;

  for (size_t i = 0; i < 3; i++)
    {
      SetStdioPathW (i, prev_path[i]);
    }

  HANDLE h[3];
  h[0] = CreateThread (NULL, 0, stdin_thread, NULL, 0, NULL);
  h[1] = CreateThread (NULL, 0, stdout_thread, NULL, 0, NULL);
  h[2] = processInfo.hProcess;

  switch (WaitForMultipleObjects (sizeof (h) / sizeof (h[0]), h,
				  FALSE, INFINITE))
    {
    case WAIT_OBJECT_0:
      stop = 1;
      TerminateProcess (processInfo.hProcess, 0);
      break;
    case WAIT_OBJECT_0 + 1:
      stop = 1;
      TerminateProcess (processInfo.hProcess, 0);
      break;
    case WAIT_OBJECT_0 + 2:
      stop = 1;

      //		CloseHandle ((HANDLE) fileno (stdin));
      //		TerminateThread (h[0], 0);
      //		TerminateThread (h[1], 0);
      break;
    default:
      break;
    }

  CloseHandle (h[0]);
  CloseHandle (h[1]);
  CloseHandle (processInfo.hProcess);
  CloseHandle (processInfo.hThread);
  for (int i = 0; i < 3; i++)
    {
      CloseHandle (writeh[i]);
    }

  for (int i = 0; i < 3; i++)
    {
      CloseHandle (readh[i]);
    }
  return;
}

int WINAPI
WinMain (HINSTANCE /*hInstance*/,
	 HINSTANCE /*hPrevInstance*/,
	 LPTSTR    /*lpCmdLine*/,
	 int       /*nCmdShow*/)
{
  wchar_t* child_argv[3] = {0};

  create_inferior (L"TestClient.exe", child_argv);
  return 0;
}
