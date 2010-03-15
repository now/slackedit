/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : dlg_favorites.c
 * Created    : not known (before 07/22/99)
 * Owner      : pcppopper
 * Revised on : 07/01/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

/*****************************************************************
 * Includes
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* windows */
#include <commctrl.h>
#include <shlobj.h>

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "../settings/settings.h"
#include "../parsing/file_actions.h"

/* pcp_generic */
#include <pcp_browse.h>
#include <pcp_string.h>
#include <pcp_window.h>

/* pcp_controls */
#include <pcp_menu.h>
#include <pcp_mru.h>
#include <pcp_path.h>
#include <pcp_listview.h>

/* pcp_edit */
#include <pcp_edit.h>

/*****************************************************************
 * Type Definitions
 *****************************************************************/

typedef enum tagGETSELOPERATION
{
    GSO_OPEN,
    GSO_GETFILENAME,
    GSO_REMOVE
} GETSELOPERATION;

/*****************************************************************
 * Function Definitions
 *****************************************************************/

BOOL CALLBACK Favorites_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL Favorites_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void Favorites_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode);
static void Favorites_OnClose(HWND hwnd);
static void Favorites_OnDropFiles(HWND hwnd, HDROP hDropInfo);
static LRESULT Favorites_OnNotify(HWND hwnd, int uID, LPNMHDR pnmh);

LRESULT CALLBACK Favorites_ListView_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void Favorites_UpdateListViewImageList(HWND hwndList);
LPTSTR Favorites_GetSelectedFile(HWND hwndList, BOOL bOpen);
BOOL Favorites_AddFile(HWND hwnd, LPTSTR pszFileName);

/*****************************************************************
 * Global Variables
 *****************************************************************/

WNDPROC lpfnDefListViewWndProc;

/*****************************************************************
 * Function Implementations
 *****************************************************************/

/* Dialog_Favorites_Create() [external]
 *
 * Called to create the advanced open dialog box. Since the dialog
 * is modal, there is no return value.
 */
void Dialog_Favorites_Create(void)
{
    DialogBox(Main_GetInstance(), MAKEINTRESOURCE(IDD_FAVORITES), Main_GetWindow(), Dialog_Favorites_DlgProc);
}

/*
 * Dialog_Favorites_DlgProc() [internal]
 *
 * The DlgProc of the Favorite dialog. Called by windows.
 */
BOOL CALLBACK Favorites_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_DLG_MSG(hwndDlg, WM_INITDIALOG,  Favorite_OnInitDialog);
        HANDLE_DLG_MSG(hwndDlg, WM_COMMAND,     Favorite_OnCommand);
        HANDLE_DLG_MSG(hwndDlg, WM_CLOSE,       Favorite_OnClose);
        HANDLE_DLG_MSG(hwndDlg, WM_DROPFILES,   Favorite_OnDropFiles);
        HANDLE_DLG_MSG(hwndDlg, WM_NOTIFY,      Dialog_Favorites_OnNotify);
    }

    return (FALSE);
}

/*
 * Favorites_OnInitDialog() [internal]
 *
 * Handler for the WM_INITDIALOG message. Will return TRUE to
 * allow the creation of the dialog and FALSE otherwise.
 * Does some general set-up of the dialog, reading MRU entries and
 * setting some proper input limits.
 */
static BOOL Favorites_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HWND hwndList = GetDlgItem(hwnd, IDC_LIST_FILES);
    int i;
    LV_COLUMN lvc;
    static struct
    {
        LPTSTR  pszText;
        int     cx;
        int     fmt;
    } column[] = {
        {   _T("File Name"),        120,    0 },
        {   _T("File Path"),        420,    0 },
    };

    lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt     = LVCFMT_LEFT;

    for (i = 0; i < sizeof column/sizeof column[0]; i++)
    {
        lvc.iSubItem    = i;
        lvc.cx          = column[i].cx;
        lvc.pszText     = column[i].pszText;
        ListView_InsertColumn(hwndList, i, &lvc);
    }

    Window_SetBothIcons(hwnd, ImageList_ExtractIcon(NULL, Menu_GetImageList(TRUE), 2));

    MRU_SetListView(hwndList, _T("Favorite Files"), g_MRUSettings.nFavoritesMax);

    Favorite_UpdateListViewImageList(hwndList);

    if (g_WindowSettings.bFavoritesCentered)
        Window_CenterWindow(hwnd);
    else
        SetWindowPos(hwnd, 0, g_WindowSettings.nFavoritesXPos, g_WindowSettings.nFavoritesYPos,
                0, 0, SWP_NOZORDER | SWP_NOSIZE);

    lpfnDefWndProc = SubclassWindow(hwndList, ListView_WndProc);

    return (TRUE);
}

