/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : cmdlinewin.c
 * Created    : not known (before 01/30/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:11:47
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>
#include <process.h>

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "../settings/settings.h"
#include "../windows/wnd_output.h"
#include "../subclasses/sub_pcp_edit.h"

/* pcp_edit */
#include <pcp_edit.h>

/* pcp_generic */
#include <pcp_string.h>
#include <pcp_rect.h>
#include <pcp_window.h>
#include <pcp_mem.h>
#include <pcp_editctrl.h>
#include <pcp_mru.h>
#include <pcp_browse.h>
#include <pcp_path.h>

#define MAX_CMD     524

typedef struct tagPIPEDATA
{
    LPTSTR lpCmdLine;
    LPTSTR lpWorkDir;
} PIPEDATA, *LPPIPEDATA;

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

BOOL CALLBACK CmdLine_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CmdLine_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void CmdLine_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode);
static void CmdLine_OnClose(HWND hwnd);

DWORD WINAPI CmdLine_PipingProc(LPVOID lpParameter);
static LPTSTR CmdLine_ProcessCommand(LPTSTR pszCommand);

static BOOL bWinCmd;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

void CmdLine_Initialize(HWND hwndOwner, BOOL bWinCmd)
{
    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_DOSCMD), hwndOwner, CmdLine_DlgProc, (LPARAM)bWinCmd);
}

BOOL CALLBACK CmdLine_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_DLG_MSG(hwndDlg, WM_INITDIALOG,  CmdLine_OnInitDialog);
        HANDLE_DLG_MSG(hwndDlg, WM_COMMAND,     CmdLine_OnCommand);
        HANDLE_DLG_MSG(hwndDlg, WM_CLOSE,       CmdLine_OnClose);
    }
    return (FALSE);
}

static BOOL CmdLine_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HWND hwndCboCmd = GetDlgItem(hwnd, IDC_COMBO_COMMAND);
    HWND hwndCboWd  = GetDlgItem(hwnd, IDC_COMBO_WORKINGDIR);

    bWinCmd = (BOOL)lParam;

    Edit_LimitText(hwndCboCmd, MAX_PATH);
    Edit_LimitText(hwndCboWd, MAX_CMD);

    if (g_WindowSettings.bCmdLineCentered)
        Window_CenterWindow(hwnd);
    else
        SetWindowPos(hwnd, 0, g_WindowSettings.nCmdLineXPos, g_WindowSettings.nCmdLineYPos,
                0, 0, SWP_NOZORDER | SWP_NOSIZE);

    if (bWinCmd == TRUE) // Win Command
    {
        SetWindowText(hwnd, _T("Winslows Command"));
        MRU_SetComboBox(hwndCboCmd, _T("WINCMD Commands"), g_MRUSettings.nWinCmdMax);
        MRU_SetComboBox(hwndCboWd, _T("WINCMD WorkDirs"), g_MRUSettings.nWinWorkDirMax);
    }
    else
    {
        MRU_SetComboBox(hwndCboCmd, _T("DOSCMD Commands"), g_MRUSettings.nDosCmdMax);
        MRU_SetComboBox(hwndCboWd, _T("DOSCMD WorkDirs"), g_MRUSettings.nDosWorkDirMax);
    }

    return (TRUE);
}

static void CmdLine_OnClose(HWND hwnd)
{
    RECT r;
    GetWindowRect(hwnd, &r);

    g_WindowSettings.nCmdLineXPos = r.left;
    g_WindowSettings.nCmdLineYPos = r.top;

    EndDialog(hwnd, 0);
}

