/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_tip_of_da_day.c
 * Created    : not known (before 01/04/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:32:23
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

#include "resource.h"

/* pcp_controls */
#include "pcp_tip_of_da_day.h"

/* pcp_paint */
#include <pcp_paint.h>

/* pcp_generic */
#include <pcp_path.h>
#include <pcp_rect.h>
#include <pcp_string.h>
#include <pcp_window.h>

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

#define MAX_TIP_LEN     1024

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

BOOL CALLBACK TipOfDay_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL TipOfDay_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void TipOfDay_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode);
static HBRUSH TipOfDay_OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);
static void TipOfDay_OnPaint(HWND hwnd);
static void TipOfDay_OnClose(HWND hwnd);

static void TipOfDay_DoPaint(HWND hwnd, HDC hdc, BOOL bUpdateTextOnly);
static BOOL TipOfDay_GetNextTip(LPTSTR pszTip);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

// Could just pass value of Startup back to the stuff calling

// Set up the default values
static TCHAR s_szTipFileName[MAX_PATH] = _T("Tips.tip");
static HBITMAP s_hBulbBitmap = NULL;
static FILE *s_fTips = NULL;
static RECT s_rcDraw;
static TCHAR s_szCurrentTip[MAX_TIP_LEN];
static BOOL s_bAtStartup;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

BOOL TipOfDay_CreateDialogBox(HWND hwndParent, BOOL bAtStartup)
{
    s_bAtStartup = bAtStartup;

    DialogBoxParam(PCPControls_GetHandle(),
                        MAKEINTRESOURCE(IDD_TIPOFDAY), hwndParent,
                        TipOfDay_DlgProc, 0);

    return (s_bAtStartup);
}

void TipOfDay_SetTipFileName(LPCTSTR pszFileName)
{
    _tcsncpy(s_szTipFileName, pszFileName, MAX_PATH);
}

void TipOfDay_SetBitmap(HBITMAP hBitmap)
{
    s_hBulbBitmap = Paint_Bitmap_CopyBitmap(hBitmap);
}

BOOL CALLBACK TipOfDay_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_DLG_MSG(hwndDlg, WM_INITDIALOG,      TipOfDay_OnInitDialog);
        HANDLE_DLG_MSG(hwndDlg, WM_COMMAND,         TipOfDay_OnCommand);
        HANDLE_DLG_MSG(hwndDlg, WM_CTLCOLOREDIT,    TipOfDay_OnCtlColor);
        HANDLE_DLG_MSG(hwndDlg, WM_PAINT,           TipOfDay_OnPaint);
        HANDLE_DLG_MSG(hwndDlg, WM_CLOSE,           TipOfDay_OnClose);
    }

    return (FALSE);
}

static BOOL TipOfDay_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    TCHAR szTipFile[MAX_PATH] = _T("");
    BOOL bMoreTips;

    if (GetModuleFileName(PCPControls_GetHandle(), szTipFile, MAX_PATH))
        Path_RemoveFileName(szTipFile);
    
    _tcscat(szTipFile, s_szTipFileName);

    if ((s_fTips = _tfopen(szTipFile, _T("r"))) == NULL)
    {
        HANDLE hOldHandle = String_SetResourceHandle(PCPControls_GetHandle());
        
        _tcscpy(s_szCurrentTip, String_LoadString(IDS_TIPFILE_NOTFOUND));
        String_SetResourceHandle(hOldHandle);
    }
    
    GetWindowRect(GetDlgItem(hwnd, IDC_STATIC_FRAME), &s_rcDraw);
    Window_ScreenToClientRect(hwnd, &s_rcDraw);
    DestroyWindow(GetDlgItem(hwnd, IDC_STATIC_FRAME));

    CheckDlgButton(hwnd, IDC_CHK_STARTUP, s_bAtStartup);

    if (s_hBulbBitmap == NULL)
        s_hBulbBitmap = LoadBitmap(PCPControls_GetHandle(), MAKEINTRESOURCE(IDB_BULB));

    SendMessage(GetDlgItem(hwnd, IDC_BMP_BULB), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hBulbBitmap);

    bMoreTips = TipOfDay_GetNextTip(s_szCurrentTip);

    if (!bMoreTips)
        Window_EnableDlgItem(hwnd, IDC_BTN_NEXTTIP, FALSE);

    return (TRUE);
}

static void TipOfDay_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode)
{
    switch (idCtl)
    {
    case IDOK:
    {
        BOOL bMoreTips;
        HDC hdc = GetDC(hwnd);

        bMoreTips = TipOfDay_GetNextTip(s_szCurrentTip);
        TipOfDay_DoPaint(hwnd, hdc, TRUE);
        ReleaseDC(hwnd, hdc);

        if (!bMoreTips)
            EnableWindow(hwndCtl, FALSE);
    }
    break;
    case IDCANCEL:
        SendMessage(hwnd, WM_CLOSE, 0, 0L);
    break;
    }
}

static HBRUSH TipOfDay_OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    if (GetWindowLong(hwndChild, GWL_ID) == IDC_TXT_TIPSTRING)
        return (HBRUSH)GetStockObject(WHITE_BRUSH);

    return ((HBRUSH)DefWindowProc(hwnd, WM_CTLCOLOREDIT, (WPARAM)hdc, (LPARAM)hwndChild));
}

