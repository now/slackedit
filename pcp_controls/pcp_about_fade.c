/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_about_fade.c
 * Created    : 01/09/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:45:14
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* standard */
#include <process.h>

/* pcp_controls */
#include "pcp_about_fade.h"

/* pcp_generic */
#include <pcp_linkedlist.h>
#include <pcp_mem.h>
#include <pcp_rect.h>
#include <pcp_string.h>

static PVOID PageList_CreateData(PVOID pData);
static INT PageList_DeleteData(PVOID pData);

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

typedef struct tagFADECTRL
{
    RECT            rc;
    DWORD           dwCounter;
    PLINKEDLIST     pPageList;
    PFADECTRLPAGE   pfcCurrentPage;
    BOOL            bActive;
    BOOL            bFadingIn;
    HANDLE          hTread;
    DWORD           dwThreadID;
    HWND            hwnd;
} FADECTRL, *LPFADECTRL;

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

static LRESULT CALLBACK FadeCtrl_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static DWORD WINAPI FadeCtrl_ThreadProc(LPVOID lpParameter);

static void FadeCtrl_CalcRect(HDC hdc, LPRECT lprcTo, LPRECT lprcIn, LPCTSTR pszString);
static COLORREF FadeCtrl_GetColor(COLORREF crTextTo, COLORREF crTextFrom, int nPercent);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

BOOL FadeCtrl_RegisterControl(HMODULE hModule)
{
    WNDCLASSEX wcex;
    WNDCLASS wc;

    ZeroMemory(&wc, sizeof(WNDCLASS));
    wcex.cbSize = sizeof(WNDCLASSEX);

    if (GetClassInfoEx(hModule, ABOUT_FADE_CLASS, &wcex) != 0)
        return (TRUE);

    wc.style            = CS_SAVEBITS | CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_GLOBALCLASS;
    wc.lpfnWndProc      = (WNDPROC)FadeCtrl_WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hModule;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = ABOUT_FADE_CLASS;

    if (!RegisterClass(&wc))
        return (FALSE);

    return (TRUE);
}

void FadeCtrl_UnregisterControl(HMODULE hModule)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    if (GetClassInfoEx(hModule, ABOUT_FADE_CLASS, &wcex) == FALSE)
        return;

    UnregisterClass(ABOUT_FADE_CLASS, hModule);
}

static LRESULT CALLBACK FadeCtrl_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPFADECTRL lpfc = (LPFADECTRL)GetWindowLong(hwnd, GWL_USERDATA);

    switch (uMsg)
    {
        case WM_NCCREATE:
            lpfc = (LPFADECTRL)Mem_Alloc(sizeof(FADECTRL));

            if (lpfc == NULL)
                return (FALSE);

            SetWindowLong(hwnd, GWL_USERDATA, (LONG)lpfc);
        return (TRUE);
        case WM_CREATE:
            lpfc->bFadingIn = TRUE;
            GetClientRect(hwnd, &lpfc->rc);
            lpfc->hwnd      = hwnd;
            lpfc->pPageList = List_CreateList(PageList_CreateData, PageList_DeleteData, NULL, NULL, 0);
        return (TRUE);
        case WM_DESTROY:
            FadeCtrl_Deactivate(hwnd);
            List_DestroyList(lpfc->pPageList);
        break;
        case FC_ADDPAGE:
            List_AddNodeAtTail(lpfc->pPageList, (PVOID)lParam);
        return (TRUE);
        case FC_REMOVEPAGE:
            List_DeleteNode(lpfc->pPageList, List_GetNode(lpfc->pPageList, (INT)wParam));
        return (TRUE);
        case FC_GETCURRENTPAGE:
        return ((LRESULT)lpfc->pfcCurrentPage);
        case FC_ACTIVATE:
            if ((lpfc->hTread = BEGINTHREADEX(0, 0, FadeCtrl_ThreadProc, lpfc, 0, &lpfc->dwThreadID)) == NULL)
                return (FALSE);

            // We don't need the handle so close it
            CloseHandle(lpfc->hTread);

            lpfc->bActive = TRUE;
        return (TRUE);
        case FC_DEACTIVATE:
            if (!PostThreadMessage(lpfc->dwThreadID, WM_QUIT, 0, 0L))
                return (FALSE); // Still active

            lpfc->bActive = FALSE;
        return (TRUE);
        case FC_TOGGLEACTIVE:
            if (lpfc->bActive)
                return (FadeCtrl_Deactivate(hwnd));
            else
                return (FadeCtrl_Activate(hwnd));
        break;
    }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}

