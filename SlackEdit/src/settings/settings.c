/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : config.c
 * Created    : not known
 * Owner      : pcppopper
 * Revised on : 07/02/00
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
#include "../parsing/file_actions.h"
#include "../rebar/bar_toolbar.h"
#include "../settings/settings.h"

/* pcp_generic */
#include <pcp_browse.h>
#include <pcp_combobox.h>
#include <pcp_listbox.h>
#include <pcp_mem.h>
#include <pcp_mru.h>
#include <pcp_path.h>
#include <pcp_rebar.h>
#include <pcp_registry.h>
#include <pcp_spin.h>
#include <pcp_string.h>
#include <pcp_toolbar.h>
#include <pcp_window.h>

/* pcp_edit */
#include <pcp_edit.h>

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

int CALLBACK Config_PropSheet_DlgProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);
BOOL CALLBACK Config_Generic_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK Config_General_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Config_Edit_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Config_MRU_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Config_GUI_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Config_Filter_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Config_Associations_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Config_Scripts_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Config_Syntax_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static BOOL LoadGuiBitmap(HWND hwndDlg, UINT idCtl, LPTSTR szFileName);

BOOL CALLBACK Config_ResetEditControlsEnumProc(HWND hwnd, LPARAM lParam);

static void Config_RegisterFileExtension(LPFILETYPEDATA lpftd);
static void Config_GetRegisteredFileExtensions(HWND hwndList);
static void Config_RemoveFileExtension(LPFILETYPEDATA lpftd);

static void Config_PutFiltersInList(HWND hwndList);
static void Config_PutListInFilters(HWND hwndList);

static UINT uLastTab = 0;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

/****************************************************************
 * Config_Edit
 *
 * returns - nothing
 ****************************************************************/

void Config_Edit(HWND hwndOwner) //add hInstance
{
    PROPSHEETPAGE psp[7];
    PROPSHEETHEADER psh;
    int i;

    for (i = 0; i < DIMOF(psp); i++)
    {
        INITSTRUCT(psp[i], TRUE);
        psp[i].hInstance    = g_hInstance;
    }

    psp[0].pszTemplate  = MAKEINTRESOURCE(IDD_CONFIG_WINDOW);
    psp[0].pfnDlgProc   = Config_General_DlgProc;

    psp[1].pszTemplate  = MAKEINTRESOURCE(IDD_CONFIG_EDIT);
    psp[1].pfnDlgProc   = Config_Edit_DlgProc;

    psp[2].pszTemplate  = MAKEINTRESOURCE(IDD_CONFIG_MRU);
    psp[2].pfnDlgProc   = Config_MRU_DlgProc;

    psp[3].pszTemplate  = MAKEINTRESOURCE(IDD_CONFIG_FILTER);
    psp[3].pfnDlgProc   = Config_Filter_DlgProc;

    psp[4].pszTemplate  = MAKEINTRESOURCE(IDD_CONFIG_GUI);
    psp[4].pfnDlgProc   = Config_GUI_DlgProc;

    psp[5].pszTemplate  = MAKEINTRESOURCE(IDD_CONFIG_ASSOCIATIONS);
    psp[5].pfnDlgProc   = Config_Associations_DlgProc;

    psp[6].pszTemplate  = MAKEINTRESOURCE(IDD_CONFIG_SCRIPTS);
    psp[6].pfnDlgProc   = Config_Scripts_DlgProc;

//  psp[7].pszTemplate  = MAKEINTRESOURCE(IDD_CONFIG_SYNTAX);
//  psp[7].pfnDlgProc   = Config_Syntax_DlgProc;

    INITSTRUCT(psh, TRUE);
    psh.dwFlags         = PSH_PROPSHEETPAGE | PSH_USECALLBACK;
    psh.hwndParent      = hwndOwner;
    psh.hInstance       = g_hInstance;
    psh.pszCaption      = String_LoadString(IDS_TITLE_OPTIONSDIALOG);
    psh.nPages          = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage      = uLastTab;
    psh.ppsp            = (LPCPROPSHEETPAGE)&psp;
    psh.pfnCallback     = Config_PropSheet_DlgProc;

    PropertySheet(&psh);

    if (g_WindowSettings.bMainSticky)
        SetWindowLong(hwndOwner, GWL_USERDATA, magicDWord);
    else
        SetWindowLong(hwndOwner, GWL_USERDATA, 0);

    if (g_WindowSettings.bMainOnTop)
        SetWindowPos(hwndOwner, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    else
        SetWindowPos(hwndOwner, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

/****************************************************************
 * Config_PropSheet_DlgProc
 *
 * A callback function used by windows to allow modification of
 * the property sheet
 *
 * returns - TRUE or FALSE, used by windows
 ****************************************************************/

int CALLBACK Config_PropSheet_DlgProc(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
    switch (uMsg)
    {
        case PSCB_PRECREATE:
        {
            DLGTEMPLATE *lpdt = (DLGTEMPLATE *)lParam;

            if (g_WindowSettings.bConfigCentered)
            {
                RECT rWorkArea;

                SystemParametersInfo(SPI_GETWORKAREA, 0, &rWorkArea, 0);

                lpdt->x = (rWorkArea.right / 2) - (lpdt->cx / 2);
                lpdt->y = (rWorkArea.bottom / 2) - (lpdt->cy / 2);
            }
            else
            {
                lpdt->x = g_WindowSettings.nConfigXPos;
                lpdt->y = g_WindowSettings.nConfigYPos;
            }
        }
        break;
    }

    return (FALSE);
}

/****************************************************************
 * Config_Generic_DlgProc
 *
 * This function should be called from each other DlgProc since
 * it does all the generic stuff.
 *
 * returns - TRUE or FALSE, used by windows
 ****************************************************************/

BOOL CALLBACK Config_Generic_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);
    return (TRUE);
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_APPLY:
            EnumChildWindows(hwndDlg, Config_ResetEditControlsEnumProc, 0);
            Config_Write();
            SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
        break;
        case PSN_KILLACTIVE:
            SetWindowLong(hwndDlg, DWL_MSGRESULT, FALSE);
        return (TRUE);
        case PSN_RESET:
            SetWindowLong(hwndDlg, DWL_MSGRESULT, FALSE);
        break;
        }
    break;
    case WM_COMMAND:
    {
        TCHAR szClass[80];

        GetClassName((HWND)lParam, szClass, 80);

        if (String_Equal(szClass, _T("EDIT"), FALSE))
        {
            if (Edit_GetModify((HWND)lParam))
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
        }
        else
        {
            PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
        }
    }
    break;
    case WM_DESTROY:
    {
        RECT rc;

        GetWindowRect(GetParent(hwndDlg), &rc);

        g_WindowSettings.nConfigXPos = rc.left;
        g_WindowSettings.nConfigYPos = rc.top;
    }
    break;
    }

    return (FALSE);
}


/****************************************************************
 * Config_General_DlgProc
 *
 * A callback function used by the General property sheet
 *
 * returns - TRUE or FALSE, used by windows
 ****************************************************************/

BOOL CALLBACK Config_General_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        // Main Window
        CheckDlgButton(hwndDlg, IDC_CHECK_MAIN_ONTOP, g_WindowSettings.bMainOnTop);
        CheckDlgButton(hwndDlg, IDC_CHECK_MAIN_STICKY, g_WindowSettings.bMainSticky);
        CheckDlgButton(hwndDlg, IDC_CHECK_MAIN_CENTERED, g_WindowSettings.bMainCentered);
        // Tray
        CheckDlgButton(hwndDlg, IDC_CHECK_TRAY_ONCLOSE, g_GlobalSettings.fGotoSystray);
        // MDI
        CheckDlgButton(hwndDlg, IDC_CHECK_SAVEOPENWINS, g_GlobalSettings.bSaveOpenWindows);
        CheckDlgButton(hwndDlg, IDC_CHECK_ONLYRELOADIFNOCMDLINE, g_GlobalSettings.bOnlyReloadIfCommandLineEmpty);
        CheckDlgButton(hwndDlg, IDC_CHECK_CRASHPROTECTION, g_GlobalSettings.bCrashProtection);
        // Dialogs
        CheckDlgButton(hwndDlg, IDC_CHECK_FIND_CENTER, g_WindowSettings.bFindCentered);
        CheckDlgButton(hwndDlg, IDC_CHECK_REPLACE_CENTER, g_WindowSettings.bReplaceCentered);
        CheckDlgButton(hwndDlg, IDC_CHECK_ADVANCEDOPEN_CENTER, g_WindowSettings.bAdvancedOpenCentered);
        CheckDlgButton(hwndDlg, IDC_CHECK_FAVORITES_CENTER, g_WindowSettings.bFavoritesCentered);
        CheckDlgButton(hwndDlg, IDC_CHECK_PRINT_CENTER, g_WindowSettings.bPrintCentered);
        CheckDlgButton(hwndDlg, IDC_CHECK_SETTINGS_CENTER, g_WindowSettings.bConfigCentered);
        CheckDlgButton(hwndDlg, IDC_CHECK_ABOUT_CENTER, g_WindowSettings.bAboutCentered);
        CheckDlgButton(hwndDlg, IDC_CHECK_CMDLINE_CENTER, g_WindowSettings.bCmdLineCentered);
        CheckDlgButton(hwndDlg, IDC_CHECK_WINDOWLIST_CENTER, g_WindowSettings.bWindowListCentered);
    break;
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) 
        {
            case PSN_SETACTIVE:
                uLastTab = 0;
            break;
            case PSN_APPLY:
                //Main Window
                g_WindowSettings.bMainOnTop             = IsDlgButtonChecked(hwndDlg, IDC_CHECK_MAIN_ONTOP);
                g_WindowSettings.bMainSticky            = IsDlgButtonChecked(hwndDlg, IDC_CHECK_MAIN_STICKY);
                g_WindowSettings.bMainCentered          = IsDlgButtonChecked(hwndDlg, IDC_CHECK_MAIN_CENTERED);
                //Tray
                g_GlobalSettings.fGotoSystray           = IsDlgButtonChecked(hwndDlg, IDC_CHECK_TRAY_ONCLOSE);
                //MDI
                g_GlobalSettings.bSaveOpenWindows       = IsDlgButtonChecked(hwndDlg, IDC_CHECK_SAVEOPENWINS);
                g_GlobalSettings.bOnlyReloadIfCommandLineEmpty  = IsDlgButtonChecked(hwndDlg, IDC_CHECK_ONLYRELOADIFNOCMDLINE);
                g_GlobalSettings.bCrashProtection       = IsDlgButtonChecked(hwndDlg, IDC_CHECK_CRASHPROTECTION);

                g_WindowSettings.bFindCentered          = IsDlgButtonChecked(hwndDlg, IDC_CHECK_FIND_CENTER);
                g_WindowSettings.bReplaceCentered       = IsDlgButtonChecked(hwndDlg, IDC_CHECK_REPLACE_CENTER);
                g_WindowSettings.bAdvancedOpenCentered  = IsDlgButtonChecked(hwndDlg, IDC_CHECK_ADVANCEDOPEN_CENTER);
                g_WindowSettings.bFavoritesCentered     = IsDlgButtonChecked(hwndDlg, IDC_CHECK_FAVORITES_CENTER);
                g_WindowSettings.bPrintCentered         = IsDlgButtonChecked(hwndDlg, IDC_CHECK_PRINT_CENTER);
                g_WindowSettings.bConfigCentered        = IsDlgButtonChecked(hwndDlg, IDC_CHECK_SETTINGS_CENTER);
                g_WindowSettings.bAboutCentered         = IsDlgButtonChecked(hwndDlg, IDC_CHECK_ABOUT_CENTER);
                g_WindowSettings.bCmdLineCentered       = IsDlgButtonChecked(hwndDlg, IDC_CHECK_CMDLINE_CENTER);
                g_WindowSettings.bWindowListCentered    = IsDlgButtonChecked(hwndDlg, IDC_CHECK_WINDOWLIST_CENTER);
            break;
        }
    break;
    }

    return (CallWindowProc(Config_Generic_DlgProc, hwndDlg, uMsg, wParam, lParam));
}

