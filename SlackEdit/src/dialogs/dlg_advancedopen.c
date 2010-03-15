/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : dlg_advancedopen.c
 * Created    : not known (before 08/18/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:11:00
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

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "../file_handling/file_interface.h"
#include "../settings/settings.h"

/* pcp_generic */
#include <pcp_string.h>
#include <pcp_window.h>
#include <pcp_mru.h>
#include <pcp_browse.h>
#include <pcp_path.h>

/* pcp_edit */
#include <pcp_textedit.h>

/*****************************************************************
 * Type Definitions
 *****************************************************************/

/*****************************************************************
 * Function Definitions
 *****************************************************************/

BOOL CALLBACK AdvancedOpen_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL AdvancedOpen_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void AdvancedOpen_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode);
static void AdvancedOpen_OnClose(HWND hwnd);
static void AdvancedOpen_OnDropFiles(HWND hwnd, HDROP hDropInfo);

/*****************************************************************
 * Global Variables
 *****************************************************************/

/*****************************************************************
 * Function Implementations
 *****************************************************************/

/*
 * Dialog_AdvancedOpen_Create() [external]
 *
 * Called to create the advanced open dialog box. Since the dialog
 * is modal, there is no return value.
 */
void Dialog_AdvancedOpen_Create(void)
{
    DialogBox(Main_GetInstance(), MAKEINTRESOURCE(IDD_ADVANCEDOPEN), Main_GetWindow(), AdvancedOpen_DlgProc);
}

/*
 * AdvancedOpen_DlgProc() [internal]
 *
 * The DlgProc of the AdvancedOpen dialog. Called by windows.
 */
BOOL CALLBACK AdvancedOpen_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_DLG_MSG(hwndDlg, WM_INITDIALOG,  AdvancedOpen_OnInitDialog);
        HANDLE_DLG_MSG(hwndDlg, WM_COMMAND,     AdvancedOpen_OnCommand);
        HANDLE_DLG_MSG(hwndDlg, WM_DROPFILES,   AdvancedOpen_OnDropFiles);
        HANDLE_DLG_MSG(hwndDlg, WM_CLOSE,       AdvancedOpen_OnClose);
    }

    return (FALSE);
}

/*
 * AdvancedOpen_OnInitDialog() [internal]
 *
 * Handler for the WM_INITDIALOG message. Will return TRUE to
 * allow the creation of the dialog and FALSE otherwise.
 * Does some general set-up of the dialog, reading MRU entries and
 * setting some proper input limits.
 */
static BOOL AdvancedOpen_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HWND hwndDirectoryCombo = GetDlgItem(hwnd, IDC_COMBO_PATH);
    HWND hwndFileCombo      = GetDlgItem(hwnd, IDC_COMBO_FILE);
    LPFILEVIEW lpFileView;

    ASSERT(hwndDirectoryCombo != NULL && hwndFileCombo != NULL);

    /* set the proper limits to the combo boxes input */
    ComboBox_LimitText(hwndDirectoryCombo, MAX_PATH);
    ComboBox_LimitText(hwndFileCombo, MAX_PATH);

    /* retrieve and set the MRU info for the combo boxes */
    MRU_SetComboBox(hwndDirectoryCombo, _T("Advanced Open Directory MRU"), g_MRUSettings.nAdvancedOpenDirMax);
    MRU_SetComboBox(hwndFileCombo, _T("Advanced Open File MRU"), g_MRUSettings.nAdvancedOpenFileMax);

    lpFileView = FileInterface_GetFileView(MDI_GetActive(MDIClient_GetWindow(), NULL), FILE_FIND_BY_VIEW_HWND);

    /* set up some info in the combo boxes */
    if (FILEISSOMETHING(lpFileView->dwFileType))
    {
        TCHAR szFileName[MAX_PATH];

        _tcscpy(szFileName, lpFileView->pszFileName);

        SetWindowText(hwndFileCombo, Path_GetFileName(szFileName));
        SetWindowText(hwndDirectoryCombo, Path_RemoveFileName(szFileName));
    }
    else
    {
        if (ComboBox_GetCount(hwndDirectoryCombo) > 0)
        {
            /* set the combo boxes text to the last used item (index 0) */
            ComboBox_SetCurSel(hwndDirectoryCombo, 0);
        }
        else
        {
            TCHAR szCurrentPath[MAX_PATH];

            /* set the directory to the current directory */
            GetCurrentDirectory(MAX_PATH, (LPTSTR)&szCurrentPath);
            _tcscat(szCurrentPath, _T("\\"));
            SetWindowText(hwndDirectoryCombo, szCurrentPath);
        }
    }

    /* adjust the dialogs position */
    if (g_WindowSettings.bAdvancedOpenCentered)
        Window_CenterWindow(hwnd);
    else
        SetWindowPos(hwnd, 0, g_WindowSettings.nAdvancedOpenXPos, g_WindowSettings.nAdvancedOpenYPos,
                0, 0, SWP_NOZORDER | SWP_NOSIZE);

    return (TRUE);
}

/*
 * AdvancedOpen_OnClose() [internal]
 *
 * Handler for the WM_CLOSE message.
 * Saves some dialog position info then destroys the dialog.
 */
