/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : pcp_hexedit.c
 * Created    : 01/31/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:34:37
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "pcp_hexedit.h"

/* pcp_generic */
#include <pcp_rect.h>
#include <pcp_mem.h>
#include <pcp_menu.h>

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

TCHAR hextable[16] = { _T('0'), _T('1'), _T('2'), _T('3'), _T('4'), _T('5'), _T('6'), _T('7'),
                     _T('8'), _T('9'), _T('A'), _T('B'), _T('C'), _T('D'), _T('E'), _T('F')};

#define TOHEX(a, b) { *(b)++ = hextable[(a) >> 4]; *(b)++ = hextable[(a) & 0xf]; }

#define HE_NOSELECTION      (-2)

#define HEF_HALFPAGE        0x00000001

#define UPDATE_HORZRANGE    0x00000001
#define UPDATE_VERTRANGE    0x00000002
#define UPDATE_CHARINFO     0x00000004

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

LRESULT CALLBACK HexEdit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void HexEdit_SetCharDimensions(LPHEXEDIT lphe);
void HexEdit_Move(LPHEXEDIT lphe, int x, int y);
__inline BOOL HexEdit_IsSelection(LPHEXEDIT lphe);
int HexEdit_GetLineHeight(LPHEXEDIT lphe);
int HexEdit_GetCharWidth(LPHEXEDIT lphe);
int HexEdit_SetBytesPerLine(LPHEXEDIT lphe, int nBytesPerLine);
POINT HexEdit_CalcPos(LPHEXEDIT lphe, int x, int y);
void HexEdit_CreateAddressCaret(LPHEXEDIT lphe);
void HexEdit_CreateEditCaret(LPHEXEDIT lphe);
void HexEdit_SetSelection(LPHEXEDIT lphe, int nStart, int nEnd);
void HexEdit_RepositionCaret(LPHEXEDIT lphe, int nPoint);
void HexEdit_OnEditDelete(LPHEXEDIT lphe);
void HexEdit_OnEditCopy(LPHEXEDIT lphe);
void HexEdit_OnEditCut(LPHEXEDIT lphe);
void HexEdit_OnEditPaste(LPHEXEDIT lphe);
void HexEdit_OnEditSelectAll(LPHEXEDIT lphe);
void HexEdit_OnEditUndo(LPHEXEDIT lphe);
void HexEdit_NormalizeSelection(LPHEXEDIT lphe);
void HexEdit_DeleteRange(LPHEXEDIT lphe, int nStart, int nEnd);
void HexEdit_InsertRange(LPHEXEDIT lphe, int nStart, int nEnd);
POINT HexEdit_GetSelection(LPHEXEDIT lphe);
void HexEdit_SetData(LPHEXEDIT lphe, LPBYTE pb, int nLen);
void HexEdit_SetOriginalData(LPHEXEDIT lphe, LPBYTE pb, int nLen);
void HexEdit_ResetView(LPHEXEDIT lphe);
int HexEdit_GetData(LPHEXEDIT lphe, LPBYTE pb, int nLen);
int HexEdit_GetOriginalData(LPHEXEDIT lphe, LPBYTE *ppb);
int HexEdit_GetCurrentAddress(LPHEXEDIT lphe);
BOOL HexEdit_SetCurrentAddress(LPHEXEDIT lphe, int nNewAddress, BOOL bRelative);
int HexEdit_GetScreenChars(LPHEXEDIT lphe);
int HexEdit_GetLineLength(LPHEXEDIT lphe);
void HexEdit_ScrollToChar(LPHEXEDIT lphe, int nNewOffsetChar, BOOL bTrackScrollBar);
void HexEdit_RecalcHorzScrollBar(LPHEXEDIT lphe, BOOL bPositionOnly);
void HexEdit_RecalcVertScrollBar(LPHEXEDIT lphe, BOOL bPositionOnly);
int HexEdit_GetScreenLines(LPHEXEDIT lphe);
int HexEdit_GetLineCount(LPHEXEDIT lphe);
void HexEdit_UpdateView(LPHEXEDIT lphe, DWORD dwFlags);
void HexEdit_ScrollToLine(LPHEXEDIT lphe, int nNewTopLine, BOOL bTrackScrollBar);
int HexEdit_GetLineFromAddress(LPHEXEDIT lphe, int nAddress);
int HexEdit_GetLinePosFromAddress(LPHEXEDIT lphe, int nAddress);
int HexEdit_CalculateActualOffset(LPHEXEDIT lphe, int nAddress);
void HexEdit_EnsureVisible(LPHEXEDIT lphe, int nAddress);

void HexEdit_OnHScroll(LPHEXEDIT lphe, UINT nSBCode, UINT nPos, HWND hwndScrollbar);
void HexEdit_OnVScroll(LPHEXEDIT lphe, UINT nSBCode, UINT nPos, HWND hwndScrollbar);
void HexEdit_OnPaint(LPHEXEDIT lphe);
void HexEdit_OnSetFocus(LPHEXEDIT lphe);
void HexEdit_OnMouseMove(LPHEXEDIT lphe, UINT nFlags, POINT pt);
void HexEdit_OnLButtonDown(LPHEXEDIT lphe, UINT nFlags, POINT pt);
void HexEdit_OnChar(LPHEXEDIT lphe, TCHAR chChar, UINT nRepCnt, UINT nFlags);
void HexEdit_OnKeyDown(LPHEXEDIT lphe, TCHAR chChar, UINT nRepCnt, UINT nFlags);
void HexEdit_OnContextMenu(LPHEXEDIT lphe, POINT pt);
void HexEdit_OnSize(LPHEXEDIT lphe, UINT nType, int cx, int cy);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

BOOL HexEdit_RegisterControl(HMODULE hModule)
{
    WNDCLASSEX wcex;
    WNDCLASS wc;

    ZeroMemory(&wc, sizeof(WNDCLASS));
    wcex.cbSize = sizeof(WNDCLASSEX);

    if (GetClassInfoEx(hModule, WC_HEXEDIT, &wcex) != 0)
        return (TRUE);

    wc.style            = CS_SAVEBITS | CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_GLOBALCLASS;
    wc.lpfnWndProc      = (WNDPROC)HexEdit_WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hModule;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_IBEAM);
    wc.hbrBackground    = (HBRUSH)GetSysColorBrush(COLOR_WINDOW);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = WC_HEXEDIT;

    if (!RegisterClass(&wc))
        return (FALSE);

    return (TRUE);
}

void HexEdit_UnregisterControl(HMODULE hModule)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    if (GetClassInfoEx(hModule, WC_HEXEDIT, &wcex) == FALSE)
        return;

    UnregisterClass(WC_HEXEDIT, hModule);
}

