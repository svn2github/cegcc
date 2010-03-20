/* Windows CE GUI for gdbserver

   Copyright (C) 2010 Free Software Foundation, Inc.
   Written by Danny Backx.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <aygshell.h>
#include "wgdbs.h"

#include "server.h"
#include <ctype.h>

HINSTANCE	MainInstance = NULL;
HWND		MainWindow = 0, CmdBar = NULL, list = NULL, Status = NULL;
HWND		button1 = NULL, button2 = NULL, button3 = NULL;
HWND		tf1 = NULL, tf2 = NULL, tf3 = NULL;
RECT		rect, cbrect;
DWORD		font;
int		wx, wy, ww, wh;

#define	WPORTLEN	64
#define	WEXELEN		64
#define	WARGSLEN	64

wchar_t	wport[WPORTLEN], wexe[WEXELEN], wargs[WARGSLEN];
// char	port[WPORTLEN];
char	exe[WEXELEN], args[WARGSLEN];

static int	thread_started = 0;
HANDLE		gdbsthread = 0;

void gui_statusw(const wchar_t *s)
{
    SendMessage(Status, WM_SETTEXT, 0, (LPARAM)s);
}

void gui_status(const char *s)
{
	wchar_t	*wcs = NULL;
	int l = strlen(s);
	wcs = (wchar_t *)malloc(2 * (l+1));	/* WinCE wchar_t has size 2 */
	mbstowcs(wcs, s, l+1);
	gui_statusw(wcs);
	free((void *)wcs);
}

DWORD WINAPI thread_gdbserver_main(void *param)
{
	gdbserver_initialize();
	gdbserver_start_inferior();
	gdbserver_main();
	return 0;
}

DWORD WINAPI thread_gdbserver_multi(void *param)
{
	gdbserver_initialize();
	multi_mode = 1;
	gdbserver_main();
	return 0;
}

void gui_exit (const char *msg)
{
	if (gdbsthread) {
		TerminateThread(gdbsthread, (DWORD)0);
		gdbsthread = 0;
	}
	kill_inferior();
}


#define	WCS_COPYRIGHT_STRING	\
	L"Copyright (c) 2007, 2008, 2010 by Danny Backx\r\n" \
	"danny@backx.info"

TCHAR szAppName[ ] = TEXT("gdbserver");
TCHAR szTitle[ ]   = TEXT("gdbserver");

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
VOID MenuHandler(HWND hWnd, INT idval);

SHACTIVATEINFO	sai;

#define	WNAMELEN	64
TCHAR		wname[WNAMELEN];
char		name[WNAMELEN];

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	MSG		msg;
	WNDCLASS	wc;

	wc.style          = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc    = (WNDPROC) WndProc;
	wc.cbClsExtra     = 0;
	wc.cbWndExtra     = 0;
	wc.hInstance      = hInstance;
	wc.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(GDB_ICON));
	wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground  = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName   = NULL;
	wc.lpszClassName  = szAppName;

	RegisterClass(&wc);

	MainInstance = hInstance;      // Save handle to create command bar

	/* If we're already running, don't start a second process. */
	MainWindow = FindWindow(szAppName, szTitle);
	if (MainWindow) {
		SetForegroundWindow((HWND) ((ULONG)MainWindow | 0x0001));
		return 0;
	}

	InitCommonControls();	// Initialize common controls - command bar
	SHInitExtraControls();	// Initialize SIPPREF

	/* Be able to resize with the SIP */
	memset(&sai, 0, sizeof(sai));
	sai.cbSize = sizeof(sai);

	MainWindow = CreateWindow(szAppName,	// Class
			szTitle,		// Title
			WS_VISIBLE,		// Style
			CW_USEDEFAULT,		// x-position
			CW_USEDEFAULT,		// y-position
			CW_USEDEFAULT,		// x-size
			CW_USEDEFAULT,		// y-size
			NULL,			// Parent handle
			NULL,			// Menu handle
			hInstance,		// Instance handle
			NULL);			// Creation
	if (! MainWindow)
		return 0;

	/*
	 * Find out some things
	 */
	GetClientRect(MainWindow, &rect);
	font = (DWORD) GetStockObject(DEFAULT_GUI_FONT);

	if (! port) {
		port = malloc(WPORTLEN);
	}

	/* Compute sizes for areas covered by windows. */
	wx = rect.left;
	wy = rect.top + 26;
	ww = rect.right - rect.left;
	wh = rect.bottom - rect.top;

	if (port == NULL) {
		port = malloc(WPORTLEN);
	}

	/*
	 * Create stuff in the window
	 */
	tf1 = CreateWindow(L"EDIT", L"/temp/wm61/main.exe",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			wx + 10, wy, ww - 50, 20,
			MainWindow, NULL,
			MainInstance,
			NULL);
	SendMessage(tf1, WM_SETFONT, font, TRUE);

	tf2 = CreateWindow(L"EDIT", L":9999",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			wx + 10, wy + 26, ww - 50, 20,
			MainWindow, NULL,
			MainInstance,
			NULL);
	SendMessage(tf2, WM_SETFONT, font, TRUE);

	tf3 = CreateWindow(L"EDIT", L"a b c",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			wx + 10, wy + 52, ww - 50, 20,
			MainWindow, NULL,
			MainInstance,
			NULL);
	SendMessage(tf3, WM_SETFONT, font, TRUE);

	button1 = CreateWindow(L"BUTTON", L"Start",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,
			ww - 40, wy, 30, 20,
			MainWindow, NULL,
			MainInstance,
			NULL);
	SendMessage(button1, WM_SETFONT, font, TRUE);

	button2 = CreateWindow(L"BUTTON", L"Stop",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,
			ww - 40, wy + 26, 30, 20,
			MainWindow, NULL,
			MainInstance,
			NULL);
	SendMessage(button2, WM_SETFONT, font, TRUE);

	button3 = CreateWindow(L"BUTTON", L"Start multi",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,
			ww - 80, wy + 100, 70, 20,
			MainWindow, NULL,
			MainInstance,
			NULL);
	SendMessage(button3, WM_SETFONT, font, TRUE);

	Status = CreateWindow(L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			wx + 10, wy + 74, ww - 50, 20,
			MainWindow, NULL,
			MainInstance,
			NULL);
	SendMessage(Status, WM_SETFONT, font, TRUE);

	CmdBar = CommandBar_Create(MainInstance, MainWindow, 1);
	CommandBar_InsertMenubar(CmdBar, MainInstance, IDR_MAINMENU, 0);
	CommandBar_AddAdornments(CmdBar, 0, 0);

	/*
	 * Show what you've got
	 */
	SetFocus(tf1);
	ShowWindow(MainWindow, SW_SHOW);
	UpdateWindow(MainWindow);

	/* Event loop */
	while (GetMessage(&msg, NULL, 0, 0) != FALSE) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return(msg.wParam);
}

