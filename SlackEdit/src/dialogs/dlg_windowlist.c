/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : windowlist.c
 * Created    : 01/15/99
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:35:14
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "../settings/settings.h"
#include "../parsing/file_actions.h"

/* pcp_generic */
#include <pcp_listbox.h>
#include <pcp_window.h>

/* pcp_controls */
#include <pcp_menu.h>
#include <pcp_path.h>
#include <pcp_mdi.h>

/* pcp_edit */
#include <pcp_edit.h>

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

BOOL CALLBACK WindowList_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL WindowList_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void WindowList_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode);
static void WindowList_OnClose(HWND hwnd);

LRESULT CALLBACK WindowList_ListWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

WNDPROC lpfnOldListWndProc = NULL;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

int WindowList_CreateDialogBox(HWND hwndParent)
{
    return (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_WINDOWLIST), hwndParent, WindowList_DlgProc, (LPARAM)NULL));
}

BOOL CALLBACK WindowList_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_DLG_MSG(hwndDlg, WM_INITDIALOG,  WindowList_OnInitDialog);
        HANDLE_DLG_MSG(hwndDlg, WM_COMMAND,     WindowList_OnCommand);
        HANDLE_DLG_MSG(hwndDlg, WM_CLOSE,       WindowList_OnClose);
    }

    return (FALSE);
}

static BOOL WindowList_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HWND hwndMDI;
    HWND hwndList = GetDlgItem(hwnd, IDC_LIST_WINDOWS);
    TCHAR szTitle[MAX_PATH + 10];

    for (
        hwndMDI = GetWindow(g_hwndMDIClient, GW_CHILD);
        hwndMDI != (HWND)NULL;
        hwndMDI = GetWindow(hwndMDI, GW_HWNDNEXT)
        )
    {
        int nIndex;

        GetWindowText(hwndMDI, szTitle, sizeof(szTitle));
        
        nIndex  = ListBox_AddString(hwndList, szTitle);
        ListBox_SetItemData(hwndList, nIndex, hwndMDI);
    }

    if (ListBox_GetCount(hwndList) == 0)
    {
        Window_EnableDlgItem(hwnd, IDOK, FALSE);
        Window_EnableDlgItem(hwnd, IDC_BUTTON_CLOSE, FALSE);
        Window_EnableDlgItem(hwnd, IDC_BUTTON_SAVE, FALSE);
    }

    if (g_WindowSettings.bWindowListCentered)
    {
        Window_CenterWindow(hwnd);
    }
    else
    {
        SetWindowPos(hwnd, (HWND)NULL,
            g_WindowSettings.nWindowListX, g_WindowSettings.nWindowListY,
            0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }

    ListBox_SetSel(hwndList, TRUE, 0);

    lpfnOldListWndProc = SubclassWindow(hwndList, WindowList_ListWndProc);

    return (TRUE);
}

static void WindowList_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode)
{
    switch (idCtl)
    {
    case IDOK:
    {
        HWND hwndList = GetDlgItem(hwnd, IDC_LIST_WINDOWS);
        int nIndex = ListBox_GetFirstSel(hwndList);
        HWND hwndMDI = (HWND)ListBox_GetItemData(hwndList, nIndex);

        MDI_Activate(g_hwndMDIClient, hwndMDI);
        Window_Close(hwnd);
    }
    return;
    case IDC_BUTTON_CLOSE:
    {
        HWND hwndList = GetDlgItem(hwnd, IDC_LIST_WINDOWS);
        int nCount = ListBox_GetCount(hwndList);
        LPINT anIndex = (LPINT)_alloca(sizeof(int) * nCount);
        int i;

        nCount = ListBox_GetSelItems(hwndList, nCount, anIndex);

        for (i = (nCount - 1); i >= 0; i--)
        {
            HWND hwndMDI = (HWND)ListBox_GetItemData(hwndList, anIndex[i]);

            Window_Close(hwndMDI);
            ListBox_DeleteString(hwndList, anIndex[i]);
        }

        if (ListBox_SetSel(hwndList, TRUE, anIndex[i]) == LB_ERR)
            ListBox_SetSel(hwndList, TRUE, 0);
    }
    return;
    case IDC_BUTTON_SAVE:
    {
        HWND hwndList = GetDlgItem(hwnd, IDC_LIST_WINDOWS);
        int nCount = ListBox_GetCount(hwndList);
        LPINT anIndex = (LPINT)_alloca(sizeof(int) * nCount);
        int i;

        nCount = ListBox_GetSelItems(hwndList, nCount, anIndex);

        for (i = 0; i < nCount; i++)
        {
            HWND hwndMDI = (HWND)ListBox_GetItemData(hwndList, anIndex[i]);
            FILEINFO fi;

            Main_GetFileInfo(hwndMDI, &fi);

            if (!(fi.dwFileType & FI_TYPE_MASK_EDIT) && Path_FileExists(fi.szFileName))
                File_Save(hwndMDI, fi.szFileName, CRLF_STYLE_AUTOMATIC);
            else
                File_SaveByDialog(hwndMDI, _T(""));
        }
    }
    return;
    case IDCANCEL:
        Window_Close(hwnd);
    return;
    }
}

static void WindowList_OnClose(HWND hwnd)
{
    RECT rc;

    GetWindowRect(hwnd, &rc);

    g_WindowSettings.nWindowListX   = rc.left;
    g_WindowSettings.nWindowListY   = rc.top;

    EndDialog(hwnd, 0);
}

LRESULT CALLBACK WindowList_ListWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_LBUTTONDBLCLK:
        // Just a clearer way of doing a LBN_DBLCLK
        PostMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDOK, 0), (LPARAM)hwnd);
    return (0);
    case WM_CONTEXTMENU:
    {
        HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_WINDOWLISTMENU));
        HMENU hMenuPopup = GetSubMenu(hMenu, 0);
        RECT rc;

        if (lParam == -1) // Context Menu
        {
            int nCaretIndex = ListBox_GetCaretIndex(hwnd);
            int nItemHeight = ListBox_GetItemHeight(hwnd, nCaretIndex);

            ListBox_GetItemRect(hwnd, nCaretIndex, &rc);
            Window_ClientToScreenRect(hwnd, &rc);
            OffsetRect(&rc, 10, nItemHeight);
        }
        else // Right Mouse Button
        {
            POINT pt;

            POINTSTOPOINT(pt, lParam);

            rc.left = pt.x;
            rc.top  = pt.y;
        }

        Menu_TrackPopupMenu(hMenuPopup, rc.left, rc.top, hwnd);
        DestroyMenu(hMenu);
    }
    return (0);
    }

    return (CallWindowProc(lpfnOldListWndProc, hwnd, uMsg, wParam, lParam));
}
