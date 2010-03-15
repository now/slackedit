/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_urlctrl.c
 * Created    : not known (before 12/01/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:38:21
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* windows */
#include <commctrl.h>
#include <intshcut.h>
#include <process.h>
#include <shlobj.h>
#include <winnetwk.h>
#include <winnls.h>

#include "resource.h"

/* pcp_controls */
#include "pcp_urlctrl.h"
#include "pcp_menu.h"

/* pcp_generic */
#include <pcp_clipboard.h>
#include <pcp_mem.h>
#include <pcp_string.h>
#include <pcp_window.h>

#define URL_STATE_NORMAL    0
#define URL_STATE_OVER      1
#define URL_STATE_DOWN      2

#define CX_SEPARATION       4

typedef struct tagURLCTRL
{
    WORD            wID;
    WORD            wState;
    DWORD           dwStyle;
    TCHAR           szTitle[MAX_PATH];
    TCHAR           szURL[MAX_PATH * 2];
    TCHAR           szDescription[MAX_PATH * 2];
    HWND            hwndOwner;
    HFONT           hFont;
    URLCTRLCOLORS   ucc;
    HBITMAP         hBitmap;
} URLCTRL, FAR *LPURLCTRL;

static HWND g_hwndToolTip = NULL;

enum Way
{
    OPEN,
    EDIT,
    PRINT
};

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

LRESULT CALLBACK UrlCtrl_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK UrlCtrl_Properties_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL UrlCtrl_OpenUsingShellExecute(LPURLCTRL lpuc, int nWay);
BOOL UrlCtrl_OpenUsingCom(LPURLCTRL lpuc, int nWay);
BOOL UrlCtrl_Open(LPURLCTRL lpuc, int nWay);

BOOL UrlCtrl_SaveURL(LPURLCTRL lpuc, LPCTSTR pszFile);
BOOL UrlCtrl_AddToSpecialFolder(LPURLCTRL lpuc, int nFolder);

void UrlCtrl_StartPropertyThread(HWND hwndParent);
DWORD WINAPI UrlCtrl_PropertyThreadProc(LPVOID lpParameter);

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

BOOL UrlCtrl_RegisterControl(HMODULE hModule)
{
    WNDCLASSEX wcex;
    WNDCLASS wc;
    HKEY hKey;
    TCHAR szCursorFile[MAX_PATH] = _T("");
    DWORD cbData = sizeof(szCursorFile);

    RegOpenKey(HKEY_CURRENT_USER, _T("Control Panel\\Cursors"), &hKey);
    RegQueryValueEx(hKey, _T("Hand"), NULL, NULL, (LPBYTE)szCursorFile, &cbData);
    RegCloseKey(hKey);

    ZeroMemory(&wc, sizeof(WNDCLASS));
    wcex.cbSize = sizeof(WNDCLASSEX);

    if (GetClassInfoEx(hModule, URLCTRL_CLASS, &wcex) != 0)
        return (TRUE);

    wc.style            = CS_SAVEBITS | CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_GLOBALCLASS;
    wc.lpfnWndProc      = (WNDPROC)UrlCtrl_WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hModule;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadImage(hModule, szCursorFile, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
    if (wc.hCursor == NULL)
        LoadCursor(hModule, MAKEINTRESOURCE(IDC_PP_HAND));
    wc.hbrBackground    = (HBRUSH)GetSysColorBrush(COLOR_BTNFACE);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = URLCTRL_CLASS;

    if (!RegisterClass(&wc))
        return (FALSE);

    g_hwndToolTip = CreateWindowEx(0, TOOLTIPS_CLASS, _T(""),
                        TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL,
                        hModule, NULL);
    SetWindowPos(g_hwndToolTip, HWND_TOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    CoInitialize(NULL);

    return (TRUE);
}

void UrlCtrl_UnregisterControl(HMODULE hModule)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    if (GetClassInfoEx(hModule, URLCTRL_CLASS, &wcex) == FALSE)
        return;

    UnregisterClass(URLCTRL_CLASS, hModule);
}

LRESULT CALLBACK UrlCtrl_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPURLCTRL lpuc = (LPURLCTRL)GetWindowLong(hwnd, GWL_USERDATA);

    switch (uMsg)
    {
        case WM_NCCREATE:
            lpuc = (LPURLCTRL)Mem_Alloc(sizeof(URLCTRL));

            if (lpuc == NULL)
                return (TRUE);

            lpuc->hwndOwner = GetParent(hwnd);
            
            SetWindowLong(hwnd, GWL_USERDATA, (LONG)lpuc);
        return (TRUE);
        case WM_CREATE:
        {
            HDC             hdc;
            SIZE            sTextSize;
            HFONT           hFontOld;
            LPCREATESTRUCT  lpcs = (LPCREATESTRUCT)lParam;

            _tcscpy(lpuc->szTitle, lpcs->lpszName);
            _tcscpy(lpuc->szURL, "");

            lpuc->wID           = (WORD)lpcs->hMenu;
            lpuc->hwndOwner     = lpcs->hwndParent;
            lpuc->ucc.crNormal  = RGB(0, 0, 255);
            lpuc->ucc.crDown    = RGB(255, 0, 0);
            lpuc->ucc.crOver    = RGB(255, 0, 0);
            lpuc->hBitmap       = NULL;
            lpuc->hFont         = (HFONT)SendMessage(lpuc->hwndOwner, WM_GETFONT, 0, 0);
            lpuc->wState        = URL_STATE_NORMAL;

            if (lpcs->style & US_LEFT)
                lpuc->dwStyle |= US_LEFT;
            else if (lpcs->style & US_CENTER)
                lpuc->dwStyle |= US_CENTER;
            else if (lpcs->style & US_RIGHT)
                lpuc->dwStyle |= US_CENTER;
            else
                lpuc->dwStyle |= US_LEFT;
            
            hdc = GetDC(lpcs->hwndParent);
            hFontOld = SelectObject(hdc, lpuc->hFont);
            GetTextExtentPoint32(hdc, lpuc->szTitle, _tcslen(lpuc->szTitle), &sTextSize);
            DeleteObject(SelectObject(hdc, hFontOld));
            ReleaseDC(hwnd, hdc);

            SetWindowPos(hwnd, 0, 0, 0,
                sTextSize.cx,
                sTextSize.cy + 2,
                SWP_NOZORDER | SWP_NOMOVE);
        }
        return (TRUE);
        case WM_MOUSEMOVE:
        {
            MSG     msgToolTip;
            RECT    rc;
            POINT   pt = { LOWORD(lParam), HIWORD(lParam) };

            msgToolTip.hwnd     = hwnd;
            msgToolTip.message  = uMsg;
            msgToolTip.wParam   = wParam;
            msgToolTip.lParam   = lParam;

            SendMessage(g_hwndToolTip, TTM_RELAYEVENT, 0, (LPARAM)&msgToolTip);

            GetClientRect(hwnd, &rc);

            if (PtInRect(&rc, pt))
            {
                if (lpuc->wState != URL_STATE_OVER)
                {
                    lpuc->wState = URL_STATE_OVER;
                    SetCapture(hwnd);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            else
            {
                if (lpuc->wState == URL_STATE_OVER ||
                    lpuc->wState == URL_STATE_DOWN)
                {
                    lpuc->wState = URL_STATE_NORMAL;
                    ReleaseCapture();
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }

/*
            if (lpuc->uTimerID == 0)
            {
                lpuc->uTimerID = SetTimer(hwnd, 1, 5, NULL);
                InvalidateRect(hwnd, NULL, TRUE);
            }*/
        }
        return (TRUE);
        case WM_LBUTTONDOWN:
            lpuc->wState = URL_STATE_DOWN;
            InvalidateRect(hwnd, NULL, FALSE);
            PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDM_URLMENU_OPEN, 0), (LPARAM)hwnd);
        return (TRUE);
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC         hdc = BeginPaint(hwnd, &ps);
            UINT        uFormat;
            COLORREF    crOld = 0;
            int         iOldBkMode;
            HFONT       hFontOld, hFont;
            RECT        rcDraw;
            LOGFONT     lf;

            GetClientRect(hwnd, &rcDraw);

            if (lpuc->hBitmap != NULL)
            {
                BITMAP  bm;
                HDC     hdcMem = CreateCompatibleDC(hdc);

                GetObject(lpuc->hBitmap, sizeof(BITMAP), &bm);
                SelectObject(hdcMem, lpuc->hBitmap);

                BitBlt(hdc, rcDraw.left, rcDraw.top, bm.bmWidth, bm.bmHeight,
                    hdcMem, 0, 0, SRCCOPY);
                
                DeleteDC(hdcMem);

                rcDraw.left += bm.bmWidth + CX_SEPARATION;
            }

            if (lpuc->dwStyle & US_LEFT)
                uFormat = DT_LEFT;
            else if (lpuc->dwStyle & US_CENTER)
                uFormat = DT_CENTER;
            else if (lpuc->dwStyle & US_RIGHT)
                uFormat = DT_RIGHT;
            uFormat |= DT_VCENTER | DT_SINGLELINE;

            iOldBkMode = SetBkMode(hdc, TRANSPARENT);

            GetObject(lpuc->hFont, sizeof(LOGFONT), &lf);
            lf.lfUnderline = TRUE;
            hFont = CreateFontIndirect(&lf);
            hFontOld = SelectObject(hdc, hFont);

            switch (lpuc->wState)
            {
                case URL_STATE_NORMAL:
                    crOld = SetTextColor(hdc, lpuc->ucc.crNormal);
                break;
                case URL_STATE_OVER:
                    crOld = SetTextColor(hdc, lpuc->ucc.crOver);
                break;
                case URL_STATE_DOWN:
                    crOld = SetTextColor(hdc, lpuc->ucc.crDown);
                break;
            }
            
            DrawText(hdc, lpuc->szTitle, _tcslen(lpuc->szTitle), &rcDraw, uFormat);

            DeleteObject(SelectObject(hdc, hFontOld));
            SetBkMode(hdc, iOldBkMode);
            SetTextColor(hdc, crOld);
            EndPaint(hwnd, &ps);
        }
        return (TRUE);
        case WM_DESTROY:
            if (lpuc != NULL)
            {
                SetWindowLong(hwnd, GWL_USERDATA, 0);
                if (lpuc->hBitmap != NULL)
                    DeleteObject(lpuc->hBitmap);
                DeleteObject(lpuc->hFont);
                Mem_Free(lpuc);
            }
        return (FALSE);
        case WM_CONTEXTMENU:
        {
            int x = LOWORD(lParam), y = HIWORD(lParam);
            HMENU hMenu, hMenuPopup;

            lpuc->wState    = URL_STATE_NORMAL;
            ReleaseCapture();
            InvalidateRect(hwnd, NULL, FALSE);

            if (x == -1 && y == -1)
            {
                RECT rc;

                GetClientRect(hwnd, &rc);
                Window_ClientToScreenRect(hwnd, &rc);

                x = rc.left;
                y = rc.top;
            }

            hMenu = LoadMenu(PCPControls_GetHandle(), MAKEINTRESOURCE(IDR_URLMENU));
            hMenuPopup = GetSubMenu(hMenu, 0);

            SetMenuDefaultItem(hMenuPopup, IDM_URLMENU_OPEN, FALSE);
            Menu_CreateMenu(hMenuPopup, (int *)IDP_NONE);
            Menu_TrackCreatedPopupMenu(hMenuPopup, x, y, hwnd);
            DestroyMenu(hMenu);
        }
        return (TRUE);
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDM_URLMENU_OPEN:
                    UrlCtrl_Open(lpuc, OPEN);
                return (0);
                case IDM_URLMENU_EDIT:
                    UrlCtrl_Open(lpuc, EDIT);
                return (0);
                case IDM_URLMENU_PRINT:
                    UrlCtrl_Open(lpuc, PRINT);
                return (0);
                case IDM_URLMENU_COPYSHORTCUT:
                    Clipboard_SetText(lpuc->szURL);
                return (0);
                case IDM_URLMENU_ADDTOFAVORITES:
                    UrlCtrl_AddToSpecialFolder(lpuc, CSIDL_FAVORITES);
                return (0);
                case IDM_URLMENU_ADDTODESKTOP:
                    UrlCtrl_AddToSpecialFolder(lpuc, CSIDL_DESKTOP);
                return (0);
                case IDM_URLMENU_PROPERTIES:
                    UrlCtrl_StartPropertyThread(hwnd);
                return (0);
            }
        break;
        case UCM_SETURL:
        {
            TOOLINFO ti;

            _tcscpy(lpuc->szURL, (LPTSTR)lParam);

            ZeroMemory(&ti, sizeof(TOOLINFO));
            ti.cbSize   = sizeof(TOOLINFO);
            ti.uFlags   = TTF_CENTERTIP | TTF_IDISHWND;
            ti.hwnd     = hwnd;
            ti.uId      = (UINT)hwnd;
            ti.lpszText = lpuc->szURL;

            SendMessage(g_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

            InvalidateRect(hwnd, NULL, TRUE);
        }
        return (TRUE);
        case UCM_GETURL:
            _tcsncpy((LPTSTR)lParam, lpuc->szURL, (size_t)wParam);
        return (TRUE);
        case UCM_SETDESCRIPTION:
        {
            _tcscpy(lpuc->szDescription, (LPTSTR)lParam);
        }
        return (TRUE);
        case UCM_GETDESCRIPTION:
        {
            _tcsncpy((LPTSTR)lParam, lpuc->szDescription, (size_t)wParam);
        }
        return (TRUE);
        case UCM_SETCOLORS:
        {
            LPURLCTRLCOLORS lpucc = (LPURLCTRLCOLORS)lParam;
            lpuc->ucc.crNormal  = lpucc->crNormal;
            lpuc->ucc.crOver    = lpucc->crOver;
            lpuc->ucc.crDown    = lpucc->crDown;
        }
        return (TRUE);
        case UCM_SETBITMAP:
        {
            BITMAP      bm;
            HDC         hdc = GetDC(hwnd);
            HFONT       hFontOld;
            SIZE        sTextSize;

            lpuc->hBitmap = (HBITMAP)lParam;

            if ((int)lpuc->hBitmap == UB_DEFAULTMAIL)
                lpuc->hBitmap = LoadBitmap(PCPControls_GetHandle(),
                                    MAKEINTRESOURCE(IDB_MAIL));

            hFontOld = SelectObject(hdc, lpuc->hFont);
            GetTextExtentPoint32(hdc, lpuc->szTitle, _tcslen(lpuc->szTitle), &sTextSize);
            DeleteObject(SelectObject(hdc, hFontOld));
            ReleaseDC(hwnd, hdc);

            GetObject(lpuc->hBitmap, sizeof(BITMAP), &bm);

            SetWindowPos(hwnd, 0, 0, 0,
                bm.bmWidth + sTextSize.cx + CX_SEPARATION,
                max(bm.bmHeight, sTextSize.cy) + 2,
                SWP_NOZORDER | SWP_NOMOVE);

            InvalidateRect(hwnd, NULL, TRUE);
        }
        return (TRUE);
        //could add WM_SETTEXT (UCM_SETTEXT) and resize
    }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}

BOOL UrlCtrl_Open(LPURLCTRL lpuc, int nWay)
{
    BOOL bSuccess;
    
    bSuccess = UrlCtrl_OpenUsingCom(lpuc, nWay);

    if (!bSuccess)
        bSuccess = UrlCtrl_OpenUsingShellExecute(lpuc, nWay);

    return (bSuccess);
}

BOOL UrlCtrl_OpenUsingCom(LPURLCTRL lpuc, int nWay)
{
    PIUniformResourceLocator pURL;
    URLINVOKECOMMANDINFO uici;
    HRESULT hResult = CoCreateInstance(&CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, &IID_IUniformResourceLocator, (LPVOID *)&pURL);

    if (!SUCCEEDED(hResult))
    {

        return (FALSE);
    }

    hResult = pURL->lpVtbl->SetURL(pURL, lpuc->szURL, IURL_SETURL_FL_GUESS_PROTOCOL);

    if (!SUCCEEDED(hResult))
    {
        pURL->lpVtbl->Release(pURL);

        return (FALSE);
    }

    uici.dwcbSize   = sizeof(URLINVOKECOMMANDINFO);
    uici.dwFlags    = IURL_INVOKECOMMAND_FL_ALLOW_UI;
    uici.hwndParent = lpuc->hwndOwner;

    switch (nWay)
    {
    case OPEN:
        uici.pcszVerb = _T("open");
    break;
    case EDIT:
        uici.pcszVerb = _T("edit");
    break;
    case PRINT:
        uici.pcszVerb = _T("print");
    break;
    default:
        ASSERT(FALSE); // Unreachable
    break;
    }

    hResult = pURL->lpVtbl->InvokeCommand(pURL, &uici);

    if (!SUCCEEDED(hResult))
    {
        pURL->lpVtbl->Release(pURL);
        
        return (FALSE);
    }

    pURL->lpVtbl->Release(pURL);

    return (TRUE);
}

BOOL UrlCtrl_OpenUsingShellExecute(LPURLCTRL lpuc, int nWay)
{
    LPCTSTR pszMode;
    HINSTANCE hResult;

    switch (nWay)
    {
    case OPEN:
        pszMode = _T("open");
    break;
    case EDIT:
        pszMode = _T("edit");
    break;
    case PRINT:
        pszMode = _T("print");
    break;
    default:
        ASSERT(FALSE);
        pszMode = _T("open");
    break;
    }

    hResult = ShellExecute(lpuc->hwndOwner, pszMode, lpuc->szURL, NULL, NULL, SW_SHOWNORMAL);

    if ((int)hResult <= HINSTANCE_ERROR)
    {
        return (FALSE);
    }

    return (TRUE);
}

BOOL UrlCtrl_AddToSpecialFolder(LPURLCTRL lpuc, int nFolder)
{
    IMalloc *pMalloc;
    LPITEMIDLIST pidlFolder;
    HRESULT hResult;
    TCHAR szFolder[MAX_PATH];
    TCHAR szShortcutFile[MAX_PATH];
    BOOL bSuccess;

    if (!SUCCEEDED(SHGetMalloc(&pMalloc)))
    {
        TRACE(_T("Failed to malloc in Urcl_Ctrl_AddToSpecialFolder"));
        
        return (FALSE);
    }

    hResult = SHGetSpecialFolderLocation(NULL, nFolder, &pidlFolder);

    if (!SUCCEEDED(hResult))
    {
        TRACE(_T("Failed to get special folder %d."), nFolder);

        pMalloc->lpVtbl->Release(pMalloc);

        return (FALSE);
    }

    if (!SHGetPathFromIDList(pidlFolder, szFolder))
    {
        TRACE(_T("Failed to get folder from idlist"));

        pMalloc->lpVtbl->Free(pMalloc, pidlFolder);
        pMalloc->lpVtbl->Release(pMalloc);

        return (FALSE);
    }

    _tmakepath(szShortcutFile, NULL, szFolder, lpuc->szTitle, _T("url"));

    pMalloc->lpVtbl->Free(pMalloc, pidlFolder);

    bSuccess = UrlCtrl_SaveURL(lpuc, szShortcutFile);

    pMalloc->lpVtbl->Release(pMalloc);

    return (bSuccess);
}

BOOL UrlCtrl_SaveURL(LPURLCTRL lpuc, LPCTSTR pszFile)
{
    PIUniformResourceLocator    pURL;
    IPersistFile                *pPFile;
    HRESULT hResult = CoCreateInstance(&CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, &IID_IUniformResourceLocator, (LPVOID *)&pURL);
    WORD wszSave[MAX_PATH];

    if (!SUCCEEDED(hResult))
        return (FALSE);

    hResult = pURL->lpVtbl->SetURL(pURL, lpuc->szURL, IURL_SETURL_FL_GUESS_PROTOCOL);

    if (!SUCCEEDED(hResult))
    {
        pURL->lpVtbl->Release(pURL);

        return (FALSE);
    }

    hResult = pURL->lpVtbl->QueryInterface(pURL, &IID_IPersistFile, (LPVOID *)&pPFile);

    if (!SUCCEEDED(hResult))
    {
        TRACE(_T("Failed to get IPersistFile interface\n"));

        return (FALSE);
    }

#ifndef _UNICODE
    MultiByteToWideChar(CP_ACP, 0, pszFile, -1, wszSave, MAX_PATH);
    hResult = pPFile->lpVtbl->Save(pPFile, wszSave, TRUE);
#else
    hResult = pPFile->lpVtbl->Save(pPFile, pszFile, TRUE);
#endif /* _UNICODE */

    if (!SUCCEEDED(hResult))
    {
        TRACE(_T("IPersistFile->Save() Failed!\n"));

        pPFile->lpVtbl->Release(pPFile);
        pURL->lpVtbl->Release(pURL);

        return (FALSE);
    }

    pPFile->lpVtbl->Release(pPFile);
    pURL->lpVtbl->Release(pURL);

    return (TRUE);
}




void UrlCtrl_StartPropertyThread(HWND hwndParent)
{
    UINT uPropertyThreadID;
    HANDLE hThread;

    hThread = BEGINTHREADEX(0, 0, UrlCtrl_PropertyThreadProc, hwndParent, 0, &uPropertyThreadID);

    // We don't need the handle to this thread so close it
    CloseHandle(hThread);
}

DWORD WINAPI UrlCtrl_PropertyThreadProc(LPVOID lpParameter)
{
    PROPSHEETPAGE psp;
    PROPSHEETHEADER psh;
    HINSTANCE hOldResource = String_SetResourceHandle(PCPControls_GetHandle());
    HWND hwndProperties;
    HWND hwndParent = (HWND)lpParameter;
    HWND hwndGeneralPage;
    TCHAR szUrl[MAX_PATH * 2] = _T(""), szDescription[MAX_PATH * 2] = _T("");
    MSG msg;

    INITSTRUCT(psp, TRUE);
    psp.hInstance   = PCPControls_GetHandle();
    psp.pszTemplate = MAKEINTRESOURCE(IDD_URLPROPERTIES);
    psp.pfnDlgProc  = UrlCtrl_Properties_DlgProc;

    INITSTRUCT(psh, TRUE);
    psh.dwFlags         = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_MODELESS;
    psh.hwndParent      = hwndParent;
    psh.hInstance       = PCPControls_GetHandle();
    psh.pszCaption      = String_LoadString(IDS_URLSHEETTITLE);
    psh.nPages          = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.ppsp            = (LPCPROPSHEETPAGE)&psp;

    String_SetResourceHandle(hOldResource);

    hwndProperties = (HWND)PropertySheet(&psh);

    UrlCtrl_GetUrl(hwndParent, sizeof(szUrl), szUrl);
    UrlCtrl_GetDescription(hwndParent, sizeof(szUrl), szDescription);

    hwndGeneralPage = PropSheet_GetCurrentPageHwnd(hwndProperties);
    SetWindowText(GetDlgItem(hwndGeneralPage, IDC_URL_URL), szUrl);
    SetWindowText(GetDlgItem(hwndGeneralPage, IDC_URL_DESCRIPTION), szDescription);
//  PropSheet_CancelToClose(hwndProperties);

    while (GetMessage(&msg, hwndProperties, 0, 0))
    {
        if (!PropSheet_IsDialogMessage(hwndProperties, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else if (PropSheet_GetCurrentPageHwnd(hwndProperties) == NULL)
        {
            break;
        }
    }

    DestroyWindow(hwndProperties);

    return (0);
}


BOOL CALLBACK UrlCtrl_Properties_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            Window_CenterWindow(hwndDlg);
        return (TRUE);
    }

    return (FALSE);
}
