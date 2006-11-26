#ifndef	_AYGSHELL_H
#define	_AYGSHELL_H

#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#if _WIN32_WCE >= 400

#include <windows.h>
#include <basetyps.h>	/* Make sure we have a CLSID definition */
#include <shellapi.h>	/* for WINSHELLAPI */

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Menu Bar
 */
typedef struct tagSHMENUBARINFO {
	DWORD		cbSize;
	HWND		hwndParent;
	DWORD		dwFlags;
	UINT		nToolBarId;
	HINSTANCE	hInstRes;
	int nBmpId;
	int cBmpImages;
	HWND		hwndMB;
	COLORREF	clrBk;
} SHMENUBARINFO, *PSHMENUBARINFO;

/* Values for dwFlags */
#define	SHCMBF_EMPTYBAR		0x01
#define	SHCMBF_HIDDEN		0x02
#define	SHCMBF_HIDESIPBUTTON	0x04
#define	SHCMBF_COLORBK		0x08
#define	SHCMBF_HMENU		0x10

typedef struct tagSHACTIVATEINFO {
	DWORD		cbSize;
	HWND		hwndLastFocus;
	UINT		fSipUp:1;
	UINT		fSipOnDeactivation:1;
	UINT		fActive:1;
	UINT		fReserved:29;
} SHACTIVATEINFO, *PSHACTIVATEINFO;

WINSHELLAPI BOOL WINAPI SHCreateMenuBar(SHMENUBARINFO *);
WINSHELLAPI HWND WINAPI SHFindMenuBar(HWND);
WINSHELLAPI HRESULT WINAPI SHCreateNewItem(HWND,REFCLSID);
WINSHELLAPI BOOL WINAPI SHFullScreen(HWND,DWORD);
WINSHELLAPI BOOL WINAPI SHSipInfo(UINT,UINT,PVOID,UINT);
/* next exported by ordinal only: @84 */
WINSHELLAPI BOOL WINAPI SHHandleWMActivate(HWND,WPARAM,LPARAM,SHACTIVATEINFO*,DWORD);
/* next exported by ordinal only: @83 */
WINSHELLAPI BOOL WINAPI SHHandleWMSettingChange(HWND,WPARAM,LPARAM,SHACTIVATEINFO*);

/* The following are not in device ROMs. */
extern BOOL SHInvokeContextMenuCommand(HWND,UINT,HANDLE);

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

/*
 * SIPPREF appears to be some magic control to automatically display the SIP.
 * Use with
 *	CONTROL  "",-1,WC_SIPPREF, NOT WS_VISIBLE,-10,-10,5,5
 * See http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wceui40/html/_cerefwc_sippref.asp
 */
#define	WC_SIPPREF	L"SIPPREF"

/*
 * Stuff for SHRecognizeGesture
 *
 * See
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wceui40/html/_cerefshrginfo.asp
 * and
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wceui40/html/_cerefshrecognizegesture.asp
 */
#if (_WIN32_WCE >= 0x0420)
typedef struct tagSHRGI {
	DWORD	cbSize;
	HWND	hwndClient;
	POINT	ptDown;
	DWORD	dwFlags;
} SHRGINFO, *PSHRGINFO;

WINSHELLAPI DWORD SHRecognizeGesture(SHRGINFO *shrg);
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* _WIN32_WCE >= 400 */

#endif	/* _AYGSHELL_H */
