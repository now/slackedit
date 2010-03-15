
/*****************************************************************
 *                        REVISION LOG ENTRY                     *
 *****************************************************************
 * Project    : SmartFTP
 *
 * File       : Band_Control.h
 * Revision By: Mike Walter
 * Revised on : 12/01/99 // 06.Mai.1998 18:03:20
 * Comments   : 
 *
 *****************************************************************/

#ifndef __BAND_CONTROL_H
#define __BAND_CONTROL_H

#include "pcp_controls.h"

#define PUT_WORD(ptr,w)   (*(WORD  *)(ptr) = (w))
#define GET_WORD(ptr)     (*(WORD  *)(ptr))
#define PUT_DWORD(ptr,dw) (*(DWORD *)(ptr) = (dw))
#define GET_DWORD(ptr)    (*(DWORD *)(ptr))

FOREXPORT BOOL Band_RegisterControl(HMODULE hModule);
FOREXPORT void Band_UnRegisterControl(HMODULE hModule);

#define WC_BANDCONTROL     "Ctrl_Band32"

#define BCM_SETDATA				(WM_USER + 300)
#define BCM_GETDATA				(WM_USER + 301)
#define BCM_SETIMAGELIST		(WM_USER + 302)
#define BCM_GETIMAGELIST		(WM_USER + 303)
#define BCM_SETTITLEIMAGE		(WM_USER + 304)
#define BCM_GETTITLEIMAGE		(WM_USER + 305)
#define BCM_SETCLIENTWND		(WM_USER + 306)
#define BCM_GETCLIENTWND		(WM_USER + 307)
#define BCM_GETCLIENTRECT		(WM_USER + 308)
#define BCM_GETALIGNMENT		(WM_USER + 309)
#define BCM_SETALIGNMENT		(WM_USER + 310)
#define BCM_SETMAXSIZE			(WM_USER + 311)
#define BCM_GETMAXSIZE			(WM_USER + 312)
#define BCM_SETDLGTEMPLATE		(WM_USER + 313)
#define BCM_SETALLOWABLEALIGNS	(WM_USER + 314)
#define BCM_GETALLOWABLEALIGNS	(WM_USER + 315)

#define BandCtrl_SetData(hwndCtl, lpData)						((BOOL)SNDMSG((hwndCtl), BCM_SETDATA, 0, (LPARAM)(lpData)))
#define BandCtrl_GetData(hwndCtl)								((LPARAM)SNDMSG((hwndCtl), BCM_GETDATA, 0, 0L))
#define BandCtrl_SetImageList(hwndCtl, himl)					((HIMAGELIST)SNDMSG((hwndCtl), BCM_SETIMAGELIST, 0, (LPARAM)(HIMAGELIST)(himl)))
#define BandCtrl_GetImageList(hwdnCtl)							((HIMAGELIST)SNDMSG((hwndCtl), BCM_GETIMAGELIST, 0, 0L))
#define BandCtrl_SetTitleImage(hwndCtl, nIndex)					((INT)SNDMSG((hwndCtl), BCM_SETTITLEIMAGE, (WPARAM)(INT)(nIndex), 0L))
#define BandCtrl_GetTitleImage(hwndCtl)							((INT)SNDMSG((hwndCtl), BCM_GETTITLEIMAGE, 0, 0L))
#define BandCtrl_SetClientWindow(hwndCtl, hwnd)					((BOOL)SNDMSG((hwndCtl), BCM_SETCLIENTWND, 0, (LPARAM)(HWND)(hwnd)))
#define BandCtrl_GetClientWindow(hwndCtl)						((HWND)SNDMSG((hwndCtl), BCM_GETCLIENTWND, 0, 0L))
#define BandCtrl_GetClientRect(hwndCtl, lprc)					((BOOL)SNDMSG((hwndCtl), BCM_GETCLIENTRECT, 0, (LPARAM)(LPRECT)(lprc)))
#define BandCtrl_GetAlignment(hwndCtl)							((INT)SNDMSG((hwndCtl), BCM_GETALIGNMENT, 0, 0L))
#define BandCtrl_SetAlignment(hwndCtl, nAlign)					((VOID)SNDMSG((hwndCtl), BCM_SETALIGNMENT, (WPARAM)(INT)(nAlign), 0L))
#define BandCtrl_SetMaxSize(hwndCtl, cx, cy)					((INT)SNDMSG((hwndCtl), BCM_SETMAXSIZE, 0, (LPARAM)MAKELPARAM(cx, cy)))
#define BandCtrl_GetMaxSize(hwndCtl, lppt)						((VOID)SNDMSG((hwndCtl), BCM_GETMAXSIZE, 0, (LPARAM)(LPPOINT)(lppt)))
#define BandCtrl_SetDlgTemplate(hwndCtl, resource, instance)	((VOID)SNDMSG((hwndCtl), BCM_SETDLGTEMPLATE, (WPARAM)(INT)(resource), (LPARAM)(HINSTANCE)(instance)))
#define BandCtrl_SetAllowableAligns(hwndCtl, dwAligns)			((VOID)SNDMSG((hwndCtl), BCM_SETALLOWABLEALIGNS, 0, (LPARAM)(DWORD)(dwAligns)))
#define BandCtrl_GetAllowableAligns(hwndCtl)					((DWORD)SNDMSG((hwndCtl), BCM_GETALLOWABLEALIGNS, 0, 0L))

#define BCS_PAGER			0x00008000L
#define BCS_FLOAT			0x00004000L
#define BCS_LEFT			0x00002000L
#define BCS_RIGHT			0x00001000L
#define BCS_TOP				0x00000800L
#define BCS_BOTTOM			0x00000400L
#define BCS_FIXED			0x00000200L
#define BCS_FORWARDCOMMAND	0x00000100L
#define BCS_FORWARDNOTIFY	0x00000080L
#define BCS_NOTIFYSIZE		0x00000040L

#define BCN_DROP			0x00000001L
#define BCN_BANDCLOSE		0x00000002L
#define BCN_STYLECHANGED	0x00000003L
#define BCN_CTRLCOMMAND		0x00000004L
#define BCN_CTRLNOTIFY		0x00000005L
#define BCN_POSCHANGED		0x00000006L
#define BCN_GETCLIENTRECT	0x00000007L

typedef struct tagBCNM
{
	NMHDR	hdr;
	RECT	rcClient;
	BOOL	bVisible;
	HWND	hwndClient;
	LPARAM	lParam;
	WPARAM	wParam;
	DWORD	dwNewStyle;
	DWORD	dwOldStyle;
} BCNM, FAR *LPBCNM;

#endif /* __BAND_CONTROL_H */