/****************************************************************
 * Config_Edit_DlgProc
 *
 * A callback function used by the Edit property sheet
 *
 * returns - TRUE or FALSE, used by windows
 ****************************************************************/

BOOL CALLBACK Config_Edit_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        CheckDlgButton(hwndDlg, IDC_CHECK_PASTETOCLIPBOARD, g_EditSettings.bRecentToClipboard);

        Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_ASMS), 60, 0);
        SetDlgItemText(hwndDlg, IDC_EDIT_AUTOSAVEEXT, g_BackupSettings.szAutoSaveExt);
        CheckDlgButton(hwndDlg, IDC_CHECK_AUTOSAVE, g_BackupSettings.bAutoSave);
        SetDlgItemInt(hwndDlg, IDC_EDIT_AUTOSAVEINTERVAL, g_BackupSettings.uAutoSaveInterval, FALSE);
        SetDlgItemText(hwndDlg, IDC_EDIT_AUTOSAVEDIR, g_BackupSettings.szAutoSaveDirectory);
        CheckDlgButton(hwndDlg, IDC_CHECK_USEAUTOSAVEDIR, g_BackupSettings.bUseAutoSaveDirectory);

        SetDlgItemText(hwndDlg, IDC_EDIT_BACKUPEXT, g_BackupSettings.szBackupExt);
        CheckDlgButton(hwndDlg, IDC_CHECK_BACKUPONSAVE, g_BackupSettings.bBackup);
        SetDlgItemText(hwndDlg, IDC_EDIT_BACKUPDIR, g_BackupSettings.szBackupDirectory);
        CheckDlgButton(hwndDlg, IDC_CHECK_USEBACKUPDIR, g_BackupSettings.bUseBackupDirectory);
    break;
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) 
        {
        case PSN_SETACTIVE:
            uLastTab = 1;
        break;
        case PSN_APPLY:
            g_EditSettings.bRecentToClipboard   = IsDlgButtonChecked(hwndDlg,
                                                        IDC_CHECK_PASTETOCLIPBOARD);
            GetDlgItemText(hwndDlg, IDC_EDIT_AUTOSAVEEXT, g_BackupSettings.szAutoSaveExt, _MAX_EXT);
            g_BackupSettings.bAutoSave      = IsDlgButtonChecked(hwndDlg, IDC_CHECK_AUTOSAVE);
            g_BackupSettings.uAutoSaveInterval  = GetDlgItemInt(hwndDlg, IDC_EDIT_AUTOSAVEINTERVAL, NULL, FALSE);
            GetDlgItemText(hwndDlg, IDC_EDIT_AUTOSAVEDIR, g_BackupSettings.szAutoSaveDirectory, MAX_PATH);
            g_BackupSettings.bUseAutoSaveDirectory  = IsDlgButtonChecked(hwndDlg, IDC_CHECK_USEAUTOSAVEDIR);

            GetDlgItemText(hwndDlg, IDC_EDIT_BACKUPEXT, g_BackupSettings.szBackupExt, _MAX_EXT);
            g_BackupSettings.bBackup        = IsDlgButtonChecked(hwndDlg, IDC_CHECK_BACKUPONSAVE);
            GetDlgItemText(hwndDlg, IDC_EDIT_BACKUPDIR, g_BackupSettings.szBackupDirectory, MAX_PATH);
            g_BackupSettings.bUseBackupDirectory    = IsDlgButtonChecked(hwndDlg, IDC_CHECK_USEBACKUPDIR);
        break;
        }
    break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_BROWSEAUTOSAVEDIR:
        {
            BROWSEINFOEX bix;
            TCHAR szPath[MAX_PATH];
            TCHAR szStartPath[MAX_PATH];

            GetDlgItemText(hwndDlg, IDC_EDIT_AUTOSAVEDIR, szStartPath, MAX_PATH);

            INITSTRUCT(bix, FALSE);
            bix.pszPath         = szPath;
            bix.pszStartPath    = szStartPath;
            bix.pszTitle        = _T("Browse for Auto Save Directory");
            bix.hwndOwner       = GetParent(hwndDlg);
        
            if (Browse_ForPath(&bix))
                SetDlgItemText(hwndDlg, IDC_EDIT_AUTOSAVEDIR, szPath);
        }
        break;
        case IDC_BUTTON_BROWSEBACKUPDIR:
        {
            BROWSEINFOEX bix;
            TCHAR szPath[MAX_PATH];
            TCHAR szStartPath[MAX_PATH];

            GetDlgItemText(hwndDlg, IDC_EDIT_BACKUPDIR, szStartPath, MAX_PATH);

            INITSTRUCT(bix, FALSE);
            bix.pszPath         = szPath;
            bix.pszStartPath    = szStartPath;
            bix.pszTitle        = _T("Browse for Backup Directory");
            bix.hwndOwner       = GetParent(hwndDlg);
        
            if (Browse_ForPath(&bix))
                SetDlgItemText(hwndDlg, IDC_EDIT_BACKUPDIR, szPath);
        }
        case IDC_EDIT_AUTOSAVEDIR:
            Window_EnableDlgItem(hwndDlg, IDC_CHECK_USEAUTOSAVEDIR, Window_GetDlgItemTextLength(hwndDlg, IDC_EDIT_AUTOSAVEDIR));

            if (Edit_GetModify(GetDlgItem(hwndDlg, LOWORD(wParam))))
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
        return (FALSE);
        case IDC_EDIT_BACKUPDIR:
            Window_EnableDlgItem(hwndDlg, IDC_CHECK_USEBACKUPDIR, Window_GetDlgItemTextLength(hwndDlg, IDC_EDIT_BACKUPDIR));

            if (Edit_GetModify(GetDlgItem(hwndDlg, LOWORD(wParam))))
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
        return (FALSE);
        case IDC_EDIT_AUTOSAVEEXT:
        case IDC_EDIT_AUTOSAVEINTERVAL:
        case IDC_EDIT_BACKUPEXT:
            if (Edit_GetModify(GetDlgItem(hwndDlg, LOWORD(wParam))))
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
        return (FALSE);
        }
    break;
    }

    return (CallWindowProc(Config_Generic_DlgProc, hwndDlg, uMsg, wParam, lParam));
}

/****************************************************************
 * Config_MRU_DlgProc
 *
 * A callback function used by the MRU property sheet
 *
 * returns - TRUE or FALSE, used by windows
 ****************************************************************/

BOOL CALLBACK Config_MRU_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_RD), 29, 1);
            Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_PD), 29, 1);
            Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_REP), 29, 1);
            Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_POP), 29, 1);
            Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_POF), 29, 1);
            Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_GL), 29, 1);
            Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_FAV), 29, 1);
            Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_DC), 29, 1);
            Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_DW), 29, 1);
            Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_WC), 29, 1);
            Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_WW), 29, 1);
            Spin_SetRange(GetDlgItem(hwndDlg, IDC_SPIN_TC), 29, 1);
            SetDlgItemInt(hwndDlg, IDC_EDIT_MRU_RECENTFILES, min(g_MRUSettings.nFileMax, 29), FALSE);
            SetDlgItemInt(hwndDlg, IDC_EDIT_MRU_FIND,       min(g_MRUSettings.nFindMax, 29), FALSE);
            SetDlgItemInt(hwndDlg, IDC_EDIT_MRU_REPLACE,    min(g_MRUSettings.nReplaceMax, 29), FALSE);
            SetDlgItemInt(hwndDlg, IDC_EDIT_MRU_ADVANCEDOPEN_PATH,      min(g_MRUSettings.nAdvancedOpenDirMax, 29), FALSE);
            SetDlgItemInt(hwndDlg, IDC_EDIT_MRU_ADVANCEDOPEN_FILE,      min(g_MRUSettings.nAdvancedOpenFileMax, 29), FALSE);
            SetDlgItemInt(hwndDlg, IDC_EDIT_MRU_FAVORITE_FILES, min(g_MRUSettings.nFavoritesMax, 29), FALSE);
            SetDlgItemInt(hwndDlg, IDC_EDIT_MRU_GOTO_LINES,     min(g_MRUSettings.nLinesMax, 29), FALSE);
            SetDlgItemInt(hwndDlg, IDC_EDIT_MRU_TCLCMD,     min(g_MRUSettings.nTclCmdMax, 29), FALSE);
        break;
        case WM_NOTIFY:
            switch (((LPNMHDR)lParam)->code) 
            {
            case PSN_SETACTIVE:
                uLastTab = 2;
            break;
            case PSN_APPLY:
            {
                BOOL bSuccess;

                g_MRUSettings.nFileMax          = min(GetDlgItemInt(hwndDlg, IDC_EDIT_MRU_RECENTFILES, &bSuccess, FALSE), 29);
                g_MRUSettings.nFindMax          = min(GetDlgItemInt(hwndDlg, IDC_EDIT_MRU_FIND, &bSuccess, FALSE), 29);
                g_MRUSettings.nReplaceMax       = min(GetDlgItemInt(hwndDlg, IDC_EDIT_MRU_REPLACE, &bSuccess, FALSE), 29);
                g_MRUSettings.nAdvancedOpenDirMax   = min(GetDlgItemInt(hwndDlg, IDC_EDIT_MRU_ADVANCEDOPEN_PATH, &bSuccess, FALSE), 29);
                g_MRUSettings.nAdvancedOpenFileMax  = min(GetDlgItemInt(hwndDlg, IDC_EDIT_MRU_ADVANCEDOPEN_FILE, &bSuccess, FALSE), 29);
                g_MRUSettings.nFavoritesMax     = min(GetDlgItemInt(hwndDlg, IDC_EDIT_MRU_FAVORITE_FILES, &bSuccess, FALSE), 29);
                g_MRUSettings.nLinesMax         = min(GetDlgItemInt(hwndDlg, IDC_EDIT_MRU_GOTO_LINES, &bSuccess, FALSE), 29);
                g_MRUSettings.nTclCmdMax        = min(GetDlgItemInt(hwndDlg, IDC_EDIT_MRU_TCLCMD, &bSuccess, FALSE), 29);

                if (!bSuccess)
                {
                    MB(_T("You need to specify a number for all of the MRU Items"));

                    return (FALSE);
                }
            }
            break;
            }
        break;
        case WM_COMMAND:
            if (Edit_GetModify(GetDlgItem(hwndDlg, LOWORD(wParam))))
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
        return (FALSE);
    }

    return (CallWindowProc(Config_Generic_DlgProc, hwndDlg, uMsg, wParam, lParam));
}

/****************************************************************
 * Config_Filters_DlgProc
 *
 * A callback function used by the Filters property sheet
 *
 * returns - TRUE or FALSE, used by windows
 ****************************************************************/

