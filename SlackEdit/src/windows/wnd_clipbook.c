/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : clipbook.c
 * Created    : not known (before 06/26/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:34:05
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* resources */
#include <slack_resource.h>
#include <slack_picdef.h>

/* SlackEdit */
#include "../slack_main.h"
#include "wnd_clipbook.h"
#include "../parsing/parse_ecp.h"
#include "../parsing/file_actions.h"
#include "../settings/settings.h"
#include "../settings/settings_font.h"
#include "../windows/wnd_statusbar.h"

/* pcp_generic */
#include <pcp_combobox.h>
#include <pcp_editctrl.h>
#include <pcp_linkedlist.h>
#include <pcp_mdi.h>
#include <pcp_mem.h>
#include <pcp_rect.h>
#include <pcp_string.h>
#include <pcp_toolbar.h>
#include <pcp_window.h>

/* pcp_controls */
#include <pcp_basebar.h>
#include <pcp_menu.h>
#include <pcp_path.h>
#include <pcp_statusbar.h>

/* pcp_edit */

#define IDC_LIST_CLIPS          2001

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

static void Clipbook_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode);
static void Clipbook_OnSize(HWND hwnd, UINT state, int cx, int cy);
static LRESULT Clipbook_OnNotify(HWND hwnd, int id, LPNMHDR pnmh);
static void Clipbook_OnWindowPosChanged(HWND hwnd, const LPWINDOWPOS lpwpos);
static void Clipbook_OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
static void Clipbook_OnDestroy(HWND hwnd);
static void Clipbook_OnFontChange(HWND hwnd, const LPLOGFONT lplf);

void Clipbook_AddClipFiles(HWND hwndCombo);
BOOL Clipbook_ReadFile(HWND hwndOwner, PECPCLIPBOOK pcb);
void Clipbook_DeleteList(HWND hwndList);
HWND Clipbook_CreateListWindow(HWND hwndOwner, HWND hwndOldList, PECPCLIPBOOK pClipbook);

LRESULT CALLBACK Clipbook_List_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Clipbook_Cbo_Cbo_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

static WNDPROC lpfnDefListWndProc;
static WNDPROC lpfnDefCboCboWndProc;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

HWND Clipbook_Create(HWND hwndParent)
{
    COMBOBOXEXITEM cbxi;
    HWND hwndCombo;
    HWND hwndClipbook;
    HWND hwndDialog;

    hwndClipbook    = BaseBar_Create(hwndParent, _T("Clipbook"), g_hInstance, g_WindowSettings.nClipbookWidth, IDC_CLIPBOOK, g_WindowSettings.nClipbookAlignment);
    hwndDialog      = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_CLIPBOOK), hwndClipbook, Clipbook_WndProc);
    hwndCombo       = GetDlgItem(hwndDialog, IDC_COMBO_CLIPFILES);

    BandCtrl_SetClientWindow(hwndClipbook, hwndDialog);
    BandCtrl_SetAllowableAligns(hwndClipbook, CCS_LEFT | CCS_RIGHT);
    BandCtrl_SetImageList(hwndClipbook, Menu_GetImageList(TRUE));
    BandCtrl_SetTitleImage(hwndClipbook, IDP_CLIPBOOK);

    Clipbook_AddClipFiles(hwndCombo);

    ComboBox_SetCurSel(hwndCombo, g_CBSettings.nLastIndex);

    cbxi.mask   = CBEIF_LPARAM;
    cbxi.iItem  = ComboBox_GetCurSel(hwndCombo);

    ComboBoxEx_GetItem(hwndCombo, &cbxi);
    Clipbook_ReadFile(hwndDialog, (PECPCLIPBOOK)cbxi.lParam);

    lpfnDefCboCboWndProc    = SubclassWindow(
                                    ComboBoxEx_GetComboControl(hwndCombo),
                                    Clipbook_Cbo_Cbo_WndProc);

    SetWindowPos(hwndClipbook, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME);

    return (hwndClipbook);
}

HWND Clipbook_Destroy(HWND hwndClipbook)
{
    if (DestroyWindow(hwndClipbook))
    {
        return (NULL);
    }
    else
    {
        return (hwndClipbook);
    }
}

LRESULT CALLBACK Clipbook_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_COMMAND,            Clipbook_OnCommand);
        HANDLE_MSG(hwnd, WM_SIZE,               Clipbook_OnSize);
        HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGED,   Clipbook_OnWindowPosChanged);
        HANDLE_MSG(hwnd, WM_DESTROY,            Clipbook_OnDestroy);
        HANDLE_MSG(hwnd, WM_MYFONTCHANGE,       Clipbook_OnFontChange);
        case WM_SETFOCUS:
            SetFocus(GetDlgItem(hwnd, IDC_LIST_CLIPS));
        break;
    }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}

