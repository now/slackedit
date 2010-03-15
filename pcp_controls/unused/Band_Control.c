
/*****************************************************************
 *                                              REVISION LOG ENTRY                                       *
 *****************************************************************
 * Project      : SmartFTP
 *
 * File             : Band_Control.c
 * Revision By: Mike Walter
 * Revised on : 0606/27/00 21:33:559
 * Comments     : 
 *
 *****************************************************************/

#define OEMRESOURCE

#include "../pcp_generic/pcp_includes.h"
#include <commctrl.h>

#include "resource.h"

//#include "Controls.h"
//#include "Controls_Paint.h"
#include "Band_Control.h"
#include "paint.h"
#include "pcp_menu.h"
#include "../pcp_generic/pcp_mem.h"
#include "../pcp_generic/pcp_window.h"
#include "../pcp_generic/pcp_rect.h"

/************************************
                     Global Variables
 ************************************/

#define BAND_CHILD_CLASS                "BandCtrl_ChildClass"
#define BAND_DEFAULT_BORDER             0
#define BAND_DEFAULT_CAPTION_CY         22
#define BAND_DEFAULT_CRBORDER           GetSysColor(COLOR_ACTIVEBORDER);
#define BAND_DEFAULT_CRBACK             GetSysColor(COLOR_BTNFACE);

typedef struct tagBandCtrl
{
    DWORD cbSize;   // Size of this Structure for check the correct struct
    DWORD dwFlags;      // Flags of this Band
    DWORD dwStyle;      // Style of this Band
    DWORD dwOldStyle;   // The Old Style for Restoring a Floating Control
    DWORD dwAllowableAligns;
    TCHAR szTitle[100];     // Title of Window max 100 chars
    HWND hwndOwner;     // For receiving some Messages from the child controls
    HWND hwndPager;
    HWND hwndClient;
    UINT timerID;
    UINT wID;
    POINT ptMinSize;
    POINT ptMaxSize;
    RECT rcBand;
    RECT rcCaption;
    RECT rcClose;
    RECT rcClient;
    RECT rcIcon;
    RECT rcTop;
    RECT rcLeft;
    RECT rcRight;
    RECT rcBottom;
    BOOL bOnClose;
    BOOL bDraging;
    BOOL bMouseDrag;
    BOOL bNotify;
    DWORD dwBorder;
    HIMAGELIST hil;

    int iImage;
    COLORREF crBack;
    COLORREF crBorder;
    LPARAM lParam;
    WINDOWPLACEMENT wpl;
} BANDCTRL, *LPBANDCTRL;

#define PREDEFINEDCNTRLBIT           0x80
#define BUTTONCNTRLCODE              0x80
#define EDITCNTRLCODE                0x81
#define STATICCNTRLCODE              0x82
#define LISTBOXCNTRLCODE             0x83
#define SCROLLBARCNTRLCODE           0x84
#define COMBOBOXCNTRLCODE            0x85

static const WCHAR class_names[6][10] =
{
    {_T('B'), _T('u'), _T('t'), _T('t'), _T('o'), _T('n'),},                            /* 0x80 */
    {_T('E'), _T('d'), _T('i'), _T('t'),},                                              /* 0x81 */
    {_T('S'), _T('t'), _T('a'), _T('t'), _T('i'), _T('c'),},                            /* 0x82 */
    {_T('L'), _T('i'), _T('s'), _T('t'), _T('B'), _T('o'), _T('x'),},                   /* 0x83 */
    {_T('S'), _T('c'), _T('r'), _T('o'), _T('l'), _T('l'), _T('B'), _T('a'), _T('r'),}, /* 0x84 */
    {_T('C'), _T('o'), _T('m'), _T('b'), _T('o'), _T('B'), _T('o'), _T('x'),}           /* 0x85 */
};

    /* Dialog control information */
typedef struct
{
    DWORD style;
    DWORD exStyle;
    DWORD helpId;
    short x;
    short y;
    short cx;
    short cy;
    WORD id;
    LPCTSTR className;
    LPCTSTR windowName;
    LPVOID data;
} DLG_CONTROL_INFO;
    /* Dialog template */
typedef struct
{
    DWORD style;
    DWORD exStyle;
    DWORD helpId;
    WORD nbItems;
    short x;
    short y;
    short cx;
    short cy;
    LPCTSTR menuName;
    LPCTSTR className;
    LPCTSTR caption;
    WORD pointSize;
    WORD weight;
    BOOL italic;
    LPCTSTR faceName;
    BOOL dialogEx;
} DLG_TEMPLATE;

/************************************
    Funktion Definitions Starts here
 ************************************/

void Band_DrawCloseButton(HWND hwnd, LPBANDCTRL lpbc, BOOL bOver);

HRESULT Band_ForwardNotify(HWND hwnd, WPARAM wParam, LPARAM lParam, LPBANDCTRL lpbc, long code);
HRESULT Band_NotifyParent(HWND hwnd, LPBANDCTRL lpbc, long code);

DWORD Band_ModifyStyle(DWORD dwStyle, DWORD dwAdd);
void Band_RecalcSize(HWND hwnd, LPBANDCTRL lpbc);

BOOL Band_SetDialogTemplate(HWND hwnd, LPBANDCTRL lpbc, WPARAM wParam, LPARAM lParam);
const TCHAR *Band_GetDialogInfo(LPCTSTR pTemplate, DLG_TEMPLATE * result);
const WORD *Band_GetControlInfo(const WORD * p, DLG_CONTROL_INFO * info, BOOL dialogEx, LPBOOL bIsID);

void Band_MapDialogRect(LPRECT lprc, int cx, int cy);
void Band_GetDialogBaseUnits(LPCTSTR pszFontFace, WORD wFontSize, SIZE * pSizePixel);

LRESULT CALLBACK BandWindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BandChildWindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

/************************************
 Funktion Implementation Starts here
 ************************************/

BOOL Band_RegisterControl(HMODULE hModule)
{
    WNDCLASSEX wcex;
    WNDCLASS wc;

    ZeroMemory(&wc, sizeof(WNDCLASS));
    wcex.cbSize = sizeof(WNDCLASSEX);

    if (GetClassInfoEx(hModule, WC_BANDCONTROL, &wcex) != 0)
        return (TRUE);

    wc.style             = CS_SAVEBITS | CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_GLOBALCLASS;
    wc.lpfnWndProc      = (WNDPROC) BandWindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hModule;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH) GetSysColorBrush(COLOR_BTNFACE);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = WC_BANDCONTROL;

    if (!RegisterClass(&wc))
        return (FALSE);

    wc.style                 = CS_BYTEALIGNCLIENT | CS_GLOBALCLASS;
    wc.lpfnWndProc   = (WNDPROC) BandChildWindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance         = hModule;
    wc.hIcon                 = NULL;
    wc.hCursor           = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(NULL_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = BAND_CHILD_CLASS;
    return (RegisterClass(&wc));
}

void Band_UnRegisterControl(HMODULE hModule)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    if (GetClassInfoEx(hModule, WC_BANDCONTROL, &wcex) == FALSE)
        return;
    
    UnregisterClass(WC_BANDCONTROL, hModule);
}

