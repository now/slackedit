/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : se_mdi_child_generic.c
 * Created    : 03/09/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:34:42
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* windows */
#include <commctrl.h>

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "mdi_child_generic.h"
#include "mdi_child_edit.h"
#include "mdi_child_hex.h"
#include "mdi_client.h"
#include "child_tclcommandline.h"
#include "../controls/clipboard_monitor.h"
#include "../dialogs/dlg_print.h"
#include "../parsing/file_actions.h"
#include "../settings/settings.h"
#include "../subclasses/sub_pcp_edit.h"
#include "../windows/wnd_statusbar.h"

/* pcp_controls */
#include <pcp_menu.h>

/* pcp_generic */
#include <pcp_path.h>
#include <pcp_string.h>
#include <pcp_toolbar.h>
#include <pcp_clipboard.h>


// These shouldn't be here...

/* pcp_edit */

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

BOOL CALLBACK MDI_Child_Generic_SaveEnumProc(HWND hwnd, LPARAM lParam);

BOOL MDI_Child_Generic_GoFullScreen(HWND hwnd);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

BOOL MDI_Child_Generic_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    TclCmd_Create(hwnd);

    if (g_BackupSettings.bAutoSave && g_BackupSettings.uAutoSaveInterval)
        SetTimer(hwnd, IDT_AUTOSAVE, g_BackupSettings.uAutoSaveInterval * 60000, NULL);

    switch (g_WindowSettings.nMDIWinState)
    {
        case MDISTATE_NORMAL:
        {
            MDI_Client_ResizeChildren(g_hwndMDIClient);
        }
        break;
        case MDISTATE_TILEHORZ:
            PostMessage(g_hwndMain, WM_COMMAND, IDM_WINDOW_TILEHORIZONTAL, 0);
        break;
        case MDISTATE_TILEVERT:
            PostMessage(g_hwndMain, WM_COMMAND, IDM_WINDOW_TILEVERTICAL, 0);
        break;
    }

    DrawMenuBar(g_hwndMain);

    return (TRUE);
}

void MDI_Child_Generic_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    DefMDIChildProc(hwnd, WM_SIZE, state, MAKELPARAM(cx, cy));
}

// This whole thing should be redone...it SUCKS SOOO MUCH!

void MDI_Child_Generic_OnMDIActivate(HWND hwnd, BOOL fActive, HWND hwndActivate, HWND hwndDeactivate)
{
    static BOOL fLastActive = TRUE;
    HMENU hMenu, hMenuFile, hMenuView;
    UINT fEnable = MF_BYPOSITION;

    if (hwndDeactivate == hwnd && hwndActivate == NULL)
        fActive = FALSE;
    else
        fActive = TRUE;

    // This should be wrong (GetSubMenu with MDI window Maximized) but...
    hMenu = GetMenu(g_hwndMain);
    hMenuFile = GetSubMenu(hMenu, 0);
    hMenuView = GetSubMenu(hMenu, 3);

    if (fActive)
        fEnable |= MF_ENABLED;
    else
        fEnable |= MF_GRAYED;

    EnableMenuItem(hMenu, IDM_MAIN_EDIT, fEnable);
    EnableMenuItem(hMenu, IDM_MAIN_SEARCH, fEnable);
    EnableMenuItem(hMenu, IDM_MAIN_WINDOW, fEnable);

    Menu_EnableMenuItem(hMenuFile, IDM_FILE_SAVE, fActive);
    Menu_EnableMenuItem(hMenuFile, IDM_FILE_SAVEAS, fActive);
    Menu_EnableMenuItem(hMenuFile, IDM_FILE_SAVEALL, fActive);
    Menu_EnableMenuItem(hMenuFile, IDM_FILE_PRINT, fActive);
    Menu_EnableMenuItem(hMenuFile, IDM_FILE_READONLY, fActive);

    Menu_EnableMenuItem(hMenuView, IDM_VIEW_FULLSCREEN, fActive);

    EnableWindow(g_hwndCboQuickSearch, fActive);
    EnableWindow(g_hwndDlgGotoline, fActive);

    Main_UpdateWindowBar(hwndActivate, WM_ACTIVATE, (LPARAM)NULL);

    DefWindowProc(hwnd, WM_MDIACTIVATE, (WPARAM)hwndActivate, (LPARAM)hwndDeactivate);

    if (!fActive)
    {
        // We don't have any more docs
        DrawMenuBar(g_hwndMain);
        fLastActive = FALSE;
    }
    else if (fActive && !fLastActive)
    {
        DrawMenuBar(g_hwndMain);
        fLastActive = TRUE;
    }
}