static void CmdLine_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode)
{
    switch (idCtl)
    {
        case IDC_BUTTON_PATH_BROWSE:
        {
            BROWSEINFOEX bix;
            TCHAR szStartPath[MAX_PATH] = _T("");
            TCHAR szPath[MAX_PATH] = _T("");

            GetDlgItemText(hwnd, IDC_COMBO_WORKINGDIR, szStartPath, MAX_PATH);
            INITSTRUCT(bix, FALSE);
            bix.hwndOwner       = hwnd;
            bix.pszStartPath    = szStartPath;
            bix.pszPath         = szPath;

            if (Browse_ForPath(&bix))
                SetDlgItemText(hwnd, IDC_COMBO_WORKINGDIR, szPath);
        }
        break;
        case IDC_BUTTON_FILE_BROWSE:
        {
            BROWSEINFOEX bix;
            TCHAR szPath[MAX_PATH] = _T("");
            TCHAR szFile[MAX_PATH] = _T("");

            GetDlgItemText(hwnd, IDC_COMBO_COMMAND, szPath, MAX_PATH);
            INITSTRUCT(bix, FALSE);
            bix.hwndOwner   = hwnd;
            bix.pszPath     = szPath;
            bix.pszFile = szFile;
            bix.pszTitle    = _T("Browse for DOS executable to run");
            bix.pszFilter   = String_MakeFilter(g_FilterSettings.szOpenFilter);

            if (Browse_ForFile(&bix))
            {
                TCHAR drive[2], dir[MAX_PATH], fname[MAX_PATH], ext[3];

                _tcscpy(szPath, szFile);
                _splitpath(szPath, drive, dir, fname, ext);

                _stprintf(szPath, _T("%s%s"), drive, dir);
    
                SetDlgItemText(hwnd, IDC_COMBO_WORKINGDIR, szPath);
                SetDlgItemText(hwnd, IDC_COMBO_COMMAND, szFile);
            }
        }
        break;
        case IDOK:
        {
            TCHAR szCmdLine[MAX_CMD], szWorkDir[MAX_PATH];
            LPTSTR pszCmd;

            GetDlgItemText(hwnd, IDC_COMBO_COMMAND, szCmdLine, MAX_CMD);
            GetDlgItemText(hwnd, IDC_COMBO_WORKINGDIR, szWorkDir, MAX_PATH);

            pszCmd = CmdLine_ProcessCommand(szCmdLine);

            if (bWinCmd)
            {
                MRU_Write(_T("WINCMD Commands"), szCmdLine, g_MRUSettings.nWinCmdMax);
                MRU_Write(_T("WINCMD WorkDirs"), szWorkDir, g_MRUSettings.nWinWorkDirMax);

                ShellExecute(g_hwndMain, _T("open"), pszCmd, NULL, szWorkDir, SW_SHOWDEFAULT);
            }
            else
            {
                HANDLE hThread;
                DWORD dwThreadID;
                LPPIPEDATA lppd = Mem_Alloc(sizeof(PIPEDATA));

                MRU_Write(_T("DOSCMD Commands"), szCmdLine, g_MRUSettings.nDosCmdMax);
                MRU_Write(_T("DOSCMD WorkDirs"), szWorkDir, g_MRUSettings.nDosWorkDirMax);

                if (!IsWindow(g_hwndOutputWindow))
                    g_hwndOutputWindow = OutputWindow_Create(g_hwndMain);

                lppd->lpCmdLine = (LPTSTR)Mem_AllocStr(_tcslen(pszCmd));
                _tcscpy(lppd->lpCmdLine, pszCmd);
                lppd->lpWorkDir = (LPTSTR)Mem_AllocStr(_tcslen(szWorkDir));
                _tcscpy(lppd->lpWorkDir, szWorkDir);

                if ((hThread = BEGINTHREADEX(0, 0, CmdLine_PipingProc, lppd, 0, &dwThreadID)) == NULL)
                {
                    MB(_T("Failed to create thread"));
                }
                else
                {
                    CloseHandle(hThread);
                }
            }

            // Free the string Mem_Alloc()'d in CmdLine_ProcessCommand
            Mem_Free(pszCmd);
        }
        case IDCANCEL:
            Window_Close(hwnd);
        break;
        default:
        break;
    }
}