static void AdvancedOpen_OnClose(HWND hwnd)
{
    RECT rc;

    GetWindowRect(hwnd, &r);

    g_WindowSettings.nAdvancedOpenXPos = rc.left;
    g_WindowSettings.nAdvancedOpenYPos = rc.top;

    EndDialog(hwnd, 0);
}

/*
 * AdvancedOpen_OnDropFiles() [internal]
 *
 * Handler for the WM_DROPFILES message.
 * Sets some info in the combo boxes corresponding to the info
 * passed by the WM_DROPFILES message.
 */
static void AdvancedOpen_OnDropFiles(HWND hwnd, HDROP hDropInfo)
{
    TCHAR szDraggedFile[MAX_PATH];

    /* retieve and set info */
    if (DragQueryFile(hDropInfo, 0, szDraggedFile, MAX_PATH))
    {
        SetDlgItemText(hwnd, IDC_COMBO_FILE, Path_GetFileName(szDraggedFile));
        SetDlgItemText(hwnd, IDC_COMBO_PATH, Path_RemoveFileName(szDraggedFile));
    }

    /* clean  up */
    DragFinish(hDropInfo);
}

/*
 * AdvancedOpen_OnCommand() [internal]
 *
 * Handler for the WM_COMMAND message.
 * Handles messages sent from four buttons,
 * IDC_BUTTON_PATHBROWSE: user wants to set the path,
 * IDC_BUTTON_FILEBROWSE: user wants to set the file,
 * IDOK: user wants to open the file in the path set,
 * IDCANCEL: user wants to cancel the dialog.
 */
static void PO_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode)
{
    switch (idCtl)
    {
    case IDC_BUTTON_PATHBROWSE:
    {
        BROWSEINFOEX bix;
        TCHAR szPath[MAX_PATH] = _T("");
        TCHAR szStartPath[MAX_PATH] = _T("");

        GetDlgItemText(hwnd, IDC_COMBO_PATH, szStartPath, MAX_PATH);

        INITSTRUCT(bix, FALSE);
        bix.hwndOwner       = hwnd;
        bix.pszPath         = szPath;
        bix.pszStartPath    = szStartPath;

        if (Browse_ForPath(&bix))
        {
            /* add a backslash if there isn't one on the end */
            Path_VerifyBackslash(szPath, TRUE);
            SetDlgItemText(hwnd, IDC_COMBO_PATH, szPath);
        }
    }
    break;
    case IDC_BUTTON_FILEBROWSE:
    {
        BROWSEINFOEX bix;
        TCHAR szPath[MAX_PATH] = _T("");
        TCHAR szFile[MAX_PATH] = _T("");

        GetDlgItemText(hwnd, IDC_COMBO_PATH, szPath, MAX_PATH);

        INITSTRUCT(bix, FALSE);
        bix.hwndOwner   = hwnd;
        bix.pszPath     = szPath;
        bix.pszFile     = szFile;
        bix.pszFilter   = g_FilterSettings.szOpenFilter;

        if (Browse_ForFile(&bix))
        {
            SetDlgItemText(hwnd, IDC_COMBO_FILE, Path_GetFileName(szFile));
            SetDlgItemText(hwnd, IDC_COMBO_PATH, Path_RemoveFileName(szFile));
        }
    }
    break;
    case IDOK:
    {
        TCHAR szPath[MAX_PATH], szFile[MAX_PATH];

        GetDlgItemText(hwnd, IDC_COMBO_PATH, szPath, MAX_PATH);
        GetDlgItemText(hwnd, IDC_COMBO_FILE, szFile, MAX_PATH);

        Path_VerifyBackslash(szPath, TRUE);

        _tcsncat(szPath, szFile, MAX_PATH);

        if (Path_FileExists(szPath))
        {
            OPENSAVEFILEINFO osfi;

            /* FIXME: this should be adjustable */
            osfi.dwFileType     = FILE_TYPE_TEXT;
            osfi.nLineFeedMode  = CRLF_STYLE_AUTOMATIC;
            FileInterface_OpenFile(szPath, &osfi);
        }
        else
        {
            MessageBox(hwnd, _T("Invalid path specified!\n\n")
                _T("Tip of the Day: Change the path to one that actually exists.\n")
                _T("It is allowed to specify a new file but we don't want new\n")
                _T("directories now do we?\n")
                _T("(If so then please tell us and we'll do something about it.)\n")
                _T("(Actually right now you can create a new file either but...)"),
                _T("File Not Found!"),
                MB_OK | MB_ICONEXCLAMATION);

            break;
        }

        if (IsDlgButtonChecked(hwnd, IDC_CHECK_FAVORITEFILEADD))
            MRU_Write(_T("Favorite Files"), szPath, g_MRUSettings.nFavoritesMax);

        /* write MRU entries */
        MRU_Write(_T("Advanced File Directory MRU"), Path_GetFileName(szFile), g_MRUSettings.nAdvancedOpenFileMax);
        MRU_Write(_T("Advanced Open Directory MRU"), Path_RemoveFileName(szPath), g_MRUSettings.nAdvancedOpenDirMax);

        /* close the dialog */
        Window_Close(hwnd);
    }
    break;
    case IDCANCEL:
        Window_Close(hwnd);
    break;
    }
}