BOOL CALLBACK Config_Filter_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        Config_PutFiltersInList(GetDlgItem(hwndDlg, IDC_LIST_FILTERS));
    break;
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) 
        {
        case PSN_SETACTIVE:
            uLastTab = 3;
        break;
        case PSN_APPLY:
            Config_PutListInFilters(GetDlgItem(hwndDlg, IDC_LIST_FILTERS));
        break;
        }
    break;
    case WM_DESTROY:
    {
        HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_FILTERS);
        int i = ListBox_GetCount(hwndList) - 1;

        for ( ; i >= 0; i--)
        {
            LPFILTERDATA lpfd = (LPFILTERDATA)ListBox_GetItemData(hwndList, i);

            ASSERT(lpfd != NULL && (int)lpfd != LB_ERR);
            Mem_Free(lpfd->pszDisplay);
            Mem_Free(lpfd->pszFileTypes);
            Mem_Free(lpfd);

            ListBox_SetItemData(hwndList, i, NULL);
        }
    }
    break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_ADD:
        {
            HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_FILTERS);
            LPFILTERDATA lpfd = (LPFILTERDATA)Mem_Alloc(sizeof(FILTERDATA));
            int nIndex;

            lpfd->pszDisplay    = Mem_AllocStr(Window_GetDlgItemTextLength(hwndDlg, IDC_EDIT_DISPLAY));
            lpfd->pszFileTypes  = Mem_AllocStr(Window_GetDlgItemTextLength(hwndDlg, IDC_EDIT_FILETYPES));

            GetDlgItemText(hwndDlg, IDC_EDIT_DISPLAY, lpfd->pszDisplay, Mem_SizeStr(lpfd->pszDisplay));
            GetDlgItemText(hwndDlg, IDC_EDIT_FILETYPES, lpfd->pszFileTypes, Mem_SizeStr(lpfd->pszFileTypes));

            nIndex = ListBox_FindString(hwndList, -1, lpfd->pszDisplay);
        
            if (nIndex == LB_ERR)
            {
                // Not in the list yet, so add it
                nIndex = ListBox_AddString(hwndList, lpfd->pszDisplay);
                if (nIndex != LB_ERR)
                    ListBox_SetItemData(hwndList, nIndex, lpfd);
                else
                    TRACE(_T("Couldn't add string %s to filter listbox."), lpfd->pszDisplay);
            }
            else
            {
                ListBox_SetItemData(hwndList, nIndex, lpfd);
            }

            Window_EnableDlgItem(hwndDlg, IDC_BUTTON_DELETE, (ListBox_GetCount(hwndList) > 0));
        }
        break;
        case IDC_BUTTON_DELETE:
        {
            HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_FILTERS);
            int nIndex = ListBox_GetCurSel(hwndList);
            LPFILTERDATA lpfd;

            ASSERT(nIndex != LB_ERR); // Can't be...or windows fuced internally
                                      // Since we have to make sure it's disabled
                                      // At all other times...

            lpfd = (LPFILTERDATA)ListBox_GetItemData(hwndList, nIndex);
            ASSERT(lpfd != NULL && (int)lpfd != LB_ERR);
            Mem_Free(lpfd->pszDisplay);
            Mem_Free(lpfd->pszFileTypes);
            Mem_Free(lpfd);

            ListBox_DeleteString(hwndList, nIndex);
            if (ListBox_SetCurSel(hwndList, nIndex) == LB_ERR)
                ListBox_SetCurSel(hwndList, 0);
        }
        break;
        case IDC_BUTTON_UP:
        {
            HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_FILTERS);
            int nIndex = ListBox_GetCurSel(hwndList);

            ASSERT(nIndex != LB_ERR && nIndex != 0);

            ListBox_MoveStringUp(hwndList, nIndex);
        }
        break;
        case IDC_BUTTON_DOWN:
        {
            HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_FILTERS);
            int nIndex = ListBox_GetCurSel(hwndList);
            int nCount = ListBox_GetCount(hwndList);

            ASSERT(nIndex != LB_ERR && nIndex != (nCount - 1));

            ListBox_MoveStringDown(hwndList, nIndex);
        }
        break;
        case IDC_LIST_FILTERS:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
            {
                HWND hwndList = (HWND)lParam;
                int nIndex = ListBox_GetCurSel(hwndList);
                int nCount = ListBox_GetCount(hwndList);

                Window_EnableDlgItem(hwndDlg, IDC_BUTTON_UP, !(nIndex == LB_ERR || nIndex == 0));
                Window_EnableDlgItem(hwndDlg, IDC_BUTTON_DOWN, !(nIndex == LB_ERR || nIndex == (nCount - 1)));
                Window_EnableDlgItem(hwndDlg, IDC_BUTTON_DELETE, !(nIndex == LB_ERR));

                if (nIndex != LB_ERR)
                {
                    LPFILTERDATA lpfd = (LPFILTERDATA)ListBox_GetItemData(hwndList, nIndex);

                    ASSERT(lpfd != NULL && (int)lpfd != LB_ERR);

                    SetDlgItemText(hwndDlg, IDC_EDIT_DISPLAY, lpfd->pszDisplay);
                    SetDlgItemText(hwndDlg, IDC_EDIT_FILETYPES, lpfd->pszFileTypes);
                }
            }
            break;
            }
        break;
        case IDC_EDIT_FILETYPES:
        case IDC_EDIT_DISPLAY:
            Window_EnableDlgItem(hwndDlg, IDC_BUTTON_ADD, (Window_GetDlgItemTextLength(hwndDlg, IDC_EDIT_DISPLAY) > 0 || Window_GetDlgItemTextLength(hwndDlg, IDC_EDIT_FILETYPES) > 0));
        /* Fallthrough */
        default:
            if (Edit_GetModify(GetDlgItem(hwndDlg, LOWORD(wParam))))
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
        return (FALSE);
        }
    break;
    }

    return (CallWindowProc(Config_Generic_DlgProc, hwndDlg, uMsg, wParam, lParam));
}

/****************************************************************
 * Config_GUI_DlgProc
 *
 * A callback function used by the GUI property sheet
 *
 * returns - TRUE or FALSE, used by windows
 ****************************************************************/

BOOL CALLBACK Config_GUI_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static TCHAR szCBFileNameBuffer[MAX_PATH] = _T("");
    static TCHAR szMDIFileNameBuffer[MAX_PATH] = _T("");
    static bMDIImage;
    static bCBImage;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        CheckDlgButton(hwndDlg, IDC_CHECK_USEREBARBKGND, g_ImageSettings.bCoolbarImage);
        CheckDlgButton(hwndDlg, IDC_CHECK_USEMDICLIENTBKGND, g_ImageSettings.bMDIImage);
        CheckDlgButton(hwndDlg, IDC_CHECK_TILEMDICLIENTBKGND, g_ImageSettings.bMDITile);
    
        _tcscpy(szCBFileNameBuffer, g_ImageSettings.szCoolbarImage);
        _tcscpy(szMDIFileNameBuffer, g_ImageSettings.szMDIImage);

        SetDlgItemText(hwndDlg, IDC_EDIT_MARGINCURSORFILE, g_ImageSettings.szMarginCursorFile);

        bCBImage    = LoadGuiBitmap(hwndDlg, IDC_STATIC_COOLBARBMP, szCBFileNameBuffer);
        bMDIImage   = LoadGuiBitmap(hwndDlg, IDC_STATIC_MDICLIENTBMP, szMDIFileNameBuffer);
    break;
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) 
        {
            case PSN_SETACTIVE:
                uLastTab = 4;
            break;
            case PSN_APPLY:
            {
                REBARBANDINFO rabbi = { sizeof(rabbi), RBBIM_BACKGROUND }; //=)
                int i;
                int nCount;

                g_ImageSettings.bCoolbarImage   = IsDlgButtonChecked(hwndDlg, IDC_CHECK_USEREBARBKGND);
                g_ImageSettings.bMDIImage       = IsDlgButtonChecked(hwndDlg, IDC_CHECK_USEMDICLIENTBKGND);
                g_ImageSettings.bMDITile        = IsDlgButtonChecked(hwndDlg, IDC_CHECK_TILEMDICLIENTBKGND);

                _tcscpy(g_ImageSettings.szCoolbarImage, szCBFileNameBuffer);
                _tcscpy(g_ImageSettings.szMDIImage, szMDIFileNameBuffer);
                GetDlgItemText(hwndDlg, IDC_EDIT_MARGINCURSORFILE, g_ImageSettings.szMarginCursorFile, MAX_PATH);

                InvalidateRect(g_hwndMDIClient, NULL, TRUE);

                rabbi.hbmBack   = LoadImage(g_hInstance,
                                    g_ImageSettings.bCoolbarImage ?
                                    g_ImageSettings.szCoolbarImage : NULL,
                                    IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

                nCount = Rebar_GetBandCount(g_hwndRebarMain);

                for (i = 0; i < nCount; i++)
                    Rebar_SetBandInfo(g_hwndRebarMain, i, &rabbi);

                RedrawWindow(g_hwndRebarMain, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
            }
            break;
        }
    break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_STATIC_MDICLIENTBMP:
        {
            BROWSEINFOEX bix;

            if (HIWORD(wParam) != STN_CLICKED)
                break;

            INITSTRUCT(bix, FALSE);
            bix.hwndOwner   = hwndDlg;
            bix.pszFile     = szMDIFileNameBuffer;
            bix.pszTitle    = _T("Browse for Bitmap");
            bix.pszFilter   = _T("Bitmap Files (*.bmp)\0*.bmp");

            if (Browse_ForFile(&bix))
            {
                SendDlgItemMessage(hwndDlg, IDC_STATIC_MDICLIENTBMP, STM_SETIMAGE, IMAGE_BITMAP,
                    (LPARAM)LoadImage(g_hInstance, szMDIFileNameBuffer,
                                        IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
                CheckDlgButton(hwndDlg, IDC_CHECK_USEMDICLIENTBKGND, BST_CHECKED);
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }
        }
        return (FALSE);
        case IDC_STATIC_COOLBARBMP:
        {
            BROWSEINFOEX bix;

            if (HIWORD(wParam) != STN_CLICKED)
                break;

            INITSTRUCT(bix, FALSE);
            bix.hwndOwner   = hwndDlg;
            bix.pszFile = szMDIFileNameBuffer;
            bix.pszTitle    = _T("Browse for Bitmap");
            bix.pszFilter   = _T("Bitmap Files (*.bmp)\0*.bmp");

            if (Browse_ForFile(&bix))
            {
                SendDlgItemMessage(hwndDlg, IDC_STATIC_REBARBMP, STM_SETIMAGE, IMAGE_BITMAP,
                        (LPARAM)LoadImage(g_hInstance, szCBFileNameBuffer,
                                        IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
                CheckDlgButton(hwndDlg, IDC_CHECK_USEREBARBKGND, BST_CHECKED);
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }
        }
        return (FALSE);
        case IDC_CHECK_USEMDICLIENTBKGND:
            if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_USEMDICLIENTBKGND) && !bMDIImage)
                PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_STATIC_MDICLIENTBMP, STN_CLICKED), (LPARAM)NULL);
        break;
        case IDC_CHECK_USEREBARBKGND:
            if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_USEREBARBKGND) && !bCBImage)
                PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_STATIC_REBARBMP, STN_CLICKED), (LPARAM)NULL);
        break;
        case IDC_BUTTON_TBCUSTOMIZE:
            Toolbar_Customize(g_hwndToolbarMain);
        break;
        case IDC_EDIT_MARGINCURSORFILE:
            if (Edit_GetModify(GetDlgItem(hwndDlg, IDC_EDIT_MARGINCURSORFILE)))
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
        return (FALSE);
        case IDC_BUTTON_BROWSE:
        {
            BROWSEINFOEX bix;
            TCHAR szPath[MAX_PATH];

            _tcscpy(szPath, g_ImageSettings.szMarginCursorFile);

            INITSTRUCT(bix, FALSE);
            bix.hwndOwner       = GetParent(hwndDlg);
            bix.pszFile         = szPath;
            bix.pszTitle        = _T("Browse for Cursor File");
            bix.pszFilter   = _T("Bitmap Files (*.cur)\0*.cur");

            if (Browse_ForFile(&bix))
            {
                SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_MARGINCURSORFILE), szPath);
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }
        }
        return (FALSE);
        }
    break;  
    }

    return (CallWindowProc(Config_Generic_DlgProc, hwndDlg, uMsg, wParam, lParam));
}

/****************************************************************
 * Config_Associations_DlgProc
 *
 * A callback function used by the Associations property sheet
 *
 * returns - TRUE or FALSE, used by windows
 ****************************************************************/

