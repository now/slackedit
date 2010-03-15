/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : se_file.c
 * Created    : not known (before 01/23/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:13:06
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
#include "../mdi/mdi_child_edit.h"
#include "../parsing/file_actions.h"
#include "../settings/settings.h"
#include "../subclasses/sub_pcp_edit.h"
#include "../windows/wnd_statusbar.h"

/* pcp_generic */
#include <pcp_string.h>
#include <pcp_mem.h>
#include <pcp_mru.h>
#include <pcp_filenotify.h>
#include <pcp_path.h>
#include <pcp_mdi.h>

/* pcp_controls */
#include <pcp_statusbar.h>

/* pcp_edit */
#include <pcp_edit.h>
#include <pcp_textedit.h>

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

BOOL CALLBACK FindOpenFileEnumProc(HWND hwnd, LPARAM lParam);

void File_Addify(HWND hwndChild, HWND hwndEdit, LPTSTR pszFile);
void File_ChangeNotifyCallback(LPFILENOTIFYDATA lpfnd, DWORD dwChange);
BOOL File_ReloadFile(LPCTSTR pszFileName);
UINT CALLBACK File_OFNHookProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

static UINT s_nNumEdits = 0;
static BOOL s_bOpen = FALSE;
static FILENOTIFYSTRUCT s_fns;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

void File_Initialize(void)
{
    FileNotify_CreateNotifier(&s_fns, File_ChangeNotifyCallback);
}

void File_Denitialize(void)
{
    FileNotify_DestroyNotifier(&s_fns);
}

void File_ChangeNotifyCallback(LPFILENOTIFYDATA lpfnd, DWORD dwChange)
{
    if ((dwChange & FN_CHANGED_TIME) || (dwChange & FN_CHANGED_SIZE))
    {
        TCHAR szMsg[MAX_PATH + 64];
        FILEINFO fi;
        HWND hwnd;

        hwnd = Main_GetFileInfoByFileName(lpfnd->pszFileName, &fi);
        ASSERT(hwnd != NULL);
        MDI_Activate(g_hwndMDIClient, hwnd);
        
        _stprintf(szMsg, String_LoadString(IDS_FORMAT_FILE_MODIFIED), lpfnd->pszFileName);

        if (MessageBox(g_hwndMDIClient, szMsg, _T("Reload File?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
            File_ReloadFile(lpfnd->pszFileName);
    }
    else if (dwChange & FN_DELETED)
    {
        TCHAR szMsg[MAX_PATH + 64];
        FILEINFO fi;
        HWND hwnd;

        hwnd = Main_GetFileInfoByFileName(lpfnd->pszFileName, &fi);
        ASSERT(hwnd != NULL);
        MDI_Activate(g_hwndMDIClient, hwnd);

        _stprintf(szMsg, String_LoadString(IDS_FORMAT_FILE_DELETED), lpfnd->pszFileName);
        MessageBox(g_hwndMDIClient, szMsg, _T("File Deleted"), MB_OK | MB_ICONWARNING);
        //FIXME: add a set unmodified routine here when se_mdi_child_generic has it

        FileNotify_RemoveFile(&s_fns, lpfnd->pszFileName);
    }
}

BOOL File_ReloadFile(LPCTSTR pszFileName)
{
    HWND hwndMDI;

    for (
        hwndMDI = GetWindow(g_hwndMDIClient, GW_CHILD);
        hwndMDI != NULL;
        hwndMDI = GetWindow(hwndMDI, GW_HWNDNEXT)
        )
    {
        FILEINFO fi;

        Main_GetFileInfo(hwndMDI, &fi);

        // This should work fine in any instance
        if (String_NumEqual(fi.szFileName, pszFileName, _tcslen(pszFileName), FALSE))
        {
            PCP_Edit_LoadFile(GetDlgItem(hwndMDI, IDC_EDIT_MDICHILD), pszFileName);

            return (TRUE);
        }
    }

    return (FALSE);
}

BOOL File_OpenByDialog(HWND hwnd)
{
    TCHAR szFileName[MAX_PATH * 4] = _T("");
    FILEINFO fi;
    OPENFILENAME ofn;
    int nLineFeedType = CRLF_STYLE_AUTOMATIC;
    LPEDITVIEW lpew = MDI_GetEditView(NULL);

    if (lpew != NULL && lpew->lpes != NULL)
        nLineFeedType = TextBuffer_GetCRLFMode(lpew->lpes);

    Main_GetFileInfo(NULL, &fi);

    INITSTRUCT(ofn, TRUE);
    ofn.hwndOwner           = hwnd;
    ofn.hInstance           = g_hInstance;
    ofn.lpstrFilter         = String_MakeFilter(g_FilterSettings.szOpenFilter);
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = szFileName;
    ofn.nMaxFile            = MAX_PATH;
    ofn.lpstrInitialDir     = (!(fi.dwFileType & FI_TYPE_MASK_EDIT) && fi.szFileName != NULL) ? Path_RemoveFileName(fi.szFileName): NULL;
    ofn.lpstrTitle          = _T("Open File(s)");
    ofn.Flags               = OFN_CREATEPROMPT | OFN_ENABLESIZING | OFN_EXPLORER | OFN_ALLOWMULTISELECT;// | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
    ofn.lCustData           = (LPARAM)&nLineFeedType;
    ofn.lpfnHook            = File_OFNHookProc;
    ofn.lpTemplateName      = MAKEINTRESOURCE(IDD_OPENFILEDIALOG);

    if (GetOpenFileName(&ofn))
    {
        WIN32_FIND_DATA wfd;
        HANDLE hSearch;
        TCHAR szDirectory[MAX_PATH] = _T(""), szGluedFileName[MAX_PATH] = _T("");

        _tcscpy(szDirectory, szFileName);

        // Check if szDirectory actually exists (either file or directory)
        if ((hSearch = FindFirstFile(szDirectory, &wfd)) == INVALID_HANDLE_VALUE && !(_tcslen(szDirectory) && Path_HasRoot(szDirectory)))
        {
            return (FALSE);
        }
        else
        {
            FindClose(hSearch);

            // Stop redrawing while opening...makes the whole thing
            // a LOT faster...
            SetWindowRedraw(g_hwndMain, FALSE);

            // is szDirectory a directory?
            if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY || (_tcslen(szDirectory) == 3 && Path_HasRoot(szDirectory)))
            {
                LPTSTR psz = szFileName;

                //this means we have multiple files
                Path_VerifyBackslash(szDirectory, TRUE);
                _tcscpy(szGluedFileName, szDirectory);

                while (TRUE)
                {
                    int nLength = _tcslen(psz);

                    //copy and strip off
                    psz += nLength + 1;
                    if (*psz == _T('\0'))
                        break;

                    _tcscat(szGluedFileName, psz);
                    File_Open(szGluedFileName, nLineFeedType);
                    _tcscpy(szGluedFileName, szDirectory);
                }
            }
            else
            {
                // only one file so open it and leave
                File_Open(szDirectory, nLineFeedType);
            }

            //redraw again
            SetWindowRedraw(g_hwndMain, TRUE);
            RedrawWindow(g_hwndMain, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
        }

        return (TRUE);
    }

    return (FALSE);
}

void File_SaveByDialog(HWND hwnd, LPTSTR pszFile)
{
    OPENFILENAME ofn;
    TCHAR pszFileNameBuffer[MAX_PATH] = _T("");
    int nLineFeedType = CRLF_STYLE_AUTOMATIC;
    LPEDITVIEW lpew = MDI_GetEditView(NULL);

    if (lpew != NULL && lpew->lpes != NULL)
        nLineFeedType = TextBuffer_GetCRLFMode(lpew->lpes);

    INITSTRUCT(ofn, TRUE);
    ofn.hwndOwner           = hwnd;
    ofn.hInstance           = g_hInstance;
    ofn.lpstrFilter         = String_MakeFilter(g_FilterSettings.szSaveFilter);
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = pszFileNameBuffer;
    ofn.nMaxFile            = MAX_PATH;
    ofn.lpstrTitle          = _T("Save File As");
    ofn.Flags               = OFN_ENABLESIZING | OFN_EXPLORER | OFN_ALLOWMULTISELECT;// | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
    ofn.lCustData           = (LPARAM)&nLineFeedType;
    ofn.lpfnHook            = File_OFNHookProc;
    ofn.lpTemplateName      = MAKEINTRESOURCE(IDD_OPENFILEDIALOG);

    if (!GetSaveFileName(&ofn))
        return;
    else
        File_Save(hwnd, pszFileNameBuffer, nLineFeedType);
}

void File_Open(LPTSTR pszFile, int nLineFeedType)
{
    MDICREATESTRUCT mcs;
    HWND hwndChild, hwndEdit, hwndPrev;
    HCURSOR hOldCursor;
//  LPEDITVIEW lpew;
    FILEINFO fi;
    int i;

    // FIXME: TEMP
    INITSTRUCT(fi, FALSE);

    SetCapture(g_hwndMain);
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    EnumChildWindows(g_hwndMDIClient, FindOpenFileEnumProc, (LPARAM)pszFile);

    if (s_bOpen)
    {
        HWND hwnd;
        TCHAR szMsg[MAX_PATH + 30];

        s_bOpen = FALSE;

        hwnd = Main_GetFileInfoByFileName(pszFile, &fi);

        ASSERT(!(fi.dwFileType & FI_TYPE_MASK_EDIT));

        MDI_Activate(g_hwndMDIClient, hwnd);

        // FIXME: Should only be done if modified
        _stprintf(szMsg, String_LoadString(IDS_FORMAT_FILE_ALREADYOPEN), pszFile);
        if (MessageBox(g_hwndMain, szMsg, g_szAppName, MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            File_ReloadFile(pszFile);
        }

        goto err_return;
    }

    hwndPrev = MDI_MyGetActive(FALSE);

    mcs.szTitle = pszFile;
    mcs.szClass = g_szEditChild; // g_szHexChild
    mcs.hOwner  = g_hInstance;
    mcs.x = mcs.cx = CW_USEDEFAULT;
    mcs.y = mcs.cy = CW_USEDEFAULT;
    mcs.style = MDIS_ALLCHILDSTYLES | WS_CLIPCHILDREN;

    // a messy way of setting the file to open for the edit control
    if (String_Equal(pszFile, _T("Edit"), TRUE))
    {
        s_nNumEdits++;

        fi.dwFileType   |= FI_TYPE_MASK_EDIT;
        _stprintf(fi.szFileName, _T("Edit%d"), s_nNumEdits);
    }
    else
    {
        ASSERT(Path_FileExists(pszFile));

        fi.dwFileType   = FI_TYPE_TEXT;
        _tcscpy(fi.szFileName, pszFile);
    }

    mcs.lParam  = (LPARAM)&fi;

    if ((!IsWindow(hwndPrev) &&
        g_WindowSettings.nMDIWinState == MDISTATE_MAXIMIZED) || 
        (IsWindow(hwndPrev) && IsMaximized(hwndPrev)))
    {
        mcs.style |= WS_MAXIMIZE;
        g_WindowSettings.nMDIWinState = MDISTATE_MAXIMIZED;
    }

    hwndChild = MDI_Create(g_hwndMDIClient, &mcs);

    ASSERT(hwndChild != NULL);

    hwndEdit = GetDlgItem(hwndChild, IDC_EDIT_MDICHILD);

    if ((_tcscmp(pszFile, _T("Edit"))) == 0)
    {
        File_Addify(hwndChild, hwndEdit, fi.szFileName);

        goto normal_return;
    }
    else
    {
        TCHAR szText[MAX_PATH];
        TC_ITEM tci;
        FILEINFO fi;

        tci.mask = TCIF_TEXT | TCIF_PARAM;
        tci.pszText = szText;
        tci.cchTextMax = MAX_PATH;
        if (TabCtrl_GetItem(g_hwndTabWindowBar, 0, &tci))
        {
            ASSERT(IsWindow((HWND)tci.lParam));
                
            Main_GetFileInfo((HWND)tci.lParam, &fi);
            
            if ((fi.dwFileType & FI_TYPE_MASK_FIRST) &&
                !EditView_GetModify(GetDlgItem((HWND)tci.lParam, IDC_EDIT_MDICHILD)))
            {
                Window_Close((HWND)tci.lParam);
            }
        }
    }

    File_Addify(hwndChild, hwndEdit, pszFile);

    FileNotify_AddFile(&s_fns, pszFile, FN_NOTIFY_DEFAULT);
    FileNotify_StartMonitoring(&s_fns);

    MRU_Write(_T("File MRU"), pszFile, g_MRUSettings.nFileMax);
    for (i = IDM_RECENTFILES; DeleteMenu(g_hMenuRecentFiles, i, MF_BYCOMMAND); i++)
        ; /* empty body */
    MRU_SetMenu(g_hMenuRecentFiles, _T("File MRU"), g_MRUSettings.nFileMax, IDM_RECENTFILES);

normal_return:
//  lpew = MDI_GetEditView(NULL);
/// TextBuffer_SetCRLFMode(lpew->lpes, (nLineFeedType == CRLF_STYLE_AUTOMATIC) ? TextBuffer_GetCRLFMode(lpew->lpes) : nLineFeedType);
//  EditChild_SetLineFeed(lpew->hwnd, TextBuffer_GetCRLFMode(lpew->lpes));
err_return:
    SetCursor(hOldCursor);
    ReleaseCapture();
}

BOOL File_Save(HWND hwnd, LPTSTR pszFile, int nLineFeedType)
{
    TCHAR szStatus[MAX_PATH + 30], szFileName[MAX_PATH];
    HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT_MDICHILD);
    LPEDITVIEW lpew = MDI_GetEditView(hwndEdit);
    LPFILEINFO lpfi = (LPFILEINFO)Mem_Alloc(sizeof(FILEINFO));
    STATUSBARPANE sp;
    HCURSOR hOldCursor;

    SetCapture(g_hwndMain);
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    SetWindowText(hwnd, pszFile);

    FileNotify_StopMonitoring(&s_fns);
    FileNotify_RemoveFile(&s_fns, pszFile);

    if (g_BackupSettings.bBackup)
    {
        TCHAR szBackupFileName[MAX_PATH];

        if (g_BackupSettings.bUseBackupDirectory && g_BackupSettings.szBackupDirectory[0] != _T('\0'))
        {
            _tcscpy(szBackupFileName, g_BackupSettings.szBackupDirectory);
            _tcscat(szBackupFileName, Path_GetFileName(pszFile));
            Path_SwapExt(szBackupFileName, g_BackupSettings.szBackupExt);
        }
        else
        {
            _tcscpy(szBackupFileName, pszFile);
            Path_SwapExt(szBackupFileName, g_BackupSettings.szBackupExt);
        }

        TextBuffer_SaveToFile(lpew->lpes, pszFile, nLineFeedType, TRUE, TRUE, szBackupFileName);
    }
    else
    {
        TextBuffer_SaveToFile(lpew->lpes, pszFile, nLineFeedType, TRUE, FALSE, NULL);
    }

    FileNotify_AddFile(&s_fns, pszFile, FN_NOTIFY_DEFAULT);
    FileNotify_StartMonitoring(&s_fns);

    TextView_SetTextType(lpew, Path_GetExt(pszFile));

    Path_CompactPath(szFileName, pszFile, 50);
    _stprintf(szStatus, _T("Saved %s for you."), szFileName);

    lpfi->dwFileType &= ~(FI_TYPE_MASK_FIRST | FI_TYPE_MASK_EDIT);
    _tcscpy(lpfi->szFileName, pszFile);

    Mem_Free((LPVOID)SetWindowLong(hwnd, GWL_USERDATA, (LONG)lpfi));

    INITSTRUCT(sp, FALSE);
    sp.fMask    = SF_TEXT | SF_CRFG | SF_CRBG;
    sp.pszText  = szStatus;
    sp.crBg     = RGB(0, 0, 127);
    sp.crFg     = RGB(255, 255, 255);
    sp.nPane    = 0;
    Statusbar_SetPane(g_hwndStatusbarMain, &sp);
    Main_UpdateWindowBar(hwnd, TCM_SETTEXT, (LPARAM)Path_GetFileName(pszFile));

    // Not MDI_Child_Generic_... since we're only saving TextViews right now
    MDI_Child_Edit_EnableToolbarButtons(hwnd, FALSE);

    SetCursor(hOldCursor);
    ReleaseCapture();

    return (TRUE);
}

BOOL File_AutoSave(HWND hwndEdit)
{
    FILEINFO fi;
    TCHAR szNewFile[MAX_PATH];

    Main_GetFileInfo(GetParent(hwndEdit), &fi);

    if (fi.dwFileType & FI_TYPE_MASK_EDIT)
    {
        if (g_BackupSettings.bUseAutoSaveDirectory && g_BackupSettings.szAutoSaveDirectory[0] != _T('\0'))
        {
            _tcscpy(szNewFile, g_BackupSettings.szAutoSaveDirectory);
        }
        else
        {
            TCHAR szCurrentDir[MAX_PATH];

            GetCurrentDirectory(MAX_PATH, szCurrentDir);
            _stprintf(szNewFile, _T("%s\\%s"),  szCurrentDir, fi.szFileName);
        }

        _tcscat(szNewFile, g_BackupSettings.szAutoSaveExt);
    }
    else
    {
        _tcscpy(szNewFile, fi.szFileName);
        Path_SwapExt(szNewFile, g_BackupSettings.szAutoSaveExt);
    }

    TextBuffer_SaveToFile((MDI_GetEditView(hwndEdit))->lpes, szNewFile, CRLF_STYLE_AUTOMATIC, FALSE, FALSE, NULL);

    return (TRUE);
}
        
BOOL CALLBACK FindOpenFileEnumProc(HWND hwnd, LPARAM lParam)
{
    TCHAR szClass[80];

    GetClassName(hwnd, szClass, 79);

    if (String_Equal(szClass, WC_PCP_TEXTEDIT, FALSE))
    {
        LPFILEINFO lpfi = (LPFILEINFO)GetWindowLong(GetParent(hwnd), GWL_USERDATA);

        if (String_Equal((LPTSTR)lParam, lpfi->szFileName, FALSE))
        {
            s_bOpen = TRUE;
            MDI_Activate(g_hwndMDIClient, hwnd);

            return (FALSE);
        }
    }

    return (TRUE);
}

void File_Addify(HWND hwndChild, HWND hwndEdit, LPTSTR pszFile)
{
    LPFILEINFO lpfi;
    TC_ITEM tci;

    lpfi = (LPFILEINFO)Mem_Alloc(sizeof(FILEINFO));

    _tcscpy(lpfi->szFileName, pszFile);

    if (String_NumEqual(pszFile, _T("Edit1"), 5, TRUE))
            lpfi->dwFileType |= FI_TYPE_MASK_FIRST;

    SetWindowLong(hwndChild, GWL_USERDATA, (LONG)lpfi);
    EditView_SetModify(hwndEdit, FALSE);
    SetWindowText(hwndChild, pszFile);

    tci.mask = TCIF_TEXT | TCIF_PARAM;
    tci.pszText = Path_GetFileName(pszFile);
    tci.lParam = (LPARAM)hwndChild;
    TabCtrl_SetCurSel(g_hwndTabWindowBar, TabCtrl_InsertItem(g_hwndTabWindowBar, TabCtrl_GetItemCount(g_hwndTabWindowBar), &tci));

    MDI_Child_Edit_EnableToolbarButtons(hwndChild, FALSE);

    Window_UpdateLayout(g_hwndTabWindowBar);
    RedrawWindow(g_hwndTabWindowBar, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
}

void File_Close(HWND hwnd)
{
    FILEINFO fi;

    Main_GetFileInfo(hwnd, &fi);

    if (!(fi.dwFileType & FI_TYPE_MASK_EDIT))
        FileNotify_RemoveFile(&s_fns, fi.szFileName);

    Main_UpdateWindowBar(hwnd, WM_CLOSE, (LPARAM)NULL);
    Mem_Free((LPVOID)GetWindowLong(hwnd, GWL_USERDATA));
    DefMDIChildProc(hwnd, WM_CLOSE, 0, 0L);
}

UINT CALLBACK File_OFNHookProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static LPINT lpnLineFeedType;

    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        OPENFILENAME *lpofn = (OPENFILENAME *)lParam;
        HWND hwndCbo = GetDlgItem(hwnd, IDC_COMBO_LINEFEED);
        int nSelectedIndex = 0;
        int i;
        struct tagLINEFEEDCOMBOITEM
        {
            TCHAR   szText[32];
            int     nType;
        } ts[4] =
        {
            { _T("Automatic"), CRLF_STYLE_AUTOMATIC },
            { _T("DOS"), CRLF_STYLE_DOS },
            { _T("UNiX"), CRLF_STYLE_UNIX },
            { _T("Mac"), CRLF_STYLE_MAC }
        };

        // Set a pointer to the passed on int)
        lpnLineFeedType = (int *)lpofn->lCustData;

        for (i = 0; i < DIMOF(ts); i++)
        {
            int nIndex = ComboBox_AddString(hwndCbo, ts[i].szText);
            if (nIndex != CB_ERR)
            {
                if (ts[i].nType == *lpnLineFeedType)
                    nSelectedIndex = nIndex;

                ComboBox_SetItemData(hwndCbo, nIndex, ts[i].nType);
            }
        }

        ComboBox_SetCurSel(hwndCbo, nSelectedIndex);
    }
    return (FALSE);
    case WM_NOTIFY:
    {
        LPNMHDR lpnmh = (LPNMHDR)lParam;

        switch (lpnmh->code)
        {
        case CDN_SHAREVIOLATION:
            SetWindowLong(hwnd, DWL_MSGRESULT, TRUE);
        return (TRUE);
        case CDN_FILEOK:
        {
            HWND hwndCbo = GetDlgItem(hwnd, IDC_COMBO_LINEFEED);
            int nIndex = ComboBox_GetCurSel(hwndCbo);

            if (nIndex != CB_ERR)
                *lpnLineFeedType = (int)ComboBox_GetItemData(hwndCbo, nIndex);
            else
                *lpnLineFeedType = CRLF_STYLE_AUTOMATIC;

            SetWindowLong(hwnd, DWL_MSGRESULT, TRUE);
        }
        return (TRUE);
        }
    }
    break;
    }

    return (FALSE);
}
