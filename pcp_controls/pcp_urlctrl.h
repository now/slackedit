/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_urlctrl.h
 * Created    : not known (before 12/01/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:35:49
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __URLCTRL_H
#define __URLCTRL_H

#include "pcp_controls.h"

#define URLCTRL_CLASS _T("URLCtrl_Class")

#define US_LEFT		0x00000001L
#define US_CENTER	0x00000002L
#define US_RIGHT	0x00000004L

#define UB_DEFAULTMAIL	0xFFFFFFFF

#define UCM_SETURL			(WM_USER + 700)
#define UCM_GETURL			(WM_USER + 701)
#define UCM_SETDESCRIPTION	(WM_USER + 702)
#define UCM_GETDESCRIPTION	(WM_USER + 703)
#define UCM_SETCOLORS		(WM_USER + 704)
#define UCM_SETBITMAP		(WM_USER + 705)

#define UrlCtrl_SetUrl(hwndCtl, pszUrl)					((DWORD)SNDMSG((hwndCtl), UCM_SETURL, 0, (LPARAM)(pszUrl)))
#define UrlCtrl_GetUrl(hwndCtl, size, pszUrl)			((DWORD)SNDMSG((hwndCtl), UCM_GETURL, (WPARAM)(size), (LPARAM)(pszUrl)))
#define UrlCtrl_SetDescription(hwndCtl, pszDesc)		((DWORD)SNDMSG((hwndCtl), UCM_SETDESCRIPTION, 0, (LPARAM)(pszDesc)))
#define UrlCtrl_GetDescription(hwndCtl, size, pszDesc)	((DWORD)SNDMSG((hwndCtl), UCM_GETDESCRIPTION, (WPARAM)(size), (LPARAM)(pszDesc)))
#define UrlCtrl_SetColors(hwndCtl, lpcc)				((DWORD)SNDMSG((hwndCtl), UCM_SETCOLORS, 0, (LPARAM)(lpcc)))
#define UrlCtrl_SetBitmap(hwndCtl, bmp)					((DWORD)SNDMSG((hwndCtl), UCM_SETBITMAP, 0, (LPARAM)(bmp)))

typedef struct tagURLCTRLCOLORS
{
	COLORREF crNormal;
	COLORREF crOver;
	COLORREF crDown;
} URLCTRLCOLORS, *LPURLCTRLCOLORS;

FOREXPORT BOOL UrlCtrl_RegisterControl(HMODULE hModule);
FOREXPORT void UrlCtrl_UnregisterControl(HMODULE hModule);

#endif /* __URLCTRL_H */