BOOL CALLBACK Config_Associations_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static DWORD dwIconIndex = 0;
    static TCHAR szIconFile[MAX_PATH];  

    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        TCHAR szInstallPath[MAX_PATH], szModulePath[MAX_PATH];
        HKEY hKey;

        Edit_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_FILEEXT), MAX_PATH);
        Edit_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_DESC), MAX_PATH);
        Edit_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_COMMAND), 20);
        Edit_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_INSTALLPATH), MAX_PATH);

        Config_GetRegisteredFileExtensions(GetDlgItem(hwndDlg, IDC_LIST_ASSOCIATED));

        Path_GetModuleFileName(g_hInstance, szModulePath, MAX_PATH);
        Path_RemoveFileName(szModulePath);
        RegOpenKey(REG_ROOT, (REG_SLACKEDIT _T("\\Global Settings")), &hKey);
        Registry_GetSZ(hKey, _T("Install Path"), szInstallPath, szModulePath, MAX_PATH);
        SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_INSTALLPATH), szInstallPath);
        RegCloseKey(hKey);

        if (RegOpenKey(HKEY_CLASSES_ROOT, _T("Directory\\shell\\SlackEdit"), &hKey) == ERROR_SUCCESS)
        {
            CheckDlgButton(hwndDlg, IDC_CHECK_SLACKEDITHERE, BST_CHECKED);
            RegCloseKey(hKey);
        }

        Path_GetModuleFileName(g_hInstance, szIconFile, MAX_PATH);
    }
    break;
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) 
        {
        case PSN_SETACTIVE:
            uLastTab = 5;
        break;
        case PSN_APPLY:
        {
            TCHAR szInstallPath[MAX_PATH];
            HKEY hKeyRoot, hKey;

            if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SLACKEDITHERE))
            {
                TCHAR szModule[MAX_PATH];
                Path_GetModuleFileName(g_hInstance, szModule, MAX_PATH);
                RegCreateKey(HKEY_CLASSES_ROOT,
                    _T("Directory\\shell\\SlackEdit"), &hKeyRoot);
                RegSetValue(hKeyRoot, NULL, REG_SZ, _T("&SlackEdit Here!"), 15);
                RegCreateKey(hKeyRoot, _T("command"), &hKey);
                RegSetValue(hKey, NULL, REG_SZ, szModule, _tcslen(szModule));
                RegCloseKey(hKey);
                RegCloseKey(hKeyRoot);
            }
            else
            {
                HKEY hKey;

                if (RegOpenKey(HKEY_CLASSES_ROOT, _T("Directory\\shell"), &hKey) == ERROR_SUCCESS)
                {
                    RegDeleteKey(hKey, _T("SlackEdit"));
                    RegCloseKey(hKey);
                }
            }
            
            GetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_INSTALLPATH), szInstallPath, MAX_PATH);
            RegCreateKey(REG_ROOT, (REG_SLACKEDIT _T("\\Global Settings")), &hKey);
            Registry_SetSZ(hKey, _T("Install Path"), szInstallPath);
            RegCloseKey(hKey);
        }
        break;
        }
    break;
    case WM_DESTROY:
    {
        HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_ASSOCIATED);
        int i;

        for (i = (ListBox_GetCount(hwndList) - 1) ; i >= 0; i--)
        {
            LPFILETYPEDATA lpftd = (LPFILETYPEDATA)ListBox_GetItemData(hwndList, i);

            ASSERT(lpftd != NULL && (int)lpftd != LB_ERR);
            Mem_Free(lpftd);

            ListBox_SetItemData(hwndList, i, NULL);
        }
    }
    break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_STATIC_ICON:
        {
            HMODULE hShell;
            FARPROC lpfnSHChangeIconDialog;

            if (HIWORD(wParam) != STN_CLICKED)
                break;

            hShell = GetModuleHandle(_T("SHELL32.DLL"));
            lpfnSHChangeIconDialog = GetProcAddress(hShell, (LPTSTR)62);

            if (lpfnSHChangeIconDialog(hwndDlg, szIconFile, 0, &dwIconIndex))
                Static_SetIcon((HWND)lParam, ExtractIcon(g_hInstance, szIconFile, dwIconIndex));
        }
        break;
        case IDC_BUTTON_ADD:
        {
            HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_ASSOCIATED);
            int nIndex = ListBox_GetCurSel(hwndList);
            LPFILETYPEDATA lpftd;
            TCHAR szExt[MAX_PATH];

            if (!GetDlgItemText(hwndDlg, IDC_EDIT_FILEEXT, szExt, MAX_PATH))
            {
                MB(String_LoadString(IDS_ERROR_ASSOCIATION_EXT));

                break;
            }

            if (nIndex != LB_ERR)
                lpftd = (LPFILETYPEDATA)ListBox_GetItemData(hwndList, nIndex);

            if (nIndex == LB_ERR || !String_Equal(lpftd->szExt, szExt, FALSE))
                lpftd = (LPFILETYPEDATA)Mem_Alloc(sizeof(FILETYPEDATA));

            ASSERT(lpftd != NULL && (int)lpftd != LB_ERR);

            _tcscpy(lpftd->szExt, szExt);

            GetDlgItemText(hwndDlg, IDC_EDIT_DESC, lpftd->szDescription, MAX_PATH);
            GetDlgItemText(hwndDlg, IDC_EDIT_COMMAND, lpftd->szCommand, 20);
            _stprintf(lpftd->szDefaultIcon, _T("%s,%d"), szIconFile, dwIconIndex);

            Config_RegisterFileExtension(lpftd);

            if (nIndex == LB_ERR || !String_Equal(lpftd->szExt, szExt, FALSE))
                ListBox_AddString(hwndList, lpftd->szExt);
            else
                ListBox_SetItemData(hwndList, nIndex, lpftd);
        }
        break;
        case IDC_BUTTON_REMOVE:
        {
            HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_ASSOCIATED);
            int nIndex = ListBox_GetCurSel(hwndList);
            LPFILETYPEDATA lpftd;

            // Can't be LB_ERR or it wouldn't be enabled...
            ASSERT(nIndex != LB_ERR);

            lpftd = (LPFILETYPEDATA)ListBox_GetItemData(hwndList, nIndex);
            ASSERT(lpftd != NULL && (int)lpftd != LB_ERR);

            // Do this before deletestring since we mess with LBN_SELCHANGE
            Window_EnableDlgItem(hwndDlg, IDC_BUTTON_REMOVE, FALSE);

            Config_RemoveFileExtension(lpftd);
            ListBox_DeleteString(hwndList, nIndex);
        }
        break;
        case IDC_LIST_ASSOCIATED:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
            {
                HWND hwndList = (HWND)lParam;
                int nIndex = ListBox_GetCurSel(hwndList);
                LPFILETYPEDATA lpftd;
                LPTSTR psz;

                if (nIndex == LB_ERR)
                {
                    Window_EnableDlgItem(hwndDlg, IDC_BUTTON_REMOVE, FALSE);

                    return (0);
                }

                if ((lpftd = (LPFILETYPEDATA)ListBox_GetItemData(hwndList, nIndex)) == NULL)
                {
                    Window_EnableDlgItem(hwndDlg, IDC_BUTTON_REMOVE, FALSE);

                    return (0);
                }

                SetDlgItemText(hwndDlg, IDC_EDIT_DESC, lpftd->szDescription);
                SetDlgItemText(hwndDlg, IDC_EDIT_COMMAND, lpftd->szCommand);
                SetDlgItemText(hwndDlg, IDC_EDIT_FILEEXT, lpftd->szExt);
                psz = _tcschr(lpftd->szDefaultIcon, _T(','));
                _tcsncpy(szIconFile, lpftd->szDefaultIcon, (psz - lpftd->szDefaultIcon));
                dwIconIndex = atol(++psz);
                SendDlgItemMessage(hwndDlg, IDC_STATIC_ICON, STM_SETICON,
                    (WPARAM)ExtractIcon(g_hInstance, szIconFile, dwIconIndex), 0);

                Window_EnableDlgItem(hwndDlg, IDC_BUTTON_REMOVE, TRUE);
            }
            break;
            }
        break;
        case IDC_EDIT_COMMAND:
        case IDC_EDIT_DESC:
        case IDC_EDIT_FILEEXT:
            switch ( HIWORD(wParam) )
            {
            case EN_CHANGE:
                if (LOWORD(wParam) == IDC_EDIT_FILEEXT)
                    Window_EnableDlgItem(hwndDlg, IDC_BUTTON_ADD, GetWindowTextLength((HWND)lParam) ? TRUE : FALSE);
            break;
            }
        break;
        case IDC_EDIT_INSTALLPATH:
            switch (HIWORD(wParam))
            {
            case EN_CHANGE:
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            break;
            }
        break;
        case IDC_BTN_BROWSE:
        {
            BROWSEINFOEX bix;
            TCHAR szPath[MAX_PATH] = _T("");

            INITSTRUCT(bix, FALSE);
            bix.hwndOwner       = GetParent(hwndDlg);
            bix.pszPath         = szPath;
            bix.pszTitle        = _T("Browse for Install Path");

            if (Browse_ForPath(&bix))
            {
                SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_INSTALLPATH), szPath);
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }
        }
        break;
        }
    break;  
    }

    return (CallWindowProc(Config_Generic_DlgProc, hwndDlg, uMsg, wParam, lParam));
}

/****************************************************************
 * Config_Scripts_DlgProc
 *
 * A callback function used by the Scripts property sheet
 *
 * returns - TRUE or FALSE, used by windows
 ****************************************************************/

BOOL CALLBACK Config_Scripts_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        int i = 0;
        TCHAR szBuffer[MAX_PATH];
        HKEY hKey;
        DWORD dwSize = sizeof(szBuffer);
        DWORD dwType = REG_SZ;

        if (RegOpenKeyEx(REG_ROOT, (REG_SLACKEDIT _T("\\Tcl Scripts")), 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
            return (TRUE); // Just break out...don't stop loading

        while (RegEnumValue(hKey, i, szBuffer, &dwSize, NULL, &dwType, NULL, NULL) != ERROR_NO_MORE_ITEMS)
        {
            TCHAR szFileName[MAX_PATH];
            DWORD dwSize2 = sizeof(szFileName);

            if (dwType != REG_SZ)
            {
                dwType = REG_SZ;
                dwSize = sizeof(szBuffer);

                continue;
            }

            RegQueryValueEx(hKey, szBuffer, NULL, &dwType, szFileName, &dwSize2);
            
            ListBox_AddNoDuplicateString(GetDlgItem(hwndDlg, IDC_LIST_SCRIPTS),
                                            szFileName);


            dwSize = sizeof(szBuffer);
            i++;
        }

        RegCloseKey(hKey);

        SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_SCRIPTPATH), g_ScriptSettings.szScriptDir);
        SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_TCLPATH), g_ScriptSettings.szTclInstallDir);
    }
    break;
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) 
        {
            case PSN_SETACTIVE:
                uLastTab = 6;
            break;
            case PSN_APPLY:
            {
                TCHAR szScript[MAX_PATH];
                int i;
                HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_SCRIPTS);
                HKEY hKey;

                RegDeleteKey(REG_ROOT, (REG_SLACKEDIT _T("\\Tcl Scripts")));
                RegCreateKey(REG_ROOT, (REG_SLACKEDIT _T("\\Tcl Scripts")), &hKey);

                for (i = 0; ListBox_GetText(hwndList, i, szScript) != LB_ERR; i++)
                    Registry_SetSZ(hKey, Path_GetFileName(szScript), szScript);

                RegCloseKey(hKey);

                GetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_TCLPATH),
                        g_ScriptSettings.szTclInstallDir, MAX_PATH);
                GetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_SCRIPTPATH),
                        g_ScriptSettings.szScriptDir, MAX_PATH);

                Path_VerifyBackslash(g_ScriptSettings.szTclInstallDir, FALSE);
                Path_VerifyBackslash(g_ScriptSettings.szScriptDir, FALSE);
            }
            break;
        }
    break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_ADD:
        {
            BROWSEINFOEX bix;
            TCHAR szFile[MAX_PATH] = _T("");

            INITSTRUCT(bix, FALSE);
            bix.hwndOwner   = GetParent(hwndDlg);
            bix.pszPath     = g_ScriptSettings.szScriptDir;
            bix.pszFile     = szFile;
            bix.pszTitle    = _T("Browse for Tcl Script File");
            bix.pszFilter   = _T("Tcl Files (*.tcl)\0*.tcl");

            if (Browse_ForFile(&bix))
            {
                if (ListBox_AddNoDuplicateString(
                        GetDlgItem(hwndDlg, IDC_LIST_SCRIPTS),
                        szFile) == -1)
                {
                    MB(_T("File already in list...we only want one right?"));
                }
                else
                {
                    PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
                }
            }
        }
        break;
        case IDC_BTN_REMOVE:
            if (ListBox_DeleteAllStrings(GetDlgItem(hwndDlg, IDC_LIST_SCRIPTS)))
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
        break;
        case IDC_BUTTON_UP:
        {
            HWND hwndLB = GetDlgItem(hwndDlg, IDC_LIST_SCRIPTS);

            ListBox_MoveAllStrings(hwndLB, -1);
        }
        break;
        case IDC_BUTTON_DOWN:
        {
            HWND hwndLB = GetDlgItem(hwndDlg, IDC_LIST_SCRIPTS);

            ListBox_MoveAllStrings(hwndLB, 1);
        }
        break;
        case IDC_BUTTON_SCRIPTBROWSE:
        {
            BROWSEINFOEX bix;
            TCHAR szPath[MAX_PATH] = _T("");

            INITSTRUCT(bix, FALSE);
            bix.hwndOwner       = GetParent(hwndDlg);
            bix.pszPath         = szPath;
            bix.pszTitle        = _T("Browse for Tcl Files Path");

            if (Browse_ForPath(&bix))
            {
                SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_SCRIPTPATH), szPath);
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }
        }
        break;
        case IDC_BUTTON_TCLBROWSE:
        {
            BROWSEINFOEX bix;
            TCHAR szPath[MAX_PATH] = _T("");

            INITSTRUCT(bix, FALSE);
            bix.hwndOwner       = GetParent(hwndDlg);
            bix.pszPath         = szPath;
            bix.pszTitle        = _T("Browse for Tcl Install Path");

            if (Browse_ForPath(&bix))
            {
                SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_TCLPATH), szPath);
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }
        }
        break;
        }
    break;  
    }

    return (CallWindowProc(Config_Generic_DlgProc, hwndDlg, uMsg, wParam, lParam));
}

