/******************************************************************
 *                          FILE HEADER                           *
 ******************************************************************
 * Project    : SlackEdit
 *
 * File       : se_rebar.c
 * Created    : not known (before 06/24/00)
 * Owner      : pcppopper
 * Revised on : 07/05/00
 * Comments   : 
 *              
 *              
 ******************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* windows */
#include <commctrl.h>

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "bar_findcombo.h"
#include "bar_gotoline.h"
#include "bar_toolbar.h"
#include "bar_main.h"
#include "../settings/settings.h"

/* pcp_generic */
#include <pcp_rebar.h>

/* pcp_controls */
#include <pcp_menu.h>
#include <pcp_rect.h>
#include <pcp_string.h>
#include <pcp_toolbar.h>

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

LRESULT CALLBACK Rebar_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

static WNDPROC lpfnDefRebarProc;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

HWND Rebar_Create(HWND hwndParent)
{
    HWND hwndRebar = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
                                WS_VISIBLE | WS_BORDER | WS_CHILD |
                                WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                                WS_OVERLAPPED | CCS_TOP |
                                CCS_NODIVIDER | CCS_NOPARENTALIGN |
                                RBS_VARHEIGHT | RBS_BANDBORDERS |
                                RBS_DBLCLKTOGGLE | RBS_AUTOSIZE,
                                0, 0, 0, 0,
                                hwndParent, (HMENU)IDC_REBAR_MAIN, g_hInstance, NULL);

    lpfnDefRebarProc = SubclassWindow(hwndRebar, Rebar_WndProc);

    return (hwndRebar);
}

HWND Rebar_Destroy(HWND hwnd)
{
    if (DestroyWindow(hwnd))
    {
        Window_UpdateLayout(g_hwndMain);

        return (NULL);
    }
    else
    {
        return (hwnd);
    }
}

LRESULT CALLBACK Rebar_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->idFrom)
        {
        case IDC_TOOLBAR_MAIN:
            switch (((LPNMHDR)lParam)->code)
            {
                case TBN_GETBUTTONINFO:
                return (MyToolbar_GetButtonInfo((LPTBNOTIFY)lParam));
                case TBN_QUERYDELETE:
                return (TRUE);
                case TBN_QUERYINSERT:
                return (TRUE);
                case TBN_CUSTHELP:
                return (TRUE);
                case TBN_TOOLBARCHANGE:
                return (0);
                case TBN_RESET:
                    Toolbar_Reset(g_hwndToolbarMain);
                return (0);
                //case TBN_DRAGOUT: //interesting cause you can drag the buttons around (auto adjust kinda)
                //break;
//              case TBN_ENDADJUST:
//                  Toolbar_EndAdjust();
//              break;
            }
        break;
/*      case IDC_COMBO_QUICKSEARCH:
            switch (((LPNMHDR)lParam)->code)
            {
            case CBEN_ENDEDIT: // seems that CBEN_ENDEDITW is sent instead...stupid...bug in commctrl.h
            {
                PNMCBEENDEDIT pnmh = (PNMCBEENDEDIT)lParam;

                if (((PNMCBEENDEDIT)lParam)->iWhy == CBENF_RETURN)
                    QuickSearch_EndEdit((PNMCBEENDEDIT)lParam);
            }
            return (0);
            }
        break;*/
        }
    break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_REBARMENU_TOOLBAR:
            Rebar_ToggleItem(IDC_TOOLBAR_MAIN);
        return (0);
        case IDM_REBARMENU_FINDCOMBO:
            Rebar_ToggleItem(IDC_COMBO_QUICKSEARCH);
        return (0);
        case IDM_REBARMENU_GOTOLINE:
            Rebar_ToggleItem(IDC_DLG_GOTOLINE);
        return (0);
        }
    break;
    case WM_RBUTTONUP:
    {
        HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_REBARMENU));
        HMENU hSubMenu;
        POINT pt;

        POINTSTOPOINT(pt, lParam);
        ClientToScreen(hwnd, &pt);
        hSubMenu = GetSubMenu(hMenu, 0);

        // Update the menu to display current state
        Menu_CheckMenuItem(hSubMenu, IDM_REBARMENU_TOOLBAR, g_hwndToolbarMain);
        Menu_CheckMenuItem(hSubMenu, IDM_REBARMENU_FINDCOMBO, g_hwndCboQuickSearch);
        Menu_CheckMenuItem(hSubMenu, IDM_REBARMENU_GOTOLINE, IsWindow(g_hwndDlgGotoline));

        Menu_TrackPopupMenu(hSubMenu, pt.x, pt.y, g_hwndRebarMain);
        DestroyMenu(hMenu);
    }
    return (0);
    }

    return (CallWindowProc(lpfnDefRebarProc, hwnd, uMsg, wParam, lParam));
}

