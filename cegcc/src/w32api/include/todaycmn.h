#ifndef _TODAYCMN_H_
#define _TODAYCMN_H_

#if __GNUC__ >=3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* List item types */
typedef enum _TODAYLISTITEMTYPE
{
	tlitOwnerInfo = 0,
	tlitAppointments,
	tlitMail,
	tlitTasks,
	tlitCustom,
	tlitNil          /* sentinel */
} TODAYLISTITEMTYPE;

#define MAX_ITEMNAME    32

/* Information for a single today item */
typedef struct _TODAYLISTITEM
{
	TCHAR               szName[MAX_ITEMNAME];
	TODAYLISTITEMTYPE   tlit;
	DWORD               dwOrder;
	DWORD               cyp;
	BOOL                fEnabled;
	BOOL                fOptions;
	DWORD               grfFlags;
	TCHAR               szDLLPath[MAX_PATH];
	HINSTANCE           hinstDLL;
	HWND                hwndCustom;
	BOOL                fSizeOnDraw;
	BYTE                *prgbCachedData;
	DWORD               cbCachedData;
} TODAYLISTITEM;

/* Maximum number of today items */
#define k_cTodayItemsMax 12

/* Custom DLL resources */
#define IDI_ICON                128
#define IDD_TODAY_CUSTOM        500

/* Custom DLL functions */
#define ORDINAL_INITIALIZEITEM      240
typedef HWND (*PFNCUSTOMINITIALIZEITEM)(TODAYLISTITEM *, HWND);
#define ORDINAL_OPTIONSDIALOGPROC   241
typedef BOOL (*PFNCUSTOMOPTIONSDLGPROC)(HWND, UINT, UINT, LONG);

/* Custom DLL messages */
#define WM_TODAYCUSTOM_CLEARCACHE           (WM_USER + 242)
#define WM_TODAYCUSTOM_QUERYREFRESHCACHE    (WM_USER + 243)
#define WM_TODAYCUSTOM_RECEIVEDSELECTION    (WM_USER + 244)
#define WM_TODAYCUSTOM_LOSTSELECTION        (WM_USER + 245)
#define WM_TODAYCUSTOM_USERNAVIGATION       (WM_USER + 246)
#define WM_TODAYCUSTOM_ACTION               (WM_USER + 247)

/* Messages to parent window */
#define TODAYM_DRAWWATERMARK                (WM_USER + 101)
#define TODAYM_TOOKSELECTION                (WM_USER + 102)

/* Today screen system colors */
#define TODAYCOLOR_HIGHLIGHT                (0x10000022)
#define TODAYCOLOR_HIGHLIGHTEDTEXT          (0x10000023)

/* Watermark drawing info */
typedef struct {
	HDC hdc;
	RECT rc;
	HWND hwnd;
} TODAYDRAWWATERMARKINFO;

#ifdef __cplusplus
}
#endif

#endif  /* !_TODAYCMN_H_ */