LRESULT CALLBACK HexEdit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPHEXEDIT lphe = (LPHEXEDIT)GetWindowLong(hwnd, GWL_USERDATA);

    switch (uMsg)
    {
    case WM_NCCREATE:
        lphe = (LPHEXEDIT)Mem_Alloc(sizeof(HEXEDIT));
        ZeroMemory(lphe, sizeof(lphe));

        if (lphe == NULL)
            return (FALSE);

        SetWindowLong(hwnd, GWL_USERDATA, (LONG)lphe);
    return (TRUE);
    case WM_CREATE:
        lphe->hwnd              = hwnd;
        lphe->pData             = NULL;
        lphe->nLength           = 0;
        lphe->nTopLine          = 0;
        lphe->nLineHeight       = -1;
        lphe->nCharWidth        = -1;
        lphe->nScreenChars      = -1;
        lphe->nScreenLines      = -1;
        lphe->nBytesPerLine     = 16;
        lphe->nCurrentAddress   = 0;
        lphe->bShowAddress      = TRUE;
        lphe->bShowAscii        = TRUE;
        lphe->bShowHex          = TRUE;
        lphe->bWideAddress      = TRUE;
        lphe->dwFlags           = 0; // HE_HALF_PAGE
        lphe->nOffsetAddress    = 0;
        lphe->nOffsetAscii      = 0;
        lphe->nOffsetHex        = 0;
        lphe->emCurrentMode     = EDIT_NONE;
        lphe->ptEditPos.x       = 0;
        lphe->ptEditPos.y       = 0;
        lphe->nSelStart         = HE_NOSELECTION;
        lphe->nSelEnd           = HE_NOSELECTION;
        lphe->hFont             = CreateFont(-12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, _T("Courier New"));

        HexEdit_SetCharDimensions(lphe);

        HexEdit_InsertRange(lphe, lphe->nLength, 255);
    return (TRUE);
    case WM_PAINT:
        HexEdit_OnPaint(lphe);
    return (0);
    case WM_SETFOCUS:
        HexEdit_OnSetFocus(lphe);
    break;
    case WM_KILLFOCUS:
        DestroyCaret();
    break;
    case WM_HSCROLL:
        HexEdit_OnHScroll(lphe, (UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HWND)lParam);
    return (0);
    case WM_VSCROLL:
        HexEdit_OnVScroll(lphe, (UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HWND)lParam);
    return (0);
    case WM_ERASEBKGND:
    return (TRUE);
    case WM_LBUTTONDOWN:
    {
        POINT pt;

        POINTSTOPOINT(pt, lParam);
        HexEdit_OnLButtonDown(lphe, (UINT)wParam, pt);
    }
    return (0);
    case WM_LBUTTONUP:
        if (HexEdit_IsSelection(lphe))
            ReleaseCapture();
    break;
    case WM_MOUSEMOVE:
    {
        POINT pt;

        POINTSTOPOINT(pt, lParam);
        HexEdit_OnMouseMove(lphe, (UINT)wParam, pt);
    }
    return (0);
    case WM_COPY:
        HexEdit_OnEditCopy(lphe);
    return (0);
    case WM_CUT:
        HexEdit_OnEditCut(lphe);
    return (0);
    case WM_PASTE:
        HexEdit_OnEditPaste(lphe);
    return (0);
    case WM_CHAR:
        HexEdit_OnChar(lphe, (TCHAR)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
    return (0);
    case WM_KEYDOWN:
        HexEdit_OnKeyDown(lphe, (TCHAR)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
    return (0);
    case WM_CONTEXTMENU:
    {
        POINT pt;

        POINTSTOPOINT(pt, lParam);
        HexEdit_OnContextMenu(lphe, pt);
    }
    return (0);
    case WM_SIZE:
        HexEdit_OnSize(lphe, (UINT)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
    return (0);
    }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}

void HexEdit_OnPaint(LPHEXEDIT lphe)
{
    PAINTSTRUCT ps;
    HDC hdc;
    HDC hdcMem;
    HBITMAP hMemBitmap;
    HBITMAP hOldMemBitmap;
    HBRUSH hWindowBrush;
    HFONT hOldMemFont;
    RECT rc;
    int nHeight = 0;
    int x = 0;
    int y = 0;
    TCHAR szBuffer[256];
    COLORREF crOldText;
    COLORREF crText;
    COLORREF crOldBackground;
    COLORREF crBackground;
    int nOldBkMode;

    GetClientRect(lphe->hwnd, &rc);

    hdc             = BeginPaint(lphe->hwnd, &ps);
    hdcMem          = CreateCompatibleDC(hdc);
    hMemBitmap      = CreateCompatibleBitmap(hdc, Rect_Width(&rc), Rect_Height(&rc));
    hOldMemBitmap   = SelectObject(hdcMem, hMemBitmap);
    hOldMemFont     = SelectObject(hdcMem, lphe->hFont);
    hWindowBrush    = GetSysColorBrush(COLOR_WINDOW);
    crText          = GetSysColor(COLOR_WINDOWTEXT);
    crBackground    = GetSysColor(COLOR_WINDOW);
    nOldBkMode      = SetBkMode(hdcMem, OPAQUE);

    FillRect(hdcMem, &rc, hWindowBrush);
    crOldText       = SetTextColor(hdcMem, crText);
    crOldBackground = SetBkColor(hdcMem, crBackground);

    ASSERT(HexEdit_GetCurrentAddress(lphe) >= 0);
    ASSERT(lphe->nTopLine >= 0);

    // FIXME: is this neccessary?
    SetBoundsRect(hdcMem, &rc, DCB_DISABLE);

    // Do we have anything to draw?
    if (lphe->pData != NULL)
    {
        int nLineHeight = HexEdit_GetLineHeight(lphe);
        int nCharWidth  = HexEdit_GetCharWidth(lphe);
        int nLineCount  = HexEdit_GetLineCount(lphe);
        TRACE(_T("%i, %i, %i, %i\n"), lphe->nTopLine, lphe->nSelStart, lphe->nLength, lphe->nCurrentAddress);

        nHeight = Rect_Height(&rc);

        if (lphe->bShowAddress)
        {
            TCHAR szAddressFormat[8] = _T("%08lX");
            int i;
            RECT rcDraw = rc;

            y = 0;

            if (!lphe->bWideAddress)
                szAddressFormat[2] = _T('4');

            rcDraw.left = lphe->nOffsetAddress - lphe->nOffsetChar * HexEdit_GetCharWidth(lphe);

            // FIXME: make this faster
            for (i = lphe->nTopLine; (i < HexEdit_GetLineCount(lphe)) && (rcDraw.top < nHeight); i++)
            {
                _stprintf(szBuffer, szAddressFormat, (i * lphe->nBytesPerLine));
                DrawText(hdcMem, szBuffer, ((lphe->bWideAddress) ? 8 : 4), &rcDraw, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);

                rcDraw.top += nLineHeight;
            }
        }

        if (lphe->bShowHex)
        {
            RECT rcDraw = rc;

            y = 0;
            rcDraw.left = x = lphe->nOffsetHex - lphe->nOffsetChar * HexEdit_GetCharWidth(lphe);

            if (lphe->nSelStart != HE_NOSELECTION && (lphe->emCurrentMode == EDIT_HIGH || lphe->emCurrentMode == EDIT_LOW))
            {
                int i;
                int n = 0;
                int nSelStart = lphe->nSelStart;
                int nSelEnd = lphe->nSelEnd;

                // Could be made faster, but that would waste rows...and the swap
                // (which is not necessary for nSelStart == nSelEnd) is so fast
                // it doesn't really matter (probably slower to add more checks
                // around the swap)
                if (nSelStart >= nSelEnd)
                {
                    nSelStart ^= nSelEnd ^= nSelStart ^= nSelEnd;
                    nSelStart   -= 1;
                    nSelEnd     += 1;
                }

                for (i = (lphe->nTopLine * lphe->nBytesPerLine); (i < nSelStart) && (y < nHeight); i++)
                {
                    LPTSTR psz = szBuffer;

                    TOHEX(lphe->pData[i], psz);
                    *(psz++) = _T(' ');

                    TextOut(hdcMem, x, y, szBuffer, 3);

                    x += nCharWidth * 3;
                    n++;

                    if (n == lphe->nBytesPerLine)
                    {
                        n = 0;
                        x = rcDraw.left;
                        y += nLineHeight;
                    }
                    
                    // like...impossible but...
                    ASSERT(n <= lphe->nBytesPerLine);
                }

                SetTextColor(hdcMem, GetSysColor(COLOR_HIGHLIGHTTEXT));
                SetBkColor(hdcMem, GetSysColor(COLOR_HIGHLIGHT));

                for ( ; (i < nSelEnd) && (i < lphe->nLength) && (y < nHeight); i++)
                {
                    LPTSTR psz = szBuffer;

                    TOHEX(lphe->pData[i], psz);
                    *(psz++) = _T(' ');

                    TextOut(hdcMem, x, y, szBuffer, 3);

                    x += nCharWidth * 3;
                    n++;

                    if (n == lphe->nBytesPerLine)
                    {
                        n = 0;
                        x = rcDraw.left;
                        y += nLineHeight;
                    }
                    
                    // again, almost impossible but anyhow...
                    ASSERT(n <= lphe->nBytesPerLine);
                }

                SetTextColor(hdcMem, crText);
                SetBkColor(hdcMem, crBackground);

                for ( ; (i < lphe->nLength) && (y < nHeight); i++)
                {
                    LPTSTR psz = szBuffer;

                    TOHEX(lphe->pData[i], psz);
                    *(psz++) = _T(' ');

                    TextOut(hdcMem, x, y, szBuffer, 3);

                    x += nCharWidth * 3;
                    n++;

                    if (n == lphe->nBytesPerLine)
                    {
                        n = 0;
                        x = rcDraw.left;
                        y += nLineHeight;
                    }

                    // and again, almost impossible but anyhow...
                    ASSERT(n <= lphe->nBytesPerLine);
                }
            }
            else
            {
                int i;

                for (i = (lphe->nTopLine * lphe->nBytesPerLine); (i < lphe->nLength) && (rcDraw.top < nHeight); )
                {
                    int n;
                    LPTSTR psz = szBuffer;

                    // n and i are incremented in the for ()
                    for (n = 0; (n < lphe->nBytesPerLine) && (i < lphe->nLength); n++, i++)
                    {
                        TOHEX(lphe->pData[i], psz);
                        *(psz++) = _T(' ');
                    }

                    while (n < lphe->nBytesPerLine)
                    {
                        *(psz++) = _T(' ');
                        *(psz++) = _T(' '); 
                        *(psz++) = _T(' ');
                        n++;
                    }

                    DrawText(hdcMem, szBuffer, lphe->nBytesPerLine * 3, &rcDraw, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
                    rcDraw.top += nLineHeight;
                }
            }
        }
        if (lphe->bShowAscii)
        {
            RECT rcDraw = rc;

            y = 0;
            rcDraw.left = x = lphe->nOffsetAscii - lphe->nOffsetChar * HexEdit_GetCharWidth(lphe);

            if (lphe->nSelStart != HE_NOSELECTION && lphe->emCurrentMode == EDIT_ASCII)
            {
                int i;
                int n = 0;
                int nSelStart = lphe->nSelStart;
                int nSelEnd = lphe->nSelEnd;

                if (nSelStart >= nSelEnd)
                {
                    nSelStart ^= nSelEnd ^= nSelStart ^= nSelEnd;
//                  nSelStart   -= 1;
//                  nSelEnd     += 1;
                }

                for (i = (lphe->nTopLine * lphe->nBytesPerLine); (i < nSelStart) && (y < nHeight); i++)
                {
                    szBuffer[0] = (isprint(lphe->pData[i])) ? lphe->pData[i] : _T('.');

                    TextOut(hdcMem, x, y, szBuffer, 1);

                    x += nCharWidth;
                    n++;

                    if (n == lphe->nBytesPerLine)
                    {
                        n = 0;
                        x = rcDraw.left;
                        y += nLineHeight;
                    }

                    // and for the 4th time, almost impossible but anyhow...
                    ASSERT(n <= lphe->nBytesPerLine);
                }

                SetTextColor(hdcMem, GetSysColor(COLOR_HIGHLIGHTTEXT));
                SetBkColor(hdcMem, GetSysColor(COLOR_HIGHLIGHT));

                for ( ; (i < nSelEnd) && (i < lphe->nLength) && (y < nHeight); i++)
                {
                    szBuffer[0] = (isprint(lphe->pData[i])) ? lphe->pData[i] : _T('.');

                    TextOut(hdcMem, x, y, szBuffer, 1);

                    x += nCharWidth;
                    n++;

                    if (n == lphe->nBytesPerLine)
                    {
                        n = 0;
                        x = rcDraw.left;
                        y += nLineHeight;
                    }

                    // 5th time, almost impossible but anyhow...
                    ASSERT(n <= lphe->nBytesPerLine);
                }

                SetTextColor(hdcMem, crText);
                SetBkColor(hdcMem, crBackground);

                for ( ; (i < lphe->nLength) && (y < nHeight); i++)
                {
                    szBuffer[0] = (isprint(lphe->pData[i])) ? lphe->pData[i] : _T('.');

                    TextOut(hdcMem, x, y, szBuffer, 1);

                    x += nCharWidth;
                    n++;

                    if (n == lphe->nBytesPerLine)
                    {
                        n = 0;
                        x = rcDraw.left;
                        y += nLineHeight;
                    }

                    // and for the 6th time, almost impossible but anyhow...
                    ASSERT(n <= lphe->nBytesPerLine);
                }
            }
            else
            {
                int i;

                for (i = (lphe->nTopLine * lphe->nBytesPerLine); (i < lphe->nLength) && (rcDraw.top < nHeight); )
                {
                    int n;
                    LPTSTR psz = szBuffer;

                    for (n = 0; (n < lphe->nBytesPerLine) && (i < lphe->nLength); n++, i++)
                    {
                        *(psz++) = (isprint(lphe->pData[i])) ? lphe->pData[i] : _T('.');
                    }

                    DrawText(hdcMem, szBuffer, n, &rcDraw, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
                    rcDraw.top += nLineHeight;
                }
            }
        }
    }
    
    BitBlt(hdc, 0, 0, Rect_Width(&rc), Rect_Height(&rc), hdcMem, 0, 0, SRCCOPY);

    SetBkMode(hdcMem, nOldBkMode);
    SetBkColor(hdcMem, crOldBackground);
    SetTextColor(hdcMem, crOldText);
    DeleteBrush(hWindowBrush);
    DeleteObject(SelectObject(hdcMem, hOldMemBitmap));
    DeleteDC(hdcMem);

    EndPaint(lphe->hwnd, &ps);
}

void HexEdit_OnSetFocus(LPHEXEDIT lphe)
{
    if (lphe->pData != NULL && !HexEdit_IsSelection(lphe))
    {
        if (lphe->ptEditPos.x == 0 && lphe->bShowAddress)
            HexEdit_CreateAddressCaret(lphe);
        else
            HexEdit_CreateEditCaret(lphe);

        SetCaretPos(lphe->ptEditPos.x, lphe->ptEditPos.y);
        ShowCaret(lphe->hwnd);
    }
}

void HexEdit_OnHScroll(LPHEXEDIT lphe, UINT nSBCode, UINT nPos, HWND hwndScrollbar)
{
    SCROLLINFO si;
    int nScreenChars;
    int nLineLength;
    int nNewOffset = 0;

    DefWindowProc(lphe->hwnd, WM_HSCROLL, MAKEWPARAM(nSBCode, nPos), (LPARAM)hwndScrollbar);

    if (lphe->pData == NULL)
        return;

    INITSTRUCT(si, TRUE);
    si.fMask = SIF_ALL;

    VERIFY(GetScrollInfo(lphe->hwnd, SB_HORZ, &si));

    nScreenChars    = HexEdit_GetScreenChars(lphe);
    // FIXME
    nLineLength     = HexEdit_GetLineLength(lphe) - nScreenChars + 2;

    switch (nSBCode)
    {
    case SB_LEFT:
        nNewOffset = 0;
    break;
    case SB_BOTTOM:
        nNewOffset = nLineLength - nScreenChars + 1;
    break;
    case SB_LINEUP:
        nNewOffset = lphe->nOffsetChar - 1;
    break;
    case SB_LINEDOWN:
        nNewOffset = lphe->nOffsetChar + 1;
    break;
    case SB_PAGEUP:
        nNewOffset = lphe->nOffsetChar - si.nPage + 1;
    break;
    case SB_PAGEDOWN:
        nNewOffset = lphe->nOffsetChar + si.nPage - 1;
    break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        nNewOffset = si.nTrackPos;
    break;
    default:
    return;
    }

    if (nNewOffset >= nLineLength)
        nNewOffset = nLineLength - 1;
    if (nNewOffset < 0)
        nNewOffset = 0;

    HexEdit_ScrollToChar(lphe, nNewOffset, TRUE);
}

void HexEdit_OnVScroll(LPHEXEDIT lphe, UINT nSBCode, UINT nPos, HWND hwndScrollbar)
{
    SCROLLINFO si;
    int nTopLine = lphe->nTopLine;
    int nNewTopLine = 0;
    int nScreenLines;
    int nLineCount;

    DefWindowProc(lphe->hwnd, WM_HSCROLL, MAKEWPARAM(nSBCode, nPos), (LPARAM)hwndScrollbar);

    if (lphe->pData == NULL)
        return;

    INITSTRUCT(si, TRUE);
    si.fMask = SIF_ALL;

    VERIFY(GetScrollInfo(lphe->hwnd, SB_VERT, &si));

    nLineCount      = HexEdit_GetLineCount(lphe);
    nScreenLines    = HexEdit_GetScreenLines(lphe);

    switch (nSBCode)
    {
    case SB_TOP:
        nNewTopLine = 0;
    break;
    case SB_BOTTOM:
        nNewTopLine = nLineCount - nScreenLines + 1;
    break;
    case SB_LINEUP:
        nNewTopLine = lphe->nTopLine - 1;
    break;
    case SB_LINEDOWN:
        nNewTopLine = lphe->nTopLine + 1;
    break;
    case SB_PAGEUP:
        nNewTopLine = lphe->nTopLine - si.nPage + 1;
    break;
    case SB_PAGEDOWN:
        nNewTopLine = lphe->nTopLine + si.nPage - 1;
    break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        lphe->nTopLine = si.nTrackPos;
    break;
    default:
    return;
    }

    if (nNewTopLine < 0)
        nNewTopLine = 0;
    if (nNewTopLine >= nLineCount)
        nNewTopLine = nLineCount - 1;

    HexEdit_ScrollToLine(lphe, nNewTopLine, TRUE);
}

void HexEdit_OnLButtonDown(LPHEXEDIT lphe, UINT nFlags, POINT pt)
{
    POINT point;

    SetFocus(lphe->hwnd);

    if (lphe->pData == NULL || !lphe->pData)
        return;

    if (nFlags & MK_SHIFT)
         lphe->nSelStart = HexEdit_GetCurrentAddress(lphe);

    point = HexEdit_CalcPos(lphe, pt.x, pt.y);

    if (point.x > -1)
    {
        lphe->ptEditPos = point;
        point.x *= HexEdit_GetCharWidth(lphe);
        point.y *= HexEdit_GetLineHeight(lphe);

        if (point.x == 0 && lphe->bShowAddress)
            HexEdit_CreateAddressCaret(lphe);
        else
            HexEdit_CreateEditCaret(lphe);

        SetCaretPos(point.x, point.y);

        if (nFlags & MK_SHIFT)
        {
            lphe->nSelEnd = HexEdit_GetCurrentAddress(lphe);

            if (lphe->emCurrentMode == EDIT_HIGH || lphe->emCurrentMode == EDIT_LOW)
                lphe->nSelEnd++;

            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
        }
    }

    if (!(nFlags & MK_SHIFT))
    {
        if (DragDetect(lphe->hwnd, pt))
        {
            lphe->nSelStart     = HexEdit_GetCurrentAddress(lphe);
            lphe->nSelEnd       = lphe->nSelStart;

            SetCapture(lphe->hwnd);
        }
        else
        {
            BOOL bSel = lphe->nSelStart != HE_NOSELECTION;

            lphe->nSelStart     = HE_NOSELECTION;
            lphe->nSelEnd       = HE_NOSELECTION;

            if (bSel)
                RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
        }
    }

    if (!HexEdit_IsSelection(lphe))
    {
        ShowCaret(lphe->hwnd);
    }
}

void HexEdit_OnMouseMove(LPHEXEDIT lphe, UINT nFlags, POINT pt)
{
    if (lphe->pData == NULL)
        return;

    if (nFlags & MK_LBUTTON && lphe->nSelStart != HE_NOSELECTION)
    {
        RECT rc;
        int nOldSelEnd;

        GetClientRect(lphe->hwnd, &rc);

        if (!PtInRect(&rc, pt))
        {
            if (pt.y < 0)
            {
                SendMessage(lphe->hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), (LPARAM)NULL);
                pt.y = 0;
            }
            else if (pt.y > Rect_Height(&rc))
            {
                SendMessage(lphe->hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), (LPARAM)NULL);
                pt.y = Rect_Height(&rc) - 1;
            }
        }

        // Selecting

        nOldSelEnd = lphe->nSelEnd;
        pt = HexEdit_CalcPos(lphe, pt.x, pt.y);

        if (pt.x > -1)
        {
            lphe->nSelEnd = HexEdit_GetCurrentAddress(lphe);
            
            if (lphe->emCurrentMode == EDIT_HIGH || lphe->emCurrentMode == EDIT_LOW)
                lphe->nSelEnd++;
        }

        if (HexEdit_IsSelection(lphe))
            DestroyCaret();

        if (nOldSelEnd != lphe->nSelEnd)
            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
    }
}

void HexEdit_OnKeyDown(LPHEXEDIT lphe, TCHAR chChar, UINT nRepCnt, UINT nFlags)
{
    BOOL fShift = IsShiftDown();
    BOOL fControl = IsCtrlDown();

    switch (chChar)
    {
    case VK_DOWN:
        if (fShift)
        {
            if (!HexEdit_IsSelection(lphe))
                HexEdit_SetSelection(lphe, lphe->nCurrentAddress, lphe->nCurrentAddress);

            HexEdit_Move(lphe, 0, 1);

            HexEdit_SetSelection(lphe, lphe->nSelStart, lphe->nCurrentAddress);

            if (lphe->emCurrentMode == EDIT_HIGH || lphe->emCurrentMode == EDIT_LOW)
                lphe->nSelEnd++;

            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

            break;
        }
        else
        {
            HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);
        }

        HexEdit_Move(lphe, 0, 1);
    break;
    case VK_UP:
        if (fShift)
        {
            if (!HexEdit_IsSelection(lphe))
                lphe->nSelStart = HexEdit_GetCurrentAddress(lphe);

            HexEdit_Move(lphe, 0, -1);
    
            lphe->nSelEnd = HexEdit_GetCurrentAddress(lphe);
    
            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
        
            break;
        }
        else
        {
            HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);
        }

        HexEdit_Move(lphe, 0, -1);
    break;
    case VK_LEFT:
        if (fShift)
        {
            if (!HexEdit_IsSelection(lphe))
                lphe->nSelStart = HexEdit_GetCurrentAddress(lphe);

            HexEdit_Move(lphe, -1, 0);

            lphe->nSelEnd = HexEdit_GetCurrentAddress(lphe) + 1;

            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

            break;
        }
        else
        {
            HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);
        }

        HexEdit_Move(lphe, -1, 0);
    break;
    case VK_RIGHT:
        if (fShift)
        {
            if (!HexEdit_IsSelection(lphe))
                lphe->nSelStart = HexEdit_GetCurrentAddress(lphe);

            HexEdit_Move(lphe, 1, 0);

            lphe->nSelEnd   = HexEdit_GetCurrentAddress(lphe);

            if (lphe->emCurrentMode == EDIT_HIGH || lphe->emCurrentMode == EDIT_LOW)
                lphe->nSelEnd++;

            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

            break;
        }
        else
        {
            HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);
        }

        HexEdit_Move(lphe, 1, 0);
    break;
    case VK_PRIOR:
        if (fShift)
        {
            if (!HexEdit_IsSelection(lphe))
                lphe->nSelStart = HexEdit_GetCurrentAddress(lphe);

            SendMessage(lphe->hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), (LPARAM)NULL);

            HexEdit_Move(lphe, 0, 0);

            lphe->nSelEnd   = HexEdit_GetCurrentAddress(lphe);

            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

            break;
        }
        else
        {
            HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);
        }

        SendMessage(lphe->hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), (LPARAM)NULL);

        HexEdit_Move(lphe, 0, 0);

    break;
    case VK_NEXT:
        if (fShift)
        {
            if (!HexEdit_IsSelection(lphe))
                lphe->nSelStart = HexEdit_GetCurrentAddress(lphe);

            SendMessage(lphe->hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), (LPARAM)NULL);

            HexEdit_Move(lphe, 0, 0);

            lphe->nSelEnd   = HexEdit_GetCurrentAddress(lphe);

            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

            break;
        }
        else
        {
            HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);
        }

        SendMessage(lphe->hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), (LPARAM)NULL);

        HexEdit_Move(lphe, 0, 0);
    break;
    case VK_HOME:
        if (fShift)
        {
            if (!HexEdit_IsSelection(lphe))
                lphe->nSelStart = HexEdit_GetCurrentAddress(lphe);

            if (fControl)
            {
                HexEdit_ScrollToLine(lphe, 0, TRUE);

                HexEdit_Move(lphe, 0, 0);
            }
            else
            {
                int nNewAddress = HexEdit_GetCurrentAddress(lphe);

                nNewAddress /= lphe->nBytesPerLine;
                nNewAddress *= lphe->nBytesPerLine;

                HexEdit_SetCurrentAddress(lphe, nNewAddress, FALSE);

                HexEdit_Move(lphe, 0, 0);
            }

            lphe->nSelEnd   = HexEdit_GetCurrentAddress(lphe);

            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

            break;
        }
        else
        {
            HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);
        }

        if (fControl)
        {
            SendMessage(lphe->hwnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, 0), (LPARAM)NULL);

            HexEdit_SetCurrentAddress(lphe, 0, FALSE);

            HexEdit_Move(lphe, 0, 0);
        }
        else
        {
            int nNewAddress = HexEdit_GetCurrentAddress(lphe);

            nNewAddress /= lphe->nBytesPerLine;
            nNewAddress *= lphe->nBytesPerLine;

            HexEdit_SetCurrentAddress(lphe, nNewAddress, FALSE);

            HexEdit_Move(lphe, 0, 0);
        }
    break;
    case VK_END:
        if (fShift)
        {
            if (!HexEdit_IsSelection(lphe))
                lphe->nSelStart = HexEdit_GetCurrentAddress(lphe);

            if (fControl)
            {
                HexEdit_SetCurrentAddress(lphe, lphe->nLength - 1, FALSE);
                HexEdit_ScrollToLine(lphe, HexEdit_GetLineCount(lphe) - 1, TRUE);

                HexEdit_Move(lphe, 0, 0);
            }
            else
            {
                int nNewAddress = HexEdit_GetCurrentAddress(lphe);

                nNewAddress /= lphe->nBytesPerLine;
                nNewAddress *= lphe->nBytesPerLine;
                nNewAddress += lphe->nBytesPerLine - 1;

                HexEdit_SetCurrentAddress(lphe, nNewAddress, FALSE);

                if (HexEdit_GetCurrentAddress(lphe) > lphe->nLength)
                    HexEdit_SetCurrentAddress(lphe, lphe->nLength, FALSE);

                HexEdit_Move(lphe, 0, 0);
            }

            lphe->nSelEnd   = HexEdit_GetCurrentAddress(lphe);

            if (lphe->emCurrentMode == EDIT_HIGH || lphe->emCurrentMode == EDIT_LOW)
                lphe->nSelEnd++;

            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

            break;
        }
        else
        {
            HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);
        }

        if (fControl)
        {
            HexEdit_SetCurrentAddress(lphe, lphe->nLength - 1, FALSE);

            HexEdit_ScrollToLine(lphe, HexEdit_GetLineCount(lphe) - 1, TRUE);

            HexEdit_Move(lphe, 0, 0);
        }
        else
        {
            int nNewAddress = HexEdit_GetCurrentAddress(lphe);

            nNewAddress /= lphe->nBytesPerLine;
            nNewAddress *= lphe->nBytesPerLine;
            nNewAddress += lphe->nBytesPerLine - 1;

            HexEdit_SetCurrentAddress(lphe, nNewAddress, FALSE);

            if (HexEdit_GetCurrentAddress(lphe) > lphe->nLength)
                HexEdit_SetCurrentAddress(lphe, lphe->nLength - 1, FALSE);

            HexEdit_Move(lphe, 0, 0);
        }

    break;
    case VK_INSERT:
        HexEdit_InsertRange(lphe, HexEdit_GetCurrentAddress(lphe), max(1, lphe->nSelEnd - lphe->nSelStart));

        RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
    break;
    case VK_DELETE:
        if (HexEdit_IsSelection(lphe))
        {
            HexEdit_OnEditDelete(lphe);
        }
        else
        {
            HexEdit_DeleteRange(lphe, HexEdit_GetCurrentAddress(lphe), HexEdit_GetCurrentAddress(lphe) + 1);

            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
        }
    break;
    case VK_TAB:
        switch (lphe->emCurrentMode)
        {
        case EDIT_NONE:
            lphe->emCurrentMode = EDIT_HIGH;
        break;
        case EDIT_HIGH:
        case EDIT_LOW:
            lphe->emCurrentMode = EDIT_ASCII;
        break;
        case EDIT_ASCII:
            lphe->emCurrentMode = EDIT_HIGH;
        break;
        }

        HexEdit_Move(lphe, 0, 0);
    break;
    }
}