static void Clipbook_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode)
{
    switch (idCtl)
    {
        case IDC_COMBO_CLIPFILES:
            switch (uNotifyCode)
            {
                case CBN_SELENDOK:
                {
                    COMBOBOXEXITEM cbxi;
                    HWND hwndList = GetDlgItem(hwnd, IDC_LIST_CLIPS);

                    cbxi.mask   = CBEIF_LPARAM;
                    cbxi.iItem  = ComboBox_GetCurSel(hwndCtl);

                    g_CBSettings.nLastIndex = cbxi.iItem;

                    ComboBoxEx_GetItem(hwndCtl, &cbxi);

                    Clipbook_DeleteList(hwndList);
                    Clipbook_ReadFile(hwnd, (PECPCLIPBOOK)cbxi.lParam);
                }
                break;
            }
        break;
        case IDC_LIST_CLIPS:
            switch (uNotifyCode)
            {
                case LBN_DBLCLK:
                {
                    int iIndex = ListBox_GetCurSel(hwndCtl);
                    PECPITEM pItem;
                    
                    if (iIndex == LB_ERR)
                        break;

                    pItem = (PECPITEM)ListBox_GetItemData(hwndCtl, iIndex);

                    ECP_ProcessItem(pItem, TRUE);
                }
                break;
            }
        break;
        case IDM_CLIPMENU_EDITCLIPFILE:
        {
            COMBOBOXEXITEM cbxi;
            HWND hwndCombo = GetDlgItem(hwnd, IDC_COMBO_CLIPFILES);

            cbxi.mask   = CBEIF_LPARAM;
            cbxi.iItem  = ComboBox_GetCurSel(hwndCombo);

            if (ComboBoxEx_GetItem(hwndCombo, &cbxi))
                File_Open(((PECPCLIPBOOK)cbxi.lParam)->szFileName, CRLF_STYLE_AUTOMATIC);
            else
                MessageBox(g_hwndMain, _T("Trying to edit something that doesn't exists are we?\nPlease see to it that there are some\nclipfiles around before trying to edit them"), _T("Oops..."), MB_OK);
        }
        break;
        case IDM_CLIPMENU_RELOADCLIPFILE:
        {
            COMBOBOXEXITEM cbxi;
            HWND hwndCombo = GetDlgItem(hwnd, IDC_COMBO_CLIPFILES);

            cbxi.mask   = CBEIF_LPARAM;
            cbxi.iItem  = ComboBox_GetCurSel(hwndCombo);

            if (ComboBoxEx_GetItem(hwndCombo, &cbxi))
            {
                HWND hwndList = GetDlgItem(hwnd, IDC_LIST_CLIPS);

                Clipbook_DeleteList(hwndList);
                Clipbook_ReadFile(hwnd, (PECPCLIPBOOK)cbxi.lParam);
            }
            else
            {
                MessageBox(g_hwndMain, _T("Damnit.\nThe file couldn't be reloaded for some weird reason.\nSorry."), _T("Ladidadida"), MB_ICONEXCLAMATION | MB_OK);
            }
        }
        break;
    }
}

static void Clipbook_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    if (state != SIZE_MINIMIZED)
    {
        RECT rcCombo;
        HWND hwndCombo = GetDlgItem(hwnd, IDC_COMBO_CLIPFILES);
        HWND hwndList = GetDlgItem(hwnd, IDC_LIST_CLIPS);
        HDWP hdwp = BeginDeferWindowPos(2);

        if (cx == -1 || cy == -1)
        {
            RECT rc;

            GetClientRect(hwnd, &rc);

            cx = Rect_Width(&rc);
            cy = Rect_Height(&rc);
        }

        hdwp = DeferWindowPos(hdwp, hwndCombo, 0, 1, 1,
                        (cx - 2), 160, SWP_NOZORDER);
        
        GetClientRect(hwndCombo, &rcCombo);
        
        hdwp = DeferWindowPos(hdwp, hwndList, 0, 1, (rcCombo.bottom + 2),
                        (cx - 2), cy - (rcCombo.bottom + 2) - 1,
                        SWP_NOZORDER);

        EndDeferWindowPos(hdwp);
    }
}