/****************************************************************
 * MRU_SetListView
 *
 * Fills a listview with mru items, i.e. filenames. 
 *
 * returns - nothing
 ****************************************************************/

void MRU_SetListView(HWND hwndList, LPTSTR pszSubKey, int nMaxItems)
{
    LV_ITEM lvi;
    TCHAR szShortFileName[MAX_PATH] = _T("");
    int i;
    CREATEMRULIST cml;
    HANDLE hList;
    TCHAR szBuffer[MAX_MRU];

    INITSTRUCT(cml, TRUE);
    cml.nMaxItems   = nMaxItems;
    cml.dwFlags     = MRUF_STRING_LIST;
    cml.hKey        = MRU_GetRoot();
    cml.lpszSubKey  = pszSubKey;
    cml.lpfnCompare = NULL;

    hList = CreateMRUList(&cml);

    for (i = 0; EnumMRUList(hList, i, szBuffer, MAX_MRU) != -1; i++)
    {
        _tcscpy(szShortFileName, Path_GetFileName(szBuffer));

        lvi.mask        = LVIF_TEXT;
        lvi.iItem       = ListView_GetItemCount(hwndList);
        lvi.iSubItem    = 0;
        lvi.pszText     = szShortFileName;

        ListView_InsertItem(hwndList, &lvi);

        lvi.iSubItem    = 1;
        lvi.pszText     = szBuffer;
                    
        ListView_SetItem(hwndList, &lvi);
    }

    FreeMRUList(hList);
}

/****************************************************************
 * Config_Read
 *
 * Reads all the settings saved in the windows registry
 *
 * returns - nothing
 ****************************************************************/

void Config_Read(void)
{
    HKEY hKeyRoot, hSubKey;

    RegOpenKey(REG_ROOT, REG_SLACKEDIT, &hKeyRoot);

    RegOpenKey(hKeyRoot, _T("Window Settings"), &hSubKey);
    
    g_WindowSettings.nMainXPos      = Registry_GetDW(hSubKey, _T("Main X"), 100);
    g_WindowSettings.nMainYPos      = Registry_GetDW(hSubKey, _T("Main Y"), 100);
    g_WindowSettings.nMainWidth     = Registry_GetDW(hSubKey, _T("Main Width"), 600);
    g_WindowSettings.nMainHeight    = Registry_GetDW(hSubKey, _T("Main Height"), 360);
    g_WindowSettings.bMainCentered  = Registry_GetDW(hSubKey, _T("Main Centered"), TRUE);
    g_WindowSettings.bMainSticky    = Registry_GetDW(hSubKey, _T("Main Sticky"), FALSE);
    g_WindowSettings.bMainOnTop     = Registry_GetDW(hSubKey, _T("Main OnTop"), FALSE);
    g_WindowSettings.bMainMaximized = Registry_GetDW(hSubKey, _T("Main Maximized"), FALSE);

    g_WindowSettings.nMDIWinState   = Registry_GetDW(hSubKey, _T("MDI Windows State"), MDISTATE_NORMAL);

    g_WindowSettings.bAdvancedOpenCentered  = Registry_GetDW(hSubKey, _T("Advanced Open Centered"), TRUE);
    g_WindowSettings.nAdvancedOpenXPos      = Registry_GetDW(hSubKey, _T("Advanced Open X"), 250);
    g_WindowSettings.nAdvancedOpenYPos      = Registry_GetDW(hSubKey, _T("Advanced Open Y"), 200);
    
    g_WindowSettings.bFindCentered  = Registry_GetDW(hSubKey, _T("Find Centered"), TRUE);
    g_WindowSettings.nFindXPos      = Registry_GetDW(hSubKey, _T("Find X"), FALSE);
    g_WindowSettings.nFindYPos      = Registry_GetDW(hSubKey, _T("Find Y"), FALSE);
    
    g_WindowSettings.bReplaceCentered   = Registry_GetDW(hSubKey, _T("Replace Centered"), TRUE);
    g_WindowSettings.nReplaceXPos       = Registry_GetDW(hSubKey, _T("Replace X"), 250);
    g_WindowSettings.nReplaceYPos       = Registry_GetDW(hSubKey, _T("Replace Y"), 200);
    
    g_WindowSettings.bFavoritesCentered = Registry_GetDW(hSubKey, _T("Favorites Centered"), TRUE);
    g_WindowSettings.nFavoritesXPos     = Registry_GetDW(hSubKey, _T("Favorites X"), 250);
    g_WindowSettings.nFavoritesYPos     = Registry_GetDW(hSubKey, _T("Favorites Y"), 200);
    
    g_WindowSettings.bPrintCentered = Registry_GetDW(hSubKey, _T("Print Centered"), TRUE);
    g_WindowSettings.nPrintXPos     = Registry_GetDW(hSubKey, _T("Print X"), 250);
    g_WindowSettings.nPrintYPos     = Registry_GetDW(hSubKey, _T("Print Y"), 200);
    
    g_WindowSettings.bConfigCentered    = Registry_GetDW(hSubKey, _T("Config Centered"), TRUE);
    g_WindowSettings.nConfigXPos        = Registry_GetDW(hSubKey, _T("Config X"), 250);
    g_WindowSettings.nConfigYPos        = Registry_GetDW(hSubKey, _T("Config Y"), 200);
    
    g_WindowSettings.bAboutCentered = Registry_GetDW(hSubKey, _T("About Centered"), TRUE);
    g_WindowSettings.nAboutXPos     = Registry_GetDW(hSubKey, _T("About X"), 250);
    g_WindowSettings.nAboutYPos     = Registry_GetDW(hSubKey, _T("About Y"), 200);

    g_WindowSettings.bCmdLineCentered   = Registry_GetDW(hSubKey, _T("CmdLine Centered"), TRUE);
    g_WindowSettings.nCmdLineXPos       = Registry_GetDW(hSubKey, _T("CmdLine X"), 250);
    g_WindowSettings.nCmdLineYPos       = Registry_GetDW(hSubKey, _T("CmdLine Y"), 200);

    g_WindowSettings.bWindowListCentered    = Registry_GetDW(hSubKey, _T("Window List Centered"), TRUE);
    g_WindowSettings.nWindowListX           = Registry_GetDW(hSubKey, _T("Window List X"), 250);
    g_WindowSettings.nWindowListY           = Registry_GetDW(hSubKey, _T("Window List Y"), 200);

    g_WindowSettings.nClipbookXPos      = Registry_GetDW(hSubKey, _T("Clipbook X"), 50);
    g_WindowSettings.nClipbookYPos      = Registry_GetDW(hSubKey, _T("Clipbook Y"), 50);
    g_WindowSettings.nClipbookWidth     = Registry_GetDW(hSubKey, _T("Clipbook Width"), 162);
    g_WindowSettings.nClipbookHeight    = Registry_GetDW(hSubKey, _T("Clipbook Height"), 232);
    g_WindowSettings.nClipbookAlignment = Registry_GetDW(hSubKey, _T("Clipbook Alignment"), CCS_LEFT);

    g_WindowSettings.nOutputWindowXPos  = Registry_GetDW(hSubKey, _T("OutputWindow X"), 50);
    g_WindowSettings.nOutputWindowYPos  = Registry_GetDW(hSubKey, _T("OutputWindow Y"), 50);
    g_WindowSettings.nOutputWindowWidth = Registry_GetDW(hSubKey, _T("OutputWindow Width"), 300);
    g_WindowSettings.nOutputWindowHeight    = Registry_GetDW(hSubKey, _T("OutputWindow Height"), 70);
    g_WindowSettings.nOutputWindowAlignment = Registry_GetDW(hSubKey, _T("OutputWindow Alignment"), CCS_BOTTOM);
    g_WindowSettings.bOutputWindowActive    = Registry_GetDW(hSubKey, _T("OutputWindow Active"), FALSE);

    g_WindowSettings.bToolbarMainActive     = Registry_GetDW(hSubKey, _T("ToolbarMain Active"), TRUE);
    g_WindowSettings.bQuickSearchActive     = Registry_GetDW(hSubKey, _T("QuickSearch Active"), TRUE);
    g_WindowSettings.bGotoLineActive        = Registry_GetDW(hSubKey, _T("GotoLine Active"), FALSE);
    g_WindowSettings.bTabWindowBarActive    = Registry_GetDW(hSubKey, _T("TabWindowBar Active"), TRUE);
    g_WindowSettings.bStatusbarActive       = Registry_GetDW(hSubKey, _T("Statusbar Active"), TRUE);

    RegCloseKey(hSubKey);
    RegOpenKey(hKeyRoot, _T("Edit Settings"), &hSubKey);

    g_EditSettings.bRecentToClipboard   = Registry_GetDW(hSubKey, _T("Paste Recent To Clipboard"), TRUE);

    RegCloseKey(hSubKey);
    RegOpenKey(hKeyRoot, _T("General Settings"), &hSubKey);

    g_GlobalSettings.fGotoSystray           = Registry_GetDW(hSubKey, _T("Goto Systray"), FALSE);
    g_GlobalSettings.bMultipleInstances     = Registry_GetDW(hSubKey, _T("Multiple Instances"), FALSE);
    g_GlobalSettings.bSaveOpenWindows       = Registry_GetDW(hSubKey, _T("Save Open Windows"), FALSE);
    g_GlobalSettings.bOnlyReloadIfCommandLineEmpty  = Registry_GetDW(hSubKey, _T("Only Reload If Command Line Empty"), TRUE);
    g_GlobalSettings.bTipOfDaDay            = Registry_GetDW(hSubKey, _T("Tip-of-da-Day at Startup"), TRUE);
    g_GlobalSettings.bCrashProtection       = Registry_GetDW(hSubKey, _T("Crash Protection"), TRUE);
    g_GlobalSettings.bLastExitSuccessful    = Registry_GetDW(hSubKey, _T("Last Exit Successful"), TRUE);

    Registry_SetDW(hSubKey, _T("Last Exit Successful"), FALSE);

    RegCloseKey(hSubKey);
    RegOpenKey(hKeyRoot, _T("Find Settings"), &hSubKey);

    g_FindSettings.bWholeWords  = Registry_GetDW(hSubKey, _T("Whole Words"), FALSE);
    g_FindSettings.bCase        = Registry_GetDW(hSubKey, _T("Case Sensitive"), FALSE);
    g_FindSettings.bRegExp      = Registry_GetDW(hSubKey, _T("Regular Expression"), FALSE);
    g_FindSettings.bWrapSearch  = Registry_GetDW(hSubKey, _T("Wrap Search"), TRUE);
    g_FindSettings.nDirection   = Registry_GetDW(hSubKey, _T("Find Direction"), IDC_RADIO_DIRECTION_DOWN);
    g_FindSettings.nContext     = Registry_GetDW(hSubKey, _T("Find Context"), IDC_RADIO_DIRECTION_DOWN);

    RegCloseKey(hSubKey);
    RegOpenKey(hKeyRoot, _T("Find Settings"), &hSubKey);

    g_MRUSettings.nAdvancedOpenDirMax   = min(Registry_GetDW(hSubKey, _T("Advanced Open Directory Max"), 16), 29);
    g_MRUSettings.nAdvancedOpenFileMax  = min(Registry_GetDW(hSubKey, _T("Advanced Open File Max"), 16), 29);
    g_MRUSettings.nFileMax          = min(Registry_GetDW(hSubKey, _T("File Max"), 16), 29);
    g_MRUSettings.nFindMax          = min(Registry_GetDW(hSubKey, _T("Find Max"), 16), 29);
    g_MRUSettings.nReplaceMax       = min(Registry_GetDW(hSubKey, _T("Replace Max"), 16), 29);
    g_MRUSettings.nFavoritesMax     = min(Registry_GetDW(hSubKey, _T("Favorites Max"), 16), 29);
    g_MRUSettings.nLinesMax         = min(Registry_GetDW(hSubKey, _T("Goto Lines Max"), 16), 29);
    g_MRUSettings.nTclCmdMax        = min(Registry_GetDW(hSubKey, _T("TclCmd Max"), 16), 29);

    RegCloseKey(hSubKey);
    RegOpenKey(hKeyRoot, _T("Clipbook Settings"), &hSubKey);

    g_WindowSettings.bClipbookActive = Registry_GetDW(hSubKey, _T("Active"), TRUE);
    g_CBSettings.nLastIndex = Registry_GetDW(hSubKey, _T("Last Index"), FALSE);

    RegCloseKey(hSubKey);
    RegOpenKey(hKeyRoot, _T("GUI Settings"), &hSubKey);

    g_ImageSettings.bCoolbarImage   = Registry_GetDW(hSubKey, _T("Coolbar Image"), FALSE);

    Registry_GetSZ(hSubKey, _T("Coolbar Image File"), g_ImageSettings.szCoolbarImage, _T(""), MAX_PATH);

    if (!Path_FileExists(g_ImageSettings.szCoolbarImage))
        g_ImageSettings.bCoolbarImage = FALSE;

    g_ImageSettings.bMDIImage   = Registry_GetDW(hSubKey, _T("MDI Image"), FALSE);
    Registry_GetSZ(hSubKey, _T("MDI Image File"), g_ImageSettings.szMDIImage, _T(""), MAX_PATH);

    if (!Path_FileExists(g_ImageSettings.szMDIImage))
        g_ImageSettings.bMDIImage = FALSE;

    g_ImageSettings.bMDITile    = Registry_GetDW(hSubKey, _T("MDI Tile"), FALSE);

    Registry_GetSZ(hSubKey, _T("Margin Cursor"), g_ImageSettings.szMarginCursorFile, _T(""), MAX_PATH);

    RegCloseKey(hSubKey);
    RegOpenKey(hKeyRoot, _T("Filters"), &hSubKey);

    Registry_GetSZ(hSubKey, _T("Open Filter"), g_FilterSettings.szOpenFilter, _T("All Files (*.*)|*.*"), MAX_FILTER);
    Registry_GetSZ(hSubKey, _T("Save Filter"), g_FilterSettings.szSaveFilter, _T("All Files (*.*)|*.*"), MAX_FILTER);

    RegCloseKey(hSubKey);
    RegOpenKey(hKeyRoot, _T("Backup Settings"), &hSubKey);

    Registry_GetSZ(hSubKey, _T("AutoSave Extension"), g_BackupSettings.szAutoSaveExt, _T(".asf"), _MAX_EXT);
    g_BackupSettings.bAutoSave          = Registry_GetDW(hSubKey, _T("Auto Save"), FALSE);
    g_BackupSettings.uAutoSaveInterval  = Registry_GetDW(hSubKey, _T("Auto Save Interval"), 15);
    Registry_GetSZ(hSubKey, _T("Auto Save Directory"), g_BackupSettings.szAutoSaveDirectory, _T(""), MAX_PATH);
    g_BackupSettings.bUseAutoSaveDirectory  = Registry_GetDW(hSubKey, _T("Use Auto Save Directory"), FALSE);

    Registry_GetSZ(hSubKey, _T("Backup Extension"), g_BackupSettings.szBackupExt, _T(".bak"), _MAX_EXT);
    g_BackupSettings.bBackup            = Registry_GetDW(hSubKey, _T("Backup on Save"), FALSE);
    Registry_GetSZ(hSubKey, _T("Backup Directory"), g_BackupSettings.szBackupDirectory, _T(""), MAX_PATH);
    g_BackupSettings.bUseBackupDirectory    = Registry_GetDW(hSubKey, _T("Use Backup Directory"), FALSE);

    RegCloseKey(hSubKey);
    RegOpenKey(hKeyRoot, _T("Script Settings"), &hSubKey);

    {
        TCHAR szPath[MAX_PATH];
        HKEY hKey;

        GetModuleFileName(g_hInstance, szPath, MAX_PATH);
        Path_RemoveFileName(szPath);
        _tcscat(szPath, _T("\\TclFiles"));

        Registry_GetSZ(hSubKey, _T("Script Dir"), g_ScriptSettings.szScriptDir, szPath, MAX_PATH);

        if (RegOpenKey(HKEY_LOCAL_MACHINE,
                _T("Software\\Scriptics\\Tcl\\8.2"), &hKey) == ERROR_SUCCESS)
        {
            Registry_GetSZ(hKey, _T("Root"), szPath, _T(""), MAX_PATH);
            RegCloseKey(hKey);
        }

        Registry_GetSZ(hSubKey, _T("Tcl Install Dir"), g_ScriptSettings.szTclInstallDir, szPath, MAX_PATH);
    }

    RegCloseKey(hSubKey);

    RegOpenKey(hKeyRoot, _T("Print Settings"), &hSubKey);

    g_PrintSettings.uMarginBottom   = Registry_GetDW(hSubKey, _T("Bottom Margin"), DEFAULT_PRINT_MARGIN);
    g_PrintSettings.uMarginLeft     = Registry_GetDW(hSubKey, _T("Left Margin"), DEFAULT_PRINT_MARGIN);
    g_PrintSettings.uMarginRight    = Registry_GetDW(hSubKey, _T("Right Margin"), DEFAULT_PRINT_MARGIN);
    g_PrintSettings.uMarginTop      = Registry_GetDW(hSubKey, _T("Top Margin"), DEFAULT_PRINT_MARGIN);

    RegCloseKey(hSubKey);
    RegCloseKey(hKeyRoot);
}