DWORD WINAPI CmdLine_PipingProc(LPVOID lpParameter)
{
    SECURITY_ATTRIBUTES sa;
    HANDLE              hReadPipe, hWritePipe;
    DWORD               dwBytesRead;
    LPEDITINTERFACE lpInterface = PCP_Edit_GetInterface(GetDlgItem(g_hwndOutputWindow, IDC_OUTPUTWINDOW_EDIT));
    LPPIPEDATA lppd = (LPPIPEDATA)lpParameter;

    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle       = TRUE;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
    {
        MB(_T("Error Creating Pipe"));
    }
    else
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        if (*(lppd->lpWorkDir) == _T('\0'))
            lppd->lpWorkDir = NULL;

        Window_SetRedraw(hwnd, FALSE);
        PCP_Edit_SetSelectionPos(lpInterface, 0, -1);
        PCP_Edit_DeleteCurrentSelection(lpInterface);
        Window_SetRedraw(lpew->hwnd, TRUE);
        RedrawWindow(lpew->hwnd, NULL, NULL, RDW_INVALIDATE);

        si.cb       = sizeof(STARTUPINFO);
        GetStartupInfo(&si);
        
        si.hStdOutput   = si.hStdError = hWritePipe;
        si.dwFlags      |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        si.wShowWindow  = SW_HIDE;

        if (!CreateProcess(NULL, lppd->lpCmdLine, NULL, NULL, TRUE, 0,
                        NULL, lppd->lpWorkDir, &si, &pi))
        {
            MB(_T("Error Creating Process"));
        }
        else
        {
            TCHAR szBuffer[1024];
            int x;
            int y;

            SetFocus(GetDlgItem(g_hwndOutputWindow, IDC_OUTPUTWINDOW_EDIT));

            CloseHandle(hWritePipe);

            while (TRUE)
            {
                ZeroMemory(szBuffer, 1024 * sizeof(TCHAR));

                if (!ReadFile(hReadPipe, szBuffer, 1023, &dwBytesRead, NULL) &&
                    GetLastError() == ERROR_BROKEN_PIPE)
                {
                    break;
                }

                _tcscat(szBuffer, _T("\r\n")); //we want a new line each time

                PCP_Edit_InsertText(lpInterface, PCP_Edit_GetLineCount(lpInterface) - 1, 0, szBuffer, &x, &y, CE_ACTION_INTERNALINSERT);
                PCP_Edit_EnsureVisible(lpInterface, PCP_Edit_GetCursorPos(lpew));
            }
        }

        CloseHandle(hReadPipe);
    }

    Mem_Free(lppd->lpCmdLine);
    if (lppd->lpWorkDir != NULL)
        Mem_Free(lppd->lpWorkDir);
    Mem_Free(lppd);

    _endthreadex(0);

    return (0);
}

static LPTSTR CmdLine_ProcessCommand(LPTSTR pszCommand)
{
    LPTSTR pszProcessed = (LPTSTR)Mem_AllocStr(_tcslen(pszCommand));
    LPTSTR psz = pszCommand;
    LPTSTR pszProcessedBase = pszProcessed;
    FILEINFO fi;

    Main_GetFileInfo(NULL, &fi);

    while (*psz != _T('\0'))
    {
        if (*psz == _T('%'))
        {
            switch (*(++psz))
            {
                case _T('%'): // Escape
                    *(pszProcessed++) = *psz;
                break;
                case _T('d'): // Insert Directory
                case _T('D'):
                {
                    TCHAR szPath[MAX_PATH];

                    _tcscpy(szPath, fi.szFileName);
                    Path_RemoveFileName(szPath);

                    pszProcessed = Mem_ReAllocStr(pszProcessed,
                        (Mem_SizeStr(pszProcessed) + _tcslen(szPath) - 2 + SZ));
                    _tcscat(pszProcessed, szPath);
                }
                break;
                case _T('p'): // Insert Path
                case _T('P'):
                {
                    pszProcessed = Mem_ReAllocStr(pszProcessed,
                        (Mem_SizeStr(pszProcessed) + _tcslen(fi.szFileName) - 2 + SZ));

                    _tcscat(pszProcessed, fi.szFileName);
                }
                break;
                case _T('f'): // Insert Filename
                case _T('F'):
                {
                    TCHAR pszTemp[MAX_PATH];

                    _tcscpy(pszTemp, Path_GetFileName(fi.szFileName));

                    pszProcessed = Mem_ReAllocStr(pszProcessed,
                        (Mem_SizeStr(pszProcessed) + _tcslen(pszTemp) - 2 + SZ));
                
                    _tcscat(pszProcessed, pszTemp);
                }
                break;
                default:
                    *(pszProcessed++) = *psz;
                break;
            }

            // Skip over the 2nd char of the escape sequence (first in switch)
            psz++;
        }
        else
        {
            *(pszProcessed++) = *(psz++);
        }
    }

    return (pszProcessedBase);
}