VOID MenuHandler(HWND hWnd, INT idval)
{
	switch (idval) {	
	case IDM_FILE_EXIT:
		PostQuitMessage(0);
		break;
	case IDM_ABOUT:
		MessageBoxW(MainWindow, WCS_COPYRIGHT_STRING, L"About", MB_OK);
		break;
	default:
		break;
	}
}

static void ButtonHandlerStart(void)
{
	int	r, i;
	char	*p, *q;
	DWORD	tid;

	if (thread_started || gdbsthread)
		return;

	thread_started++;

	r = GetWindowText(tf1, wexe, WEXELEN);
	r = GetWindowText(tf2, wport, WPORTLEN);
	r = GetWindowText(tf3, wargs, WARGSLEN);

	wcstombs(exe, wexe, wcslen(wexe)+1);
	wcstombs(port, wport, wcslen(wport)+1);
	wcstombs(args, wargs, wcslen(wargs)+1);

	printf("Exe [%s] args [%s] port [%s]\n", exe, args, port);

	strcat(args, " ");

	for (i=0, r=3; args[i]; i++)
		if (isspace(args[i])) r++;

	program_argv = malloc(r * sizeof(char *));
	program_argv[0] = exe;
	for (i=1, p=q=args; *p; p++) {
		if (q < p && isspace(*p)) {
			program_argv[i] = malloc(p-q+2);
			strncpy(program_argv[i], q, p-q);
			program_argv[i][p-q] = '\0';

			printf("Arg %d [%s]\n", i, program_argv[i]);
			i++;
			q = p+1;
		}
		program_argv[i] = 0;
	}

	gdbsthread = CreateThread(NULL, 0, thread_gdbserver_main, NULL, 0, &tid);
}

static void ButtonHandlerStartMulti(void)
{
	int	r;
	DWORD	tid;

	if (thread_started || gdbsthread)
		return;

	thread_started++;

	if (port == NULL) {
		port = (char *)malloc(WPORTLEN+1);
	}

	r = GetWindowText(tf2, wport, WPORTLEN);

	wcstombs(port, wport, wcslen(wport)+1);

	strcat(args, " ");

	multi_mode = 1;
	gdbsthread = CreateThread(NULL, 0, thread_gdbserver_multi, NULL, 0, &tid);
}

static void ButtonHandlerStop(void)
{
	kill_inferior();
	exit(0);
}

static void Paint(HWND h, UINT msg, WPARAM w, LPARAM l)
{
	PAINTSTRUCT	ps;
	HPEN	hOld;
	RECT	rect;
	HBRUSH	brush;
	HDC	hdc;

	hdc = BeginPaint (h, &ps);
	GetClientRect (h, &rect);

//	hPen = CreatePen (PS_SOLID, 1, RGB (200, 200, 200));
	brush = CreateSolidBrush (RGB (200, 200, 200));
	hOld = (HPEN)SelectObject (hdc, brush);
	Rectangle (hdc, rect.left, rect.top, rect.right, rect.bottom);
	SelectObject (hdc, hOld);
	DeleteObject (brush);

	EndPaint (h, &ps);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_PAINT:
		Paint(hWnd, message, wParam, lParam);
		break;
	case WM_COMMAND:
		if ((HWND)lParam == button1) {
			ButtonHandlerStart();
			return 0;
		}
		if ((HWND)lParam == button2) {
			ButtonHandlerStop();
			return 0;
		}
		if ((HWND)lParam == button3) {
			ButtonHandlerStartMulti();
			return 0;
		}

		MenuHandler(hWnd, LOWORD(wParam));
		break;

	case WM_HIBERNATE:
		MessageBox(NULL, TEXT("MEMORY IS LOW"), szAppName, MB_OK);
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_ACTIVATE:
		SHHandleWMActivate(hWnd, wParam, lParam, &sai, 0);
		break;

	case WM_SETFOCUS:
		if (hWnd != tf1) {
			SetFocus(tf1);
			SHSipPreference(MainWindow, 0 /* SIP_UP */);
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