void MDI_Child_Generic_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT uNotifyCode)
{
    switch (id)
    {
        case IDM_FILE_SAVE:
        {
            FILEINFO fi;
            Main_GetFileInfo(hwnd, &fi);

            if (fi.dwFileType & FI_TYPE_MASK_EDIT || !Path_FileExists(fi.szFileName))
                File_SaveByDialog(hwnd, fi.szFileName);
            else
                File_Save(hwnd, fi.szFileName, CRLF_STYLE_AUTOMATIC); // CRLF from lpes
        }
        return;
        case IDM_FILE_SAVEAS:
        {
            FILEINFO fi;
            Main_GetFileInfo(hwnd, &fi);

            File_SaveByDialog(hwnd, fi.szFileName);
        }
        return;
        case IDM_FILE_SAVEALL:
            EnumChildWindows(g_hwndMDIClient, MDI_Child_Generic_SaveEnumProc, 0);
        return;
        case IDM_FILE_PRINT:
        {
            FILEINFO fi;
            Main_GetFileInfo(hwnd, &fi);

            if (!Print_File(g_hInstance, hwnd, GetDlgItem(hwnd, IDC_EDIT_MDICHILD),
                            fi.szFileName))
            {
                TCHAR szMsg[MAX_PATH];

                _stprintf(szMsg, String_LoadString(IDS_FORMAT_PRINT_FAILED), Path_GetFileName(fi.szFileName));
                MessageBox(g_hwndMain, szMsg, g_szAppName, MB_OK | MB_ICONEXCLAMATION);
            }
        }
        return;
        case IDM_EDIT_CUT:
            SendDlgItemMessage(hwnd, IDC_EDIT_MDICHILD, WM_CUT, 0, 0L);
        return;
        case IDM_EDIT_COPY:
            SendDlgItemMessage(hwnd, IDC_EDIT_MDICHILD, WM_COPY, 0, 0);
        return;
        case IDM_EDIT_PASTE:
            SendDlgItemMessage(hwnd, IDC_EDIT_MDICHILD, WM_PASTE, 0, 0);
        return;
        case IDM_EDIT_DELETE:
            SendDlgItemMessage(hwnd, IDC_EDIT_MDICHILD, WM_CLEAR, 0, 0L);
        return;
        case IDM_VIEW_FULLSCREEN:
            Menu_CheckMenuItem(Main_GetMainSubMenu(g_hwndMain, IDM_MAIN_VIEW), IDM_VIEW_FULLSCREEN, MDI_Child_Generic_GoFullScreen(hwnd));
        return;
        case IDC_EDIT_MDICHILD:
            switch (uNotifyCode)
            {
                case EN_CHANGE:
                    MDI_Child_Generic_EnableToolbarButtons(hwnd);

                    // Needs to support hex edits as well
                    if (PCP_Edit_GetModified(PCP_Edit_GetInterface(hwndCtl)))
                    {
                        TCHAR szTitle[MAX_PATH + 6];
                        LPTSTR psz;

                        GetWindowText(hwnd, szTitle, MAX_PATH);
                        psz = szTitle + _tcslen(szTitle) - 2;

                        if (!String_Equal(psz, _T(" *"), FALSE))
                        {
                            _tcscat(szTitle, _T(" *"));
                            SetWindowText(hwnd, szTitle);
                            Main_UpdateWindowBar(hwnd, TCM_SETTEXT, (LPARAM)Path_GetFileName(szTitle));
                        }

                        Toolbar_EnableButton(g_hwndToolbarMain, IDM_FILE_SAVEALL, TRUE);
                    }
                    else
                    {
                        TCHAR szTitle[MAX_PATH + 6];
                        LPTSTR psz;

                        GetWindowText(hwnd, szTitle, MAX_PATH);
                        psz = szTitle + _tcslen(szTitle) - 2;

                        if (String_Equal(psz, _T(" *"), FALSE))
                        {
                            szTitle[_tcslen(szTitle) - 2] = _T('\0');
                            SetWindowText(hwnd, szTitle);
                            Main_UpdateWindowBar(hwnd, TCM_SETTEXT, (LPARAM)Path_GetFileName(szTitle));
                        }

                        Toolbar_EnableButton(g_hwndToolbarMain, IDM_FILE_SAVEALL, FALSE);
                    }
            }
        return;
        default:
            //The clipboard stuff (most recent clips)
            if (id >= IDM_EDIT_FIRSTCLIP && id <= (IDM_EDIT_FIRSTCLIP + (MAX_CLIPS - 1)))
            {
                LPCTSTR pszClip = ClipboardMonitor_GetClip(id - IDM_EDIT_FIRSTCLIP);

                if (g_EditSettings.bRecentToClipboard)
                {
                    Clipboard_SetText(pszClip);
                    SendDlgItemMessage(hwnd, IDC_EDIT_MDICHILD, WM_PASTE, 0, 0L);
                }
                else
                {
                    MENUITEMINFO mii;
                    HMENU hMenu = Main_GetMainSubMenu(hwnd, IDM_MAIN_EDIT);

                    PCP_Edit_ReplaceSelection(PCP_Edit_GetInterface(hwnd, IDC_EDIT_MDICHILD), pszClip);

                    INITSTRUCT(mii, TRUE);
                    mii.fMask   = MIIM_SUBMENU;
                    GetMenuItemInfo(hMenu, 6, TRUE, &mii);

                    Clip_RecreateMenu(&mii.hSubMenu);
                    SetMenuItemInfo(hMenu, 6, TRUE, &mii);
                }
            }
            else if (id >= SC_SIZE && id <= SC_RESTORE)
            {
                SendMessage(hwnd, WM_SYSCOMMAND, id, 0);
            }
            else
            {
                Window_Command(GetDlgItem(hwnd, IDC_EDIT_MDICHILD), id, uNotifyCode, hwndCtl);
            }
        return;
    }
}