void HexEdit_OnChar(LPHEXEDIT lphe, TCHAR chChar, UINT nRepCnt, UINT nFlags)
{
    if (lphe->pData == NULL || lphe->bReadOnly)
        return;

    if (chChar == _T('\t'))
        return;

    if (chChar == VK_BACK)
    {
        int nCurrentAddress = HexEdit_GetCurrentAddress(lphe);

        if (nCurrentAddress > 0)
        {
            HexEdit_DeleteRange(lphe, (nCurrentAddress - 1), nCurrentAddress);
            HexEdit_SetCurrentAddress(lphe, -1, TRUE);
            HexEdit_RepositionCaret(lphe, HexEdit_GetCurrentAddress(lphe));
            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
        }

        return;
    }

    HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);

    switch (lphe->emCurrentMode)
    {
    case EDIT_NONE:
    return;
    case EDIT_HIGH:
    case EDIT_LOW:
        if ((chChar >= _T('0') && chChar <= _T('9')) || (chChar >= _T('a') && chChar <= _T('f')))
        {
            UINT uByte = chChar - _T('0');
      
            if (uByte > 9) 
                uByte = 10 + chChar - _T('a');

            if (lphe->emCurrentMode == EDIT_HIGH)
                lphe->pData[HexEdit_GetCurrentAddress(lphe)] = (unsigned char)((lphe->pData[HexEdit_GetCurrentAddress(lphe)] & 0x0F) | (uByte << 4));
            else
                lphe->pData[HexEdit_GetCurrentAddress(lphe)] = (unsigned char)((lphe->pData[HexEdit_GetCurrentAddress(lphe)] & 0xF0) | uByte);

            HexEdit_SetModify(lphe, TRUE);

            HexEdit_Move(lphe, 1, 0);
        }
    break;
    case EDIT_ASCII:
        lphe->pData[HexEdit_GetCurrentAddress(lphe)] = (unsigned char)chChar;

        HexEdit_SetModify(lphe, TRUE);

        HexEdit_Move(lphe, 1, 0);
    break;
    }

    RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