/****************************************************************
 * Config_Write
 *
 * Write all the settings to the windows registry
 *
 * returns - nothing
 ****************************************************************/

void Config_Write()
{
    HKEY hKeyRoot, hSubKey;
    TBSAVEPARAMS tbsp;

    RegCreateKey(REG_ROOT, REG_SLACKEDIT, &hKeyRoot);

    RegCreateKey(hKeyRoot, _T("Window Settings"), &hSubKey);

    Registry_SetDW(hSubKey, _T("Main X"),   g_WindowSettings.nMainXPos);
    Registry_SetDW(hSubKey, _T("Main Y"),   g_WindowSettings.nMainYPos);
    Registry_SetDW(hSubKey, _T("Main Width"),   g_WindowSettings.nMainWidth);
    Registry_SetDW(hSubKey, _T("Main Height"), g_WindowSettings.nMainHeight);
    Registry_SetDW(hSubKey, _T("Main Centered"), g_WindowSettings.bMainCentered);
    Registry_SetDW(hSubKey, _T("Main OnTop"),   g_WindowSettings.bMainOnTop);
    Registry_SetDW(hSubKey, _T("Main Sticky"), g_WindowSettings.bMainSticky);
    Registry_SetDW(hSubKey, _T("Main Maximized"), g_WindowSettings.bMainMaximized);

    Registry_SetDW(hSubKey, _T("Find Centered"), g_WindowSettings.bFindCentered);
    Registry_SetDW(hSubKey, _T("Find X"), g_WindowSettings.nFindXPos);
    Registry_SetDW(hSubKey, _T("Find Y"), g_WindowSettings.nFindYPos);

    Registry_SetDW(hSubKey, _T("Replace Centered"), g_WindowSettings.bReplaceCentered);
    Registry_SetDW(hSubKey, _T("Replace X"), g_WindowSettings.nReplaceXPos);
    Registry_SetDW(hSubKey, _T("Replace Y"), g_WindowSettings.nReplaceYPos);

    Registry_SetDW(hSubKey, _T("Advanced Open Centered"), g_WindowSettings.bAdvancedOpenCentered);
    Registry_SetDW(hSubKey, _T("Advanced Open X"), g_WindowSettings.nAdvancedOpenXPos);
    Registry_SetDW(hSubKey, _T("Advanced Open Y"), g_WindowSettings.nAdvancedOpenYPos);

    Registry_SetDW(hSubKey, _T("Favorites Centered"), g_WindowSettings.bFavoritesCentered);
    Registry_SetDW(hSubKey, _T("Favorites X"), g_WindowSettings.nFavoritesXPos);
    Registry_SetDW(hSubKey, _T("Favorites Y"), g_WindowSettings.nFavoritesYPos);

    Registry_SetDW(hSubKey, _T("Print Centered"), g_WindowSettings.bPrintCentered);
    Registry_SetDW(hSubKey, _T("Print X"), g_WindowSettings.nPrintXPos);
    Registry_SetDW(hSubKey, _T("Print Y"), g_WindowSettings.nPrintYPos);

    Registry_SetDW(hSubKey, _T("Config Centered"), g_WindowSettings.bConfigCentered);
    Registry_SetDW(hSubKey, _T("Config X"), g_WindowSettings.nConfigXPos);
    Registry_SetDW(hSubKey, _T("Config Y"), g_WindowSettings.nConfigYPos);

    Registry_SetDW(hSubKey, _T("About Centered"), g_WindowSettings.bAboutCentered);
    Registry_SetDW(hSubKey, _T("About X"), g_WindowSettings.nAboutXPos);
    Registry_SetDW(hSubKey, _T("About Y"), g_WindowSettings.nAboutYPos);

    Registry_SetDW(hSubKey, _T("CmdLine Centered"), g_WindowSettings.bCmdLineCentered);
    Registry_SetDW(hSubKey, _T("CmdLine X"), g_WindowSettings.nCmdLineXPos);
    Registry_SetDW(hSubKey, _T("CmdLine Y"), g_WindowSettings.nCmdLineYPos);

    Registry_SetDW(hSubKey, _T("Clipbook X"), g_WindowSettings.nClipbookXPos);
    Registry_SetDW(hSubKey, _T("Clipbook Y"), g_WindowSettings.nClipbookYPos);
    Registry_SetDW(hSubKey, _T("Clipbook Width"), g_WindowSettings.nClipbookWidth);
    Registry_SetDW(hSubKey, _T("Clipbook Height"), g_WindowSettings.nClipbookHeight);
    Registry_SetDW(hSubKey, _T("Clipbook Alignment"), g_WindowSettings.nClipbookAlignment);

    Registry_SetDW(hSubKey, _T("OutputWindow X"), g_WindowSettings.nOutputWindowXPos);
    Registry_SetDW(hSubKey, _T("OutputWindow Y"), g_WindowSettings.nOutputWindowYPos);
    Registry_SetDW(hSubKey, _T("OutputWindow Width"), g_WindowSettings.nOutputWindowWidth);
    Registry_SetDW(hSubKey, _T("OutputWindow Height"), g_WindowSettings.nOutputWindowHeight);
    Registry_SetDW(hSubKey, _T("OutputWindow Alignment"), g_WindowSettings.nOutputWindowAlignment);
    Registry_SetDW(hSubKey, _T("OutputWindow Active"), g_WindowSettings.bOutputWindowActive);

    Registry_SetDW(hSubKey, _T("ToolbarMain Active"), g_WindowSettings.bToolbarMainActive);
    Registry_SetDW(hSubKey, _T("QuickSearch Active"), g_WindowSettings.bQuickSearchActive);
    Registry_SetDW(hSubKey, _T("GotoLine Active"), g_WindowSettings.bGotoLineActive);
    Registry_SetDW(hSubKey, _T("TabWindowBar Active"), g_WindowSettings.bTabWindowBarActive);
    Registry_SetDW(hSubKey, _T("Statusbar Active"), g_WindowSettings.bStatusbarActive);

    Registry_SetDW(hSubKey, _T("Window List Centered"), g_WindowSettings.bWindowListCentered);
    Registry_SetDW(hSubKey, _T("Window List X"), g_WindowSettings.nWindowListX);
    Registry_SetDW(hSubKey, _T("Window List Y"), g_WindowSettings.nWindowListY);

    Registry_SetDW(hSubKey, _T("MDI Windows State"), g_WindowSettings.nMDIWinState);

    RegCloseKey(hSubKey);
    RegCreateKey(hKeyRoot, _T("General Settings"), &hSubKey);

    Registry_SetDW(hSubKey, _T("Goto Systray"), g_GlobalSettings.fGotoSystray);
    Registry_SetDW(hSubKey, _T("Multiple Instances"), g_GlobalSettings.bMultipleInstances);
    Registry_SetDW(hSubKey, _T("Save Open Windows"), g_GlobalSettings.bSaveOpenWindows);
    Registry_SetDW(hSubKey, _T("Only Reload If Command Line Empty"), g_GlobalSettings.bOnlyReloadIfCommandLineEmpty);
    Registry_SetDW(hSubKey, _T("Tip-of-da-Day at Startup"), g_GlobalSettings.bTipOfDaDay);
    Registry_SetDW(hSubKey, _T("Crash Protection"), g_GlobalSettings.bCrashProtection);
    Registry_SetDW(hSubKey, _T("Last Exit Successful"), g_GlobalSettings.bLastExitSuccessful);

    RegCloseKey(hSubKey);
    RegCreateKey(hKeyRoot, _T("Edit Settings"), &hSubKey);

    Registry_SetDW(hSubKey, _T("Paste-Recent To Clipboard"), g_EditSettings.bRecentToClipboard);

    RegCloseKey(hSubKey);
    RegCreateKey(hKeyRoot, _T("Find Settings"), &hSubKey);

    Registry_SetDW(hSubKey, _T("Whole Words"), g_FindSettings.bWholeWords);
    Registry_SetDW(hSubKey, _T("Case Sensitive"), g_FindSettings.bCase);
    Registry_SetDW(hSubKey, _T("Regular Expression"), g_FindSettings.bRegExp);
    Registry_SetDW(hSubKey, _T("Wrap Search"), g_FindSettings.bWrapSearch);
    Registry_SetDW(hSubKey, _T("Find Direction"), g_FindSettings.nDirection);
    Registry_SetDW(hSubKey, _T("Find Context"), g_FindSettings.nContext);

    RegCloseKey(hSubKey);
    RegCreateKey(hKeyRoot, _T("MRU Settings"), &hSubKey);

    Registry_SetDW(hSubKey, _T("Advanced Open Directory Max"), min(g_MRUSettings.nAdvancedOpenDirMax, 29));
    Registry_SetDW(hSubKey, _T("Advanced Open File Max"), min(g_MRUSettings.nAdvancedOpenFileMax, 29));
    Registry_SetDW(hSubKey, _T("File Max"), min(g_MRUSettings.nFileMax, 29));
    Registry_SetDW(hSubKey, _T("Find Max"), min(g_MRUSettings.nFindMax, 29));
    Registry_SetDW(hSubKey, _T("Replace Max"), min(g_MRUSettings.nReplaceMax, 29));
    Registry_SetDW(hSubKey, _T("Goto Lines Max"), min(g_MRUSettings.nLinesMax, 29));
    Registry_SetDW(hSubKey, _T("TclCmd Max"), min(g_MRUSettings.nTclCmdMax, 29));

    RegCloseKey(hSubKey);
    RegCreateKey(hKeyRoot, _T("Clipbook Settings"), &hSubKey);

    Registry_SetDW(hSubKey, _T("Last Index"), g_CBSettings.nLastIndex);
    Registry_SetDW(hSubKey, _T("Active"), g_WindowSettings.bClipbookActive);

    RegCloseKey(hSubKey);
    RegCreateKey(hKeyRoot, _T("GUI Settings"), &hSubKey);

    Registry_SetDW(hSubKey, _T("CoolbarImage"), g_ImageSettings.bCoolbarImage);
    Registry_SetDW(hSubKey, _T("MDIImage"), g_ImageSettings.bMDIImage);
    Registry_SetDW(hSubKey, _T("MDI Tile"), g_ImageSettings.bMDITile);

    Registry_SetSZ(hSubKey, _T("Coolbar Image File"), g_ImageSettings.szCoolbarImage);
    Registry_SetSZ(hSubKey, _T("MDI Image File"), g_ImageSettings.szMDIImage);
    Registry_SetSZ(hSubKey, _T("Margin Cursor"), g_ImageSettings.szMarginCursorFile);

    RegCloseKey(hSubKey);
    RegCreateKey(hKeyRoot, _T("Filters"), &hSubKey);

    Registry_SetSZ(hSubKey, _T("Open Filter"), g_FilterSettings.szOpenFilter);
    Registry_SetSZ(hSubKey, _T("Save Filter"), g_FilterSettings.szSaveFilter);

    RegCloseKey(hSubKey);
    RegCreateKey(hKeyRoot, _T("Backup Settings"), &hSubKey);

    Registry_SetSZ(hSubKey, _T("Auto Save Extension"), g_BackupSettings.szAutoSaveExt);
    Registry_SetDW(hSubKey, _T("Auto Save"), g_BackupSettings.bAutoSave);
    Registry_SetDW(hSubKey, _T("Auto Save Interval"), g_BackupSettings.uAutoSaveInterval);
    Registry_SetSZ(hSubKey, _T("Auto Save Directory"), g_BackupSettings.szAutoSaveDirectory);
    Registry_SetDW(hSubKey, _T("Use Auto Save Directory"), g_BackupSettings.bUseAutoSaveDirectory);

    Registry_SetSZ(hSubKey, _T("Backup Extension"), g_BackupSettings.szBackupExt);
    Registry_SetDW(hSubKey, _T("Backup on Save"), g_BackupSettings.bBackup);
    Registry_SetSZ(hSubKey, _T("Backup Directory"), g_BackupSettings.szBackupDirectory);
    Registry_SetDW(hSubKey, _T("Use Backup Directory"), g_BackupSettings.bUseBackupDirectory);

    RegCloseKey(hSubKey);
    RegCreateKey(hKeyRoot, _T("Script Settings"), &hSubKey);

    Registry_SetSZ(hSubKey, _T("Script Dir"), g_ScriptSettings.szScriptDir);
    Registry_SetSZ(hSubKey, _T("Tcl Install Dir"), g_ScriptSettings.szTclInstallDir);

    RegCloseKey(hSubKey);
    RegCreateKey(hKeyRoot, _T("Print Settings"), &hSubKey);
    
    Registry_SetDW(hSubKey, _T("Bottom Margin"), g_PrintSettings.uMarginBottom);
    Registry_SetDW(hSubKey, _T("Left Margin"), g_PrintSettings.uMarginLeft);
    Registry_SetDW(hSubKey, _T("Right Margin"), g_PrintSettings.uMarginRight);
    Registry_SetDW(hSubKey, _T("Top Margin"), g_PrintSettings.uMarginTop);

    RegCloseKey(hSubKey);
    
    RegCloseKey(hKeyRoot);

    tbsp.hkr            = REG_ROOT;
    tbsp.pszSubKey      = (REG_SLACKEDIT _T("\\GUI Settings"));
    tbsp.pszValueName   = _T("Toolbar Buttons");

    Toolbar_SaveRestore(g_hwndToolbarMain, TRUE, &tbsp);
}