LRESULT CALLBACK BandWindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    LPBANDCTRL lpbdd = (LPBANDCTRL) GetWindowLong(hwnd, GWL_USERDATA);

    switch (uMessage)
    {
        case BCM_SETDATA:
        {
            lpbdd->lParam = lParam;
        }
        return (TRUE);
        case BCM_GETDATA:
        return ((LRESULT) lpbdd->lParam);
        case BCM_SETIMAGELIST:
        {
            HIMAGELIST hOldImageList = lpbdd->hil;

            if (!lParam)
                return ((LRESULT)NULL);

            lpbdd->hil = ImageList_Duplicate((HIMAGELIST) lParam);

            return ((LRESULT)hOldImageList);
        }
        break;
        case BCM_GETIMAGELIST:
        {
            if (!lpbdd->hil)
                return (FALSE);
            return ((LRESULT) lpbdd->hil);
        }
        return (FALSE);
        case BCM_SETTITLEIMAGE:
        {
            int iOldImage = lpbdd->iImage;

            if (!lpbdd->hil)
                return (-1);

            lpbdd->iImage = (int)wParam;

            return (iOldImage);
        }
        break;
        case BCM_GETTITLEIMAGE:
        {
            if (!lpbdd->hil)
                return (FALSE);
            return ((LRESULT) lpbdd->iImage);
        }
        return (TRUE);
        case BCM_SETCLIENTWND:
        {
            HWND hwndOld = lpbdd->hwndClient;

            if (!IsWindow((HWND) lParam))
                return (FALSE);

            if (GetParent((HWND) lParam) != hwnd)
                SetParent((HWND) lParam,hwnd);

            lpbdd->hwndClient = (HWND) lParam;

            MoveWindow(lpbdd->hwndClient, lpbdd->rcClient.left, lpbdd->rcClient.top,
                     lpbdd->rcClient.right - lpbdd->rcClient.left,
                     lpbdd->rcClient.bottom - lpbdd->rcClient.top, TRUE);

            if (lpbdd->dwStyle & BCS_PAGER)
            {
                lpbdd->ptMinSize.y = lpbdd->rcClient.bottom - lpbdd->rcClient.top;
                lpbdd->ptMinSize.x = lpbdd->rcClient.right - lpbdd->rcClient.left;

                SetParent(lpbdd->hwndClient, lpbdd->hwndPager);
                Pager_SetChild(lpbdd->hwndPager, lpbdd->hwndClient);
                Pager_RecalcSize(lpbdd->hwndPager);
            }
            return ((LRESULT)hwndOld);
        }
        break;
        case BCM_GETCLIENTWND:
        {
            if (!IsWindow((HWND) lpbdd->hwndClient))
                return (FALSE);

            return ((LRESULT) lpbdd->hwndClient);
        }
        return (TRUE);
        case BCM_GETCLIENTRECT:
        {
            if (!lParam)
                return (FALSE);
            CopyRect((LPRECT) lParam, &lpbdd->rcClient);
        }
        return (TRUE);
        case BCM_GETALIGNMENT:
        {
            if (lpbdd->dwStyle & BCS_FLOAT)
                return (BCS_FLOAT);
            if (lpbdd->dwStyle & BCS_LEFT)
                return (BCS_LEFT);
            if (lpbdd->dwStyle & BCS_RIGHT)
                return (BCS_RIGHT);
            if (lpbdd->dwStyle & BCS_TOP)
                return (BCS_TOP);
            if (lpbdd->dwStyle & BCS_BOTTOM)
                return (BCS_BOTTOM);
        }
        return (-1);
        case BCM_SETALIGNMENT:
        {
//          if (wParam != BCS_LEFT && wParam != BCS_RIGHT)
//              return (-1);
            if (wParam == BCS_LEFT)
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDM_BAND_LEFT, 0), (LPARAM)hwnd);
            else if (wParam == BCS_RIGHT)
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDM_BAND_RIGHT, 0), (LPARAM)hwnd);
            else if (wParam == BCS_TOP)
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDM_BAND_TOP, 0), (LPARAM)hwnd);
            else if (wParam == BCS_BOTTOM)
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDM_BAND_BOTTOM, 0), (LPARAM)hwnd);
            else if (wParam == BCS_FLOAT)
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDM_BAND_FLOAT, 0), (LPARAM)hwnd);
        }
        return (-1);
        case BCM_SETMAXSIZE:
        {
            if (lParam)
            {
                lpbdd->ptMaxSize.x = LOWORD(lParam);
                lpbdd->ptMaxSize.y = HIWORD(lParam);
                return (TRUE);
            }
        }
        return (-1);
        case BCM_GETMAXSIZE:
        {
            LPPOINT lppt = (LPPOINT)lParam;

            lppt->x = lpbdd->ptMaxSize.x;
            lppt->y = lpbdd->ptMaxSize.y;
        }
        return (-1);
        case BCM_SETDLGTEMPLATE:
        {
            BOOL bRet;
            RECT re;

            if (wParam == 0 || lParam == 0L)
                return (FALSE);

            bRet = Band_SetDialogTemplate(hwnd, lpbdd, wParam, lParam);

            if (bRet == FALSE)
                return (FALSE);

            if (!(lpbdd->dwStyle & BCS_PAGER))
            {
                lpbdd->dwStyle |= BCS_PAGER;
                lpbdd->hwndPager = CreateWindowEx(0, WC_PAGESCROLLER, NULL, WS_VISIBLE | WS_CHILD | PGS_VERT | PGS_HORZ, 0, 0, 0, 0, hwnd, NULL, (HANDLE) lParam, NULL);

                MoveWindow(lpbdd->hwndPager, lpbdd->rcClient.left, lpbdd->rcClient.top,
                    lpbdd->rcClient.right - lpbdd->rcClient.left,
                    lpbdd->rcClient.bottom - lpbdd->rcClient.top, TRUE);
            }

            GetClientRect(lpbdd->hwndClient, &re);

            lpbdd->ptMinSize.x = re.right  - re.left;
            lpbdd->ptMinSize.y = re.bottom - re.top;

            GetWindowRect(lpbdd->hwndClient, &re);
            lpbdd->ptMaxSize.x = re.right  - re.left + 12;
            lpbdd->ptMaxSize.y = re.bottom - re.top  + 12;

            SetParent(lpbdd->hwndClient, lpbdd->hwndPager);
            Pager_SetChild(lpbdd->hwndPager, lpbdd->hwndClient);
            Pager_RecalcSize(lpbdd->hwndPager);
            GetWindowRect(hwnd, &re);
            SendMessage(hwnd, WM_SIZE, 0, MAKELPARAM(re.right - re.left, re.bottom - re.top));
            return (bRet);
        }
        return (TRUE);
        case BCM_SETALLOWABLEALIGNS:
            lpbdd->dwAllowableAligns = (DWORD)lParam;
        return (TRUE);
        case BCM_GETALLOWABLEALIGNS:
        return (lpbdd->dwAllowableAligns);
// Common Window Messages
        case WM_DROPFILES:
            return (Band_ForwardNotify(hwnd, wParam, lParam, lpbdd, BCN_DROP));
        case WM_GETMINMAXINFO:
        {
            MINMAXINFO *lpmmi = (MINMAXINFO *) lParam;

            if ((lpmmi && lpbdd->ptMaxSize.x != 0 && lpbdd->ptMaxSize.y != 0) || (lpbdd->dwStyle & BCS_FLOAT))
            {
                if (lpmmi && lpbdd->ptMaxSize.x != 0 && lpbdd->ptMaxSize.y != 0)
                {
                    lpmmi->ptMaxTrackSize.x = (lpbdd->ptMaxSize.x);
                    lpmmi->ptMaxTrackSize.y = (lpbdd->ptMaxSize.y);
                }

                if (lpbdd->dwStyle & BCS_LEFT)
                    lpmmi->ptMinTrackSize.x = 50;
                else if (lpbdd->dwStyle & BCS_RIGHT)
                    lpmmi->ptMinTrackSize.x = 50;
                else if (lpbdd->dwStyle & BCS_TOP)
                    lpmmi->ptMinTrackSize.y = 50;
                else if (lpbdd->dwStyle & BCS_BOTTOM)
                    lpmmi->ptMinTrackSize.y = 50;
                else if (lpbdd->dwStyle & BCS_FLOAT)
                {
                    lpmmi->ptMinTrackSize.x = 56;
                    lpmmi->ptMinTrackSize.y = (lpbdd->rcCaption.bottom + lpbdd->rcCaption.top) + 4;
                }
            }
        }
        return (FALSE);
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDM_BAND_MAXIMIZE:
                {
                    RECT rc, re;

                    if (!(lpbdd->dwStyle & BCS_PAGER))
                        return (FALSE);

                    GetWindowRect(hwnd, &rc);
                    GetClientRect(GetParent(hwnd), &re);

                    if (rc.right - rc.left >= lpbdd->ptMaxSize.x || re.right - re.left < lpbdd->ptMaxSize.x)
                        return (FALSE);
                    else if (lpbdd->dwStyle & BCS_RIGHT)
                    {
                        Window_ScreenToClientRect(GetParent(hwnd), &rc);
                        SetWindowPos(hwnd, NULL, re.right - lpbdd->ptMaxSize.x, rc.top, lpbdd->ptMaxSize.x, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
                    }
                    else if (lpbdd->dwStyle & BCS_LEFT)
                    {
                        SetWindowPos(hwnd, NULL, 0, 0, lpbdd->ptMaxSize.x, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
                    }

                }
                break;
                case IDM_BAND_CLOSE:
                    Band_NotifyParent(hwnd, lpbdd, BCN_BANDCLOSE);
                break;
                case IDM_BAND_LEFT:
                    if (lpbdd->dwStyle & BCS_LEFT || lpbdd->dwStyle & BCS_FIXED)
                        break;

                    lpbdd->dwOldStyle   = lpbdd->dwStyle;
                    lpbdd->dwStyle      = Band_ModifyStyle(lpbdd->dwStyle, BCS_LEFT);

                    Band_RecalcSize(hwnd, lpbdd);
                    Band_NotifyParent(hwnd, lpbdd, BCN_STYLECHANGED);
                break;
                case IDM_BAND_RIGHT:
                    if (lpbdd->dwStyle & BCS_RIGHT || lpbdd->dwStyle & BCS_FIXED)
                        break;

                    lpbdd->dwOldStyle = lpbdd->dwStyle;
                    lpbdd->dwStyle      = Band_ModifyStyle(lpbdd->dwStyle, BCS_RIGHT);

                    Band_RecalcSize(hwnd, lpbdd);
                    Band_NotifyParent(hwnd, lpbdd, BCN_STYLECHANGED);
                break;
                case IDM_BAND_TOP:
                    if (lpbdd->dwStyle & BCS_TOP || lpbdd->dwStyle & BCS_FIXED)
                        break;

                    lpbdd->dwOldStyle = lpbdd->dwStyle;
                    lpbdd->dwStyle      = Band_ModifyStyle(lpbdd->dwStyle, BCS_TOP);

                    Band_RecalcSize(hwnd, lpbdd);
                    Band_NotifyParent(hwnd, lpbdd, BCN_STYLECHANGED);
                break;
                case IDM_BAND_BOTTOM:
                    if (lpbdd->dwStyle & BCS_BOTTOM || lpbdd->dwStyle & BCS_FIXED)
                        break;

                    lpbdd->dwOldStyle = lpbdd->dwStyle;
                    lpbdd->dwStyle      = Band_ModifyStyle(lpbdd->dwStyle, BCS_BOTTOM);

                    Band_RecalcSize(hwnd, lpbdd);
                    Band_NotifyParent(hwnd, lpbdd, BCN_STYLECHANGED);
                    break;
                case IDM_BAND_FLOAT:
                    if (lpbdd->dwStyle & BCS_FLOAT || lpbdd->dwStyle & BCS_FIXED)
                        break;

                    SendMessage(hwnd, WM_NCLBUTTONDBLCLK, HTCAPTION, 0);
                break;
                default:
                    if (!(lpbdd->dwStyle & BCS_FORWARDCOMMAND))
                        return (SendMessage(lpbdd->hwndOwner, uMessage, wParam, lParam));
                    else
                        return (Band_ForwardNotify(hwnd, wParam, lParam, lpbdd, BCN_CTRLCOMMAND));
            }
        }
        break;
        case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR) lParam;

            if (!lpbdd)
                return (FALSE);

            if (lpnmhdr->hwndFrom == lpbdd->hwndPager)
            {
                if (lpnmhdr->code == PGN_CALCSIZE)
                {
                    LPNMPGCALCSIZE lpnmcs = (LPNMPGCALCSIZE) lParam;

                    if (lpnmcs->dwFlag & PGF_CALCHEIGHT)
                        lpnmcs->iHeight = lpbdd->ptMinSize.y;
                    if (lpnmcs->dwFlag & PGF_CALCWIDTH)
                        lpnmcs->iWidth = lpbdd->ptMinSize.x;
                }
            }
            else if (!(lpbdd->dwStyle & BCS_FORWARDNOTIFY))
                return (SendMessage(lpbdd->hwndOwner, uMessage, wParam, lParam));
            else
                return (Band_ForwardNotify(hwnd, wParam, lParam, lpbdd, BCN_CTRLNOTIFY));
        }
        return (FALSE);
        case WM_DRAWITEM:
        case WM_MEASUREITEM:
            return (SendMessage(lpbdd->hwndOwner, uMessage, wParam, lParam));
        case WM_CREATE:
        {
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
            lpbdd->dwStyle = 0;
            _tcscpy(lpbdd->szTitle, lpcs->lpszName);

            lpbdd->wID           = (UINT) lpcs->hMenu;
            lpbdd->hwndOwner = lpcs->hwndParent;

            if (lpcs->style & BCS_BOTTOM)
                lpbdd->dwStyle |= BCS_BOTTOM;
            else if (lpcs->style & BCS_TOP)
                lpbdd->dwStyle |= BCS_TOP;
            else if (lpcs->style & BCS_LEFT)
                lpbdd->dwStyle |= BCS_LEFT;
            else if (lpcs->style & BCS_RIGHT)
                lpbdd->dwStyle |= BCS_RIGHT;
            else if (lpcs->style & BCS_FLOAT)
                lpbdd->dwStyle |= BCS_FLOAT;

            if (lpcs->style & BCS_PAGER)
                lpbdd->dwStyle |= BCS_PAGER;

            if (lpcs->style & BCS_FIXED)
                lpbdd->dwStyle |= BCS_FIXED;

            if (lpbdd->dwStyle & BCS_FLOAT)
            {
                //DWORD dwExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

                if (!(lpcs->style & WS_CHILD))
                    lpcs->style |= WS_CHILD;

                SetWindowLong(hwnd, GWL_STYLE, lpcs->style);
            }

            if (lpcs->style & BCS_FORWARDCOMMAND)
                lpbdd->dwStyle |= BCS_FORWARDCOMMAND;

            if (lpcs->style & BCS_FORWARDNOTIFY)
                lpbdd->dwStyle |= BCS_FORWARDNOTIFY;

            if (lpcs->style & BCS_NOTIFYSIZE)
                lpbdd->dwStyle |= BCS_NOTIFYSIZE;

            if (lpbdd->dwStyle & BCS_PAGER)
            {
                lpbdd->hwndPager = CreateWindowEx(0, WC_PAGESCROLLER, NULL, WS_VISIBLE | WS_CHILD | PGS_VERT, 0, 0, 0, 0, hwnd, NULL, lpcs->hInstance, NULL);

                if (!(lpbdd->dwStyle & BCS_FIXED))
                    lpbdd->dwStyle |= BCS_FIXED;
            }

            lpbdd->bNotify = TRUE;

            lpbdd->dwAllowableAligns    = BCS_LEFT | BCS_RIGHT | BCS_BOTTOM | BCS_TOP | BCS_FLOAT;
        }
        return (TRUE);