void HexEdit_OnContextMenu(LPHEXEDIT lphe, POINT pt)
{
    HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_HEXEDITMENU));
    HMENU hSubMenu = GetSubMenu(hMenu, 0);

    if (pt.x == -1 && pt.y == -1)
    {
        // Keyboard
        RECT rc;

        GetClientRect(lphe->hwnd, &rc);
        Window_ClientToScreenRect(lphe->hwnd, &rc);

        pt.x = rc.left + 5;
        pt.y = rc.top + 5;
    }

    if (!HexEdit_IsSelection(lphe))
    {
        Menu_EnableMenuItem(hSubMenu, IDM_HEXEDITMENU_DELETE, FALSE);
        Menu_EnableMenuItem(hSubMenu, IDM_HEXEDITMENU_CUT, FALSE);
        Menu_EnableMenuItem(hSubMenu, IDM_HEXEDITMENU_COPY, FALSE);
    }

    OpenClipboard(NULL);

    if (!IsClipboardFormatAvailable(CF_TEXT) && !IsClipboardFormatAvailable(RegisterClipboardFormat(_T("BinaryData"))))
        Menu_EnableMenuItem(hSubMenu, IDM_HEXEDITMENU_PASTE, FALSE);

    CloseClipboard();

    Menu_TrackPopupMenu(hSubMenu, pt.x, pt.y, lphe->hwnd);
}

