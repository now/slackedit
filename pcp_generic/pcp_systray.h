/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_systray.h
 * Created    : not known (before 01/02/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:33:14
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_SYSTRAY_H
#define __PCP_SYSTRAY_H

#include "pcp_generic.h"

FOREXPORT BOOL Systray_AddIcon(HWND hwnd, UINT uID, HICON hIcon, LPTSTR pszTip);
FOREXPORT BOOL Systray_ModifyIcon(HWND hwnd, UINT uID, HICON hIcon, LPTSTR pszTip);
FOREXPORT BOOL Systray_DeleteIcon(HWND hwnd, UINT uID);

FOREXPORT BOOL Systray_SetInTray(BOOL bInTray);
FOREXPORT BOOL Systray_IsInTray(void);

#define WM_TRAYNOTIFY		(WM_USER + 345)

/* void Cls_OnTrayNotify(HWND hwnd, UINT idCtl, UINT codeNotify) */
#define HANDLE_WM_TRAYNOTIFY(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (UINT)(lParam)), 0L)
#define FORWARD_WM_TRAYNOTIFY(hwnd, idCtl, codeNotify, fn) \
	(void)(fn)((hwnd), WM_TRAYNOTIFY, (WPARAM)(idCtl), (LPARAM)(codeNotify))

#endif /* __PCP_SYSTRAY_H */