/*      case WM_TIMER:
            if (lpbdd->bDraging == TRUE)
            {
                POINT pt;
                RECT re;

//              GetWindowRect(hwnd, &re);
//              re = lpbdd->rcClose;
//              Window_ClientToScreenRect(hwnd, &re);
                GetCursorPos(&pt);
                ScreenToClient(hwnd, &pt);
                pt.y += Rect_Height(&lpbdd->rcCaption) + 2; //+ GetSystemMetrics(SM_BORDER);

                if (!PtInRect(&lpbdd->rcClose, pt))
                {
                    lpbdd->bDraging = FALSE;
                    Band_DrawCloseButton(hwnd, lpbdd, FALSE);
                    KillTimer(hwnd, lpbdd->timerID);
                }
                return (0);
            }
        return (TRUE);*/
        case WM_NCACTIVATE:
        return (FALSE);
        case WM_NCCREATE:
        {
            lpbdd = (LPBANDCTRL)Mem_Alloc(sizeof(BANDCTRL));

            if (!lpbdd)
                return (TRUE);

            lpbdd->hwndOwner = GetParent(hwnd);
            lpbdd->crBack    = BAND_DEFAULT_CRBACK;
            lpbdd->crBorder  = BAND_DEFAULT_CRBORDER;
            SetWindowLong(hwnd, GWL_USERDATA, (long)lpbdd);
        }
        return (TRUE);
        case WM_SIZE:
        {
            if (lpbdd)
            {
                //POINT pt = {LOWORD(lParam), HIWORD(lParam)};
                RECT    re;
                int     x,y;

                GetClientRect(hwnd, &lpbdd->rcClient);
                GetWindowRect(hwnd, &re);
                OffsetRect(&re, -re.left, -re.top);

                CopyRect(&lpbdd->rcBand, &re);

                CopyRect(&lpbdd->rcTop, &lpbdd->rcBand);
                lpbdd->rcTop.bottom = lpbdd->rcTop.top           + 4;

                CopyRect(&lpbdd->rcLeft, &lpbdd->rcBand);
                lpbdd->rcLeft.right = lpbdd->rcLeft.left         + 4;

                CopyRect(&lpbdd->rcRight, &lpbdd->rcBand);
                lpbdd->rcRight.left = lpbdd->rcTop.right         - 4;

                CopyRect(&lpbdd->rcBottom, &lpbdd->rcBand);
                            lpbdd->rcBottom.top = lpbdd->rcBottom.bottom - 4;

                if (lpbdd->dwStyle & BCS_LEFT)
                    re.right    -= 4;
                if (lpbdd->dwStyle & BCS_RIGHT)
                    re.left     += 4;
                if (lpbdd->dwStyle & BCS_TOP)
                    re.bottom -= 4;
                if (lpbdd->dwStyle & BCS_BOTTOM)
                    re.top      += 4;

                if (lpbdd->dwStyle & BCS_FLOAT)
                {
                    re.top      += 4;
                    re.left     += 4;
                    re.bottom   -= 4;
                    re.right    -= 4;
                }

                if (lpbdd->dwStyle & BCS_TOP || lpbdd->dwStyle & BCS_BOTTOM)
                {
                    re.right = re.left + BAND_DEFAULT_CAPTION_CY;
                    CopyRect(&lpbdd->rcCaption, &re);

                    x = re.left + ((re.right - re.left) / 2) - 4;
                    y = re.top  + 6;

                    lpbdd->rcClose.top   = y;
                    lpbdd->rcClose.left  = x - 2;
                    lpbdd->rcClose.right  = lpbdd->rcClose.left + 12;
                    lpbdd->rcClose.bottom = lpbdd->rcClose.top  + 11;
                    InflateRect(&lpbdd->rcClose, 2, 2);

                    lpbdd->rcIcon.top       = (lpbdd->dwStyle & BCS_TOP ? lpbdd->rcCaption.bottom - 18 : lpbdd->rcCaption.bottom - 22);
                    lpbdd->rcIcon.bottom = lpbdd->rcIcon.top            + 16;
                    lpbdd->rcIcon.left  = lpbdd->rcCaption.left     + ((lpbdd->rcCaption.right - lpbdd->rcCaption.left) / 2) - 8;
                    lpbdd->rcIcon.right = lpbdd->rcIcon.left            + 16;
                }
                else
                {
                    re.bottom = re.top + BAND_DEFAULT_CAPTION_CY;
                    CopyRect(&lpbdd->rcCaption, &re);

                    x = re.right - 20;
                    y = re.top  + ((re.bottom - re.top) / 2) - 4;

                    lpbdd->rcClose.top   = y - 1;
                    lpbdd->rcClose.left  = x - 1;
                    lpbdd->rcClose.right  = x + 11;
                    lpbdd->rcClose.bottom = y + 10;
                    InflateRect(&lpbdd->rcClose, 2, 2);


                    lpbdd->rcIcon.top       = lpbdd->rcCaption.top  + ((lpbdd->rcCaption.bottom - lpbdd->rcCaption.top) / 2) - 8;
                    lpbdd->rcIcon.bottom = lpbdd->rcIcon.top        + 16;
                    lpbdd->rcIcon.left  = lpbdd->rcCaption.left + 4;
                    lpbdd->rcIcon.right = lpbdd->rcIcon.left        + 16;
                }

                //Added by ACiD Freak
                RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);

                if (IsWindow(lpbdd->hwndOwner))
                {
                    if (lpbdd->dwStyle & BCS_PAGER)
                        MoveWindow(lpbdd->hwndPager, lpbdd->rcClient.left, lpbdd->rcClient.top,
                             lpbdd->rcClient.right  - lpbdd->rcClient.left,
                             lpbdd->rcClient.bottom - lpbdd->rcClient.top, TRUE);

                    if (!(lpbdd->dwStyle & BCS_PAGER) && !(lpbdd->dwStyle & BCS_NOTIFYSIZE) && IsWindow(lpbdd->hwndClient))
                    {
                        MoveWindow(lpbdd->hwndClient, lpbdd->rcClient.left, lpbdd->rcClient.top,
                            lpbdd->rcClient.right   - lpbdd->rcClient.left,
                            lpbdd->rcClient.bottom - lpbdd->rcClient.top, FALSE);

                        RedrawWindow(lpbdd->hwndClient, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_NOINTERNALPAINT);
                    }

                    if (lpbdd->bNotify == TRUE)
                        Band_NotifyParent(hwnd, lpbdd, BCN_POSCHANGED);             
                }
            }
        }
        return (FALSE);
        case WM_NCHITTEST:
        {
            POINT pt1, pt = {LOWORD(lParam), HIWORD(lParam)};
            RECT re;

            GetWindowRect(hwnd, &re);
            pt1 = pt;

            pt1.y = pt1.y - re.top;
            pt1.x = pt1.x - re.left;

            if (!PtInRect(&re, pt))
                return (HTNOWHERE);

            pt = pt1;

            if (lpbdd->dwStyle & BCS_FLOAT && lpbdd->bOnClose == FALSE)
            {
                SetRect(&re, lpbdd->rcLeft.left, lpbdd->rcTop.top, lpbdd->rcLeft.right, lpbdd->rcTop.bottom);
                if (PtInRect(&re, pt))
                    return (HTTOPLEFT);

                SetRect(&re, lpbdd->rcRight.left, lpbdd->rcTop.top, lpbdd->rcRight.right, lpbdd->rcTop.bottom);
                if (PtInRect(&re, pt))
                    return (HTTOPRIGHT);

                SetRect(&re, lpbdd->rcRight.left, lpbdd->rcBottom.top, lpbdd->rcRight.right, lpbdd->rcBottom.bottom);
                if (PtInRect(&re, pt))
                    return (HTBOTTOMRIGHT);

                SetRect(&re, lpbdd->rcLeft.left, lpbdd->rcBottom.top, lpbdd->rcLeft.right, lpbdd->rcBottom.bottom);
                if (PtInRect(&re, pt))
                    return (HTBOTTOMLEFT);
            }

            if (PtInRect(&lpbdd->rcRight, pt) && ((lpbdd->dwStyle & BCS_LEFT) || (lpbdd->dwStyle & BCS_FLOAT)))
                if (lpbdd->bOnClose == FALSE)
                    return (HTRIGHT);

            if (PtInRect(&lpbdd->rcLeft, pt) && ((lpbdd->dwStyle & BCS_RIGHT) || (lpbdd->dwStyle & BCS_FLOAT)))
                if (lpbdd->bOnClose == FALSE)
                    return (HTLEFT);

            if (PtInRect(&lpbdd->rcBottom, pt) && ((lpbdd->dwStyle & BCS_TOP) || (lpbdd->dwStyle & BCS_FLOAT)))
                if (lpbdd->bOnClose == FALSE)
                    return (HTBOTTOM);

            if (PtInRect(&lpbdd->rcTop, pt) && ((lpbdd->dwStyle & BCS_BOTTOM) || (lpbdd->dwStyle & BCS_FLOAT)))
                if (lpbdd->bOnClose == FALSE)
                    return (HTTOP);

            if (PtInRect(&lpbdd->rcIcon, pt))
                return (HTSYSMENU);

            if (PtInRect(&lpbdd->rcClose, pt))
                return (HTCLOSE);

            if (PtInRect(&lpbdd->rcCaption, pt))
                return (HTCAPTION);

            if (PtInRect(&lpbdd->rcClient, pt))
                return (HTCLIENT);
        }
        return (HTNOWHERE);
        case WM_NCMOUSEMOVE:
        {
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            RECT    re;

            GetWindowRect(hwnd, &re);

            pt.y = pt.y - re.top;
            pt.x = pt.x - re.left;

            if (wParam == HTCLOSE)
            {
                if (lpbdd->bDraging)
                    return (0);
                lpbdd->bDraging = TRUE;
//              lpbdd->timerID = SetTimer(hwnd, 99, 100, NULL);
                Band_DrawCloseButton(hwnd, lpbdd, TRUE);
                return (0);
            }
            else if (lpbdd->bDraging)
            {
                lpbdd->bDraging = FALSE;
//              KillTimer(hwnd, lpbdd->timerID);
                Band_DrawCloseButton(hwnd, lpbdd, FALSE);
                return (0);
            }
        }
        break;
        case WM_LBUTTONUP:
        {
            POINTS pt1 = MAKEPOINTS(lParam);
            POINT  pt;
            RECT     re;

            pt.y = pt1.y;
            pt.x = pt1.x;

            ClientToScreen(hwnd, &pt);
            GetWindowRect(hwnd, &re);

            pt.y = pt.y - re.top;
            pt.x = pt.x - re.left;

            if (lpbdd->bOnClose == TRUE && PtInRect(&lpbdd->rcClose, pt) && GetCapture() == hwnd)
            {
                lpbdd->bOnClose = FALSE;
                Band_DrawCloseButton(hwnd, lpbdd, FALSE);
                ReleaseCapture();
                Band_NotifyParent(hwnd, lpbdd, BCN_BANDCLOSE);
                return (0);
            }
            else if (lpbdd->bOnClose == TRUE && GetCapture() == hwnd)
            {
                lpbdd->bOnClose = FALSE;
                Band_DrawCloseButton(hwnd, lpbdd, FALSE);
                ReleaseCapture();
                return (0);
            }
        }
        return (1);
        case WM_MOUSEMOVE:
        {
            POINTS pt1 = MAKEPOINTS(lParam);
            POINT pt;
            RECT re;

            pt.y = pt1.y;
            pt.x = pt1.x;

            ClientToScreen(hwnd, &pt);
            GetWindowRect(hwnd, &re);

            pt.y = pt.y - re.top;
            pt.x = pt.x - re.left;

            if (lpbdd->bOnClose == TRUE && PtInRect(&lpbdd->rcClose, pt) && GetCapture() == hwnd)
            {
                if (lpbdd->bDraging)
                    return (0);

                Band_DrawCloseButton(hwnd, lpbdd, TRUE);

                return (0);
            }
            else if (lpbdd->bOnClose == TRUE && GetCapture() == hwnd)
            {
                Band_DrawCloseButton(hwnd, lpbdd, FALSE);

                return (0);
            }
        }
        return (1);
        case WM_NCLBUTTONDBLCLK:
        {
            if (wParam == HTSYSMENU)
                Band_NotifyParent(hwnd, lpbdd, BCN_BANDCLOSE);

            if (wParam == HTCAPTION && !(lpbdd->dwStyle & BCS_FLOAT) && !(lpbdd->dwStyle & BCS_FIXED))
            {
                DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

                lpbdd->dwOldStyle = lpbdd->dwStyle;
                lpbdd->dwStyle = Band_ModifyStyle(lpbdd->dwStyle, BCS_FLOAT);

                dwStyle = Band_ModifyStyle(dwStyle, BCS_FLOAT);
                SetWindowLong(hwnd, GWL_STYLE, dwStyle);

                if (lpbdd->wpl.length != 0);
                SetWindowPlacement(hwnd, &lpbdd->wpl);

                Band_RecalcSize(hwnd, lpbdd);
                Band_NotifyParent(hwnd, lpbdd, BCN_STYLECHANGED);
            }
            else if (wParam == HTCAPTION && lpbdd->dwStyle & BCS_FLOAT && !(lpbdd->dwStyle & BCS_FIXED))
            {
                DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

                if (!(lpbdd->dwOldStyle & BCS_LEFT) &&
                !(lpbdd->dwOldStyle & BCS_TOP) &&
                !(lpbdd->dwOldStyle & BCS_RIGHT) &&
                !(lpbdd->dwOldStyle & BCS_BOTTOM))
                    return (0);
                else
                    lpbdd->dwStyle = Band_ModifyStyle(lpbdd->dwStyle, lpbdd->dwOldStyle);

                lpbdd->wpl.length = sizeof(WINDOWPLACEMENT);
                GetWindowPlacement(hwnd, &lpbdd->wpl);

                dwStyle = Band_ModifyStyle(dwStyle, lpbdd->dwOldStyle);
                SetWindowLong(hwnd, GWL_STYLE, dwStyle);

                Band_RecalcSize(hwnd, lpbdd);
                Band_NotifyParent(hwnd, lpbdd, BCN_STYLECHANGED);
            }
        }
        return (1);
        case WM_NCLBUTTONDOWN:
        {
            //POINT pt1 =   {LOWORD(lParam), HIWORD(lParam)};

            if (wParam == HTCLOSE && lpbdd->bOnClose == FALSE)
            {
                lpbdd->bOnClose = TRUE;
                Band_DrawCloseButton(hwnd, lpbdd, TRUE);
                SetCapture(hwnd);
                return (0);
            }

            if (wParam == HTCAPTION && !(lpbdd->dwStyle & BCS_FLOAT))
                return (0);

            if (wParam == HTSYSMENU)
            {
                HMENU   hMenu  = LoadMenu(PCPControls_GetHandle(), MAKEINTRESOURCE(IDR_BANDMENU));
                HMENU   hPopup = GetSubMenu(hMenu, 0);
                RECT    re;
                POINT   pt;

                if (lpbdd->dwStyle & BCS_LEFT)
                    CheckMenuItem(hMenu, IDM_BAND_LEFT, MF_BYCOMMAND | MF_CHECKED);
                else
                    CheckMenuItem(hMenu, IDM_BAND_LEFT, MF_BYCOMMAND | MF_UNCHECKED);

                if (lpbdd->dwStyle & BCS_RIGHT)
                    CheckMenuItem(hMenu, IDM_BAND_RIGHT, MF_BYCOMMAND | MF_CHECKED);
                else
                    CheckMenuItem(hMenu, IDM_BAND_RIGHT, MF_BYCOMMAND | MF_UNCHECKED);

                if (lpbdd->dwStyle & BCS_TOP)
                    CheckMenuItem(hMenu, IDM_BAND_TOP, MF_BYCOMMAND | MF_CHECKED);
                else
                    CheckMenuItem(hMenu, IDM_BAND_TOP, MF_BYCOMMAND | MF_UNCHECKED);

                if (lpbdd->dwStyle & BCS_BOTTOM)
                    CheckMenuItem(hMenu, IDM_BAND_BOTTOM, MF_BYCOMMAND | MF_CHECKED);
                else
                    CheckMenuItem(hMenu, IDM_BAND_BOTTOM, MF_BYCOMMAND | MF_UNCHECKED);

                if (lpbdd->dwStyle & BCS_FLOAT)
                    CheckMenuItem(hMenu, IDM_BAND_FLOAT, MF_BYCOMMAND | MF_CHECKED);
                else
                    CheckMenuItem(hMenu, IDM_BAND_FLOAT, MF_BYCOMMAND | MF_UNCHECKED);

                if (lpbdd->dwStyle & BCS_FIXED)
                {
                    EnableMenuItem(hMenu, IDM_BAND_FLOAT, MF_BYCOMMAND | MF_GRAYED);
                    EnableMenuItem(hMenu, IDM_BAND_BOTTOM, MF_BYCOMMAND | MF_GRAYED);
                    EnableMenuItem(hMenu, IDM_BAND_TOP, MF_BYCOMMAND | MF_GRAYED);
                    EnableMenuItem(hMenu, IDM_BAND_RIGHT, MF_BYCOMMAND | MF_GRAYED);
                    EnableMenuItem(hMenu, IDM_BAND_LEFT, MF_BYCOMMAND | MF_GRAYED);
                }

                Menu_EnableMenuItem(hMenu, IDM_BAND_FLOAT, (lpbdd->dwAllowableAligns & BCS_FLOAT));
                Menu_EnableMenuItem(hMenu, IDM_BAND_BOTTOM, (lpbdd->dwAllowableAligns & BCS_BOTTOM));
                Menu_EnableMenuItem(hMenu, IDM_BAND_TOP, (lpbdd->dwAllowableAligns & BCS_TOP));
                Menu_EnableMenuItem(hMenu, IDM_BAND_RIGHT, (lpbdd->dwAllowableAligns & BCS_RIGHT));
                Menu_EnableMenuItem(hMenu, IDM_BAND_LEFT, (lpbdd->dwAllowableAligns & BCS_LEFT));

                if (!(lpbdd->dwStyle & BCS_PAGER))
                {
                    EnableMenuItem(hMenu, IDM_BAND_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);
                }
                else
                {
                    RECT re, rc;

                    GetWindowRect(hwnd, &rc);
                    GetClientRect(GetParent(hwnd), &re);

                    if (rc.right - rc.left >= lpbdd->ptMaxSize.x || re.right - re.left < lpbdd->ptMaxSize.x)
                        EnableMenuItem(hMenu, IDM_BAND_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);
                }

                GetClientRect(hwnd, &re);

                if (lpbdd->dwStyle & BCS_TOP || lpbdd->dwStyle & BCS_BOTTOM)
                {
                    pt.x = re.left;
                    pt.y = re.bottom - 18;
                }
                else
                {
                    pt.x = re.left;
                    pt.y = re.top - 2;
                }

                ClientToScreen(hwnd, &pt);

                Menu_TrackPopupMenu(hPopup, pt.x, pt.y, hwnd);
                DestroyMenu(hMenu);
                return (TRUE);
            }
        }
        break;
        case WM_NCRBUTTONDOWN:
            if (wParam == HTCAPTION || wParam == HTSYSMENU)
            {
                HMENU   hMenu  = LoadMenu(PCPControls_GetHandle(), MAKEINTRESOURCE(IDR_BANDMENU));
                HMENU   hPopup = GetSubMenu(hMenu, 0);
                POINT   pt;

                GetCursorPos(&pt);
                Menu_TrackPopupMenu(hPopup, pt.x, pt.y, hwnd);
                DestroyMenu(hMenu);
            }
        return (TRUE); 
        case WM_NCCALCSIZE:
        {
            if (wParam == TRUE)
            {
                NCCALCSIZE_PARAMS *ncp = (NCCALCSIZE_PARAMS *) lParam;

                if (lpbdd->dwStyle & BCS_TOP || lpbdd->dwStyle & BCS_BOTTOM)
                     ncp->rgrc[0].left += BAND_DEFAULT_CAPTION_CY + 2;
                else
                     ncp->rgrc[0].top  += BAND_DEFAULT_CAPTION_CY + 2;

                if (lpbdd->dwStyle & BCS_LEFT)
                {
                    ncp->rgrc[0].right  -= 6;
                    ncp->rgrc[0].left   += 2;
                    ncp->rgrc[0].bottom -= 2;
                }

                if (lpbdd->dwStyle & BCS_RIGHT)
                {
                    ncp->rgrc[0].left   += 6;
                    ncp->rgrc[0].right  -= 2;
                    ncp->rgrc[0].bottom -= 2;
                }

                if (lpbdd->dwStyle & BCS_TOP)
                {
                    ncp->rgrc[0].bottom -= 4;
                    ncp->rgrc[0].right  -= 2;
                    ncp->rgrc[0].left   += 2;
                }

                if (lpbdd->dwStyle & BCS_BOTTOM)
                {
                    ncp->rgrc[0].top        += 4;
                    ncp->rgrc[0].right  -= 2;
                    ncp->rgrc[0].left   += 2;
                }
                if (lpbdd->dwStyle & BCS_FLOAT)
                {
                    ncp->rgrc[0].top        += 4;
                    ncp->rgrc[0].right  -= 6;
                    ncp->rgrc[0].left   += 6;
                    ncp->rgrc[0].bottom -= 6;
                }
            }
        }
        break;
        case WM_STYLECHANGED:
        {
            SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            SetWindowLong(hwnd, GWL_ID, lpbdd->wID);
        }
        break;
        case WM_NCPAINT:
        {
            HDC         hdc = GetWindowDC(hwnd);
            RECT        rc, rcClient, rcWindow;
            HIMAGELIST  hil;
            HBITMAP     hbm;
            HINSTANCE   hInstance = PCPControls_GetHandle();

            GetClientRect(hwnd, &rcClient);
            GetWindowRect(hwnd, &rcWindow);

            Window_ScreenToClientRect(hwnd, &rcWindow);
            OffsetRect(&rcClient, -rcWindow.left, -rcWindow.top);
            ExcludeClipRect(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

            if (lpbdd->dwStyle & BCS_LEFT)
                Paint_FillRect(hdc, &lpbdd->rcRight, GetSysColor(COLOR_BTNFACE));

            if (lpbdd->dwStyle & BCS_RIGHT)
                Paint_FillRect(hdc, &lpbdd->rcLeft, GetSysColor(COLOR_BTNFACE));

            if (lpbdd->dwStyle & BCS_TOP)
                Paint_FillRect(hdc, &lpbdd->rcBottom, GetSysColor(COLOR_BTNFACE));

            if (lpbdd->dwStyle & BCS_BOTTOM)
                Paint_FillRect(hdc, &lpbdd->rcTop, GetSysColor(COLOR_BTNFACE));

            if (lpbdd->dwStyle & BCS_FLOAT)
            {
                Paint_FillRect(hdc, &lpbdd->rcTop, GetSysColor(COLOR_BTNFACE));
                Paint_FillRect(hdc, &lpbdd->rcBottom, GetSysColor(COLOR_BTNFACE));
                Paint_FillRect(hdc, &lpbdd->rcLeft, GetSysColor(COLOR_BTNFACE));
                Paint_FillRect(hdc, &lpbdd->rcRight, GetSysColor(COLOR_BTNFACE));
            }

            Paint_FillRect(hdc, &lpbdd->rcCaption, GetSysColor(COLOR_BTNFACE));

            hil = ImageList_Create(10, 9, ILC_COLOR | ILC_MASK, 0, 2);
            hbm = Paint_Bitmap_LoadSysColorBitmap(hInstance, FindResource(hInstance, MAKEINTRESOURCE(IDB_CHECKBOX), RT_BITMAP), FALSE);
            ImageList_AddMasked(hil, hbm, RGB(255, 0, 255));
            DeleteObject(hbm);
            hbm = Paint_Bitmap_LoadSysColorBitmap(hInstance, FindResource(hInstance, MAKEINTRESOURCE(IDB_CHECKBOX), RT_BITMAP), FALSE);
            ImageList_Add(hil, hbm, NULL);
            DeleteObject(hbm);

            if (hil != NULL)
            {
                int x, y;

                x = lpbdd->rcClose.left + ((lpbdd->rcClose.right - lpbdd->rcClose.left) / 2) - 5;
                y = lpbdd->rcClose.top  + ((lpbdd->rcClose.bottom - lpbdd->rcClose.top) / 2) - 4;

                ImageList_Draw(hil, 0, hdc, x, y, ILD_NORMAL | ILD_TRANSPARENT);
                ImageList_Destroy(hil);

                if (lpbdd->bOnClose)
                    DrawEdge(hdc, &lpbdd->rcClose, BDR_RAISEDINNER, BF_RECT);
            }

            if (lpbdd->hil != NULL)
                ImageList_Draw(lpbdd->hil, lpbdd->iImage, hdc, lpbdd->rcIcon.left, lpbdd->rcIcon.top, ILD_NORMAL | ILD_TRANSPARENT);

            if (_tcslen(lpbdd->szTitle) > 0 && hdc)
            {
                RECT re; //, reLine;
                TCHAR szTitle[200];
//              int  mode, cx; //, cnt, y;

                if (!(lpbdd->dwStyle & BCS_TOP) && !(lpbdd->dwStyle & BCS_BOTTOM))
                {
                    re.top   = lpbdd->rcCaption.top;
                    re.right  = lpbdd->rcCaption.right - 22;
                    re.left  = lpbdd->rcCaption.left    + lpbdd->rcIcon.right + 4;
                    re.bottom = lpbdd->rcCaption.bottom;

                    SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
                    SelectObject(hdc, GetStockObject(ANSI_VAR_FONT));

                    wsprintf(szTitle, _T("  %s  "), lpbdd->szTitle);

                    re.top++;
                    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
                    DrawText(hdc, szTitle, _tcslen(szTitle), &re, DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE);
                }
            }
                    
            if (lpbdd->dwStyle & BCS_TOP || lpbdd->dwStyle & BCS_BOTTOM)
            {
                rc.top   = lpbdd->rcCaption.top;
                rc.left  = lpbdd->rcCaption.left;
                rc.right  = lpbdd->rcCaption.right;
                rc.bottom = lpbdd->rcBand.bottom;
            }
            else if(lpbdd->dwStyle & BCS_LEFT || lpbdd->dwStyle & BCS_RIGHT)
            {
                rc.top   = lpbdd->rcCaption.top;
                rc.left  = lpbdd->rcCaption.left;
                rc.right  = lpbdd->rcCaption.right;
                rc.bottom = lpbdd->rcBand.bottom;
            }
            else
            {
                rc.top   = lpbdd->rcCaption.top;
                rc.left  = lpbdd->rcCaption.left;
                rc.right  = lpbdd->rcCaption.right;
                rc.bottom = lpbdd->rcCaption.bottom;
            }

            DrawEdge(hdc, &rc, EDGE_ETCHED, BF_RECT);

            CopyRect(&rc, &lpbdd->rcCaption);

            InflateRect(&rc, -2, 0);

            if (lpbdd->dwStyle & BCS_TOP || lpbdd->dwStyle & BCS_BOTTOM)
            {
                rc.right = lpbdd->rcCaption.right + 2;
                rc.left = rc.right;
                DrawEdge(hdc, &rc, EDGE_ETCHED, BF_LEFT);

                rc.left = rc.right;
                rc.right = rc.left + 2;
                DrawEdge(hdc, &rc, EDGE_ETCHED, BF_LEFT);
            }
            else
            {
                rc.bottom = lpbdd->rcCaption.bottom;
                DrawEdge(hdc, &rc, EDGE_ETCHED, BF_BOTTOM);
             
                rc.bottom = rc.bottom + 2;
                DrawEdge(hdc, &rc, EDGE_ETCHED, BF_BOTTOM);
            }

            if (lpbdd->dwStyle & BCS_LEFT)
                DrawEdge(hdc, &lpbdd->rcRight, EDGE_ETCHED, BF_RIGHT);

            if (lpbdd->dwStyle & BCS_RIGHT)
                DrawEdge(hdc, &lpbdd->rcLeft, EDGE_ETCHED, BF_LEFT);

            if (lpbdd->dwStyle & BCS_TOP)
                DrawEdge(hdc, &lpbdd->rcBottom, EDGE_ETCHED, BF_BOTTOM);

            if (lpbdd->dwStyle & BCS_BOTTOM)
                DrawEdge(hdc, &lpbdd->rcTop, EDGE_ETCHED, BF_TOP);

            if (lpbdd->dwStyle & BCS_FLOAT)
            {
                rc.top      = lpbdd->rcCaption.bottom;
                rc.left     = lpbdd->rcCaption.left;
                rc.right    = lpbdd->rcCaption.right;
                rc.bottom = lpbdd->rcBottom.top;

                DrawEdge(hdc, &rc, EDGE_ETCHED, BF_RECT);
                DrawEdge(hdc, &lpbdd->rcBand, EDGE_RAISED, BF_RECT);
            }
            ReleaseDC(hwnd, hdc);
        }
        break;
        case WM_DESTROY:
        {
            if (lpbdd)
            {
                if (lpbdd->hil)
                    ImageList_Destroy(lpbdd->hil);

//              KillTimer(hwnd, lpbdd->timerID);
                SetWindowLong(hwnd, GWL_USERDATA, 0);
                Mem_Free(lpbdd);
            }
        }
        return (FALSE);
        default:
        break;
    }
    return (DefWindowProc(hwnd, uMessage, wParam, lParam));
}

