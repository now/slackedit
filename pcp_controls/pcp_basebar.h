/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_basebar.h
 * Created    : 01/23/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:46:05
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef _SMARTFTP_BASEBAR_H_
#define _SMARTFTP_BASEBAR_H_

#include "pcp_controls.h"

#define WC_BASEBAR			_T("BaseBarCtrl")

// Control Messages
#define CM_FIRST				(WM_USER + 531)

#define BCM_SETIMAGELIST		(CM_FIRST + 3)
#define BCM_GETIMAGELIST		(CM_FIRST + 4)
#define BCM_SETTITLEIMAGE		(CM_FIRST + 5)
#define BCM_GETTITLEIMAGE		(CM_FIRST + 6)
#define BCM_SETCLIENTWND		(CM_FIRST + 7)
#define BCM_GETCLIENTWND		(CM_FIRST + 8)
#define BCM_SETCLIENTMIN		(CM_FIRST + 9)
#define BCM_GETCLIENTMIN		(CM_FIRST + 10)
#define BCM_GETCLIENTRECT		(CM_FIRST + 11)
#define BCM_SETALIGNMENT		(CM_FIRST + 12)
#define BCM_GETALIGNMENT		(CM_FIRST + 13)
#define BCM_SETDLGTEMPLATE		(CM_FIRST + 14)
#define BCM_SETMAXSIZE			(CM_FIRST + 15)
#define BCM_GETMAXSIZE			(CM_FIRST + 16)
#define BCM_SETDATA 			(CM_FIRST + 17)
#define BCM_GETDATA 			(CM_FIRST + 18)
#define BCM_SETALLOWABLEALIGNS	(CM_FIRST + 19)
#define BCM_GETALLOWABLEALIGNS	(CM_FIRST + 20)

#define BBN_CLOSE		4000

#define BandCtrl_SetData(hwndCtl, lpData)						((BOOL)SNDMSG((hwndCtl), BCM_SETDATA, 0, (LPARAM)(lpData)))
#define BandCtrl_GetData(hwndCtl)								((LPARAM)SNDMSG((hwndCtl), BCM_GETDATA, 0, 0L))
#define BandCtrl_SetImageList(hwndCtl, himl)					((HIMAGELIST)SNDMSG((hwndCtl), BCM_SETIMAGELIST, 0, (LPARAM)(HIMAGELIST)(himl)))
#define BandCtrl_GetImageList(hwndCtl)							((HIMAGELIST)SNDMSG((hwndCtl), BCM_GETIMAGELIST, 0, 0L))
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
#define BandCtrl_SetClientMinSize(hwndCtl, lppt)				((BOOL)SNDMSG((HWND)(hwndCtl), BCM_SETCLIENTMIN, 0, (LPARAM)(const LPPOINT)lppt))
#define BandCtrl_GetClientMinSize(hwndCtl, lppt)				((BOOL)SendMessage((HWND)(hwndCtl), BCM_GETCLIENTMIN, 0, (LPARAM)(LPPOINT)lppt))

#define BandCtrl_GetWindowFromClient(hwndCtl)					(GetParent(GetParent(hwnd)))

FOREXPORT BOOL BaseBar_RegisterControl(HINSTANCE hInstance);
FOREXPORT BOOL BaseBar_UnregisterControl(HINSTANCE hInstance);

FOREXPORT HWND BaseBar_Create(HWND hwndParent, LPCTSTR pszTitle, HINSTANCE hInstance, int nSize, UINT nID, DWORD dwStyle);

#endif //#ifndef _SMARTFTP_BASEBAR_H_