static void Favorite_OnClose(HWND hwnd)
{
    RECT r;
    GetWindowRect(hwnd, &r);

    g_WindowSettings.nFavoritesXPos = r.left;
    g_WindowSettings.nFavoritesYPos = r.top;

    EndDialog(hwnd, 0);
}

static void Favorite_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode)
{
    switch (idCtl)
    {
        case IDOK:
            Favorite_GetSelectedFile(GetDlgItem(hwnd, IDC_LIST_FILES), GSO_OPEN);
        break;
        case IDC_BUTTON_ADDFILE:
        {
            //could contain the current selection
            TCHAR szFileName[MAX_PATH] = _T("");
            BROWSEINFOEX bix;
            LPTSTR pszSelFile = Favorite_GetSelectedFile(GetDlgItem(hwnd, IDC_LIST_FILES), GSO_GETFILENAME);

            if (pszSelFile != NULL)
                _tcscpy(szFileName, pszSelFile);

            INITSTRUCT(bix, FALSE);
            bix.hwndOwner   = hwnd;
            bix.pszFile = szFileName;
            bix.pszTitle    = _T("Browse for your favorite file");
            bix.pszFilter   = String_MakeFilter(g_FilterSettings.szOpenFilter);

            if (Browse_ForFile(&bix))
                Favorite_AddFile(hwnd, szFileName);
        }
        break;
        case IDC_BUTTON_ADDACTIVE:
        {
            FILEINFO fi;

            Main_GetFileInfo(NULL, &fi);

            if (!(fi.dwFileType & FI_TYPE_MASK_EDIT))
                Favorite_AddFile(hwnd, fi.szFileName);
            else
                MessageBox(g_hwndMain, _T("Sorry, but you can't add an unsaved file to your favorites."), _T("Are you reading this? no? ok, then..."), MB_OK | MB_ICONEXCLAMATION);
        }
        break;
        case IDC_REMOVE:
            Favorite_GetSelectedFile(GetDlgItem(hwnd, IDC_LIST_FILES), GSO_REMOVE);
        break;
        case IDCANCEL:
            Window_Close(hwnd);
        break;
    }
}

static void Favorite_OnDropFiles(HWND hwnd, HDROP hdrop)
{
    TCHAR szDraggedFile[MAX_PATH];
    int i;

    for (i = 0; DragQueryFile(hdrop, i, szDraggedFile, MAX_PATH); i++)
        Favorite_AddFile(hwnd, szDraggedFile);

    DragFinish(hdrop);
}

static LRESULT Main_OnNotify(HWND hwnd, int uID, LPNMHDR pnmh)
{
    switch (uID)
    {
    case IDC_LIST_FILES:
    {
        LPNMHDR lpnmhdr = (LPNMHDR)lParam;
        switch (lpnmhdr->code)
        {
            case LVN_ITEMCHANGED:
            {
                LV_ITEM lvi;
                TCHAR szFile[MAX_PATH];
                HWND hwndList   = GetDlgItem(hwndDlg, IDC_LIST_FILES);
                int iIndex      = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);

                if (iIndex != -1)
                {
                    lvi.mask        = LVIF_TEXT;
                    lvi.iItem       = iIndex;
                    lvi.iSubItem    = 1;
                    lvi.pszText     = szFile;
                    lvi.cchTextMax  = MAX_PATH;

                    ListView_GetItem(hwndList, &lvi);

                    SetDlgItemText(hwndDlg, IDC_EDIT_FILENAME, szFile);
                }
            }
            break;
        }
    }
    break;
    }
}

BOOL Favorite_AddFile(HWND hwnd, LPTSTR pszFileName)
{
    LV_ITEM lvi;
    HWND hwndList   = GetDlgItem(hwnd, IDC_LIST_FILES);

    lvi.mask        = LVIF_TEXT;
    lvi.iItem       = ListView_GetItemCount(hwndList);
    lvi.iSubItem    = 0;
    lvi.pszText     = Path_GetFileName(pszFileName);

    if (ListView_InsertUniqueItem(hwndList, &lvi) == -1)
    {
        MessageBox(hwnd, _T("You can only add files that are not already in the list"), _T("Oops"), MB_OK);

        return (FALSE);
    }

    lvi.iSubItem    = 1;
    lvi.pszText     = pszFileName;

    ListView_SetItem(hwndList, &lvi);

    Favorite_UpdateListViewImageList(hwndList);

    MRU_Write(_T("Favorite Files"), pszFileName, g_MRUSettings.nFavoritesMax);

    return (TRUE);
}

