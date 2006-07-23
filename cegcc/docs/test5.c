/*
 * $Header: /cvsroot/wince-xcompile/wince-xcompile/examples/simplistic/test5.c,v 1.1 2005/12/19 20:02:22 dannybackx Exp $
 *
 * A simple application using Windows Resources.
 *
 * Calling convention of WinMain is still experimental - need to tweak the compiler
 * for this to be more clean.
 */

#undef	DO_LOGGING

#include <stdio.h>
#include <windows.h>
#include <commctrl.h>

#include "test5.h"

char	*fn = "/storage card/devel/log.txt";

static void InitLog(char *pattern, ...);
static void Log(char *pattern, ...);
static void LogError(DWORD e, char *pattern, ...);
LRESULT CALLBACK WndProcedure(HWND, UINT, WPARAM, LPARAM);
void DoMenuActions(HWND w, INT id);

HINSTANCE	hi;
const WCHAR	*WindowCaption = L"Resource Test";

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS	WndCls;
	HWND		hWnd;
	MSG		Msg;

	const WCHAR	*ClsName = L"Resource Test";
	const WCHAR	*WndName = L"Resource Test";

	hi = hInstance;

	InitLog("Initialised, hInstance %p, CmdLine[%s], n %d\r\n", hInstance, lpCmdLine, nCmdShow);

	WndCls.style		= CS_HREDRAW | CS_VREDRAW;
	WndCls.lpfnWndProc	= WndProcedure;
	WndCls.cbClsExtra	= 0;
	WndCls.cbWndExtra	= 0;
	WndCls.hIcon		= NULL;
	WndCls.hCursor		= NULL;
	WndCls.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndCls.lpszMenuName	= NULL;
	WndCls.lpszClassName	= ClsName;
	WndCls.hInstance	= hInstance;

	Log("Before RegisterClass\r\n");
	RegisterClass(&WndCls);
	LogError(GetLastError(), "After RegisterClass\r\n");

	hWnd = CreateWindow(ClsName, WndName, WS_OVERLAPPEDWINDOW,
			0, 0,
			CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL,
			hInstance,
			NULL);

	LogError(GetLastError(), "After CreateWindow\r\n");

	if (! hWnd)
		return FALSE;

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	while (GetMessageW(&Msg, NULL, 0, 0)) {
		TranslateMessage(&Msg);
		DispatchMessageW(&Msg);
	}

	return Msg.wParam;

	exit(0);
}

LRESULT CALLBACK WndProcedure(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT	ps;
	RECT		rClient;
	HDC		hdc;
	HWND		bar;

	switch (Msg) {
		case WM_CREATE:
			bar = CommandBar_Create(hi, hWnd, 1);
			CommandBar_InsertMenubar(bar, hi, IDR_MAINMENU, 0);
			CommandBar_AddAdornments(bar, 0, 0);
			break;
		case WM_SHOWWINDOW:
			break;
		case WM_DESTROY:
			PostQuitMessage(WM_QUIT);
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rClient);
			Rectangle(ps.hdc,
					rClient.left + 10, rClient.top + 10,
					rClient.right - 10, rClient.bottom - 10);
			EndPaint(hWnd, &ps);
			break;
		case WM_COMMAND:
			/* Handle the menu items */
			DoMenuActions(hWnd, LOWORD(wParam));
			break;
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

void DoMenuActions(HWND w, INT id)
{
	HDC	hdc;
	POINT	p[2];
	int	y;

	switch (id) {
		case IDM_MENU_EXIT:
			PostQuitMessage(0);
			break;
		case ID_MENU_TEXT:
			break;
		case ID_MENU_LINE:
			hdc = GetDC(w);
			p[0].x = 120;
			p[1].x = 200;
			for (y = 40; y < 200; y += 5) {
				p[0].y = 200 - y;
				p[1].y = y;
				Polyline(hdc, p, 2);
			}
			ReleaseDC(w, hdc);
			break;
	}
}

static void InitLog(char *pattern, ...)
{
#ifdef	DO_LOGGING
	FILE	*f;
	va_list	ap;
	DWORD	len, written;

	f = fopen(fn, "w");
	va_start(ap, pattern);
	vfprintf(f, pattern, ap);
	va_end(ap);
	fclose(f);
#endif
}

static void Log(char *pattern, ...)
{
#ifdef	DO_LOGGING
	FILE	*f;
	va_list	ap;
	DWORD	len, written;

	f = fopen(fn, "a+");
	va_start(ap, pattern);
	vfprintf(f, pattern, ap);
	va_end(ap);
	fclose(f);
#endif
}

static void LogError(DWORD e, char *pattern, ...)
{
#ifdef	DO_LOGGING
	FILE	*f;
	va_list	ap;
	DWORD	len, written;

	char	s[128];
	LPVOID	buf;

	FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		e,
		0,
		(LPTSTR)&buf,
		0,
		NULL);

	f = fopen(fn, "a+");
	va_start(ap, pattern);
	vfprintf(f, pattern, ap);
	(void)wcstombs(s, buf, 128);
	fprintf(f, "Error [%s]\r\n", s);
	va_end(ap);
	fclose(f);

	LocalFree(buf);
#endif
}
