/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_window.c
 * Created    : not known (before 01/26/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:28:16
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"

#include "pcp_window.h"
#include "pcp_rect.h"

DWORD Window_ModifyStyleImpl(HWND hwnd, int nStyleOffset, DWORD dwRemove, DWORD dwAdd, UINT uFlags);

void Window_CenterWindow(HWND hwnd)
{
    RECT rWorkArea;
    RECT rWin;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rWorkArea, 0);

    GetWindowRect(hwnd, &rWin);

    SetWindowPos(hwnd, 0,
                (rWorkArea.right / 2) - (Rect_Width(&rWin) / 2),
                (rWorkArea.bottom / 2) - (Rect_Height(&rWin) / 2),
                0, 0, SWP_NOSIZE | SWP_NOZORDER);   
}

DWORD Window_ModifyStyleImpl(HWND hwnd, int nStyleOffset, DWORD dwRemove, DWORD dwAdd, UINT uFlags)
{
    DWORD dwStyle       = GetWindowLong(hwnd, nStyleOffset);
    DWORD dwNewStyle    = ((dwStyle & ~(dwRemove)) | dwAdd);

    if (dwStyle == dwNewStyle)
        return (dwStyle);

    SetWindowLong(hwnd, nStyleOffset, dwNewStyle);

    if (uFlags != 0)
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | uFlags);

    return (dwStyle);
}

DWORD Window_ModifyStyle(HWND hwnd, DWORD dwRemove, DWORD dwAdd, UINT uFlags)
{
    return (Window_ModifyStyleImpl(hwnd, GWL_STYLE, dwRemove, dwAdd, uFlags));
}

DWORD Window_ModifyStyleEx(HWND hwnd, DWORD dwRemove, DWORD dwAdd, UINT uFlags)
{
    return (Window_ModifyStyleImpl(hwnd, GWL_EXSTYLE, dwRemove, dwAdd, uFlags));
}

BOOL Window_UpdateLayout(HWND hwnd)
{
    RECT rc;

    GetWindowRect(hwnd, &rc);

    return (Window_Size(hwnd, Rect_Width(&rc), Rect_Height(&rc)));
}

void Window_ScreenToClientRect(HWND hwnd, LPRECT lprc)
{
    POINT pt;

    if (lprc == NULL)
        return;

    pt.x = lprc->left;
    pt.y = lprc->top;
    ScreenToClient(hwnd, &pt);
    lprc->left = pt.x;
    lprc->top = pt.y;
    pt.x = lprc->right;
    pt.y = lprc->bottom;
    ScreenToClient(hwnd, &pt);
    lprc->right = pt.x;
    lprc->bottom = pt.y;
}

void Window_ClientToScreenRect(HWND hwnd, LPRECT lprc)
{
    POINT pt;

    if (lprc == NULL)
        return;

    pt.x = lprc->left;
    pt.y = lprc->top;
    ClientToScreen(hwnd, &pt);
    lprc->left = pt.x;
    lprc->top = pt.y;
    pt.x = lprc->right;
    pt.y = lprc->bottom;
    ClientToScreen(hwnd, &pt);
    lprc->right = pt.x;
    lprc->bottom = pt.y;
}