void HexEdit_OnSize(LPHEXEDIT lphe, UINT nType, int cx, int cy)
{
    DefWindowProc(lphe->hwnd, WM_SIZE, (WPARAM)nType, MAKELPARAM(cx, cy));

    lphe->nScreenChars  = -1;
    lphe->nScreenLines  = -1;

    HexEdit_RecalcVertScrollBar(lphe, FALSE);
    HexEdit_RecalcHorzScrollBar(lphe, FALSE);
}

void HexEdit_Move(LPHEXEDIT lphe, int x, int y)
{
    int nNewAddress = HexEdit_GetCurrentAddress(lphe);

    switch (lphe->emCurrentMode)
    {
    case EDIT_NONE:
    return;
    case EDIT_HIGH:
    {
        if ((x != 0) && (x + nNewAddress != -1))
            lphe->emCurrentMode = EDIT_LOW;

        if (x == -1)
            nNewAddress--;

        nNewAddress += y * lphe->nBytesPerLine;
    }
    break;
    case EDIT_LOW:
    {
        if ((x != 0) && (nNewAddress + x != lphe->nLength))
            lphe->emCurrentMode = EDIT_HIGH;

        if (x == 1)
            nNewAddress++;

        nNewAddress += y * lphe->nBytesPerLine;
    }
    break;
    case EDIT_ASCII:
    {
        nNewAddress += x;

        nNewAddress += y * lphe->nBytesPerLine;
    }
    break;
    }

    if (nNewAddress < lphe->nLength && nNewAddress >= 0)
    {
        HexEdit_SetCurrentAddress(lphe, nNewAddress, FALSE);

        // FIXME
        HexEdit_EnsureVisible(lphe, HexEdit_GetCurrentAddress(lphe));
        HexEdit_RepositionCaret(lphe, HexEdit_GetCurrentAddress(lphe));
    }
}

void HexEdit_RecalcVertScrollBar(LPHEXEDIT lphe, BOOL bPositionOnly)
{
    SCROLLINFO si;

    si.cbSize   = sizeof (si);

    if (bPositionOnly)
    {
        si.fMask    = SIF_POS;
        si.nPos     = lphe->nTopLine;
    }
    else
    {
        if (HexEdit_GetScreenLines(lphe) >= HexEdit_GetLineCount(lphe) && lphe->nTopLine > 0)
        {
            lphe->nTopLine = 0;
            InvalidateRect(lphe->hwnd, NULL, FALSE);
            // FIXME
            //HexEdit_RepositionCaret(lphe, );
        }

        si.fMask    = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
        si.nMin     = 0;
        si.nMax     = HexEdit_GetLineCount(lphe) - 1;
        si.nPage    = HexEdit_GetScreenLines(lphe);
        si.nPos     = lphe->nTopLine;
    }

    SetScrollInfo(lphe->hwnd, SB_VERT, &si, TRUE);
}