static DWORD WINAPI FadeCtrl_ThreadProc(LPVOID lpParameter)
{
    MSG msg;
    LPFADECTRL lpfc = (LPFADECTRL)lpParameter;

    if (List_GetNodeCount(lpfc->pPageList) == 0)
        return (0);

    while (TRUE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else if ((GetTickCount() - lpfc->dwCounter) > 75)
        {
            HDC hdcWin;
            HDC hdcMem;
            HBITMAP hBitmap;
            HBITMAP hOldBitmap;
            LOGFONT lf;
            HFONT hFont, hOldFont;
            LOGBRUSH lb;
            HBRUSH hBrush;

            hdcWin = GetDC(lpfc->hwnd);
            hdcMem = CreateCompatibleDC(hdcWin);

            hBitmap = CreateCompatibleBitmap(hdcWin, Rect_Width(&lpfc->rc), Rect_Height(&lpfc->rc));

            INITSTRUCT(lf, FALSE);
            lf.lfHeight         = -11;
            lf.lfCharSet        = OEM_CHARSET;
            lf.lfPitchAndFamily = FIXED_PITCH;
            _tcscpy(lf.lfFaceName, _T("Verdana Console"));

            hFont       = CreateFontIndirect(&lf);
            hOldFont    = SelectObject(hdcMem, hFont);

            if (lpfc->pfcCurrentPage == NULL)
            {
                lpfc->pfcCurrentPage    = ((PFADECTRLPAGE)List_GetNode(lpfc->pPageList, 0)->pData);

                if (lpfc->pfcCurrentPage == NULL)
                    break; // Not good...some fuck0 has passed us a shit text resource

                FadeCtrl_CalcRect(hdcMem, &lpfc->pfcCurrentPage->rcText, &lpfc->rc, lpfc->pfcCurrentPage->pszPageText);
            }

            if (lpfc->pfcCurrentPage->nCurrentWaitTime == 0)
            {
                if (lpfc->bFadingIn)
                    lpfc->pfcCurrentPage->crCurrent = FadeCtrl_GetColor(lpfc->pfcCurrentPage->crFadeTo, lpfc->pfcCurrentPage->crBackground, lpfc->pfcCurrentPage->nCurrentPercentage);
                else
                    lpfc->pfcCurrentPage->crCurrent = FadeCtrl_GetColor(lpfc->pfcCurrentPage->crBackground, lpfc->pfcCurrentPage->crFadeTo, lpfc->pfcCurrentPage->nCurrentPercentage);

                if (lpfc->bFadingIn && lpfc->pfcCurrentPage->nCurrentPercentage == lpfc->pfcCurrentPage->nFadePercent)
                {
                    lpfc->bFadingIn = FALSE;
                    lpfc->pfcCurrentPage->nCurrentPercentage = 0;
                    lpfc->pfcCurrentPage->nCurrentWaitTime = lpfc->pfcCurrentPage->nFadeOutWaitTime;
                }
                else if (!lpfc->bFadingIn && lpfc->pfcCurrentPage->nCurrentPercentage == lpfc->pfcCurrentPage->nFadePercent)
                {
                    PFADECTRLPAGE pfcPage;

                    lpfc->bFadingIn = TRUE;

                    pfcPage = ((PFADECTRLPAGE)List_NextNode(List_FindNode(lpfc->pPageList, lpfc->pfcCurrentPage, 0), 1)->pData);

                    if (pfcPage == lpfc->pfcCurrentPage)
                        lpfc->pfcCurrentPage = ((PFADECTRLPAGE)List_GetNode(lpfc->pPageList, 0)->pData);
                    else
                        lpfc->pfcCurrentPage = pfcPage;

                    if (lpfc->pfcCurrentPage == NULL)
                        break;

                    lpfc->pfcCurrentPage->nCurrentPercentage = 0;
                    lpfc->pfcCurrentPage->nCurrentWaitTime = lpfc->pfcCurrentPage->nFadeInWaitTime;

                    FadeCtrl_CalcRect(hdcMem, &lpfc->pfcCurrentPage->rcText, &lpfc->rc, lpfc->pfcCurrentPage->pszPageText);
                }
                else
                {
                    lpfc->pfcCurrentPage->nCurrentPercentage += lpfc->pfcCurrentPage->nPercentageDelta;
                }
            }
            else
            {
                // Setable decrement ? why ? 
                lpfc->pfcCurrentPage->nCurrentWaitTime -= 50;
            }

            SetBkColor(hdcMem, lpfc->pfcCurrentPage->crBackground);
            SetTextColor(hdcMem, lpfc->pfcCurrentPage->crCurrent);

            hOldBitmap = SelectObject(hdcMem, hBitmap);

            lb.lbColor  = lpfc->pfcCurrentPage->crBackground;
            lb.lbStyle  = BS_SOLID;

            FillRect(hdcMem, &lpfc->rc, (hBrush = CreateBrushIndirect(&lb)));
            DrawText(hdcMem, lpfc->pfcCurrentPage->pszPageText, -1, &lpfc->pfcCurrentPage->rcText, DT_CENTER);

            BitBlt(hdcWin, 0, 0, Rect_Width(&lpfc->rc), Rect_Height(&lpfc->rc), hdcMem, 0, 0, SRCCOPY);

            DeleteObject(SelectObject(hdcMem, hOldFont));
            DeleteObject(SelectObject(hdcMem, hOldBitmap));
            DeleteObject(hBrush);
            DeleteDC(hdcMem);
            ReleaseDC(lpfc->hwnd, hdcWin);

            lpfc->dwCounter = GetTickCount();
        }

        if (lpfc->bActive == FALSE)
            break;
    }

    return (0);
}