LRESULT MDI_Child_Generic_OnNotify(HWND hwnd, int id, LPNMHDR pnmh)
{
    return (DefMDIChildProc(hwnd, WM_NOTIFY, id, (LPARAM)pnmh));
}

void MDI_Child_Generic_OnMenuSelect(HWND hwnd, HMENU hMenu, int item, HMENU hMenuPopup, UINT flags)
{
}

void MDI_Child_Generic_OnTimer(HWND hwnd, UINT id)
{
    switch (id)
    {
        case IDT_AUTOSAVE:
            File_AutoSave(GetDlgItem(hwnd, IDC_EDIT_MDICHILD));
        break;
    }
}

void MDI_Child_Generic_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
    switch (cmd)
    {
        case SC_MAXIMIZE:
            g_WindowSettings.nMDIWinState = MDISTATE_MAXIMIZED;
        break;
        case SC_RESTORE:
            g_WindowSettings.nMDIWinState = MDISTATE_NORMAL;
        break;
    }

    DefMDIChildProc(hwnd, WM_SYSCOMMAND, (WPARAM)cmd, MAKELPARAM(x, y));
}

int MDI_Child_Generic_GetFileType(HWND hwnd, BOOL bCareIfEdit)
{
    FILEINFO fi;

    VERIFY(Main_GetFileInfo(hwnd, &fi));

    return (fi.dwFileType);
}