/****************************************************************
 * Config_SaveOpenWindows
 *
 * Writes all the open files to the registry (i.e. their filenames)
 * so that they can be reopened later
 *
 * returns - nothing
 ****************************************************************/

void Config_SaveOpenWindows(void)
{
    HKEY hKey;
    DWORD dwTotal, dw;
    PBYTE pbOpenWins, pb;
    HWND hwndMDI;

    if (RegCreateKey(REG_ROOT, (REG_SLACKEDIT _T("\\Global Settings")), &hKey) != ERROR_SUCCESS)
        return;

    for (
        hwndMDI = GetWindow(g_hwndMDIClient, GW_CHILD), dwTotal = 1;
        hwndMDI;
        hwndMDI = GetWindow(hwndMDI, GW_HWNDNEXT)
        )
    {
        FILEINFO fi;

        if (GetWindow(hwndMDI, GW_OWNER))
            continue;

        Main_GetFileInfo(hwndMDI, &fi);

        if (fi.dwFileType & FI_TYPE_MASK_EDIT)
            continue;

        dwTotal += GetWindowTextLength(hwndMDI) + SZ;
    }

    pb  = (PBYTE)Mem_Alloc(dwTotal);

    if (pb == NULL)
        return;

    for (
        hwndMDI = GetWindow(g_hwndMDIClient, GW_CHILD), pbOpenWins = pb;
        hwndMDI;
        hwndMDI = GetWindow(hwndMDI, GW_HWNDNEXT)
        )
    {
        FILEINFO fi;

        if (GetWindow(hwndMDI, GW_OWNER))
            continue;

        Main_GetFileInfo(hwndMDI, &fi);

        if (fi.dwFileType & FI_TYPE_MASK_EDIT)
            continue;

        dw = GetWindowTextLength(hwndMDI);

        GetWindowText(hwndMDI, pbOpenWins, dw + SZ);

        pbOpenWins += dw + SZ;
    }

    *pbOpenWins = _T('\0');

    RegSetValueEx(hKey, _T("Last Open Windows"), 0, REG_MULTI_SZ,
                pb, dwTotal);

    Mem_Free(pb);
    RegCloseKey(hKey);
}

/****************************************************************
 * Config_LoadLastOpenWindows
 *
 * Reads all the saved filenames from the registry
 *
 * returns - nothing
 ****************************************************************/