void HexEdit_RecalcHorzScrollBar(LPHEXEDIT lphe, BOOL bPositionOnly)
{
    SCROLLINFO si;

    si.cbSize   = sizeof (si);

    if (bPositionOnly)
    {
        si.fMask    = SIF_POS;
        si.nPos     = lphe->nOffsetChar;
    }
    else
    {
        if (HexEdit_GetScreenChars(lphe) >= HexEdit_GetLineLength(lphe) && lphe->nOffsetChar > 0)
        {
            lphe->nOffsetChar = 0;
            InvalidateRect(lphe->hwnd, NULL, FALSE);
            // FIXME
            HexEdit_RepositionCaret(lphe, HexEdit_GetCurrentAddress(lphe));
        }

        si.fMask    = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
        si.nMin     = 0;
        si.nMax     = HexEdit_GetLineLength(lphe);
        si.nPage    = HexEdit_GetScreenChars(lphe);
        si.nPos     = lphe->nOffsetChar;
    }

    SetScrollInfo(lphe->hwnd, SB_HORZ, &si, TRUE);
}

__inline BOOL HexEdit_IsSelection(LPHEXEDIT lphe)
{
  return (lphe->nSelStart != HE_NOSELECTION);
}

BOOL HexEdit_SetCurrentAddress(LPHEXEDIT lphe, int nNewAddress, BOOL bRelative)
{
    if (bRelative)
        lphe->nCurrentAddress += nNewAddress;
    else
        lphe->nCurrentAddress = nNewAddress;

    if (lphe->nCurrentAddress < 0)
        lphe->nCurrentAddress = 0;
    else if (lphe->nCurrentAddress > lphe->nLength)
        lphe->nCurrentAddress = lphe->nLength;

    return (TRUE);
}

int HexEdit_GetCurrentAddress(LPHEXEDIT lphe)
{
    if (lphe->nCurrentAddress < 0)
        lphe->nCurrentAddress = 0;
    else if (lphe->nCurrentAddress > lphe->nLength)
        lphe->nCurrentAddress = lphe->nLength;

    return (lphe->nCurrentAddress);
}

void HexEdit_SetCharDimensions(LPHEXEDIT lphe)
{
    HDC hdc;
    SIZE size;
    HFONT hOldFont;

    hdc = GetDC(lphe->hwnd);
    hOldFont = SelectObject(hdc, lphe->hFont);

    GetTextExtentPoint(hdc, _T("X"), 1, &size);

    lphe->nLineHeight = size.cy;

    if (lphe->nLineHeight < 1)
        lphe->nLineHeight = 1;

    lphe->nCharWidth = size.cx;

    // FIXME: lphe->nOffsetChar
    lphe->nOffsetAddress    = 0;
    lphe->nOffsetHex        = ((lphe->bShowAddress) ? ((lphe->bWideAddress) ? lphe->nCharWidth * 9 : lphe->nCharWidth * 5) : 0);
    lphe->nOffsetAscii      = lphe->nOffsetHex + ((lphe->bShowHex) ? (lphe->nBytesPerLine * 3 * lphe->nCharWidth) : 0);

    SelectObject(hdc, hOldFont);
    ReleaseDC(lphe->hwnd, hdc);

    HexEdit_UpdateView(lphe, UPDATE_HORZRANGE | UPDATE_VERTRANGE);
}

int HexEdit_GetScreenChars(LPHEXEDIT lphe)
{
    if (lphe->nScreenChars == -1)
    {
        RECT rc;

        GetClientRect(lphe->hwnd, &rc);

        lphe->nScreenChars = Rect_Width(&rc) / HexEdit_GetCharWidth(lphe);
    }

    return (lphe->nScreenChars);
}

int HexEdit_GetLineLength(LPHEXEDIT lphe)
{
    return (((lphe->bShowAddress) ? ((lphe->bWideAddress) ? 8 : 4) : 0) +
            ((lphe->bShowHex) ? (lphe->nBytesPerLine * 3) : 0) +
            ((lphe->bShowAscii) ? lphe->nBytesPerLine : 0)
           );
}

int HexEdit_GetLineHeight(LPHEXEDIT lphe)
{
    if (lphe->nLineHeight == -1)
        HexEdit_SetCharDimensions(lphe);

    return (lphe->nLineHeight);
}

int HexEdit_GetCharWidth(LPHEXEDIT lphe)
{
    if (lphe->nCharWidth == -1)
        HexEdit_SetCharDimensions(lphe);

    return (lphe->nCharWidth);
}

BOOL HexEdit_SetModify(LPHEXEDIT lphe, BOOL bModified)
{
    BOOL fOld = lphe->bModified;

    lphe->bModified = bModified;

    return (fOld);
}

BOOL HexEdit_GetModify(LPHEXEDIT lphe)
{
    return (lphe->bModified);
}

int HexEdit_SetBytesPerLine(LPHEXEDIT lphe, int nBytesPerLine)
{
    int nOldBytesPerLine = lphe->nBytesPerLine;

    lphe->nBytesPerLine = nBytesPerLine;

    return (nOldBytesPerLine);
}

POINT HexEdit_CalcPos(LPHEXEDIT lphe, int x, int y)
{
    POINT pt = { -1, -1 };
    int nCharWidth;
    int xp;

    y /= HexEdit_GetLineHeight(lphe);

    if (y < 0 || y > HexEdit_GetScreenLines(lphe))
        return (pt);

    if ((y * lphe->nBytesPerLine) > lphe->nLength)
        return (pt);

    nCharWidth = HexEdit_GetCharWidth(lphe);
    x += nCharWidth;
    x /= nCharWidth;

    if ((lphe->bShowAddress) && x <= (lphe->bWideAddress ? 8 : 4))
    {
        // FIXME
        HexEdit_SetCurrentAddress(lphe, lphe->nTopLine + (lphe->nBytesPerLine * y), FALSE);
        lphe->emCurrentMode = EDIT_NONE;

        pt.x = 0;
        pt.y = y;
    
        return (pt);
    }

    xp = (lphe->nOffsetHex / nCharWidth) + lphe->nBytesPerLine * 3;
  
    if (lphe->bShowHex && x < xp)
    {
        if (x % 3)
            x--;

        // FIXME
        HexEdit_SetCurrentAddress(lphe, lphe->nTopLine + (lphe->nBytesPerLine * y) + (x - (lphe->nOffsetHex / nCharWidth)) / 3, FALSE);
        lphe->emCurrentMode = ((x % 3) & 0x01) ? EDIT_LOW : EDIT_HIGH;

        pt.x = x;
        pt.y = y;

        return (pt);
    }

    xp = (lphe->nOffsetAscii / nCharWidth) + lphe->nBytesPerLine;
  
    if ((lphe->bShowAscii) && x <= xp)
    {
        // FIXME
        HexEdit_SetCurrentAddress(lphe, lphe->nTopLine + (lphe->nBytesPerLine * y) + (x - (lphe->nOffsetAscii / nCharWidth)), FALSE);
        lphe->emCurrentMode = EDIT_ASCII;
    
        pt.x = x;
        pt.y = y;

        return (pt);
    }

    return (pt);
}

void HexEdit_CreateAddressCaret(LPHEXEDIT lphe)
{
    DestroyCaret();
    CreateCaret(lphe->hwnd, (HBITMAP)NULL, HexEdit_GetCharWidth(lphe) * ((lphe->bWideAddress) ? 8 : 4), HexEdit_GetLineHeight(lphe));
}

// FIXME: easy to add a variety of carets here
void HexEdit_CreateEditCaret(LPHEXEDIT lphe)
{
    DestroyCaret();
    CreateCaret(lphe->hwnd, (HBITMAP)NULL, HexEdit_GetCharWidth(lphe), HexEdit_GetLineHeight(lphe));
}

void HexEdit_SetSelection(LPHEXEDIT lphe, int nStart, int nEnd)
{
    DestroyCaret();

    lphe->nSelStart = nStart;
    lphe->nSelEnd = nEnd;
 
    RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
  
    if (lphe->ptEditPos.x == 0 && lphe->bShowAddress)
        HexEdit_CreateAddressCaret(lphe);
    else
        HexEdit_CreateEditCaret(lphe);

    SetCaretPos(lphe->ptEditPos.x, lphe->ptEditPos.y);
    ShowCaret(lphe->hwnd);
}