static void Clipbook_OnWindowPosChanged(HWND hwnd, const LPWINDOWPOS lpwpos)
{
    RECT rc;
    HWND hwndParent;

    hwndParent = BandCtrl_GetWindowFromClient(hwnd);

    if (!IsWindow(hwndParent))
        return;

    GetWindowRect(hwndParent, &rc);

    g_WindowSettings.nClipbookXPos      = rc.left;
    g_WindowSettings.nClipbookYPos      = rc.top;
    g_WindowSettings.nClipbookWidth     = Rect_Width(&rc);
    g_WindowSettings.nClipbookHeight    = Rect_Height(&rc);
    g_WindowSettings.nClipbookAlignment = BandCtrl_GetAlignment(hwndParent);
}

static void Clipbook_OnFontChange(HWND hwnd, const LPLOGFONT lplf)
{
    static HFONT hFont = NULL;
    
    if (hFont != NULL)
        DeleteObject(hFont);

    hFont = CreateFontIndirect(lplf);

    SendDlgItemMessage(hwnd, IDC_LIST_CLIPS, WM_SETFONT, (WPARAM)hFont,
                        MAKELPARAM(TRUE, 0));
}

static void Clipbook_OnDestroy(HWND hwnd)
{
    HWND hwndCombo = GetDlgItem(hwnd, IDC_COMBO_CLIPFILES);
    COMBOBOXEXITEM cbxi;
    int i = 0;

    cbxi.mask       = CBEIF_LPARAM;
    cbxi.iItem      = i;

    while (ComboBoxEx_GetItem(hwndCombo, &cbxi))
    {
        PECPCLIPBOOK pClipbook = (PECPCLIPBOOK)cbxi.lParam;
        
        Mem_Free(pClipbook->pszTitle);
        Mem_Free(pClipbook);

        cbxi.iItem  = ++i;
    }
}

void Clipbook_AddClipFiles(HWND hwndCombo)
{
    TCHAR szClipFilesPath[MAX_PATH];
    HANDLE hSearch;
    COMBOBOXEXITEM cbxi;
    WIN32_FIND_DATA wfd;

    cbxi.mask       = CBEIF_LPARAM | CBEIF_TEXT;
    cbxi.iItem      = -1;

    GetModuleFileName(g_hInstance, szClipFilesPath, MAX_PATH);

    Path_RemoveFileName(szClipFilesPath);
    _tcscat(szClipFilesPath, _T("ClipFiles\\*.*"));

    hSearch = FindFirstFile(szClipFilesPath, &wfd);

    if (hSearch != INVALID_HANDLE_VALUE)
    {
        Path_RemoveFileName(szClipFilesPath);

        do
        {
            if (wfd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
                continue;

            if (String_Equal(Path_GetExt(wfd.cFileName), _T("ecp"), FALSE))
            {
                PECPCLIPBOOK pcb = (PECPCLIPBOOK)Mem_Alloc(sizeof(ECPCLIPBOOK));

                _tcscat(szClipFilesPath, wfd.cFileName);

                ECP_GetClipbook(szClipFilesPath, pcb);

                cbxi.pszText    = pcb->pszTitle;
                cbxi.lParam     = (LPARAM)pcb;

                ComboBoxEx_InsertItem(hwndCombo, &cbxi);

                Path_RemoveFileName(szClipFilesPath);
            }
        } while (FindNextFile(hSearch, &wfd));
    }

    FindClose(hSearch);
}

BOOL Clipbook_ReadFile(HWND hwndOwner, PECPCLIPBOOK pClipbook)
{
    DWORD dwTime;
    TCHAR szText[SB_MAX_TEXT];
    HWND hwndList;

    dwTime = GetTickCount();

    hwndList = Clipbook_CreateListWindow(hwndOwner, GetDlgItem(hwndOwner, IDC_LIST_CLIPS), pClipbook);
    SetWindowRedraw(hwndList, FALSE);

    if (pClipbook != NULL)
        ECP_GetItems(hwndList, pClipbook);

    dwTime = GetTickCount() - dwTime;

    _stprintf(szText, String_LoadString(IDS_FORMAT_ECP_LOADTIMES), dwTime);

    if (IsWindow(g_hwndStatusbarMain))
        Statusbar_SetPaneText(g_hwndStatusbarMain, 0, szText);

    SetWindowRedraw(hwndList, TRUE);
    RedrawWindow(hwndList, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);

    return (TRUE);
}

void Clipbook_DeleteList(HWND hwndList)
{
    int i = ListBox_GetCount(hwndList) - 1;

    SetWindowRedraw(hwndList, FALSE);

    for (i; i >= 0; i--)
    {
        PECPITEM pItem = (PECPITEM)ListBox_GetItemData(hwndList, i);

        Mem_Free(pItem->pszBody);
        Mem_Free(pItem);

        ListBox_DeleteString(hwndList, i);
    }

    SetWindowRedraw(hwndList, TRUE);
}

HWND Clipbook_CreateListWindow(HWND hwndOwner, HWND hwndOldList, PECPCLIPBOOK pClipbook)
{
    RECT r, rCbo;
    UINT uSorted;
    HWND hwndList;


    if (pClipbook == NULL)
        uSorted = 0;
    else
        uSorted = (pClipbook->bSort) ? LBS_SORT : 0;

    if (hwndOldList != NULL)
        DestroyWindow(hwndOldList);

    GetClientRect(hwndOwner, &r);
    GetClientRect(GetDlgItem(hwndOwner, IDC_COMBO_CLIPFILES), &rCbo);

    hwndList = CreateWindowEx(WS_EX_CLIENTEDGE, _T("LISTBOX"), NULL,
                        WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
                        WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                        LBS_NOTIFY | LBS_USETABSTOPS | LBS_NOINTEGRALHEIGHT | uSorted,
                        1, rCbo.bottom + 2, r.right - 2, (r.bottom - rCbo.bottom - 2),
                        hwndOwner, (HMENU)IDC_LIST_CLIPS, g_hInstance, NULL);

    Window_SetFont(hwndList, Font_GetFont(), TRUE);

    lpfnDefListWndProc = SubclassWindow(hwndList, Clipbook_List_WndProc);

    return (hwndList);
}

LRESULT CALLBACK Clipbook_Cbo_Cbo_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CONTEXTMENU:
        {
            HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_CLIPMENU));
            HMENU hSubMenu = GetSubMenu(hMenu, 0);
            RECT rc;

            if (lParam == -1) // Context menu
            {
                int nCaretIndex = ListBox_GetCaretIndex(hwnd);
                int nItemHeight = ListBox_GetItemHeight(hwnd, nCaretIndex);

                ListBox_GetItemRect(hwnd, nCaretIndex, &rc);
                OffsetRect(&rc, 10, nItemHeight);
                Window_ClientToScreenRect(hwnd, &rc);
            }
            else // Right Mouse Button
            {
                POINT pt;

                POINTSTOPOINT(pt, lParam);

                rc.left = pt.x;
                rc.top  = pt.y;
            }

            Menu_TrackPopupMenu(hSubMenu, rc.left, rc.top, GetParent(hwnd));
            DestroyMenu(hMenu);
        }
        return (0);
        case WM_KEYDOWN:
            if (wParam == VK_LEFT || wParam == VK_RIGHT || wParam == VK_TAB)
            {
                SetFocus(GetDlgItem(GetParent(hwnd), IDC_LIST_CLIPS));

                return (0);
            }
        break;
    }

    return (CallWindowProc(lpfnDefCboCboWndProc, hwnd, uMsg, wParam, lParam));
}