LRESULT CALLBACK BandChildWindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
        case WM_CREATE:
        return (FALSE);
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
        case WM_DROPFILES:
        case WM_DRAWITEM:
        case WM_MEASUREITEM:
        case WM_COMMAND:
        case WM_NOTIFY:
        return (SendMessage(GetParent(hwnd), uMessage, wParam, lParam));
        return (FALSE);
        default:
        break;
    }
    return (DefWindowProc(hwnd, uMessage, wParam, lParam));
}

void Band_DrawCloseButton(HWND hwnd, LPBANDCTRL lpbc, BOOL bOver)
{
    HDC hdc = GetWindowDC(hwnd);

    if (bOver)
    {
        DrawEdge(hdc, &lpbc->rcClose, (lpbc->bOnClose ? BDR_SUNKENOUTER : BDR_RAISEDINNER), BF_RECT);
    }
    else
    {
        FrameRect(hdc, &lpbc->rcClose, (HBRUSH) GetClassLong(hwnd, GCL_HBRBACKGROUND));
    }

    ReleaseDC(hwnd, hdc);
}

void Band_RecalcSize(HWND hwnd, LPBANDCTRL lpbc)
{
    BCNM bcnm;
    RECT re;
    int cx, cy;

    ZeroMemory(&bcnm, sizeof(bcnm));

    bcnm.hdr.hwndFrom   = hwnd;
    bcnm.hdr.idFrom     = lpbc->wID;
    bcnm.hdr.code       = BCN_GETCLIENTRECT;

    if (SendMessage(lpbc->hwndOwner, WM_NOTIFY, (WPARAM) lpbc->wID, (LPARAM) &bcnm) == FALSE)
        return;

    GetWindowRect(hwnd, &re);

    cx = re.right - re.left;
    cy = re.bottom - re.top;

    if (lpbc->dwOldStyle & BCS_LEFT && lpbc->dwStyle & BCS_RIGHT ||
            lpbc->dwOldStyle & BCS_RIGHT && lpbc->dwStyle & BCS_LEFT)
    {
        cx = min(re.right - re.left, lpbc->ptMaxSize.x);
    }
    else if (lpbc->dwOldStyle & BCS_TOP && lpbc->dwStyle & BCS_BOTTOM ||
         lpbc->dwOldStyle & BCS_BOTTOM && lpbc->dwStyle & BCS_TOP)
    {
        cy = min(re.bottom - re.top, lpbc->ptMaxSize.y);
    }
    else if (lpbc->dwOldStyle & BCS_LEFT || lpbc->dwOldStyle & BCS_RIGHT &&
         lpbc->dwStyle & BCS_TOP || lpbc->dwStyle & BCS_BOTTOM)
    {
        cy = min(re.right - re.left, lpbc->ptMaxSize.y);
    }
    else if (lpbc->dwOldStyle & BCS_TOP || lpbc->dwOldStyle & BCS_BOTTOM &&
         lpbc->dwStyle & BCS_LEFT || lpbc->dwStyle & BCS_RIGHT)
    {
        cx = min(re.bottom - re.top, lpbc->ptMaxSize.x);
    }
    else if (lpbc->dwOldStyle & BCS_FLOAT)
    {
        if (lpbc->ptMaxSize.y > 0)
            cy = min(re.right - re.left, lpbc->ptMaxSize.y);
        else
            cy = re.right - re.left;

        if (lpbc->ptMaxSize.x > 0)
            cx = min(re.bottom - re.top, lpbc->ptMaxSize.x);
        else
            cy = re.bottom - re.top;
    }

    lpbc->bNotify = FALSE;

    if (lpbc->dwStyle & BCS_LEFT)
    {
        SetWindowPos(hwnd, NULL, bcnm.rcClient.left, bcnm.rcClient.top, cx, bcnm.rcClient.bottom - bcnm.rcClient.top, SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    else if (lpbc->dwStyle & BCS_RIGHT)
    {
        SetWindowPos(hwnd, NULL, bcnm.rcClient.right - cx, bcnm.rcClient.top, cx, bcnm.rcClient.bottom - bcnm.rcClient.top, SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    else if (lpbc->dwStyle & BCS_TOP)
    {
        SetWindowPos(hwnd, NULL, bcnm.rcClient.left, bcnm.rcClient.top, bcnm.rcClient.right - bcnm.rcClient.left, cy, SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    else if (lpbc->dwStyle & BCS_BOTTOM)
    {
        SetWindowPos(hwnd, NULL, bcnm.rcClient.left, bcnm.rcClient.bottom - cy, bcnm.rcClient.right - bcnm.rcClient.left, cy, SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_FRAMECHANGED);
    }

    lpbc->bNotify = TRUE;
}

HRESULT Band_ForwardNotify(HWND hwnd, WPARAM wParam, LPARAM lParam, LPBANDCTRL lpbc, long code)
{
    BCNM bcnm;

    ZeroMemory(&bcnm, sizeof(bcnm));

    bcnm.hdr.hwndFrom = hwnd;
    bcnm.hdr.idFrom = lpbc->wID;
    bcnm.hdr.code = code;

    CopyRect(&bcnm.rcClient, &lpbc->rcClient);
    bcnm.bVisible = (IsWindowVisible(hwnd));
    bcnm.hwndClient = lpbc->hwndClient;
    bcnm.lParam = lParam;
    bcnm.wParam = wParam;
    return (SendMessage(lpbc->hwndOwner, WM_NOTIFY, (WPARAM) lpbc->wID, (LPARAM) & bcnm));
}

HRESULT Band_NotifyParent(HWND hwnd, LPBANDCTRL lpbc, long code)
{
    BCNM bcnm;

    ZeroMemory(&bcnm, sizeof(bcnm));

    bcnm.hdr.hwndFrom = hwnd;
    bcnm.hdr.idFrom = lpbc->wID;
    bcnm.hdr.code = code;

    switch (code)
    {
        case BCN_STYLECHANGED:
            bcnm.dwNewStyle = lpbc->dwStyle;
            bcnm.dwOldStyle = lpbc->dwOldStyle;
        case BCN_POSCHANGED:
        {
            CopyRect(&bcnm.rcClient, &lpbc->rcClient);

            bcnm.bVisible = (IsWindowVisible(hwnd));
            bcnm.hwndClient = lpbc->hwndClient;
        }
        break;
        case BCN_BANDCLOSE:
        {
            bcnm.bVisible = (IsWindowVisible(hwnd));
        }
        break;
        default:
        return (0);
    }
    return (SendMessage(lpbc->hwndOwner, WM_NOTIFY, (WPARAM) lpbc->wID, (LPARAM) & bcnm));
}

DWORD Band_ModifyStyle(DWORD dwStyle, DWORD dwAdd)
{
    DWORD dwTmp = dwStyle;

    if (dwStyle & dwAdd)
        return (dwStyle);

    if (dwTmp & BCS_LEFT)
        dwTmp ^= BCS_LEFT;
    if (dwTmp & BCS_RIGHT)
        dwTmp ^= BCS_RIGHT;
    if (dwTmp & BCS_TOP)
        dwTmp ^= BCS_TOP;
    if (dwTmp & BCS_BOTTOM)
        dwTmp ^= BCS_BOTTOM;
    if (dwTmp & BCS_FLOAT)
        dwTmp ^= BCS_FLOAT;
    return (dwTmp | dwAdd);
}

BOOL Controls_CreateViewFromDialogTemplate(HWND hwndParent, UINT dlgID, HANDLE hInstance)
{
    LPCTSTR lpDlg;
    LPCTSTR lpDlgItem;
    DLG_TEMPLATE dlg;
    DLG_CONTROL_INFO dlgItem;
    HFONT hFont;
    HWND hWndChild;
    HWND hWndPrevChild = HWND_TOP;
    WORD bNumControls;
    DWORD dwStyle;
    HRSRC hrsrcDialog;
    HGLOBAL hGlblDlgTemplate;
    RECT rc, rcParent;
    TCHAR szFontName[100];
    int cx, cy, xBorder, yBorder;
    int xBase = 0, yBase = 0;

    if (!IsWindow(hwndParent))
        return (FALSE);

    hrsrcDialog = FindResource((HANDLE) hInstance, MAKEINTRESOURCE(dlgID), RT_DIALOG);

    if (hrsrcDialog == NULL)
        return (FALSE);

    hGlblDlgTemplate = LoadResource((HANDLE) hInstance, hrsrcDialog);
    lpDlg = (LPCTSTR) LockResource(hGlblDlgTemplate);

    if (lpDlg == NULL)
        return (FALSE);

    lpDlgItem = (LPCTSTR) Band_GetDialogInfo((LPCTSTR) lpDlg, &dlg);

    cx = dlg.cx;
    cy = dlg.cy;
    hFont = GetWindowFont(hwndParent);

    if (dlg.style & DS_SETFONT)
    {
        WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) dlg.faceName, -1, (LPTSTR) szFontName, sizeof(szFontName), NULL, NULL);
        hFont = CreateFont(-dlg.pointSize, 0, 0, 0,
            dlg.weight, dlg.italic, FALSE,
            FALSE, DEFAULT_CHARSET, 0, 0, PROOF_QUALITY,
            FF_DONTCARE, (LPCTSTR) szFontName);
    }

    if (!hFont)
        hFont = (HFONT) GetStockObject(ANSI_VAR_FONT);

    if (hFont)
    {
        SIZE sz;
        Band_GetDialogBaseUnits(szFontName, dlg.pointSize, &sz);

        xBase = sz.cx;
        yBase = sz.cy;
    }

    bNumControls = dlg.nbItems;

    SetRect(&rc, dlg.x, dlg.y, dlg.x + dlg.cx, dlg.y + dlg.cy);

    Band_MapDialogRect(&rc, xBase, yBase);

    GetClientRect(hwndParent, &rcParent);
    dwStyle = GetWindowLong(hwndParent, GWL_STYLE);

    xBorder = GetSystemMetrics(SM_CXFRAME) / 2;
    yBorder = GetSystemMetrics(SM_CYFRAME) / 2;

    if (dwStyle & WS_CAPTION)
        MoveWindow(hwndParent, 0, 0, rc.right - rc.left + (2 * xBorder), rc.bottom - rc.top + GetSystemMetrics(SM_CYCAPTION) + (2 * yBorder) + (GetMenu(hwndParent) == NULL ? 0 : GetSystemMetrics(SM_CYMENU)), TRUE);
    else
        MoveWindow(hwndParent, 0, 0, rc.right - rc.left + (2 * xBorder), rc.bottom - rc.top + (2 * yBorder), TRUE);

    SetWindowFont(hwndParent, hFont, FALSE);
    GetClientRect(hwndParent, &rcParent);

    while (bNumControls-- != 0)
    {
        int len;
        TCHAR szTitle[100];
        TCHAR szClass[100];
        BOOL bCode;

        lpDlgItem = (LPCTSTR) Band_GetControlInfo((WORD *) lpDlgItem, &dlgItem, dlg.dialogEx, &bCode);

        SetRect(&rc, dlgItem.x,
                dlgItem.y,
                dlgItem.x + dlgItem.cx,
                dlgItem.y + dlgItem.cy);

        Band_MapDialogRect(&rc, xBase, yBase);

        if (bCode == FALSE)
        {
            len = wcslen((LPCWSTR) dlgItem.windowName);
            len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) dlgItem.windowName, -1, (LPTSTR) szTitle, sizeof(szTitle), NULL, &bCode);
        }
        else
            szTitle[0] = _T('\0');

        len = wcslen((LPCWSTR) dlgItem.className);
        len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) dlgItem.className, -1, (LPTSTR) szClass, sizeof(szClass), NULL, &bCode);

        hWndChild = CreateWindowEx(dlgItem.exStyle | WS_EX_NOPARENTNOTIFY, (LPCTSTR) szClass, (LPCTSTR) szTitle,
                 dlgItem.style,
                 rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                 hwndParent, (HMENU)dlgItem.id, (HANDLE)hInstance,
                 dlgItem.data);

        if (hWndChild == NULL)
        {
            // The child couldn't be create
            UnlockResource(hGlblDlgTemplate);
            FreeResource(hGlblDlgTemplate);
            return (FALSE);
        }

        // Tell the new control to use the same font as dialog box
        SetWindowFont(hWndChild, hFont, FALSE);

        // Fix the Z-Order of the controls
        SetWindowPos(hWndChild, hWndPrevChild, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        hWndPrevChild = hWndChild;
    }
    UnlockResource(hGlblDlgTemplate);
    FreeResource(hGlblDlgTemplate);
    return (TRUE);
}

BOOL Band_SetDialogTemplate(HWND hwnd, LPBANDCTRL lpbc, WPARAM wParam, LPARAM lParam)
{
    LPCTSTR lpDlg;
    LPCTSTR lpDlgItem;
    DLG_TEMPLATE dlg;
    DLG_CONTROL_INFO dlgItem;
    HFONT hFont;
    HWND hWndChild;
    HWND hWndPrevChild = HWND_TOP;
    WORD bNumControls;
    HRSRC hrsrcDialog;
    HGLOBAL hGlblDlgTemplate;
    RECT rc;
    TCHAR szFontName[100];
    int cx, cy;
    int xBase = 0, yBase = 0;

    hrsrcDialog = FindResource((HANDLE) lParam, MAKEINTRESOURCE(wParam), RT_DIALOG);

    if (hrsrcDialog == NULL)
        return (FALSE);

    hGlblDlgTemplate = LoadResource((HANDLE) lParam, hrsrcDialog);
    lpDlg = (LPCTSTR) LockResource(hGlblDlgTemplate);

    if (lpDlg == NULL)
        return (FALSE);

    lpDlgItem = (LPCTSTR) Band_GetDialogInfo((LPCTSTR) lpDlg, &dlg);

    cx = dlg.cx;
    cy = dlg.cy;
    hFont = GetWindowFont(hwnd);

    if (dlg.style & DS_SETFONT)
    {
        WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) dlg.faceName, -1, (LPTSTR) szFontName, sizeof(szFontName), NULL, NULL);
        hFont = CreateFont(-dlg.pointSize, 0, 0, 0,
         dlg.weight, dlg.italic, FALSE,
         FALSE, DEFAULT_CHARSET, 0, 0, PROOF_QUALITY,
         FF_DONTCARE, (LPCTSTR) szFontName);
    }

    if (!hFont)
        hFont = (HFONT) GetStockObject(ANSI_VAR_FONT);

    if (hFont)
    {
        SIZE sz;
        Band_GetDialogBaseUnits(szFontName, dlg.pointSize, &sz);

        xBase = sz.cx;
        yBase = sz.cy;
    }

    bNumControls = dlg.nbItems;

    SetRect(&rc, dlg.x, dlg.y, dlg.x + dlg.cx, dlg.y + dlg.cy);

    Band_MapDialogRect(&rc, xBase, yBase);

    rc.top = lpbc->rcClient.top + 2;
    rc.left = lpbc->rcClient.left + 2;

    lpbc->hwndClient = CreateWindowEx(0, BAND_CHILD_CLASS, NULL, WS_CHILD | WS_VISIBLE,
                        rc.left, rc.top, rc.right, rc.bottom, hwnd,
                        NULL, (HANDLE) lParam, NULL);

    if (lpbc->hwndClient == NULL)
    {
        UnlockResource(hGlblDlgTemplate);
        FreeResource(hGlblDlgTemplate);
        return (FALSE);
    }

    SetWindowFont(lpbc->hwndClient, hFont, FALSE);

    while (bNumControls-- != 0)
    {
        int len;
        TCHAR szTitle[100];
        TCHAR szClass[100];
        BOOL bCode;

        lpDlgItem = (LPCTSTR) Band_GetControlInfo((WORD *) lpDlgItem, &dlgItem, dlg.dialogEx, &bCode);

        SetRect(&rc, dlgItem.x,
            dlgItem.y,
            dlgItem.x + dlgItem.cx,
            dlgItem.y + dlgItem.cy);

        Band_MapDialogRect(&rc, xBase, yBase);

        if (bCode == FALSE)
        {
            len = wcslen((LPCWSTR) dlgItem.windowName);
            len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) dlgItem.windowName, -1, (LPTSTR) szTitle, sizeof(szTitle), NULL, &bCode);
        }
        else
            szTitle[0] = _T('\0');

        len = wcslen((LPCWSTR) dlgItem.className);
        len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) dlgItem.className, -1, (LPTSTR) szClass, sizeof(szClass), NULL, &bCode);
        hWndChild = CreateWindowEx(dlgItem.exStyle | WS_EX_NOPARENTNOTIFY, (LPCTSTR) szClass, (LPCTSTR) szTitle,
                dlgItem.style,
                rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                lpbc->hwndClient, (HMENU)dlgItem.id, (HANDLE)lParam,
                dlgItem.data);

        if (hWndChild == NULL)
        {
            // The child couldn't be create
            DestroyWindow(lpbc->hwndClient);
            UnlockResource(hGlblDlgTemplate);
            FreeResource(hGlblDlgTemplate);
            return (FALSE);
        }

        // Tell the new control to use the same font as dialog box
        SetWindowFont(hWndChild, hFont, FALSE);

        // Fix the Z-Order of the controls
        SetWindowPos(hWndChild, hWndPrevChild, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        hWndPrevChild = hWndChild;
    }
    UnlockResource(hGlblDlgTemplate);
    FreeResource(hGlblDlgTemplate);
    return (TRUE);
}