void HexEdit_RepositionCaret(LPHEXEDIT lphe, int nPoint)
{
    int x;
    int y;
    RECT rc;

    y = HexEdit_GetLineFromAddress(lphe, nPoint) - lphe->nTopLine;
    x = nPoint % lphe->nBytesPerLine;

    switch (lphe->emCurrentMode)
    {
    case EDIT_NONE:
        HexEdit_CreateAddressCaret(lphe);

        x = 0;
    break;
    case EDIT_HIGH:
        HexEdit_CreateEditCaret(lphe);

        x *= HexEdit_GetCharWidth(lphe) * 3;
        x += lphe->nOffsetHex - lphe->nOffsetChar * HexEdit_GetCharWidth(lphe);
    break;
    case EDIT_LOW:
        HexEdit_CreateEditCaret(lphe);

        x *= HexEdit_GetCharWidth(lphe) * 3;
        x += HexEdit_GetCharWidth(lphe);
        x += lphe->nOffsetHex - lphe->nOffsetChar * HexEdit_GetCharWidth(lphe);
    break;
    case EDIT_ASCII:
        HexEdit_CreateEditCaret(lphe);
        
        x *= HexEdit_GetCharWidth(lphe);
        x += lphe->nOffsetAscii - lphe->nOffsetChar * HexEdit_GetCharWidth(lphe);
    break;
    }

    lphe->ptEditPos.x = x;
    lphe->ptEditPos.y = y * HexEdit_GetLineHeight(lphe);

    GetClientRect(lphe->hwnd, &rc);

    if (PtInRect(&rc, lphe->ptEditPos))
    {
        SetCaretPos(lphe->ptEditPos.x, lphe->ptEditPos.y);
        ShowCaret(lphe->hwnd);
    }
}

void HexEdit_OnEditDelete(LPHEXEDIT lphe) 
{
    HexEdit_SetCurrentAddress(lphe, lphe->nSelStart, FALSE);

    HexEdit_DeleteRange(lphe, lphe->nSelStart, lphe->nSelEnd);
    HexEdit_RepositionCaret(lphe, HexEdit_GetCurrentAddress(lphe));

    RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

void HexEdit_OnEditCopy(LPHEXEDIT lphe) 
{
    EmptyClipboard();

    if (lphe->emCurrentMode != EDIT_ASCII)
    {
        int         nLen = (lphe->nSelEnd - lphe->nSelStart);
        HGLOBAL     hMemBinary = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, nLen);
        HGLOBAL     hMemAscii = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, nLen * 3);
        LPTSTR      psz;
        int         i;

        if (hMemAscii == NULL || hMemBinary == NULL)
            return;

        // copy binary
        psz = (LPTSTR)GlobalLock(hMemBinary);
        memcpy(psz, lphe->pData + lphe->nSelStart, nLen);
        GlobalUnlock(hMemBinary);

        // copy ascii
        psz = (LPTSTR)GlobalLock(hMemAscii);

        for (i = 0; i < nLen; i++)
        {
            TOHEX(lphe->pData[lphe->nSelStart + i], psz);
            *(psz++) = _T(' ');
        }

        GlobalUnlock(hMemAscii);

        SetClipboardData(RegisterClipboardFormat(_T("BinaryData")), hMemBinary);
        SetClipboardData(CF_TEXT, hMemAscii);
    }
    else
    {
        int nLen = lphe->nSelEnd = lphe->nSelStart;
        HGLOBAL     hMemBinary = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, nLen);
        HGLOBAL     hMemAscii = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, nLen);
        LPBYTE      pb;
        int         i;

        if (hMemAscii == NULL || hMemBinary == NULL)
            return;

        // copy binary
        pb = (LPBYTE)GlobalLock(hMemBinary);
        memcpy(pb, lphe->pData + lphe->nSelStart, nLen);
        GlobalUnlock(hMemBinary);

        // copy ascii
        pb = (LPBYTE)GlobalLock(hMemAscii);
        memcpy(pb, lphe->pData + lphe->nSelStart, nLen);
        for (i = 0; i < nLen; i++, pb++)
        {
            if (!isprint(*pb))
                *pb = _T('.');
        }

        GlobalUnlock(hMemAscii);

        SetClipboardData(RegisterClipboardFormat(_T("BinaryData")), hMemBinary);
        SetClipboardData(CF_TEXT, hMemAscii);
  }
}

void HexEdit_OnEditCut(LPHEXEDIT lphe) 
{
    HexEdit_OnEditCopy(lphe);
    HexEdit_DeleteRange(lphe, lphe->nSelStart, lphe->nSelEnd);

    RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

void HexEdit_OnEditPaste(LPHEXEDIT lphe) 
{
    HGLOBAL hMem = NULL;

    if (OpenClipboard(lphe->hwnd))
    {

        if (IsClipboardFormatAvailable(RegisterClipboardFormat(_T("BinaryData")))) 
            hMem = GetClipboardData(RegisterClipboardFormat(_T("BinaryData")));
        else if (IsClipboardFormatAvailable(CF_TEXT)) 
            hMem = GetClipboardData(CF_TEXT);

        if (hMem != NULL)
        {
            LPBYTE  pb = (LPBYTE)GlobalLock(hMem);
            int nLen = GlobalSize(hMem);
            int   nInsert;
            int   nOldAddress = HexEdit_GetCurrentAddress(lphe);
            
            HexEdit_NormalizeSelection(lphe);

            if (lphe->nSelStart == HE_NOSELECTION)
            {
                if (lphe->emCurrentMode == EDIT_LOW)
                    HexEdit_SetCurrentAddress(lphe, 1, TRUE);

                nInsert = HexEdit_GetCurrentAddress(lphe);
                HexEdit_InsertRange(lphe, HexEdit_GetCurrentAddress(lphe), nLen);
            }
            else
            {
                nInsert = lphe->nSelStart;
                HexEdit_DeleteRange(lphe, lphe->nSelStart, lphe->nSelEnd);
                HexEdit_InsertRange(lphe, nInsert, nLen);
                HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);
            }
            
            memcpy(lphe->pData + nInsert, pb, nLen);

            HexEdit_SetCurrentAddress(lphe, nOldAddress, FALSE);

            RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

            GlobalUnlock(hMem);
        }

        CloseClipboard();
    }
}

void HexEdit_OnEditSelectAll(LPHEXEDIT lphe)
{
    lphe->nSelStart = 0;
    lphe->nSelEnd = lphe->nLength;

    DestroyCaret();
    RedrawWindow(lphe->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

void HexEdit_OnEditUndo(LPHEXEDIT lphe)
{
  // TODO: Add your command handler code here
  
}

void HexEdit_NormalizeSelection(LPHEXEDIT lphe)
{
    if (lphe->nSelStart > lphe->nSelEnd)
        lphe->nSelStart ^= lphe->nSelEnd ^= lphe->nSelStart ^= lphe->nSelEnd;
}

void HexEdit_DeleteRange(LPHEXEDIT lphe, int nStart, int nEnd)
{
    LPBYTE pb;
    
    pb = (LPBYTE)Mem_Alloc(sizeof(BYTE) * (lphe->nLength - (nEnd - nStart) + 1));
    memcpy(pb, lphe->pData, nStart * sizeof(BYTE));

    if (nStart < lphe->nLength - (nEnd - nStart)) 
        memcpy(pb + nStart, lphe->pData + nEnd, (lphe->nLength - nEnd));
    
    Mem_Free(lphe->pData);

    lphe->pData = pb;
    lphe->nLength = lphe->nLength - (nEnd - nStart);

    HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);
    HexEdit_SetModify(lphe, TRUE);

    if (HexEdit_GetCurrentAddress(lphe) > lphe->nLength)
    {
        HexEdit_SetCurrentAddress(lphe, lphe->nLength, FALSE);
        HexEdit_RepositionCaret(lphe, HexEdit_GetCurrentAddress(lphe));
    }

    // FIXME: make faster
    HexEdit_UpdateView(lphe, UPDATE_HORZRANGE | UPDATE_VERTRANGE);
}

void HexEdit_InsertRange(LPHEXEDIT lphe, int nStart, int nLen)
{
    LPBYTE pb;
    
    pb = (LPBYTE)Mem_Alloc((lphe->nLength + nLen) * sizeof(BYTE));

    memcpy(pb, lphe->pData, nStart);
    memcpy(pb + nStart + nLen, lphe->pData + nStart, (lphe->nLength - nStart));

    Mem_Free(lphe->pData);

    lphe->pData = pb;
    lphe->nLength += nLen;

    HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);
    HexEdit_SetModify(lphe, TRUE);

    HexEdit_SetCharDimensions(lphe);
}

