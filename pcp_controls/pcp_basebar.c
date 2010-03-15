/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_basebar.h
 * Created    : 01/23/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:39:08
 * Comments   : This is almost 100% SmartFTP-BaseBar (by Mike Walter)
 *              
 *              
 *****************************************************************/

/*****************************************************************
 *                        REVISION LOG ENTRY                     *
 *****************************************************************
 * Project    : SmartFTP
 *
 * File       : SmartFTP-BaseBar.cpp
 * Revision By: Mike Walter
 * Revised on : 14.00.2000
 * Comments   : Base Bar Control
 *
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* windows */
#include <commctrl.h>

#include "resource.h"

/* pcp_controls */
#include "pcp_basebar.h"
#include "pcp_menu.h"

/* pcp_generic */
#include <pcp_mem.h>
#include <pcp_string.h>

#define REBARID 0xFF123
#define IDC_TOOLBAR 0xFF124

static LPCTSTR  s_pszBaseBarProp = _T("BaseBarProp");
static ATOM     s_hBaseBarAtom;

typedef struct tagBaseBarInfo
{
    LPTSTR  pszTitle;
    HWND    hwndToolbar;
    HWND    hwndRebar;
    RECT    rcSize;
    DWORD   dwAlign;
    DWORD   dwAllowableAligns;
    LPARAM  lParam;
} BASEBARINFO, * LPBASEBARINFO;

static LRESULT CALLBACK BaseBar_WindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
static DWORD BaseBar_GetAlign(HWND hwnd);

BOOL BaseBar_RegisterControl(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;
    WNDCLASS   wc;

    ZeroMemory(&wc, sizeof(WNDCLASS));
    wcex.cbSize = sizeof(WNDCLASSEX);

    if (GetClassInfoEx(hInstance, WC_BASEBAR, &wcex) != 0)
        return (TRUE);

    wc.style         = CS_SAVEBITS;
    wc.lpfnWndProc   = (WNDPROC) BaseBar_WindowProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    wc.lpszClassName = WC_BASEBAR;

    if (!RegisterClass(&wc))
        return (FALSE);

    s_hBaseBarAtom = GlobalAddAtom(s_pszBaseBarProp);


    return (TRUE);
}

BOOL BaseBar_UnregisterControl(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    if (GetClassInfoEx(hInstance, WC_BASEBAR, &wcex) == 0)
        return (FALSE);

    UnregisterClass(WC_BASEBAR, hInstance);
    GlobalDeleteAtom(s_hBaseBarAtom);

    return (TRUE);
}

HWND BaseBar_Create(HWND hwndParent, LPCTSTR pszTitle, HINSTANCE hInstance, int nSize, UINT nID, DWORD dwStyle)
{
    int cx = 0;
    int cy = 0;
    HWND hwnd;

    if (!BaseBar_RegisterControl(hInstance))
        return (NULL);

    if ((dwStyle & CCS_VERT))
        cx = nSize;
    else
        cy = nSize;

    hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, WC_BASEBAR, pszTitle, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | dwStyle, 0, 0, cx, cy, hwndParent, (HMENU)nID, hInstance, NULL);

    return (hwnd);
}

static DWORD BaseBar_GetAlign(HWND hwnd)
{
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

    if (dwStyle & CCS_VERT)
    {
        if (dwStyle & CCS_TOP)
            return (CCS_LEFT);
        else
            return (CCS_RIGHT);
    }
    else
    {
        if (dwStyle & CCS_TOP)
            return (CCS_TOP);
        else
            return (CCS_BOTTOM);
    }

    return (-1);
}