const WORD *Band_GetControlInfo(const WORD * p, DLG_CONTROL_INFO * info, BOOL dialogEx, LPBOOL bIsID)
{
    if (dialogEx)
    {
        info->helpId = GET_DWORD(p);
        p += 2;
        info->exStyle = GET_DWORD(p);
        p += 2;
        info->style = GET_DWORD(p);
        p += 2;
    }
    else
    {
        info->helpId = 0;
        info->style = GET_DWORD(p);
        p += 2;
        info->exStyle = GET_DWORD(p);
        p += 2;
    }
    info->x = GET_WORD(p);
    p++;
    info->y = GET_WORD(p);
    p++;
    info->cx = GET_WORD(p);
    p++;
    info->cy = GET_WORD(p);
    p++;

    if (dialogEx)
    {
        /* id is a DWORD for DIALOGEX */
        info->id = (WORD) GET_DWORD(p);
        p += 2;
    }
    else
    {
        info->id = GET_WORD(p);
        p++;
    }

    if (GET_WORD(p) == 0xffff)
    {
        WORD id = GET_WORD(p + 1);
        if ((id >= 0x80) && (id <= 0x85))
            info->className = (LPCTSTR) (LPCWSTR) class_names[id - 0x80];
        else
            info->className = NULL;
        p += 2;
    }
    else
    {
        info->className = (LPCTSTR) p;
        p += wcslen((LPCWSTR) p) + 1;
    }

    if (GET_WORD(p) == 0xffff)  /* Is it an integer id? */
    {
        *bIsID = TRUE;
        info->windowName = (LPCTSTR) (DWORD) GET_WORD(p + 1);
        p += 2;
    }
    else
    {
        *bIsID = FALSE;
        info->windowName = (LPCTSTR) p;
        p += wcslen((LPCWSTR) p) + 1;
    }

    if (GET_WORD(p))
    {
        info->data = (LPVOID) (p + 1);
        p += GET_WORD(p) / sizeof(WORD);
    }
    else
    {
        info->data = NULL;
    }
    p++;
    /* Next control is on dword boundary */
    return (const WORD *)((((int)p) + 3) & ~3);
}

