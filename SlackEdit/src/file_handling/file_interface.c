/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : file_interface.c
 * Created    : 08/15/00
 * Owner      : pcppopper
 * Revised on : 08/15/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "../pcp_generic/pcp_includes.h"

/* pcp_generic */
#include <pcp_linkedlist.h>

/* pcp_edit */
#include <pcp_edit.h>

/****************************************************************
 * Type Definitions
 ****************************************************************/

/****************************************************************
 * Function Definitions
 ****************************************************************/

static PVOID FileList_CreateData(PVOID pData);
static INT FileList_DeleteData(PVOID pData);
static INT FileList_CompareData(const PVOID pData, const PVOID pFromCall, UINT uType);

static void FileInterface_ChangeNotifyCallback(LPFILENOTIFYDATA lpfnd, DWORD dwChange);

/****************************************************************
 * Global Variables
 ****************************************************************/

PLINKEDLIST s_lpFileList;
static LPFILENOTIFYSTRUCT s_lpNotifyStruct;

/****************************************************************
 * Function Implementations
 ****************************************************************/

BOOL FileInterface_InitializeFileHandling(void)
{
    s_lpFileList = List_CreateList(NULL, FileList_DeleteData,
                                    NULL, FileList_CompareData, 0);

    ASSERT(s_lpFileList != NULL);

    FileNotify_CreateNotifier(s_lpNotifyStruct, FileInterface_ChangeNotifyCallback);

    return (TRUE);
}

void FileInterface_DestroyFileHandling(void)
{
    List_DestroyList(s_lpFileList);
    FileNotify_DestroyNotifier(s_lpNotifyStruct);
}

static INT FileList_DeleteData(PVOID pData)
{
    Mem_Free(((LPFILEVIEW)pdata)->pszFileName);

    return (Mem_Free(pData));
}

static INT FileList_CompareData(const PVOID pData, const PVOID pFromCall, UINT uType)
{
    LPFILEVIEW lpNodeData = (LPFILEVIEW)pData;

    switch (uType)
    {
    case FILE_FIND_BY_VIEW_HWND:
        if (lpNodeData->hwndView == (HWND)pFromCall)
            return (0);
    return (-1);
    case FILE_FIND_BY_EDIT_HWND:
        if (lpNodeData->hwndEdit == (HWND)pFromCall)
            return (0);
    return (-1);
    case FILE_FIND_BY_FILENAME:
    return (_tcsicmp(lpNodeData->pszFileName, (LPCTSTR)pFromCall));
    }

    /* unreachable */

    ASSERT(FALSE);

    return (-1);
}

LPFILEVIEW FileInterface_GetFileView(const LPVOID lpData, UINT uDataType)
{
    static FILEINFO fiDefault = { NULL, FILE_TYPE_MASK_EDIT | FILE_TYPE_NOTHING, NULL, NULL };
    PLISTNODE pNode;

    pNode = List_FindNode(s_lpFileList, lpData, uDataType);

    if (pNode == NULL)
        return (&fiDefault);
    else
        return ((LPFILEVIEW)pNode->pData);
}