LRESULT CALLBACK Clipbook_List_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CONTEXTMENU:
        {
            HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_CLIPMENU));
            HMENU hSubMenu = GetSubMenu(hMenu, 0);
            RECT rc;

            if (lParam == -1) // Context menu
            {
                int nCaretIndex = ListBox_GetCaretIndex(hwnd);
                int nItemHeight = ListBox_GetItemHeight(hwnd, nCaretIndex);

                ListBox_GetItemRect(hwnd, nCaretIndex, &rc);
                OffsetRect(&rc, 10, nItemHeight);
                Window_ClientToScreenRect(hwnd, &rc);
            }
            else // Right Mouse Button
            {
                POINT pt;

                POINTSTOPOINT(pt, lParam);

                rc.left = pt.x;
                rc.top  = pt.y;
            }

            Menu_TrackPopupMenu(hSubMenu, rc.left, rc.top, hwnd);
            DestroyMenu(hMenu);
        }
        return (0);
        case WM_KEYDOWN:
            if (wParam == VK_LEFT || wParam == VK_RIGHT || wParam == VK_TAB)
            {
                SetFocus(GetDlgItem(GetParent(hwnd), IDC_COMBO_CLIPFILES));

                return (0);
            }
        break;
        case WM_CHAR:
            if (wParam == VK_RETURN)
            {
                int iIndex = ListBox_GetCurSel(hwnd);
                PECPITEM pItem;

                if (iIndex == LB_ERR)
                    break;

                pItem = (PECPITEM)ListBox_GetItemData(hwnd, iIndex);

                ECP_ProcessItem(pItem, FALSE);

                return (0);
            }
            else if (wParam == VK_ESCAPE)
            {
                SetFocus(MDI_MyGetActive(TRUE));

                return (0);
            }
        break;
    }

    return (CallWindowProc(lpfnDefListWndProc, hwnd, uMsg, wParam, lParam));
}