const TCHAR *Band_GetDialogInfo(LPCTSTR pTemplate, DLG_TEMPLATE * result)
{
    const WORD *p = (const WORD *)pTemplate;

    result->style = GET_DWORD(p);
    p += 2;

    if (result->style == 0xffff0001)    /* DIALOGEX resource */
    {
        result->dialogEx = TRUE;
        result->helpId = GET_DWORD(p);
        p += 2;
        result->exStyle = GET_DWORD(p);
        p += 2;
        result->style = GET_DWORD(p);
        p += 2;
    }
    else
    {
        result->dialogEx = FALSE;
        result->helpId = 0;
        result->exStyle = GET_DWORD(p);
        p += 2;
    }
    result->nbItems = GET_WORD(p);
    p++;
    result->x = GET_WORD(p);
    p++;
    result->y = GET_WORD(p);
    p++;
    result->cx = GET_WORD(p);
    p++;
    result->cy = GET_WORD(p);
    p++;

    /* Get the menu name */

    switch (GET_WORD(p))
    {
        case 0x0000:
            result->menuName = NULL;
            p++;
        break;
        case 0xffff:
            result->menuName = (LPCTSTR)(WORD)GET_WORD(p + 1);
            p += 2;
        break;
        default:
            result->menuName = (LPCTSTR) p;
            p += wcslen((LPCWSTR) p) + 1;
        break;
    }

    /* Get the class name */

    switch (GET_WORD(p))
    {
        case 0x0000:
            result->className = MAKEINTATOM(32770);
            p++;
        break;
        case 0xffff:
            result->className = (LPCTSTR)(WORD)GET_WORD(p + 1);
            p += 2;
        break;
        default:
            result->className = (LPCTSTR) p;
            p += wcslen((LPCWSTR) p) + 1;
        break;
    }

    /* Get the window caption */

    result->caption = (LPCTSTR) p;
    p += wcslen((LPCWSTR) p) + 1;

    /* Get the font name */

    if (result->style & DS_SETFONT)
    {
        result->pointSize = GET_WORD(p);
        p++;
        if (result->dialogEx)
        {
            result->weight = GET_WORD(p);
            p++;
            result->italic = LOBYTE(GET_WORD(p));
            p++;
        }
        else
        {
            result->weight = FW_DONTCARE;
            result->italic = FALSE;
        }
        result->faceName = (LPCTSTR) p;
        p += wcslen((LPCWSTR) p) + 1;
    }
    /* First control is on dword boundary */
    return (LPCTSTR) ((((int)p) + 3) & ~3);
}