static void FileInterface_ChangeNotifyCallback(LPFILENOTIFYDATA lpfnd, DWORD dwChange)
{
    LPFILEVIEW lpFileView;

    lpFileView = FileInterface_GetFileView(lpfnd->pszFileName, FILE_FIND_BY_FILENAME);

    if ((dwChange & FN_CHANGED_TIME) || (dwChange & FN_CHANGED_SIZE))
    {
        TCHAR szMsg[MAX_PATH + 64];

        MDI_Activate(g_hwndMDIClient, lpFileData->hwndView);
        
        _stprintf(szMsg, String_LoadString(IDS_FORMAT_FILE_MODIFIED), lpfnd->pszFileName);

        if (MessageBox(g_hwndMDIClient, szMsg, _T("Reload File?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
            FileInterface_ReloadFile(lpFileView);
    }
    else if (dwChange & FN_DELETED)
    {
        TCHAR szMsg[MAX_PATH + 64];

        MDI_Activate(g_hwndMDIClient, lpFileView->hwndView);

        _stprintf(szMsg, String_LoadString(IDS_FORMAT_FILE_DELETED), lpfnd->pszFileName);
        MessageBox(g_hwndMDIClient, szMsg, _T("File Deleted"), MB_OK | MB_ICONWARNING);

        PCP_Edit_SetModified(lpFileView->lpEditInterface, TRUE);

        FileNotify_RemoveFile(s_lpNotifyStruct, lpfnd->pszFileName);
    }
}

BOOL FileInterface_ReloadFile(LPFILEVIEW lpFileView)
{
    PCP_Edit_LoadFile(lpFileView->lpEditInterface, lpFileView->pszFileName);
}

BOOL FileInterface_OpenFile_WithDialog(void)
{
    TCHAR szFileNames[MAX_PATH * 4] = _T("");
    OPENFILENAME ofn;
    LPFILEVIEW lpFileView;
    OPENSAVEFILEINFO osfi;

    lpFileView = FileInterface_GetFileView(MDI_GetActive(g_hwndMDIClient), FILE_FIND_BY_VIEW_HWND);

    osfi.dwFileType     = lpFileView->dwFileType;
    osfi.nLineFeedMode  = CRLF_STYLE_AUTOMATIC;

    INITSTRUCT(ofn, TRUE);
    ofn.hwndOwner           = g_hwndMain;
    ofn.hInstance           = g_hInstance;
    ofn.lpstrFilter         = String_MakeFilter(g_FilterSettings.szOpenFilter);
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = szFileNames;
    ofn.nMaxFile            = MAX_PATH * 4;

    if (FILEISSOMETHING(lpFileView))
    {
        TCHAR szFileName;

        _tcscpy(szFileName, lpFileView->pszFileName);
        ofn.lpstrInitialDir = Path_RemoveFileName(szFileName);
    }
    else
    {
        ofn.lpstrInitialDir = (LPTSTR)NULL;
    }
    
    ofn.lpstrTitle          = String_LoadString(IDS_TITLE_OPENFILESDIALOG);
    ofn.Flags               = OFN_CREATEPROMPT | OFN_ENABLESIZING | OFN_EXPLORER | OFN_ALLOWMULTISELECT;// | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;

    if (lpFileView->dwFileType & FILE_TYPE_TEXT)
        osfi.nLineFeedMode  = PCP_TextEdit_GetCRLFMode(lpFileView->lpEditInterface);

    ofn.lCustData           = (LPARAM)&ofi;
    ofn.lpfnHook            = FileInterface_OFNHookProc;
    ofn.lpTemplateName      = MAKEINTRESOURCE(IDD_OPENFILEDIALOG);

    if (GetOpenFileName(&ofn))
    {
        if ((GetFileAttributes(szFileNames) & FILE_ATTRIBUTE_DIRECTORY) ||
            (_tcslen(szFileName) == 3 && Path_HasRoot(szFileNames)))
        {
            SetWindowRedraw(g_hwndMain, FALSE);

            Browse_HandleMultipleFileNames(szFileNames, (BROWSEMULTIPLEFILECALLBACK)FileInterface_OpenFile, (LPARAM)&osfi);

            SetWindowRedraw(g_hwndMain, TRUE);
            RedrawWindow(g_hwndMain, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
        }
        else
        {
            FileInterface_OpenFile(szFileNames, &osfi);
        }

        return (TRUE);
    }

    return (FALSE);
}

BOOL FileInterface_SaveFile_WithDialog(LPFILEVIEW lpFileView)
{
    OPENFILENAME ofn;
    TCHAR pszFileName[MAX_PATH];
    OPENSAVEFILEINFO osfi;

    osfi.dwFileType = lpFileView->dwFlags;

    if (FILEISSOMETHING(lpFileView))
    {
        osfi.bBackup    = g_BackupSettings.bBackup;

        if (lpFileView->dwFlags & FILE_TYPE_TEXT)
            osfi.nLineFeedMode = PCP_Edit_GetCRLFMode(lpFileView->lpEditInterface);

        _tcscpy(szFileName, lpFileView->pszFileName)
    }
    else
    {
        osfi.nLinefeedMode  = CRLF_STYLE_AUTOMATIC;
        _tcscpy(szFileName, _T(""));
    }

    INITSTRUCT(ofn, TRUE);
    ofn.hwndOwner           = g_hwndMain;
    ofn.hInstance           = g_hInstance;
    ofn.lpstrFilter         = String_MakeFilter(g_FilterSettings.szSaveFilter);
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = szFileName;
    ofn.nMaxFile            = MAX_PATH;
    ofn.lpstrTitle          = String_LoadString(IDS_TITLE_SAVEFILEASDIALOG);
    ofn.Flags               = OFN_ENABLESIZING | OFN_EXPLORER | OFN_ALLOWMULTISELECT;// | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
    ofn.lCustData           = (LPARAM)&osfi;
    ofn.lpfnHook            = FileInterface_OFNHookProc;
    ofn.lpTemplateName      = MAKEINTRESOURCE(IDD_OPENFILEDIALOG);

    if (!GetSaveFileName(&osfi))
        return (FALSE);
    else
    {
        osfi.bUpdateFileView    = TRUE;
        osfi.bVisual            = TRUE;

        return (FileInterface_SaveFile(lpFileView, szFileName, &osfi);
    }
}

BOOL FileInterface_OpenFile(LPCTSTR pszFileName, LPOPENSAVEFILEINFO lposfi)
{
    MDICREATESTRUCT mcs;
    HWND hwndActive;
    HCURSOR hOldCursor;
    LPFILEVIEW lpFileView;
    int i;
    PLISTNODE pNode;

    pNode = List_FindNode(s_lpFileList, pszFileName, FILE_FIND_BY_FILENAME);

    if (pNode != NULL)
    {
        lpFileView = (LPFILEVIEW)pNode->pData;

        ASSERT(FILEISSOMETHING(lpFileView));

        MDI_Activate(g_hwndMDIClient, lpFileView->hwndView);


        if (PCP_Edit_GetModified(lpFileView->lpEditInterface))
        {
            TCHAR szMsg[MAX_PATH + 30];

            _stprintf(szMsg, String_LoadString(IDS_FORMAT_FILE_ALREADYOPEN), pszFile);

            if (MessageBox(g_hwndMain, szMsg, g_szAppName, MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                FileInterface_ReloadFile(pszFile);
            }
        }
    }

    SetCapture(g_hwndMain);
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    hwndActive  = MDI_GetActive(g_hwndMDIClient, NULL);

    lpFileView  = (LPFILEVIEW)Mem_Alloc(sizeof(FILEINFO));
    lpFileView->dwFileType = lposfi->dwFileType;

    if (lpFileView->dwFileType & FILE_TYPE_TEXT)
        mcs.szClass = g_szTextEditChildClass;
    else if (lpFileView->dwFileType & FILE_TYPE_TEXT)
        mcs.szClass = g_szHexEditChildClass;
    else
        ASSERT(FALSE);

    mcs.hOwner  = g_hInstance;
    mcs.x = mcs.cx = CW_USEDEFAULT;
    mcs.y = mcs.cy = CW_USEDEFAULT;
    mcs.style = MDIS_ALLCHILDSTYLES | WS_CLIPCHILDREN;

    if (lpFileView->dwFileType & FILE_TYPE_MASK_EDIT)
    {
        PLISTNODE pNode;
        int cEdits = 0;

        lpFileView->pszFileName = Mem_AllocStr(_tcslen(_T("Edit")) + 10);

        for (pNode = s_lpFileList->pHeadNode; pNode != NULL; pNode = pNode->pNextNode)
        {
            if (((LPFILEVIEW)pNode->pData)->dwFileType & FILE_TYPE_MASK_EDIT)
                cEdits++;
        }

        _stprintf(lpFileView->pszFileName, _T("Edit%d"), cEdits + 1);
    }
    else
    {
        ASSERT(Path_FileExists(pszFileName));

        lpFileView->pszFileName = String_Duplicate(pszFileName);
    }

    mcs.szTitle = lpFileView->pszFileName;
    mcs.lParam  = (LPARAM)lpFileView;

    if ((!IsWindow(hwndActive) && g_WindowSettings.nMDIWinState == MDISTATE_MAXIMIZED) ||
        (IsWindow(hwndActive) && IsMaximized(hwndActive)))
    {
        mcs.style |= WS_MAXIMIZE;
        g_WindowSettings.nMDIWinState = MDISTATE_MAXIMIZED;
    }

    lpFileView->hwndView = MDI_Create(g_hwndMDIClient, &mcs);

    ASSERT(lpFileView->hwndView != NULL);

    lpFileView->hwndEdit        = GetDlgItem(lpFileView->hwndView, IDC_EDIT_MDICHILD);
    lpFileView->lpEditInterface = PCP_Edit_GetInterface(lpFileView->hwndEdit);

    List_AddNodeAtTail(s_lpFileList, lpFileView);
    WindowBar_Update(lpFileView, WM_CREATE);
    Window_UpdateLayout(g_hwndTabWindowBar);
    RedrawWindow(g_hwndTabWindowBar, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

    if (!(lpFileView->dwFileType & FILE_TYPE_MASK_EDIT &&
            lpFileView->dwFileType & FILE_TYPE_MASK_FIRST)
    {
        LPFILEVIEW lpFileViewFirst = s_lpFileList->pHeadNode;

        if (lpFileViewFirst != NULL)
        {
            if ((lpFileViewFirst->dwFileType & FILE_TYPE_MASK_FIRST) &&
                (lpFileViewFirst->dwFileType & FILE_TYPE_MASK_EDIT))
            {
                ASSERT(IsWindow(lpFileViewFirst->hwndView));

                if (!PCP_Edit_GetModified(lpFileViewFirst->lpEditInterface))
                    FileInterface_CloseFile(lpFileViewFirst);
            }
        }
    }

    if (lpFileView->dwFileType & FILE_TYPE_TEXT)
    {
        Sub_TextEdit_SetCRLFMode(lpFileView,
                                    (nLineFeedType == CRLF_STYLE_AUTOMATIC) ?
                                    PCP_TextEdit_GetCRLFMode(lpFileView->lpEditInterface) :
                                    lposfi->nLineFeedMode);
    }

    FileNotify_AddFile(s_lpNotifyStruct, lpFileView->pszFileName, FN_NOTIFY_DEFAULT);
    FileNotify_StartMonitoring(s_lpNotifyStruct);

    MRU_Write(_T("File MRU"), pszFile, g_MRUSettings.nFileMax);
    for (i = IDM_RECENTFILES; DeleteMenu(g_hMenuRecentFiles, i, MF_BYCOMMAND); i++)
        ; /* empty body */
    MRU_SetMenu(g_hMenuRecentFiles, _T("File MRU"), g_MRUSettings.nFileMax, IDM_RECENTFILES);

    SetCursor(hOldCursor);
    ReleaseCapture();
}

BOOL FileInterface_SaveFile(LPFILEVIEW lpFileView, LPTSTR pszFileName, LPOPENSAVEFILEINFO lposfi)
{
    HCURSOR hOldCursor;
    LPTSTR pszBackupFileName = NULL;
    BOOL bSuccess = FALSE;

    ASSERT(FILEISSOMETHING(lpFileView));

    FileNotify_StopMonitoring(s_lpNotifyStruct);

    if (lposfi->bUpdateFileView)
    {
        SetCapture(g_hwndMain);
        hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        FileNotify_RemoveFile(s_lpNotifyStruct, lpFileView->pszFileName);
    }

    if (g_BackupSettings.bBackup)
    {
        pszBackupFileName = (LPTSTR)Mem_AllocStr(MAX_PATH);

        if (g_BackupSettings.bUseBackupDirectory && g_BackupSettings.szBackupDirectory[0] != _T('\0'))
        {
            _tcscpy(szBackupFileName, g_BackupSettings.szBackupDirectory);
            _tcscat(szBackupFileName, Path_GetFileName(pszFileName));
            Path_SwapExt(szBackupFileName, g_BackupSettings.szBackupExt);
        }
        else
        {
            _tcscpy(szBackupFileName, pszFileName);
            Path_SwapExt(szBackupFileName, g_BackupSettings.szBackupExt);
        }
    }

    if (lpFileView->dwFileType & FILE_TYPE_TEXT)
    {
        if (!PCP_TextEdit_SaveToFileEx(lpFileView->lpEditInterface,
                            pszFileName, lposfi->nLineFeedType, TRUE,
                            g_BackupSettings.bBackup, pszBackupFileName))
        {
            goto ErrorReturn;
        }
    }
    else
    {
        if (!PCP_Edit_SaveToFile(lpFileView->lpEditInterface, pszFileName))
        {
            goto ErrorReturn;
        }
    }

    if (lposfi->bUpdateFileView)
    {
        SetWindowText(lpFileView->hwndView, pszFileName);
        lpFileView->dwFileType &= ~(FI_TYPE_MASK_FIRST | FI_TYPE_MASK_EDIT);
        if (lpFileView->pszFileName != NULL)
            Mem_Free(lpFileView->pszFileName);
        lpFileView->pszFileName = String_Duplicate(pszFileName);
        FileNotify_AddFile(s_lpNotifyStruct, pszFileName, FN_NOTIFY_DEFAULT);
    }

    if (lposfi->bUpdateFileView && (lpFileView->dwFileType & FILE_TYPE_TEXT))
        PCP_TextEdit_SetTextType(lpFileView, Path_GetExt(pszFileName));

    if (lposfi->bUpdateFileView)
    {
        STATUSBARPANE sbp;
        TCHAR szStatusText;

        Path_CompactPath(szFileName, pszFileName, 50);
        _stprintf(szStatusText, String_LoadString(IDS_FORMAT_FILE_SAVE), szFileName);

        INITSTRUCT(sbp, FALSE);
        sbp.fMask       = SF_TEXT | SF_CRFG | SF_CRBG;
        sbp.pszText     = szStatusText;
        sbp.crBg        = RGB(0, 0, 127);
        sbp.crFg        = RGB(255, 255, 255);
        sbp.nPane       = 0;
        Statusbar_SetPane(g_hwndStatusbarMain, &sbp);
        WindowBar_Update(hwnd, TCM_SETTEXT, (LPARAM)Path_GetFileName(pszFileName));
    }

ErrorReturn:
    if (pszBackupFileName != NULL)
        Mem_Free(pszBackupFileName);

    SetCursor(hOldCursor);
    ReleaseCapture();

    FileNotify_StartMonitoring(s_lpNotifyStruct);

    return (bSuccess);
}

BOOL FileInterface_AutoSaveFile(LPFILEVIEW lpFileView)
{
    TCHAR szNewFileName[MAX_PATH];
    OPENSAVEFILEINFO osfi;

    if (lpFileView->dwFileType & FI_TYPE_MASK_EDIT)
    {
        if (g_BackupSettings.bUseAutoSaveDirectory && g_BackupSettings.szAutoSaveDirectory[0] != _T('\0'))
        {
            _tcscpy(szNewFileName, g_BackupSettings.szAutoSaveDirectory);
        }
        else
        {
            GetCurrentDirectory(MAX_PATH, szNewFileName);
            _tcscat(szNewFileName, _T("\\"));
            _tcscat(szNewFileName, lpFileView->pszFileName);
        }

        _tcscat(szNewFileName, g_BackupSettings.szAutoSaveExt);
    }
    else
    {
        _tcscpy(szNewFileName, lpFileView.pszFileName);
        _tcscat(szNewFileName, g_BackupSettings.szAutoSaveExt);
    }

    osfi.bBackup            = FALSE;
    osfi.bUpdateFileView    = FALSE;
    osfi.nLineFeedMode      = CRLF_STYLE_AUTOMATIC;

    FileInterface_SaveFile(lpFileView, szNewFile, &osfi);

    return (TRUE);
}

void FileInterface_CloseFile(LPFILEVIEW lpFileView)
{
    if (FILEISSOMETHING(lpFileView))
        FileNotify_RemoveFile(s_lpNotifyStruct, lpFileView->pszFileName);

    Main_UpdateWindowBar(hwnd, WM_CLOSE, (LPARAM)NULL);

    DefMDIChildProc(lpFileView->hwndView, WM_CLOSE, 0, 0L);

    List_DeleteNode(s_lpFileList, List_FindNode(s_lpFileList, lpFileView->hwndView, FILE_FIND_BY_VIEW_HWND));
}

UINT CALLBACK FileInterface_OFNHookProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static LPOPENSAVEFILEINFO lposfi;

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
        lposfi = (LPOPENSAVEFILEINFO)lpofn->lCustData;

        for (i = 0; i < DIMOF(ts); i++)
        {
            int nIndex = ComboBox_AddString(hwndCbo, ts[i].szText);
            if (nIndex != CB_ERR)
            {
                if (ts[i].nType == lposfi->nLineFeedMode)
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
                lposfi->nLineFeedMode   = (int)ComboBox_GetItemData(hwndCbo, nIndex);
            else
                lposfi->nLineFeedMode   = CRLF_STYLE_AUTOMATIC;

            SetWindowLong(hwnd, DWL_MSGRESULT, TRUE);
        }
        return (TRUE);
        }
    }
    break;
    }

    return (FALSE);
}