static void TipOfDay_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    TipOfDay_DoPaint(hwnd, hdc, FALSE);

    EndPaint(hwnd, &ps);
}

static void TipOfDay_OnClose(HWND hwnd)
{
    if (s_fTips != NULL)
        fclose(s_fTips);
    s_bAtStartup = IsDlgButtonChecked(hwnd, IDC_CHK_STARTUP);
    DeleteObject(s_hBulbBitmap);

    EndDialog(hwnd, 0);
}

static void TipOfDay_DoPaint(HWND hwnd, HDC hdc, BOOL bUpdateTextOnly)
{
    RECT rcTip, rcSeperator, rcDidYou, rcBulb, rc = s_rcDraw;
    HBRUSH hBrush;
    HWND hwndStatic;
    HFONT hDidYouFont, hTipFont, hOldFont;
    TCHAR szDidYou[32];
    HANDLE hOldHandle;
    COLORREF crOldBackground;

    hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);

    hwndStatic = GetDlgItem(hwnd, IDC_TXT_TIPSTRING);
    GetWindowRect(hwndStatic, &rcTip);
    Window_ScreenToClientRect(hwnd, &rcTip);

    hwndStatic = GetDlgItem(hwnd, IDC_BMP_BULB);
    GetWindowRect(hwndStatic, &rcBulb);

    crOldBackground = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));

    if (!bUpdateTextOnly)
    {
        HBRUSH hBrushShadow = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
        HBRUSH hOldBrush;
        HDC hdcMem;
        HBITMAP hOldBitmap;
        BITMAP bm;

        Paint_Draw3DRect(hdc, &rc, GetSysColor(COLOR_BTNSHADOW), GetSysColor(COLOR_BTNSHADOW), 1);
        InflateRect(&rc, -1, -1);
        Paint_FillRect(hdc, &rc, GetSysColor(COLOR_WINDOW));
        
        rcSeperator = rc;

        rc.right = rcTip.left - 10;
        Paint_FillRect(hdc, &rc, GetSysColor(COLOR_BTNSHADOW));

        GetObject(s_hBulbBitmap, sizeof(BITMAP), &bm);

        Window_ScreenToClientRect(hwnd, &rcBulb);
        hdcMem = CreateCompatibleDC(hdc);
        hOldBitmap      = SelectObject(hdcMem, s_hBulbBitmap);
        rcBulb.bottom   = rcBulb.top + bm.bmHeight;
        rcBulb.right    = rcBulb.left + bm.bmWidth;
        hOldBrush       = SelectObject(hdcMem, hBrushShadow);
        ExtFloodFill(hdcMem, 0, 0, RGB(255, 255, 255), FLOODFILLSURFACE);
        BitBlt(hdc, rcBulb.left, rcBulb.top, Rect_Width(&rcBulb), Rect_Height(&rcBulb),
                hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, hOldBitmap);
        DeleteObject(SelectObject(hdcMem, hOldBrush));
        DeleteDC(hdcMem);

        rcSeperator.top     = rcBulb.bottom + 6;
        rcSeperator.bottom  = rcSeperator.top + 1;

        Paint_FillRect(hdc, &rcSeperator, GetSysColor(COLOR_BTNSHADOW));
    }

    hDidYouFont = CreateFont(22, 0, 0, 0, FW_BOLD, 0, 0, 0,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH | FF_SWISS, _T("Times"));

    hTipFont    = CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH | FF_SWISS, _T("Arial"));

    hOldFont = SelectObject(hdc, hDidYouFont);
    hOldHandle = String_SetResourceHandle(PCPControls_GetHandle());
    _tcsncpy(szDidYou, String_LoadString(IDS_TIP_DIDYOU), sizeof(szDidYou));
    String_SetResourceHandle(hOldHandle);
    rcDidYou        = rcTip;
    rcDidYou.top    = rcBulb.top;
    DrawText(hdc, szDidYou, -1, &rcDidYou, DT_SINGLELINE);

    SelectObject(hdc, hTipFont);
    Paint_FillRect(hdc, &rcTip, GetSysColor(COLOR_WINDOW));
    DrawText(hdc, s_szCurrentTip, -1, &rcTip, DT_WORDBREAK);
    SelectObject(hdc, hOldFont);
    SetBkColor(hdc, crOldBackground);
    DeleteObject(hDidYouFont);
    DeleteObject(hTipFont);
}

static BOOL TipOfDay_GetNextTip(LPTSTR pszTip)
{
    BOOL bStop = FALSE;

    if (s_fTips == NULL)
        return (FALSE);

    while (!bStop)
    {
        if (_fgetts(pszTip, MAX_TIP_LEN, s_fTips) == NULL)
        {
            if (fseek(s_fTips, 0, SEEK_SET) != 0)
            {
                HANDLE hOldHandle = String_SetResourceHandle(PCPControls_GetHandle());

                _tcscpy(pszTip, String_LoadString(IDS_TIPFILE_CORRUPT));
                String_SetResourceHandle(hOldHandle);

                return (FALSE);
            }
        }
        else
        {
            if (*pszTip != _T(' ') && *pszTip != _T('\t') &&
                *pszTip != _T('\n') && *pszTip != _T(';'))
            {
                bStop = TRUE;
            }
        }
    }

    return (TRUE);
}