static void FadeCtrl_CalcRect(HDC hdc, LPRECT lprcTo, LPRECT lprcIn, LPCTSTR pszString)
{
    int nLines = String_CountSubChars(pszString, _T('\n')) + 1;
    TEXTMETRIC tm;

    GetTextMetrics(hdc, &tm);

    lprcTo->top     = (Rect_Height(&*lprcIn) / 2) - ((tm.tmHeight * nLines) / 2);
    lprcTo->bottom  = (Rect_Height(&*lprcIn) / 2) + ((tm.tmHeight * nLines) / 2);
    lprcTo->left    = lprcIn->left;
    lprcTo->right   = lprcIn->right;
}

static COLORREF FadeCtrl_GetColor(COLORREF crTextTo, COLORREF crTextFrom, int nPercent)
{
    if (nPercent == 100)
    {
        return (crTextTo);
    }
    else
    {
        short sFromR    = GetRValue(crTextFrom);
        short sFromG    = GetGValue(crTextFrom);
        short sFromB    = GetBValue(crTextFrom);

        short sToR      = GetRValue(crTextTo);
        short sToG      = GetGValue(crTextTo);
        short sToB      = GetBValue(crTextTo);

        short sDiffR    = (short)(sFromR - sToR);
        short sDiffG    = (short)(sFromG - sToG);
        short sDiffB    = (short)(sFromB - sToB);

        short sNewR     = (short)(sFromR - (short)(((float)sDiffR / (float)100) * nPercent));
        short sNewG     = (short)(sFromG - (short)(((float)sDiffG / (float)100) * nPercent));
        short sNewB     = (short)(sFromB - (short)(((float)sDiffB / (float)100) * nPercent));

        return (RGB(sNewR, sNewG, sNewB));
    }
}

static PVOID PageList_CreateData(PVOID pData)
{
    PFADECTRLPAGE pCall = (PFADECTRLPAGE)pData;
    PFADECTRLPAGE pCreate = (PFADECTRLPAGE)Mem_Alloc(sizeof(FADECTRLPAGE));

    pCreate->crBackground       = pCall->crBackground;
    pCreate->crFadeTo           = pCall->crFadeTo;
    pCreate->nFadeInWaitTime    = (pCall->nFadeInWaitTime == -1) ? FC_DEFAULT_FADEIN_WAITTIME : pCall->nFadeInWaitTime;
    pCreate->nFadeOutWaitTime   = (pCall->nFadeOutWaitTime == -1) ? FC_DEFAULT_FADEOUT_WAITTIME : pCall->nFadeOutWaitTime;
    pCreate->nFadePercent       = (pCall->nFadePercent == -1) ? FC_DEFAULT_PERCENTAGE : pCall->nFadePercent;
    pCreate->nPercentageDelta   = (pCall->nPercentageDelta == -1) ? FC_DEFAULT_PERCENTAGE_INCREMENT : pCall->nPercentageDelta;
    pCreate->pszPageText        = String_Duplicate(pCall->pszPageText);

    return (pCreate);
}

static INT PageList_DeleteData(PVOID pData)
{
    Mem_Free(((PFADECTRLPAGE)pData)->pszPageText);
    Mem_Free(pData);

    return (TRUE);
}