void Favorite_UpdateListViewImageList(HWND hwndList)
{
    HIMAGELIST hImageList;
    SHFILEINFO shfi;
    LV_ITEM lvi;
    TCHAR szFileName[MAX_PATH];
    int i, iListCount;

    iListCount = ListView_GetItemCount(hwndList);

    if ((hImageList = ListView_GetImageList(hwndList, LVSIL_SMALL)) != NULL)
        ImageList_Destroy(hImageList);
    
    hImageList = ImageList_Create(16, 16, ILC_COLOR16 | ILC_MASK, iListCount, 0);
    
    ListView_SetImageList(hwndList, hImageList, LVSIL_SMALL);

    for (i = 0; i < iListCount; i++)
    {
        lvi.mask        = LVIF_TEXT;
        lvi.iItem       = i;
        lvi.iSubItem    = 1;
        lvi.pszText     = szFileName;
        lvi.cchTextMax  = MAX_PATH;
        
        ListView_GetItem(hwndList, &lvi);

        SHGetFileInfo(szFileName, 0, &shfi, sizeof(shfi),
                        SHGFI_ICON | SHGFI_SMALLICON);

        if (shfi.hIcon == NULL)
            shfi.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_CANCEL));

        ImageList_AddIcon(hImageList, shfi.hIcon);

        lvi.mask        = LVIF_IMAGE;
        lvi.iImage      = i;
        lvi.iSubItem    = 0;

        ListView_SetItem(hwndList, &lvi);
    }
}

LRESULT CALLBACK ListView_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_LBUTTONDBLCLK:
            Favorite_GetSelectedFile(hwnd, GSO_OPEN);
        return (0);
        case WM_CONTEXTMENU:
        {
            HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_FAVORITESMENU));
            HMENU hMenuPopup = GetSubMenu(hMenu, 0);
            RECT rc;

            if (lParam == -1)
            {
                int nCaretIndex = ListBox_GetCaretIndex(hwnd);
                int nItemHeight = ListBox_GetItemHeight(hwnd, nCaretIndex);

                ListBox_GetItemRect(hwnd, nCaretIndex, &rc);
                Window_ClientToScreenRect(hwnd, &rc);
                OffsetRect(&rc, 10, nItemHeight);
            }
            else
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
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDM_FAVORITESMENU_OPEN:
                    Favorite_GetSelectedFile(hwnd, GSO_OPEN);
                return (0);
                case IDM_FAVORITESMENU_REMOVE:
                    Favorite_GetSelectedFile(hwnd, GSO_REMOVE);
                return (0);
            }
        break;
        case WM_DESTROY:
        return (0);
    }

    return (CallWindowProc(lpfnDefWndProc, hwnd, uMsg, wParam, lParam));
}

LPTSTR Favorite_GetSelectedFile(HWND hwndList, GETSELOPERATION gso)
{
    LV_ITEM lvi;
    static TCHAR szFile[MAX_PATH];
    int iIndex      = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);

    if (iIndex != -1)
    {
        lvi.mask        = LVIF_TEXT;
        lvi.iItem       = iIndex;
        lvi.iSubItem    = 1;
        lvi.pszText     = szFile;
        lvi.cchTextMax  = MAX_PATH;

        ListView_GetItem(hwndList, &lvi);

        if (gso == GSO_OPEN)
        {
            File_Open(szFile, CRLF_STYLE_AUTOMATIC);

            return (szFile);
        }
        else if (gso == GSO_GETFILENAME)
        {
            return (szFile);
        }
        else if (gso == GSO_REMOVE)
        {
            CREATEMRULIST cml;
            HANDLE hList;
            int nItemPos;

            INITSTRUCT(cml, TRUE);
            cml.nMaxItems   = g_MRUSettings.nFavoritesMax;
            cml.dwFlags     = MRUF_STRING_LIST;
            cml.hKey        = MRU_GetRoot();
            cml.lpszSubKey  = _T("Favorite Files");
            cml.lpfnCompare = NULL;

            hList = CreateMRUList(&cml);

            nItemPos = FindMRUString(hList, szFile, NULL);

            if (nItemPos != -1)
            {
                DelMRUString(hList, nItemPos);
                ListView_DeleteItem(hwndList, iIndex);
            }

            FreeMRUList(hList);

            return (szFile);
        }
    }
    else if (gso != GSO_GETFILENAME)
    {
            MessageBox(hwndList, _T("You must first choose a file in the list and then press open"),
                    _T("Oops"), MB_OK | MB_ICONEXCLAMATION);
    }

    return (NULL);
}