void Band_MapDialogRect(LPRECT lprc, int cx, int cy)
{
    lprc->left = lprc->left * cx / 4;
    lprc->right = lprc->right * cx / 4;
    lprc->top = lprc->top * cy / 8;
    lprc->bottom = lprc->bottom * cy / 8;
}

void Band_GetDialogBaseUnits(LPCTSTR pszFontFace, WORD wFontSize, SIZE * pSizePixel)
{
    // Attempt to create the font to be used in the dialog box
    UINT cxSysChar, cySysChar;
    HFONT hNewFont;
    LOGFONT lf;
    HDC hDC = GetDC(NULL);

    memset(&lf, 0, sizeof(LOGFONT));

    lf.lfHeight = -MulDiv(wFontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    _tcscpy(lf.lfFaceName, pszFontFace);

    hNewFont = CreateFontIndirect(&lf);

    if (hNewFont != NULL)
    {
        HFONT hFontOld = (HFONT) SelectObject(hDC, hNewFont);
        TEXTMETRIC tm;
        SIZE size;

        GetTextMetrics(hDC, &tm);
        cySysChar = tm.tmHeight + tm.tmExternalLeading;
        GetTextExtentPoint(hDC, TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"), 52, &size);
        cxSysChar = (size.cx + 26) / 52;
        SelectObject(hDC, hFontOld);
        DeleteObject(hNewFont);
    }
    else
    {
        // Could not create the font so just use the system's values
        cxSysChar = LOWORD(GetDialogBaseUnits());
        cySysChar = HIWORD(GetDialogBaseUnits());
    }
    
    ReleaseDC(NULL, hDC);
    
    pSizePixel->cx = cxSysChar;
    pSizePixel->cy = cySysChar;
}