POINT HexEdit_GetSelection(LPHEXEDIT lphe)
{
    POINT pt;

    pt.x = lphe->nSelStart;
    pt.y = lphe->nSelEnd;

    return (pt);
}

void HexEdit_SetData(LPHEXEDIT lphe, LPBYTE pb, int nLen)
{
    PBYTE pNewOriginalData = (LPBYTE)Mem_Alloc(nLen);

    if (lphe->pData != NULL)
        Mem_Free(lphe->pData);

    memcpy(pNewOriginalData, pb, nLen);
  
    HexEdit_SetOriginalData(lphe, pNewOriginalData, nLen);
}

void HexEdit_SetOriginalData(LPHEXEDIT lphe, LPBYTE pb, int nLen)
{
    lphe->pData = pb;
    lphe->nLength = nLen;

    HexEdit_ResetView(lphe);
}

void HexEdit_ResetView(LPHEXEDIT lphe)
{
    HexEdit_SetSelection(lphe, HE_NOSELECTION, HE_NOSELECTION);

    HexEdit_SetCurrentAddress(lphe, 0, FALSE);
    lphe->ptEditPos.x = 0;
    lphe->ptEditPos.y = 0;
    lphe->emCurrentMode = EDIT_HIGH;
    lphe->nTopLine = 0;

    HexEdit_SetCharDimensions(lphe);
}

int HexEdit_GetData(LPHEXEDIT lphe, LPBYTE pb, int nLen)
{
    memcpy(pb, lphe->pData, min(nLen, lphe->nLength));

    return (lphe->nLength);
}

int HexEdit_GetOriginalData(LPHEXEDIT lphe, LPBYTE *ppb)
{
    *ppb = lphe->pData;

    return (lphe->nLength);
}

void HexEdit_ScrollToChar(LPHEXEDIT lphe, int nNewOffsetChar, BOOL bTrackScrollBar)
{
    if (lphe->nOffsetChar != nNewOffsetChar)
    {
        RECT rcScroll;
        int nScrollChars;
        
        nScrollChars = lphe->nOffsetChar - nNewOffsetChar;

        lphe->nOffsetChar   = nNewOffsetChar;

        GetClientRect(lphe->hwnd, &rcScroll);

        ScrollWindow(lphe->hwnd, nScrollChars * HexEdit_GetCharWidth(lphe), 0, &rcScroll, &rcScroll);
        UpdateWindow(lphe->hwnd);

        if (bTrackScrollBar)
            HexEdit_RecalcHorzScrollBar(lphe, FALSE);
    }
}

void HexEdit_ScrollToLine(LPHEXEDIT lphe, int nNewTopLine, BOOL bTrackScrollBar)
{
    if (lphe->nTopLine != nNewTopLine)
    {
        int nScrollLines = lphe->nTopLine - nNewTopLine;

        lphe->nTopLine = nNewTopLine;

        ScrollWindow(lphe->hwnd, 0, nScrollLines * HexEdit_GetLineHeight(lphe), NULL, NULL);
        UpdateWindow(lphe->hwnd);

        if (bTrackScrollBar)
            HexEdit_RecalcVertScrollBar(lphe, TRUE);
    }
}

int HexEdit_GetScreenLines(LPHEXEDIT lphe)
{
//  if (lphe->nLineHeight == -1 || lphe->nCharWidth == -1)
//      HexEdit_SetCharDimensions(lphe);

    if (lphe->nScreenLines == -1)
    {
        RECT rc;

        GetClientRect(lphe->hwnd, &rc);

        if (Rect_Height(&rc) == 0)
            return (0);

        lphe->nScreenLines = Rect_Height(&rc) / HexEdit_GetLineHeight(lphe);

/*  lphe->dwFlags &= ~HEF_HALFPAGE;

    if ((lphe->nLinesPerPage * lphe->nBytesPerLine) > lphe->nLength)
    {
        lphe->nLinesPerPage = (lphe->nLength + (lphe->nBytesPerLine / 2)) / lphe->nBytesPerLine;

        if (lphe->nLength % lphe->nBytesPerLine != 0)
        {
            lphe->dwFlags |= HEF_HALFPAGE;
            lphe->nLinesPerPage++;
        }
    }
*/
//      HexEdit_UpdateScrollbars(lphe);
    }

    return (lphe->nScreenLines);
}

int HexEdit_GetLineCount(LPHEXEDIT lphe)
{
    if (lphe->pData == NULL)
    {
        return (1);
    }
    else
    {
        int nLineCount = lphe->nLength / lphe->nBytesPerLine + 1;

        return (max(1, nLineCount));
    }
}

void HexEdit_UpdateView(LPHEXEDIT lphe, DWORD dwFlags)
{
    if (dwFlags & UPDATE_HORZRANGE)
    {
        HexEdit_RecalcHorzScrollBar(lphe, FALSE);
    }

    if (dwFlags & UPDATE_VERTRANGE)
    {
        HexEdit_RecalcVertScrollBar(lphe, FALSE);
    }

    if (dwFlags & UPDATE_CHARINFO)
    {
        HexEdit_SetCharDimensions(lphe);
    }
}

int HexEdit_GetLineFromAddress(LPHEXEDIT lphe, int nAddress)
{
    ASSERT(nAddress >= 0 && nAddress <= lphe->nLength);

    return (nAddress / lphe->nBytesPerLine);
}

int HexEdit_GetLinePosFromAddress(LPHEXEDIT lphe, int nAddress)
{
    ASSERT(nAddress >= 0 && nAddress <= lphe->nLength);

    return (nAddress % lphe->nBytesPerLine);
}

int HexEdit_CalculateActualOffset(LPHEXEDIT lphe, int nAddress)
{
    int nOffset = 0;
    int nLinePos = HexEdit_GetLinePosFromAddress(lphe, nAddress) + 1;

    if (lphe->bShowAddress)
    {
        if (lphe->bWideAddress)
            nOffset = 8;
        else
            nOffset = 4;
    }

    if (lphe->bShowHex && lphe->emCurrentMode == EDIT_LOW || lphe->emCurrentMode == EDIT_HIGH)
    {
        nOffset += nLinePos * 3;
    }
    
    if (lphe->bShowAscii && lphe->emCurrentMode == EDIT_ASCII)
    {
        if (lphe->bShowHex)
            nOffset += lphe->nBytesPerLine * 3;

        nOffset += nLinePos;
    }

    return (nOffset);
}

void HexEdit_EnsureVisible(LPHEXEDIT lphe, int nAddress)
{
    int nLineCount      = HexEdit_GetLineCount(lphe);
    int nNewTopLine     = lphe->nTopLine;
    int nCurrentLine    = HexEdit_GetLineFromAddress(lphe, nAddress);
    int nScreenChars    = HexEdit_GetScreenChars(lphe);
    int nLineLength     = HexEdit_GetLineLength(lphe) - nScreenChars + 2;
    int nNewOffset;
    int nActualPos;

    if (nCurrentLine >= (nNewTopLine + HexEdit_GetScreenLines(lphe)))
    {
        nNewTopLine = nCurrentLine - HexEdit_GetScreenLines(lphe) + 1;
    }
    if (nCurrentLine < nNewTopLine)
    {
        nNewTopLine = nCurrentLine;
    }

    if (nNewTopLine < 0)
        nNewTopLine = 0;
    else if (nNewTopLine > nLineCount)
        nNewTopLine = nLineCount - 1;

    if (lphe->nTopLine != nNewTopLine)
    {
        HexEdit_ScrollToLine(lphe, nNewTopLine, TRUE);
    }

    nActualPos  = HexEdit_CalculateActualOffset(lphe, nAddress);
    nNewOffset  = lphe->nOffsetChar;

    if (nActualPos >= (nNewOffset + nScreenChars))
    {
        nNewOffset = nActualPos - nScreenChars + 16;
    }
    else if (nActualPos < nNewOffset + 8) // If we're at the first chars we wanna show address
    {
        //This won't get in enough damnit
        nNewOffset = nActualPos - 24;
    }

    if (nNewOffset >= nLineLength)
        nNewOffset = nLineLength - 1;
    if (nNewOffset < 0)
        nNewOffset = 0;

    if (lphe->nOffsetChar != nNewOffset)
    {
        HexEdit_ScrollToChar(lphe, nNewOffset, TRUE);
        HexEdit_RepositionCaret(lphe, nAddress);
    }
}