void Config_LoadLastOpenWindows(void)
{
    HKEY hKey;
    DWORD dwTotal, dwType;
    LPTSTR pszFileList = NULL, p;

    if (!g_GlobalSettings.bLastExitSuccessful && g_GlobalSettings.bCrashProtection)
        return;

    if (RegOpenKey(REG_ROOT, (REG_SLACKEDIT _T("\\Global Settings")), &hKey) != ERROR_SUCCESS)
        return;

    if (RegQueryValueEx(hKey, _T("Last Open Windows"),
            NULL, &dwType,
            NULL, &dwTotal) != ERROR_SUCCESS)
        return;

    if (dwType != REG_MULTI_SZ)
        return;

    pszFileList = (LPTSTR)Mem_AllocStr(dwTotal);

    if (pszFileList == NULL)
        return;

    if (RegQueryValueEx(hKey, _T("Last Open Windows"), NULL, &dwType,
                (LPBYTE)pszFileList, &dwTotal) != ERROR_SUCCESS)
        return;

    for (p = pszFileList; *p; )
    {
        File_Open(p, CRLF_STYLE_AUTOMATIC);

        for (; *p++; )
            ;
    }

    Mem_Free(pszFileList);
    RegCloseKey(hKey);
}

/****************************************************************
 * LoadGuiBitmap
 *
 * Loads a bitmap from a file into a static control
 *
 * returns - TRUE on success, FALSE on failure
 ****************************************************************/

static BOOL LoadGuiBitmap(HWND hwndDlg, UINT idCtl, LPTSTR szFileName)
{
    HBITMAP hbm = NULL;
    
    if (Path_FileExists(szFileName))
        hbm = (HBITMAP)LoadImage(g_hInstance, szFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    if (hbm != NULL)
    {
        SendDlgItemMessage(hwndDlg, idCtl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm);

        return (TRUE);
    }
    else
    {
        ICONINFO ii;
        HICON hIcon;
        
        hIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_CANCEL), IMAGE_ICON,
                        0, 0, LR_DEFAULTSIZE | LR_LOADTRANSPARENT);

        GetIconInfo(hIcon, &ii);
        SendDlgItemMessage(hwndDlg, idCtl, STM_SETIMAGE, IMAGE_BITMAP,
                    (LPARAM)ii.hbmColor);

        DeleteObject(hIcon);

        return (FALSE);
    }
}

/****************************************************************
 * Config_ResetEditControlsEnumProc
 *
 * Resets all the edit controls in a dialog to the unmodified state
 * Should be used after the changes have been applied (PSN_APPLY)
 *
 * returns - TRUE or FALSE, used by windows
 ****************************************************************/

BOOL CALLBACK Config_ResetEditControlsEnumProc(HWND hwnd, LPARAM lParam)
{
    TCHAR szClassName[80];

    GetClassName(hwnd, szClassName, 79);

    if (String_Equal(szClassName, _T("EDIT"), FALSE))
        Edit_SetModify(hwnd, FALSE);

    return (TRUE);
}

/****************************************************************
 * Config_RegisterFileExtension
 *
 * Changes or adds information about a file extension in the windows
 * registry
 *
 * returns - nothing
 ****************************************************************/

static void Config_RegisterFileExtension(LPFILETYPEDATA lpftd)
{
    HKEY hExtKey, hSlackEditKey, hCommandKey, hDefaultIconKey;
    TCHAR szExt[MAX_PATH];
    TCHAR szCommand[MAX_PATH + 20];
    TCHAR szFileName[MAX_PATH];

    if (lpftd->szExt[0] == _T('.'))
        _stprintf(szExt, (REG_SLACKEDIT_TYPE _T("%s")), lpftd->szExt);
    else
        _stprintf(szExt, (REG_SLACKEDIT_TYPE _T(".%s")), lpftd->szExt);

    Path_GetModuleFileName(g_hInstance, szFileName, MAX_PATH);
    if (lpftd->szCommand[0] == _T('\0'))
        _tcscpy(lpftd->szCommand, _T("%1"));
    _stprintf(szCommand, _T("\"%s\" %s"), szFileName, lpftd->szCommand);

    RegCreateKey(HKEY_CLASSES_ROOT, lpftd->szExt, &hExtKey);
    RegSetValueEx(hExtKey, (LPTSTR)NULL, 0, REG_SZ, szExt, _tcslen(szExt));
    RegCloseKey(hExtKey);

    RegCreateKey(HKEY_CLASSES_ROOT, szExt, &hSlackEditKey);
    RegSetValueEx(hSlackEditKey, (LPTSTR)NULL, 0, REG_SZ, lpftd->szDescription, _tcslen(lpftd->szDescription));

    RegCreateKey(hSlackEditKey, _T("Shell\\Open\\Command"), &hCommandKey);
    RegSetValueEx(hCommandKey, (LPTSTR)NULL, 0, REG_SZ, szCommand, _tcslen(szCommand));
    RegCloseKey(hCommandKey);

    RegCreateKey(hSlackEditKey, _T("DefaultIcon"), &hDefaultIconKey);
    RegSetValueEx(hDefaultIconKey, (LPTSTR)NULL, 0, REG_SZ, lpftd->szDefaultIcon, _tcslen(lpftd->szDefaultIcon));
    RegCloseKey(hDefaultIconKey);

    RegCloseKey(hSlackEditKey);
}

/****************************************************************
 * Config_GetRegisteredFileExtensions
 *
 * Reads all the file extensions (and associated information)
 * associated with SlackEdit
 *
 * returns - nothing
 ****************************************************************/


static void Config_GetRegisteredFileExtensions(HWND hwndList)
{
    int i;
    TCHAR szKeyName[MAX_PATH];
    DWORD dwSizeOfName = sizeof(szKeyName);

    for (i = 0; RegEnumKeyEx(HKEY_CLASSES_ROOT, i, szKeyName, &dwSizeOfName, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS; i++, dwSizeOfName = sizeof(szKeyName))
    {
        TCHAR szType[MAX_PATH];
        DWORD dwSizeOfType = sizeof(szType);
        HKEY hTypeKey;

        // To save loads of time only check the extension keys
        // And they are ordered first so when the first char isn't
        // '.' then one can safely break out of the for ()
        if (szKeyName[0] != _T('.'))
            continue;

        if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szKeyName, 0, KEY_READ, &hTypeKey) != ERROR_SUCCESS)
            continue;

        // Retrieve default value (which contatins the file type specifier)
        RegQueryValueEx(hTypeKey, (LPTSTR)NULL, (LPDWORD)NULL, (LPDWORD)NULL, szType, &dwSizeOfType);

        if (String_NumEqual(szType, REG_SLACKEDIT_TYPE, _tcslen(REG_SLACKEDIT_TYPE), FALSE))
        {
            LPFILETYPEDATA lpftd = (LPFILETYPEDATA)Mem_Alloc(sizeof(FILETYPEDATA));
            int nIndex = ListBox_AddString(hwndList, szKeyName);
            HKEY hSlackFileKey;
            HKEY hCommandKey;
            HKEY hDefaultIconKey;
            LPTSTR psz;
            DWORD dwSizeOfDefaultIcon = sizeof(lpftd->szDefaultIcon);
            DWORD dwSizeOfDescription = sizeof(lpftd->szDescription);

            if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szType, 0, KEY_READ, &hSlackFileKey) != ERROR_SUCCESS)
            {
                RegCloseKey(hTypeKey);
                
                continue;
            }

            psz = szType + _tcslen(REG_SLACKEDIT_TYPE);
            _tcscpy(lpftd->szExt, psz);
            if (RegOpenKeyEx(hSlackFileKey, _T("Shell\\Open\\Command"), 0, KEY_READ, &hCommandKey) == ERROR_SUCCESS)
            {
                LPTSTR psz;
                TCHAR szCommand[MAX_PATH + 20];
                DWORD dwSizeOfCommand = sizeof(szCommand);

                RegQueryValueEx(hCommandKey, NULL, NULL, NULL, szCommand, &dwSizeOfCommand);
                RegCloseKey(hCommandKey);

                psz = _tcsrchr(szCommand, _T('\"'));
                _tcsncpy(lpftd->szCommand, (psz + 2), 20);
            }
            if (RegOpenKeyEx(hSlackFileKey, _T("DefaultIcon"), 0, KEY_READ, &hDefaultIconKey) == ERROR_SUCCESS)
            {
                RegQueryValueEx(hDefaultIconKey, NULL, NULL, NULL, lpftd->szDefaultIcon, &dwSizeOfDefaultIcon);
                RegCloseKey(hCommandKey);
            }
            RegQueryValueEx(hSlackFileKey, (LPCTSTR)NULL, (LPDWORD)NULL, (LPDWORD)NULL, lpftd->szDescription, &dwSizeOfDescription);

            ListBox_SetItemData(hwndList, nIndex, lpftd);

            RegCloseKey(hSlackFileKey);
        }

        RegCloseKey(hTypeKey);
    }
}

/****************************************************************
 * Config_RemoveFileExtension
 *
 * Removes a file extension associated with SlackEdit from the 
 * Windows registry
 *
 * returns - nothing
 ****************************************************************/

static void Config_RemoveFileExtension(LPFILETYPEDATA lpftd)
{
    TCHAR szExt[MAX_PATH];
    TCHAR szSlackEdit[MAX_PATH];

    if (lpftd->szExt[0] == _T('.'))
        _tcscpy(szExt, lpftd->szExt);
    else
        _stprintf(szExt, _T(".%s"), lpftd->szExt);

    _stprintf(szSlackEdit, (REG_SLACKEDIT_TYPE _T("%s")), szExt);

    RegDeleteKey(HKEY_CLASSES_ROOT, szExt);
    RegDeleteKey(HKEY_CLASSES_ROOT, szSlackEdit);
}

/****************************************************************
 * Config_PutFiltersInList
 *
 * Reads file open, file save filters from a storage variable
 *
 * returns - nothing
 ****************************************************************/

static void Config_PutFiltersInList(HWND hwndList)
{
    TCHAR szFilters[MAX_FILTER];
    LPTSTR pszElement;

    _tcsncpy(szFilters, g_FilterSettings.szOpenFilter, MAX_FILTER);

    for (pszElement = _tcstok(szFilters, _T("|")); pszElement != NULL; pszElement = _tcstok(NULL, _T("|")))
    {
        LPFILTERDATA lpfd = (LPFILTERDATA)Mem_Alloc(sizeof(FILTERDATA));
        int nIndex;

        ASSERT(lpfd != NULL); // ...or else we didn't get any memory alloced!

        lpfd->pszDisplay = Mem_AllocStr(_tcslen(pszElement));
        _tcscpy(lpfd->pszDisplay, pszElement);

        pszElement = _tcstok(NULL, _T("|"));

        if (pszElement == NULL)
        {
            // Weird...shouldn't happen ( =\ ) but we need to deal with it
            lpfd->pszFileTypes = Mem_AllocStr(3);
            _tcscpy(lpfd->pszFileTypes, _T("*.*"));

            break;
        }
        else
        {
            lpfd->pszFileTypes = Mem_AllocStr(_tcslen(pszElement));
            _tcscpy(lpfd->pszFileTypes, pszElement);
        }

        nIndex = ListBox_AddString(hwndList, lpfd->pszDisplay);
        if (nIndex != LB_ERR)
            ListBox_SetItemData(hwndList, nIndex, lpfd);
        else
            TRACE(_T("Error while adding string in filter listbox...String: %s"), lpfd->pszDisplay);
    }
}

/****************************************************************
 * Config_PutListInFilters
 *
 * Reads in a listbox contents to a filter storage variable
 *
 * returns - nothing
 ****************************************************************/

static void Config_PutListInFilters(HWND hwndList)
{
    TCHAR szFilters[MAX_FILTER] = _T("");
    int i;
    int nCount = ListBox_GetCount(hwndList);
    LPFILTERDATA lpfd;

    for (i = 0; i < nCount; i++)
    {
        lpfd = (LPFILTERDATA)ListBox_GetItemData(hwndList, i);

        ASSERT(lpfd != NULL);

        _tcscat(szFilters, lpfd->pszDisplay);
        _tcscat(szFilters, _T("|"));
        _tcscat(szFilters, lpfd->pszFileTypes);
        if ((i + 1) != nCount)
            _tcscat(szFilters, _T("|")); // Only add for all items except last
    }

    _tcscpy(g_FilterSettings.szOpenFilter, szFilters);
}
