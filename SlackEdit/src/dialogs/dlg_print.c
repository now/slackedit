/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : print.c
 * Created    : not known   (before 05/31/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:05:06
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* windows */
#include <commdlg.h>

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../settings/settings_font.h"
#include "../settings/settings.h"

/* pcp_generic */
#include <pcp_window.h>
#include <pcp_mem.h>

/* pcp_edit */
#include <pcp_edit.h>

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

BOOL CALLBACK Print_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Print_AbortProc(HDC hPrinterDC, int iCode);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

BOOL bUserAbort;
HWND hwndPrintDlg;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

BOOL CALLBACK Print_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_GRAYED);
            if (g_WindowSettings.bPrintCentered)
                Window_CenterWindow(hwnd);
            else
                SetWindowPos(hwnd, 0, g_WindowSettings.nPrintXPos,
                        g_WindowSettings.nPrintYPos, 0, 0,
                        SWP_NOZORDER | SWP_NOSIZE);
        return (TRUE);
        case WM_COMMAND:
            bUserAbort = TRUE;
            EnableWindow(GetParent(hwnd), TRUE);
            DestroyWindow(hwnd);
            hwndPrintDlg = NULL;
        return (TRUE);
        case WM_DESTROY:
        {
            RECT r;
            GetWindowRect(hwnd, &r);

            g_WindowSettings.nPrintXPos = r.left;
            g_WindowSettings.nPrintYPos = r.top;
        }
        return (TRUE);
    }
    
    return (FALSE);
}
    
