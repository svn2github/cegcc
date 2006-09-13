#include <windows.h>
#include <commctrl.h>
#include "draw.h"

TCHAR szTitle[ ]   = TEXT("Draw Demo");

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
VOID MenuHandler(HWND hWnd, INT idval);

HINSTANCE  hInst = NULL;
HWND       hWndCB = NULL;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	HWND         hWnd;
	MSG          msg;
	WNDCLASS     wc;

	wc.style          = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc    = (WNDPROC) WndProc;
	wc.cbClsExtra     = 0;
	wc.cbWndExtra     = 0;
	wc.hInstance      = hInstance;
	wc.hIcon          = NULL;
	wc.hCursor        = NULL;
	wc.hbrBackground  = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName   = NULL;
	wc.lpszClassName  = szTitle;

	RegisterClass(&wc);

	hInst = hInstance;
	InitCommonControls();

	hWnd = CreateWindow(szTitle, szTitle, WS_VISIBLE,
			0, 0,
			CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, hInstance, NULL);
	if (! hWnd)
		return 0;

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	while ( GetMessage(&msg, NULL, 0, 0) != FALSE ) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return(msg.wParam);
}

VOID MenuHandler(HWND hWnd, INT idval)
{
	HDC		hDC;
	POINT		pPnt[2];
	int		x;

	switch (idval) {
		case ACTION_MENU_EXIT:
			PostQuitMessage(0);
			break;
		case ACTION_MENU_DRAW:
			hDC = GetDC(hWnd);
			for (x=0; x<=150; x += 5) {
				pPnt[0].x = 50; pPnt[0].y = 100 + x;
				pPnt[1].x = 200; pPnt[1].y = 150 - x;
				Polyline(hDC, pPnt, 2);
			}
			ReleaseDC(hWnd, hDC);
			break;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM uParam, LPARAM lParam)
{
	HDC		hdc;
	RECT		rClient;
	PAINTSTRUCT	ps;

	switch (message) {
		case WM_COMMAND:
			MenuHandler(hWnd, LOWORD(uParam));
			break;

		case WM_CREATE:
			hWndCB = CommandBar_Create(hInst, hWnd, 1);
			CommandBar_InsertMenubar(hWndCB, hInst, ACTION_MAIN_MENU, 0);
			CommandBar_AddAdornments(hWndCB, 0, 0);
			break;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rClient);
			rClient.top += CommandBar_Height (hWndCB);

			Rectangle(ps.hdc,
					rClient.left + 10,  rClient.top + 10,
					rClient.right - 10, rClient.bottom - 10);
			EndPaint(hWnd, &ps);
			break;

		case WM_HIBERNATE:
			MessageBox(NULL, TEXT("MEMORY IS LOW"), szTitle, MB_OK);
			break;

		case WM_SHOWWINDOW:
			break;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, uParam, lParam);
	}
	return 0;
}