static LRESULT CALLBACK BaseBar_WindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    LPBASEBARINFO lpbi = (LPBASEBARINFO)GetProp(hwnd, (LPCTSTR)MAKEWORD(s_hBaseBarAtom, 0));//GetWindowLong(hwnd, GWL_USERDATA);

    switch (uMessage)
    {
    case WM_CREATE:
    {
        LPCREATESTRUCT lpcs    = (LPCREATESTRUCT)lParam;
        DWORD          dwStyle = RBS_VERTICALGRIPPER | RBS_REGISTERDROP | RBS_AUTOSIZE | RBS_VARHEIGHT | RBS_DBLCLKTOGGLE | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NORESIZE;
        REBARBANDINFO  rbBand;
        TBBUTTON tbb;

        if (lpcs->lpszName)
            lpbi->pszTitle = String_Duplicate(lpcs->lpszName);

        lpbi->dwAlign = BaseBar_GetAlign(hwnd);
        lpbi->dwAllowableAligns = CCS_LEFT | CCS_RIGHT | CCS_TOP | CCS_BOTTOM;

        if (lpbi->dwAlign == CCS_LEFT || lpbi->dwAlign == CCS_RIGHT)
            dwStyle |= CCS_VERT;

        if (!(lpbi->hwndRebar = CreateWindowEx(0, REBARCLASSNAME, NULL, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | dwStyle,
            0, 0, 0, 0, hwnd, (HMENU) REBARID, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL)))
            return (0);

        SendMessage(lpbi->hwndRebar, RB_SETBKCOLOR, 0, (LPARAM)GetSysColor(COLOR_BTNFACE));

        ZeroMemory(&rbBand, sizeof(REBARBANDINFO));

        rbBand.cbSize     = sizeof(REBARBANDINFO);
        rbBand.fMask      = RBBIM_HEADERSIZE | RBBIM_ID | RBBIM_STYLE | RBBIM_COLORS;
        rbBand.fStyle     = RBBS_FIXEDSIZE | RBBS_VARIABLEHEIGHT | RBBS_NOGRIPPER;
        rbBand.clrFore    = GetSysColor(COLOR_BTNTEXT);
        rbBand.clrBack    = GetSysColor(COLOR_BTNFACE);
        rbBand.cxHeader   = ((lpcs->style & CCS_VERT) ? 21 : 24);
        rbBand.wID        = (UINT)lpcs->hMenu;

        if (lpcs->style & CCS_VERT)
        {
            rbBand.fMask |= RBBIM_TEXT;
            rbBand.lpText = (char*)lpcs->lpszName;
        }

        SendMessage(lpbi->hwndRebar, RB_INSERTBAND, (int) -1, (LPARAM) &rbBand);

        ZeroMemory(&tbb, sizeof(TBBUTTON));

        tbb.iBitmap   = 0; 
        tbb.idCommand = IDM_BAND_CLOSE;
        tbb.fsState   = TBSTATE_ENABLED; 
        tbb.fsStyle   = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

        lpbi->hwndToolbar = CreateToolbarEx(lpbi->hwndRebar, WS_CHILDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER  | CCS_NOPARENTALIGN | CCS_NOMOVEY | CCS_NORESIZE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
            IDC_TOOLBAR, 1, PCPControls_GetHandle(), IDB_CHECKBOX, &tbb, 1, 13, 12, 11, 10, sizeof (TBBUTTON));

        SendMessage(lpbi->hwndToolbar, TB_SETBUTTONSIZE, 0, (LPARAM) MAKELONG(20, 18));
        SendMessage(lpbi->hwndToolbar, TB_SETBITMAPSIZE, 0, (LPARAM) MAKELONG(10, 9));
    }
    return (TRUE);
    case WM_SIZE:
    {
        RECT rcNew;

        GetClientRect(hwnd, &rcNew);

        if (wParam == SIZE_RESTORED && !EqualRect(&rcNew, &lpbi->rcSize))
        {
            NMHDR nmhdr;

            MoveWindow(lpbi->hwndRebar, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);

            nmhdr.code     = RBN_HEIGHTCHANGE;
            nmhdr.hwndFrom = hwnd;
            nmhdr.idFrom   = GetDlgCtrlID(hwnd);
            SendMessage(GetParent(hwnd), WM_NOTIFY, nmhdr.idFrom, (LPARAM)&nmhdr);

            if (lpbi->hwndToolbar)
            {
                RECT  rcBand;
                DWORD dwSize = (DWORD)SendMessage(lpbi->hwndToolbar, TB_GETBUTTONSIZE, 0, 0);

                SendMessage(lpbi->hwndRebar, RB_GETBANDBORDERS, 0, (LPARAM)&rcBand);

                if (lpbi->dwAlign == CCS_LEFT || lpbi->dwAlign == CCS_RIGHT)
                {
                    int x = (LOWORD(lParam) - 24);
                    int y = ((rcBand.top - HIWORD(dwSize)) / 2);
                    MoveWindow(lpbi->hwndToolbar, x, y, 20, 18, TRUE);
                }
                else
                {
                    MoveWindow(lpbi->hwndToolbar, 2, 2, 20, 18, TRUE);
                }
            }
        }

        GetClientRect(hwnd, &lpbi->rcSize);
    }
    break;
    case WM_NOTIFY:
    {
        LPNMHDR lphdr = (LPNMHDR)lParam;

        if (lphdr->code == NM_NCHITTEST && lphdr->idFrom == REBARID)
        {
            if (HIBYTE(GetAsyncKeyState(VK_RBUTTON)))
            {
                HMENU hMenu = LoadMenu(PCPControls_GetHandle(), MAKEINTRESOURCE(IDR_BANDMENU));
                HMENU hPopupMenu = GetSubMenu(hMenu, 0);
                POINT pt;

                if (lpbi->dwAllowableAligns & CCS_VERT)
                {
                    if (!(lpbi->dwAllowableAligns & CCS_LEFT))
                        Menu_EnableMenuItem(hPopupMenu, IDM_BAND_LEFT, FALSE);
                    if (!(lpbi->dwAllowableAligns & CCS_RIGHT))
                        Menu_EnableMenuItem(hPopupMenu, IDM_BAND_RIGHT, FALSE);

                        Menu_EnableMenuItem(hPopupMenu, IDM_BAND_TOP, FALSE);
                        Menu_EnableMenuItem(hPopupMenu, IDM_BAND_BOTTOM, FALSE);
                }
                else
                {
                    if (!(lpbi->dwAllowableAligns & CCS_TOP))
                        Menu_EnableMenuItem(hPopupMenu, IDM_BAND_TOP, FALSE);
                    if (!(lpbi->dwAllowableAligns & CCS_BOTTOM))
                        Menu_EnableMenuItem(hPopupMenu, IDM_BAND_BOTTOM, FALSE);

                        Menu_EnableMenuItem(hPopupMenu, IDM_BAND_LEFT, FALSE);
                        Menu_EnableMenuItem(hPopupMenu, IDM_BAND_RIGHT, FALSE);
                }

                GetCursorPos(&pt);

                Menu_TrackPopupMenu(hPopupMenu, pt.x, pt.y, hwnd);
            }
            return (HTCLIENT);
        }
        else if (lphdr->code == NM_CUSTOMDRAW && lphdr->idFrom == REBARID)
        {
            LPNMCUSTOMDRAW lpnmcd = (LPNMCUSTOMDRAW) lParam;

            if (lpnmcd->dwDrawStage == CDDS_PREPAINT)
                return (CDRF_NOTIFYPOSTPAINT);
            else if (lpnmcd->dwDrawStage == CDDS_POSTPAINT && (lpbi->dwAlign == CCS_TOP || lpbi->dwAlign == CCS_BOTTOM))
            {
                DrawEdge(lpnmcd->hdc, &lpnmcd->rc, EDGE_ETCHED, BF_RIGHT);
            }
        }
        else if (lphdr->idFrom == IDC_TOOLBAR)
        {
        }
    }
    return (SendMessage(GetParent(hwnd), uMessage, wParam, lParam));
    case WM_COMMAND:
    {
        DWORD dwAlign;

        switch (LOWORD(wParam))
        {
        case IDM_BAND_LEFT:
            dwAlign = CCS_LEFT;
        break;
        case IDM_BAND_RIGHT:
            dwAlign = CCS_RIGHT;
        break;
        case IDM_BAND_TOP:
            dwAlign = CCS_TOP;
        break;
        case IDM_BAND_BOTTOM:
            dwAlign = CCS_BOTTOM;
        break;
        case IDM_BAND_CLOSE:
        {
            NMHDR nmhdr;

            nmhdr.code     = BBN_CLOSE;
            nmhdr.hwndFrom = hwnd;
            nmhdr.idFrom   = GetDlgCtrlID(hwnd);

            SendMessage(GetParent(hwnd), WM_NOTIFY, nmhdr.idFrom, (LPARAM)&nmhdr);
        }
        return (0);
        default:
        return (0);
        }

        SendMessage(hwnd, BCM_SETALIGNMENT, (WPARAM)dwAlign, 0L);
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO *lpmmi = (MINMAXINFO *)lParam;
        
        if (lpmmi != NULL)
        {
            int cx = GetSystemMetrics(SM_CXSCREEN);
            int cy = GetSystemMetrics(SM_CYSCREEN);
            
            if (lpbi->dwAlign == CCS_LEFT || lpbi->dwAlign == CCS_RIGHT)
                lpmmi->ptMaxTrackSize.x = cx / 2;
            else
                lpmmi->ptMaxTrackSize.y = cy / 2;
            
            if (lpbi->dwAlign == CCS_LEFT)
            {
                lpmmi->ptMinTrackSize.x = 50;
            }
            else if (lpbi->dwAlign == CCS_RIGHT)
            {
                lpmmi->ptMinTrackSize.x = 50;
            }
            else if (lpbi->dwAlign == CCS_TOP)
            {
                lpmmi->ptMinTrackSize.y = 50;
            }
            else if (lpbi->dwAlign == CCS_BOTTOM)
            {
                lpmmi->ptMinTrackSize.y = 50;
            }
            else
            {
                lpmmi->ptMinTrackSize.x = 56;
                lpmmi->ptMinTrackSize.y = 24;
            }
        }
    }
    return (FALSE);
    case WM_NCCREATE:
    {
        lpbi = Mem_Alloc(sizeof(BASEBARINFO));

        if (lpbi == NULL)
            return (FALSE);

        SetProp(hwnd, (LPCTSTR)MAKEWORD(s_hBaseBarAtom, 0), lpbi);
    }
    return (TRUE);
    case WM_NCDESTROY:
    {
        RemoveProp(hwnd, (LPCTSTR)MAKEWORD(s_hBaseBarAtom, 0));

        if (lpbi != NULL)
        {
            if (lpbi->pszTitle != NULL)
                Mem_Free(lpbi->pszTitle);

            Mem_Free(lpbi);
        }
    }
    return (TRUE);
    case WM_NCHITTEST:
    {
        POINT pt = {LOWORD(lParam), HIWORD(lParam)};
        RECT  re;

        GetWindowRect(hwnd, &re);

        if (!PtInRect(&re, pt))
            return (HTNOWHERE);

        if (lpbi->dwAlign == CCS_LEFT)
        {
            re.left = re.right - 4;

            if (PtInRect(&re, pt))
                return (HTRIGHT);
        }
        else if (lpbi->dwAlign == CCS_RIGHT)
        {
            re.right = re.left + 4;

            if (PtInRect(&re, pt))
                return (HTLEFT);
        }
        else if (lpbi->dwAlign == CCS_BOTTOM)
        {
            re.bottom = re.top + 4;

            if (PtInRect(&re, pt))
                return (HTTOP);
        }
        else if (lpbi->dwAlign == CCS_TOP)
        {
            re.top = re.bottom - 4;

            if (PtInRect(&re, pt))
                return (HTBOTTOM);
        }
    }
    return (HTCLIENT);
    case WM_NCCALCSIZE:
    {
        if (wParam == TRUE)
        {
            NCCALCSIZE_PARAMS *ncp = (NCCALCSIZE_PARAMS *) lParam;

            if (lpbi->dwAlign == CCS_LEFT)
            {
                ncp->rgrc[0].top    += 4;
                ncp->rgrc[0].right  -= 4;
            }
            else if (lpbi->dwAlign == CCS_RIGHT)
            {
                ncp->rgrc[0].top    += 4;
                ncp->rgrc[0].left   += 4;
            }
            else if (lpbi->dwAlign == CCS_BOTTOM)
            {
                ncp->rgrc[0].top    += 4;
            }
            else if (lpbi->dwAlign == CCS_TOP)
            {
                ncp->rgrc[0].bottom -= 4;
            }
            else
            {
                ncp->rgrc[0].top    += 4;
                ncp->rgrc[0].right  -= 4;
                ncp->rgrc[0].left   += 4;
                ncp->rgrc[0].bottom -= 4;
            }
        }
    }
    break;
    case WM_ERASEBKGND:
    return (TRUE);
    case WM_NCPAINT:
    {
        HDC  hdc = GetWindowDC(hwnd);
        RECT rcWindow, rcClient;

        if (hdc == NULL)
            break;

        GetClientRect(hwnd, &rcClient);
        GetWindowRect(hwnd, &rcWindow);

        MapWindowPoints(NULL, hwnd, (LPPOINT)&rcWindow, 2);
        OffsetRect(&rcClient, -rcWindow.left, -rcWindow.top);
        OffsetRect(&rcWindow, -rcWindow.left, -rcWindow.top);
        ExcludeClipRect(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

        FillRect(hdc, &rcWindow, (HBRUSH)GetSysColorBrush(COLOR_BTNFACE));
        ReleaseDC(hwnd, hdc);
    }
    return (TRUE);
    case BCM_SETCLIENTWND:
    {
        REBARBANDINFO rbBand;
        ZeroMemory(&rbBand, sizeof(REBARBANDINFO));

        rbBand.cbSize     = sizeof(REBARBANDINFO);
        rbBand.fMask      = RBBIM_CHILDSIZE | RBBIM_CHILD;
        rbBand.cyIntegral = 1;
        rbBand.cyMaxChild = -1;
        rbBand.hwndChild  = (HWND)lParam;

        return (SendMessage(lpbi->hwndRebar, RB_SETBANDINFO, 0, (LPARAM)&rbBand));
    }
    case BCM_GETCLIENTWND:
    {
        REBARBANDINFO rbBand;
        ZeroMemory(&rbBand, sizeof(REBARBANDINFO));

        rbBand.cbSize     = sizeof(REBARBANDINFO);
        rbBand.fMask      = RBBIM_CHILD;
        SendMessage(lpbi->hwndRebar, RB_GETBANDINFO, 0, (LPARAM)&rbBand);

        return (LRESULT)((IsWindow(rbBand.hwndChild) ? rbBand.hwndChild : NULL));
    }
    case BCM_SETIMAGELIST:
    {
        REBARINFO  rbi;
        HIMAGELIST hilOld = BandCtrl_GetImageList(hwnd);

        rbi.cbSize = sizeof(REBARINFO);
        rbi.fMask  = RBIM_IMAGELIST;
        rbi.himl   = (HIMAGELIST)lParam;

        if (!SendMessage(lpbi->hwndRebar, RB_SETBARINFO, 0, (LPARAM) &rbi))
            return ((LRESULT)NULL);

        return ((LRESULT)hilOld);
    }
    case BCM_GETIMAGELIST:
    {
        REBARINFO rbi;

        rbi.cbSize = sizeof(REBARINFO);
        rbi.fMask  = RBIM_IMAGELIST;
        rbi.himl   = 0;

        if (!SendMessage(lpbi->hwndRebar, RB_GETBARINFO, 0, (LPARAM) &rbi))
            return ((LRESULT)NULL);

        return ((LRESULT)rbi.himl);
    }
    case BCM_SETTITLEIMAGE:
    {
        REBARBANDINFO rbBand;

        ZeroMemory(&rbBand, sizeof(REBARBANDINFO));

        rbBand.cbSize = sizeof(REBARBANDINFO);
        rbBand.fMask  = RBBIM_IMAGE;
        rbBand.iImage = wParam;

        return (SendMessage(lpbi->hwndRebar, RB_SETBANDINFO, 0, (LPARAM)&rbBand));
    }
    case BCM_GETTITLEIMAGE:
    {
        REBARBANDINFO rbBand;

        ZeroMemory(&rbBand, sizeof(REBARBANDINFO));

        rbBand.cbSize = sizeof(REBARBANDINFO);
        rbBand.fMask  = RBBIM_IMAGE;
        rbBand.iImage = -1;
        SendMessage(lpbi->hwndRebar, RB_GETBANDINFO, 0, (LPARAM)&rbBand);

        return (rbBand.iImage);
    }
    case BCM_GETALIGNMENT:
    return ((LRESULT)lpbi->dwAlign);
    case BCM_SETALIGNMENT:
    {
        NMHDR nmhdr;
        DWORD dwStyle;

        if (lpbi->dwAlign == wParam)
            return (wParam);

        lpbi->dwAlign = wParam;

        dwStyle = GetWindowLong(lpbi->hwndRebar, GWL_STYLE);
        dwStyle &= ~(CCS_BOTTOM | CCS_TOP | CCS_LEFT | CCS_RIGHT);
        dwStyle |= lpbi->dwAlign;

        SetWindowLong(lpbi->hwndRebar, GWL_STYLE, dwStyle);

        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

        nmhdr.code     = RBN_HEIGHTCHANGE;
        nmhdr.hwndFrom = hwnd;
        nmhdr.idFrom   = GetDlgCtrlID(hwnd);
        SendMessage(GetParent(hwnd), WM_NOTIFY, nmhdr.idFrom, (LPARAM)&nmhdr);

        return (lpbi->dwAlign);
    }
    case BCM_SETDATA:
    {
        LPARAM lParamOld = lpbi->lParam;
        lpbi->lParam = lParam;
        return (lParamOld);
    }
    case BCM_GETDATA:
    return (lpbi->lParam);
    case BCM_SETALLOWABLEALIGNS:
    {
        DWORD dwOldAllowableAligns = lpbi->dwAllowableAligns;

        lpbi->dwAllowableAligns = lParam;

        return (dwOldAllowableAligns);
    }
    case BCM_GETALLOWABLEALIGNS:
    return (lpbi->dwAllowableAligns);
    }

    return (DefWindowProc(hwnd, uMessage, wParam, lParam));
}