BOOL CALLBACK MDI_Child_Generic_SaveEnumProc(HWND hwnd, LPARAM lParam)
{
    TCHAR szClassName[81];

    GetClassName(hwnd, szClassName, 80);

    if (String_Equal(szClassName, g_szEditChild, FALSE) ||
        String_Equal(szClassName, g_szHexChild, FALSE))
    {
        FILEINFO fi;

        Main_GetFileInfo(hwnd, &fi);

        if (Path_FileExists(fi.szFileName))
            File_Save(GetParent(hwnd), fi.szFileName, CRLF_STYLE_AUTOMATIC);
        else
            File_SaveByDialog(GetParent(hwnd), fi.szFileName);
    }

    return (TRUE);
}

BOOL MDI_Child_Generic_GoFullScreen(HWND hwnd)
{
    static BOOL bFullScreen = FALSE;
    static RECT rMain;

    if (bFullScreen)
    {
        // Turn fullscreen off
        DWORD dwStyle = GetWindowLong(g_hwndMain, GWL_STYLE);
        
        dwStyle |= WS_CAPTION;
        SetWindowLong(g_hwndMain, GWL_STYLE, dwStyle);
        ShowWindow(g_hwndStatusbarMain, SW_SHOW);
        ShowWindow(g_hwndRebarMain, SW_SHOW);
        ShowWindow(g_hwndTabWindowBar, SW_SHOW);
        if (IsWindow(g_hwndClipbook))
            ShowWindow(g_hwndClipbook, SW_SHOW);

        SetWindowPos(g_hwndMain, NULL, rMain.left, rMain.top,
            rMain.right - rMain.left, rMain.bottom - rMain.top, SWP_NOZORDER);

        if (g_WindowSettings.bMainMaximized)
            ShowWindow(g_hwndMain, SW_SHOWMAXIMIZED);
        else
            ShowWindow(g_hwndMain, SW_RESTORE);

        bFullScreen = FALSE;
    }
    else
    {
        // Turn fullscreen on
        DWORD dwStyle = GetWindowLong(g_hwndMain, GWL_STYLE);
        int cxScreen, cyScreen;

        GetWindowRect(g_hwndMain, &rMain);

        ShowWindow(g_hwndStatusbarMain, SW_HIDE);
        ShowWindow(g_hwndRebarMain, SW_HIDE);
        ShowWindow(g_hwndTabWindowBar, SW_HIDE);
        if (IsWindow(g_hwndClipbook))
            ShowWindow(g_hwndClipbook, SW_HIDE);

        g_WindowSettings.bMainMaximized = (dwStyle & WS_MAXIMIZE) ? TRUE : FALSE;
        if (g_WindowSettings.bMainCentered)
            ShowWindow(g_hwndMain, SW_RESTORE);

        dwStyle &= ~(WS_CAPTION|WS_MAXIMIZE);
        SetWindowLong(g_hwndMain, GWL_STYLE, dwStyle);

        cxScreen = GetSystemMetrics(SM_CXSCREEN);
        cyScreen = GetSystemMetrics(SM_CYSCREEN);

        SetWindowPos(g_hwndMain, NULL, -4, -4, cxScreen + 8, cyScreen + 8, SWP_NOZORDER);

        ShowWindow(hwnd, SW_SHOWMAXIMIZED);

        bFullScreen = TRUE;
    }

    return (bFullScreen);
}

void MDI_Child_Generic_EnableToolbarButtons(HWND hwnd)
{
    FILEINFO fi;
    Main_GetFileInfo(hwnd, &fi);

    fi.dwFileType &= ~FI_TYPE_MASK_FIRST;
    fi.dwFileType &= ~FI_TYPE_MASK_EDIT;

    switch (fi.dwFileType)
    {
    case FI_TYPE_TEXT:
        MDI_Child_Edit_EnableToolbarButtons(hwnd, FALSE);
    break;
    case FI_TYPE_HEX:
        MDI_Child_Hex_EnableToolbarButtons(hwnd, FALSE);
    break;
    }
}
