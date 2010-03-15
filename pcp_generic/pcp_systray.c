/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_systray.c
 * Created    : not known (before 01/02/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:32:18
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"
#include "pcp_definitions.h"

#include "pcp_systray.h"

static BOOL s_bInTray = FALSE;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

BOOL Systray_AddIcon(HWND hwnd, UINT uID, HICON hIcon, LPTSTR pszTip)
{
    NOTIFYICONDATA nid;

    INITSTRUCT(nid, TRUE);
    nid.hWnd                = hwnd;
    nid.uID                 = uID;
    nid.uFlags              = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage    = WM_TRAYNOTIFY;
    nid.hIcon               = hIcon;
    _tcsncpy(nid.szTip, pszTip, (sizeof(nid.szTip) - 1));
    
    return (Shell_NotifyIcon(NIM_ADD, &nid));
}

BOOL Systray_ModifyIcon(HWND hwnd, UINT uID, HICON hIcon, LPTSTR pszTip)
{
    NOTIFYICONDATA nid;

    INITSTRUCT(nid, TRUE);
    nid.hWnd    = hwnd;
    nid.uID     = uID;
    nid.uFlags  = ((hIcon == NULL) ? 0 : NIF_ICON) | ((pszTip == NULL) ? 0 : NIF_TIP);
    nid.hIcon   = hIcon;
    _tcsncpy(nid.szTip, pszTip, (sizeof(nid.szTip) - 1));

    return (Shell_NotifyIcon(NIM_MODIFY, &nid));
}

BOOL Systray_DeleteIcon(HWND hwnd, UINT uID)
{
    NOTIFYICONDATA nid;
    
    INITSTRUCT(nid, TRUE);
    nid.hWnd    = hwnd;
    nid.uID     = uID;

    return (Shell_NotifyIcon(NIM_DELETE, &nid));
}

BOOL Systray_SetInTray(BOOL bInTray)
{
    BOOL bTemp = s_bInTray;
    
    s_bInTray = bInTray;

    return (bTemp);
}

BOOL Systray_IsInTray(void)
{
    return (s_bInTray);
}