BOOL CALLBACK Print_AbortProc(HDC hPrinterDC, int iCode)
{
    MSG msg;

    for ( ; !bUserAbort && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ; )
    {
        if (!hwndPrintDlg || !IsDialogMessage(hwndPrintDlg, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return !bUserAbort;
}

BOOL Print_File(HINSTANCE hInstance, HWND hwnd, HWND hwndEdit, PTSTR szTitleName)
{
    static DOCINFO  di = { sizeof(DOCINFO) };
    static PRINTDLG pd = { sizeof(PRINTDLG), NULL, NULL, NULL, NULL,
                            PD_COLLATE | PD_RETURNDC,
                            0, 0, 0, 0, 1, NULL, 0L, NULL, NULL, NULL, NULL, NULL, NULL };
    BOOL            bSuccess;
    int             x, y, yChar, iCharsPerLine, iLinesPerPage, iTotalLines,
                    iTotalPages, iPage, iLine, iLineNum, iWidthLine, iHeightPage;
    PTSTR           pstrBuffer;
    TCHAR           szJobName[64 + MAX_PATH], szHeader[MAX_PATH + 64],
                    szFooter[64], szPage[20];
    TEXTMETRIC      tm;
    WORD            iColCopy, iNoiColCopy;
    SIZE            szMetric;
    HFONT           hFont;
    RECT            rc;
    POINT           p;
    LPEDITINTERFACE lpInterface;

    //Invoke Print common dialog box
    pd.hwndOwner    = hwnd;

    if (!PrintDlg(&pd))
        return (TRUE);

    lpInterface = PCP_Edit_GetInterface(hwndEdit);

    if ((iTotalLines = PCP_Edit_GetLineCount(lpInterface)) == 0)
        return (TRUE);

    hFont = Window_GetFont(hwndEdit);
    
    SelectObject(pd.hDC, hFont);

    //Calculate necessary metrics for file
    GetTextMetrics(pd.hDC, &tm);
    yChar = tm.tmHeight + tm.tmExternalLeading;

    iWidthLine      = (GetDeviceCaps(pd.hDC, HORZRES) - 150 * 2); 
    iCharsPerLine   = iWidthLine / tm.tmAveCharWidth;
    iHeightPage     = GetDeviceCaps(pd.hDC, VERTRES);
    iLinesPerPage   = ((iHeightPage / yChar) - 8);
    iTotalPages     = ((iTotalLines + iLinesPerPage - 1) / iLinesPerPage);

    //Allocate a buffer for each line of text
    pstrBuffer = (PTSTR)Mem_Alloc(/*sizeof(TCHAR) * */(iCharsPerLine + SZ));

    //Display the printing dialog box
    EnableWindow(hwnd, FALSE);

    bSuccess   = TRUE;
    bUserAbort = FALSE;

    hwndPrintDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_PRINT), hwnd, Print_DlgProc);

    SetDlgItemText(hwndPrintDlg, IDC_FILENAME, szTitleName);
    SetAbortProc(pd.hDC, Print_AbortProc);

    //Start the document
    GetWindowText(hwnd, szJobName, sizeof(szJobName));
    di.lpszDocName = szJobName;

    if (pd.Flags & PD_PAGENUMS)
        iTotalPages = pd.nMaxPage;
    
    if (StartDoc(pd.hDC, &di) > 0)
    {
        //Collation requires this loop and iNoiColCopy
        for (iColCopy = 0;
            iColCopy < ((WORD) pd.Flags & PD_COLLATE ? pd.nCopies : 1);
            iColCopy++)
        {
            for (iPage = (pd.Flags & PD_PAGENUMS) ? pd.nMinPage : 0;
                iPage < iTotalPages; iPage++)
            {
                for (iNoiColCopy = 0;
                    iNoiColCopy < (pd.Flags & PD_COLLATE ? 1 : pd.nCopies);
                    iNoiColCopy++)
                {
                    //Start the page
                    if (StartPage(pd.hDC) < 0)
                    {
                        bSuccess = FALSE;
                        break;
                    }

                    _stprintf(szPage, _T("Page: %d/%d"), (iPage + 1), iTotalPages);

                    SetDlgItemText(hwndPrintDlg, IDC_STATIC_PAGE, szPage);

                    _stprintf(szHeader, _T("File: %s"), szTitleName);

                    GetTextExtentPoint32(pd.hDC, szHeader, 
                                        _tcslen(szHeader), 
                                        &szMetric); 

                    x = 150; //((iWidthLine / 2) - (szMetric.cx / 2)); 
                    y = (szMetric.cy * 2);

                    TextOut(pd.hDC, x, y, szHeader, 
                            _tcslen(szHeader));

                    MoveToEx(pd.hDC, x, (y + szMetric.cy + 4), &p);
                    LineTo(pd.hDC, iWidthLine + 160, (y + szMetric.cy + 3));

                    //For each page, print the lines
                    for (iLine = 0; iLine < iLinesPerPage; iLine++)
                    {
                        iLineNum = iLinesPerPage * iPage + iLine;

                        if (iLineNum > iTotalLines)
                            break;

                        rc.left     = 150;
                        rc.right    = 150 + iWidthLine;
                        rc.top      = yChar * (iLine + 4);
                        rc.bottom   = iHeightPage - y * 2;

                        DrawText(pd.hDC, pstrBuffer,
                                PCP_Edit_GetLine(lpInterface, iLineNum, pstrBuffer),
                                &rc, DT_NOPREFIX | DT_EXPANDTABS);
                    }

                    _stprintf(szFooter, _T("Page: %d/%d"), (iPage + 1), iTotalPages);

                    GetTextExtentPoint32(pd.hDC, szFooter, 
                                        _tcslen(szFooter), 
                                        &szMetric); 

                    rc.left     = 150;
                    rc.right    = 150 + iWidthLine;
                    rc.top      = iHeightPage - (szMetric.cy * 2);
                    rc.bottom   = iHeightPage;

                    DrawText(pd.hDC, szFooter, _tcslen(szFooter), &rc,
                            DT_NOPREFIX | DT_EXPANDTABS | DT_RIGHT);

                    MoveToEx(pd.hDC, rc.left, (rc.top - 4), &p);
                    LineTo(pd.hDC, iWidthLine + 160, (rc.top - 4));

                    if (EndPage(pd.hDC) < 0)
                    {
                        bSuccess = FALSE;
                        break;
                    }

                    if (bUserAbort) break;
                }

                if (!bSuccess || bUserAbort) break;
            }

            if (!bSuccess || bUserAbort) break;
        }
    }
    else
        bSuccess = FALSE;

    if (bSuccess)
        EndDoc(pd.hDC);

    if (!bUserAbort)
    {
        EnableWindow(hwnd, TRUE);
        DestroyWindow(hwndPrintDlg);
    }

    Mem_Free(pstrBuffer);
    DeleteDC(pd.hDC);

    return (bSuccess && !bUserAbort);
}
