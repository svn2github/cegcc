/*
 * $Header: /cvsroot/wince-xcompile/wince-xcompile/include/aygshell.h,v 1.3 2006/01/12 22:32:20 dannybackx Exp $
 *
 * This file is part of the WinCE X-compiler project.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef	_AYGSHELL_H_INCLUDE_
#define	_AYGSHELL_H_INCLUDE_

#include <windows.h>

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Menu Bar
 */
typedef struct SHMENUBARINFO {
	DWORD		cbSize;
	HWND		hwndParent;
	DWORD		dwFlags;
	UINT		nToolBarId;
	HINSTANCE	hInstRes;
	int		nBmpId,
			cBmpImages;
	HWND		hwndMB;
	COLORREF	clrBk;
} SHMENUBARINFO, *PSHMENUBARINFO;

/* Values for dwFlags */
#define	SHCMBF_EMPTYBAR		0x01
#define	SHCMBF_HIDDEN		0x02
#define	SHCMBF_HIDESIPBUTTON	0x04
#define	SHCMBF_COLORBK		0x08
#define	SHCMBF_HMENU		0x10

extern BOOL SHCreateMenuBar(SHMENUBARINFO *);

/*
 *
 */
typedef struct SHACTIVATEINFO {
	DWORD		cbSize;
	HWND		hwndLastFocus;
	UINT		fSipUp:1;
	UINT		fSipOnDeactivation:1;
	UINT		fActive:1;
	UINT		fReserved:29;
} SHACTIVATEINFO, *PSHACTIVATEINFO;

extern BOOL SHHandleWMSettingChange(HWND, WPARAM, LPARAM, SHACTIVATEINFO *);
extern BOOL SHHandleWMActivate(HWND, WPARAM, LPARAM, SHACTIVATEINFO *, DWORD);

/*
 * Query the SIP state
 */
typedef	struct SIPINFO {
	DWORD		cbSize;
	DWORD		fdwFlags;
	RECT		rcVisibleDesktop;
	RECT		rcSipRect;
	DWORD		dwImDataSize;
	void		*pvImData;
} SIPINFO, *PSIPINFO;

#define	SPI_SETCOMPLETIONINFO	223
#define	SPI_SETSIPINFO		224
#define	SPI_GETSIPINFO		225
#define	SPI_SETCURRENTIM	226
#define	SPI_GETCURRENTIM	227
#define	SPI_APPBUTTONCHANGE	228
#define	SPI_RESERVED		229
#define	SPI_SYNCSETTINGSCHANGE	230

#define	SIPF_OFF	0
#define	SIPF_ON		1
#define	SIPF_DOCKED	2
#define	SIPF_LOCKED	4

extern void SHSipInfo(int, int, PVOID, int);

/*
 * Work with the PocketPC "New" menu.
 */
typedef struct NMNEWMENU {
	NMHDR	hdr;
	TCHAR	szReg[80];
	HMENU	hMenu;
	CLSID	clsId;
} NMNEWMENU, *PNMNEWMENU;

#define	NMN_GETAPPREGKEY	1101
#define	NMN_NEWMENUDESTROY	1102
#define	NMN_INVOKECOMMAND	1103

#define	IDM_NEWMENUMAX		3000

/* Values for npPriority */
typedef enum {
	SHNP_INFORM = 0x1b1,
	SHNP_ICONIC
} SHNP;

/*
 * PocketPC Notifications
 */
typedef struct SHNOTIFICATIONDATA {
	DWORD		cbStruct,
			dwID;
	SHNP		npPriority;
	DWORD		csDuration;
	HICON		hicon;
	DWORD		grfFlags;
	CLSID		clsid;
	HWND		hwndSink;
	LPCTSTR		pszHTML,
			pszTitle;
	LPARAM		lParam;
} SHNOTIFICATIONDATA, *PSHNOTIFICATIONDATA;

extern LRESULT SHNotificationGetData(const CLSID *, DWORD, SHNOTIFICATIONDATA *);
extern LRESULT SHNotificationUpdate(DWORD, SHNOTIFICATIONDATA *);
extern LRESULT SHNotificationRemove(const CLSID *, DWORD);

/* Values for grfFlags */
#define	SHNF_STRAIGHTTOTRAY	1
#define	SHNF_CRITICAL		2
#define	SHNF_FORCEMESSAGE	8
#define	SHNF_DISPLAYON		16
#define	SHNF_SILENT		32

/*
 * Fullscreen applications
 */
extern BOOL SHFullScreen(HWND, DWORD);

/* Values for the second parameter to SHFullScreen */
#define	SHFS_SHOWTASKBAR	1
#define	SHFS_HIDETASKBAR	2
#define	SHFS_SHOWSIPBUTTON	4
#define	SHFS_HIDESIPBUTTON	8
#define	SHFS_SHOWSTARTICON	16
#define	SHFS_HIDESTARTICON	32

#ifdef	__cplusplus
}
#endif

#endif	/* _AYGSHELL_H_INCLUDE_ */
