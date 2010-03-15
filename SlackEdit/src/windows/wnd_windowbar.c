/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : se_windowbar.c
 * Created    : not known (before 06/26/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:18:50
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* windows */
#include <commctrl.h>

/* SlackEdit */
#include "../slack_main.h"

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

static HFONT hFontWindowBar;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

HWND Windowbar_Create(HWND hwndParent)
{
    NONCLIENTMETRICS nm;

    HWND hwndWindowBar = CreateWindowEx(
                            0x80,
                            WC_TABCONTROL, NULL,
                            WS_CHILDWINDOW | WS_VISIBLE |
                            WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                            WS_OVERLAPPED | TCS_FIXEDWIDTH |
                            TCS_BUTTONS | TCS_FLATBUTTONS |
                            TCS_HOTTRACK | TCS_FOCUSNEVER |
                            TCS_TOOLTIPS | TCS_FORCEICONLEFT,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                            hwndParent, (HMENU)IDC_TAB_WINDOWBAR, g_hInstance, 0);

    nm.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0); 
    hFontWindowBar = CreateFontIndirect(&nm.lfMenuFont);

    Window_SetFont(hwndWindowBar, hFontWindowBar, TRUE);
    TabCtrl_SetMinTabWidth(hwndWindowBar, 80);

    return (hwndWindowBar);
}

HWND Windowbar_Destroy(HWND hwndTab)
{
    if (DestroyWindow(hwndTab))
    {
        DeleteObject(hFontWindowBar);

        return (NULL);
    }
    else
    {
        return (hwndTab);
    }
}

BOOL Windowbar_ToolTip(LPTOOLTIPTEXT lpttt)
{
    TC_ITEM tci;
    TCHAR szFile[MAX_PATH];
    
    lpttt->hinst = g_hInstance;

    tci.mask        = TCIF_TEXT;
    tci.pszText     = szFile;
    tci.cchTextMax  = MAX_PATH;

    if (TabCtrl_GetItem(g_hwndTabWindowBar, lpttt->hdr.idFrom, &tci))
    {
        lpttt->lpszText = szFile;
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

void WindowBar_Update(LPFILEVIEW lpFileView, UINT nAction)
{
    int cItems, i;

    if (nAction == WM_CREATE)
    {
        tci.mask = TCIF_TEXT | TCIF_PARAM;
        tci.pszText = Path_GetFileName(lpFileView->pszFileName);
        tci.lParam = (LPARAM)lpFileView->hwndView;
        TabCtrl_SetCurSel(g_hwndTabWindowBar,
                    TabCtrl_InsertItem(g_hwndTabWindowBar,
                            TabCtrl_GetItemCount(g_hwndTabWindowBar), &tci));

        return;
    }

    cItems = TabCtrl_GetItemCount(g_hwndTabWindowBar);

    for (i = 0; i < iItems; i++)
    {
        TCITEM tci;

        tci.mask    = TCIF_PARAM;
        tci.lParam  = 0;

        TabCtrl_GetItem(g_hwndTabWindowBar, i, &tci);

        if ((HWND)tci.lParam == lpFileView->hwndView)
        {
            switch (nAction)
            {
                case WM_CLOSE:
                {
                    TabCtrl_DeleteItem(g_hwndTabWindowBar, i);
                    MDI_Next(g_hwndMDIClient, lpFileView->hwndView, FALSE);
                }
                break;
                case WM_ACTIVATE:
                {
                    if (!IsIconic(lpFileView->hwndView))
                        TabCtrl_SetCurSel(g_hwndTabWindowBar, cnt);
                }
                break;
                case SIZE_MINIMIZED:
                {
                    ShowWindow(lpFileView->hwndView, SW_HIDE);
                    TabCtrl_DeselectAll(g_hwndTabWindowBar, FALSE);
                    MDI_Next(g_hwndMDIClient, lpFileView->hwndView, FALSE);
                }
                break;
                case SIZE_RESTORED:
                {
                    if (IsIconic(lpFileView->hwndView))
                    {
                        ShowWindow(lpFileView->hwndView, SW_RESTORE);
                        ShowWindow(lpFileView->hwndView, SW_SHOW);
                    }
                }
                break;
                case TCM_SETTEXT:
                {
                    tci.mask = TCIF_TEXT | TCIF_PARAM;
                    tci.pszText = lpFileView->pszFileName;
                    tci.lParam = (LPARAM)lpFileView->hwndView;
                    TabCtrl_SetItem(g_hwndTabWindowBar, i, &tci);
                }
                break;
            }
        }
    }
}