BOOL Rebar_ToggleItem(UINT uItem)
{
    BOOL bIsWindow = FALSE;
    UINT uMenuID = 0;
    HMENU hMenu;

    if (!IsWindow(g_hwndRebarMain))
        g_hwndRebarMain = Rebar_Create(g_hwndMain);

    // boy OOP would be good here
    switch (uItem)
    {
    case IDC_TOOLBAR_MAIN:
        if (IsWindow(g_hwndToolbarMain))
        {
            if (g_WindowSettings.bToolbarMainActive)
            {
                ShowWindow(g_hwndToolbarMain, SW_HIDE);
                Rebar_DeleteBand(g_hwndRebarMain, Rebar_IDToIndex(g_hwndRebarMain, IDC_TOOLBAR_MAIN));
                g_WindowSettings.bToolbarMainActive = FALSE;
            }
            else
            {
                Rebar_MyInsertBand(String_LoadString(IDS_REBARTITLE_TOOLBAR),
                            g_hwndToolbarMain, IDC_TOOLBAR_MAIN,
                            LOWORD(Toolbar_GetButtonSize(g_hwndToolbarMain)) *
                            Toolbar_ButtonCount(g_hwndToolbarMain), TOOLBARHEIGHT);

                ShowWindow(g_hwndToolbarMain, SW_SHOW);
                g_WindowSettings.bToolbarMainActive = TRUE;
            }
        }
        else
        {
            g_hwndToolbarMain = Toolbar_Create(g_hwndRebarMain);

            if (IsWindow(g_hwndToolbarMain))
                g_WindowSettings.bToolbarMainActive = TRUE;
        }

        bIsWindow   = g_WindowSettings.bToolbarMainActive;
        uMenuID     = IDM_VIEW_REBARITEMS_TOOLBAR;
    break;
    case IDC_COMBO_QUICKSEARCH:
        if (IsWindow(g_hwndCboQuickSearch))
        {
            g_hwndCboQuickSearch = FindCombo_Destroy(g_hwndCboQuickSearch);

            if (g_hwndCboQuickSearch == NULL)
                Rebar_DeleteBand(g_hwndRebarMain, Rebar_IDToIndex(g_hwndRebarMain, IDC_COMBO_QUICKSEARCH));
        }
        else
        {
            g_hwndCboQuickSearch = FindCombo_Create(g_hwndRebarMain);
        }

        bIsWindow   = IsWindow(g_hwndCboQuickSearch);
        uMenuID     = IDM_VIEW_REBARITEMS_QUICKSEARCH;
        g_WindowSettings.bQuickSearchActive = bIsWindow;
    break;
    case IDC_DLG_GOTOLINE:
        if (IsWindow(g_hwndDlgGotoline))
        {
            g_hwndDlgGotoline = GotoLine_Destroy(g_hwndDlgGotoline);

            if (g_hwndDlgGotoline == NULL)
                Rebar_DeleteBand(g_hwndRebarMain, Rebar_IDToIndex(g_hwndRebarMain, IDC_DLG_GOTOLINE));
        }
        else
        {
            g_hwndDlgGotoline = GotoLine_Create(g_hwndRebarMain);
        }

        bIsWindow   = IsWindow(g_hwndDlgGotoline);
        uMenuID     = IDM_VIEW_REBARITEMS_GOTOLINE;
        g_WindowSettings.bGotoLineActive = bIsWindow;
    break;
    default:
        ASSERT(FALSE);
    break;
    }

    hMenu = Main_GetMainSubMenu(g_hwndMain, IDM_MAIN_VIEW);
    Menu_CheckMenuItem(hMenu, uMenuID, bIsWindow);

    if (IsWindow(g_hwndToolbarMain))
        Toolbar_CheckButton(g_hwndToolbarMain, uMenuID, bIsWindow);

    /*
     * check if the only rebar item is the toolbar
     * if it is and the toolbar isn't active then hide
     * the rebar else show it
     */
    if (bIsWindow)
        ShowWindow(g_hwndRebarMain, SW_SHOW);
    else if (!bIsWindow && Rebar_GetBandCount(g_hwndRebarMain) == 0)
        ShowWindow(g_hwndRebarMain, SW_HIDE);

    Window_UpdateLayout(g_hwndMain);

    Window_UpdateLayout(g_hwndRebarMain);
    RedrawWindow(g_hwndRebarMain, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

    return (bIsWindow);
}

void Rebar_MyInsertBand(LPCTSTR pszTitle, HWND hwndItem, WORD wID, int cx, int cy)
{
    REBARBANDINFO rbbi;
    SIZE sTextSize;
    HDC hdc;

    hdc = GetDC(g_hwndRebarMain);
    GetTextExtentPoint32(hdc, pszTitle, _tcslen(pszTitle), &sTextSize);
    ReleaseDC(g_hwndRebarMain, hdc);

    rbbi.cbSize     = sizeof(rbbi);
    rbbi.fMask      = RBBIM_COLORS | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_TEXT |
                            RBBIM_STYLE | RBBIM_ID;
    rbbi.clrFore    = GetSysColor(COLOR_BTNTEXT);
    rbbi.clrBack    = GetSysColor(COLOR_BTNFACE);
    rbbi.fStyle     = RBBS_NOVERT | RBBS_CHILDEDGE | RBBS_FIXEDBMP;
    rbbi.lpText     = (LPTSTR)pszTitle;
    rbbi.hwndChild  = hwndItem;
    rbbi.wID        = wID;
    rbbi.cxMinChild = cx + sTextSize.cx;
    rbbi.cyMinChild = cy;

    if (g_ImageSettings.bCoolbarImage)
    {
        rbbi.fMask      |= RBBIM_BACKGROUND;
        rbbi.hbmBack    = LoadImage(g_hInstance,
                                    g_ImageSettings.szCoolbarImage,
                                    IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }

    Rebar_InsertBand(g_hwndRebarMain, -1, &rbbi);
    Rebar_MaximizeBand(g_hwndRebarMain, Rebar_GetBandCount(g_hwndRebarMain) - 1, FALSE);
}
