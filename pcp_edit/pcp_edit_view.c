/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_view.c
 * Created    : not known (before 01/24/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:39:33
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* windows */
#include <commctrl.h>
#include <zmouse.h>

/* standard */
#include <math.h>

#include "resource.h"

/* pcp_edit */
#include "pcp_edit_view.h"
#include "pcp_edit_internal.h"
#include "pcp_edit_buffer.h"
#include "pcp_edit_move.h"
#include "pcp_edit_syntax.h"

/* pcp_paint */
#include <pcp_paint.h>

/* pcp_generic */
#include <pcp_linkedlist.h>
#include <pcp_mem.h>
#include <pcp_point.h>
#include <pcp_rect.h>
#include <pcp_string.h>

#define TAB_CHARACTER           _T('\xBB');
#define SPACE_CHARACTER         _T('\x95');
#define SMOOTH_SCROLL_FACTOR    6

LRESULT CALLBACK Edit_View_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Edit_View_OnChar(LPEDITVIEW lpev, UINT nChar, UINT nRepCnt, UINT nFlags);
void Edit_View_OnKeyDown(LPEDITVIEW lpev, UINT nVirtKey, UINT cRepeat, UINT fFlags);
void Edit_View_OnCut(LPEDITVIEW lpev);
void Edit_View_OnPaste(LPEDITVIEW lpev);
int Edit_View_OnCreate(LPEDITVIEW lpev, LPCREATESTRUCT lpCreateStruct);
void Edit_View_OnKillFocus(LPEDITVIEW lpev, HWND hwndGetFocus);
void Edit_View_OnSysColorChange(LPEDITVIEW lpev);
void Edit_View_OnSetFocus(LPEDITVIEW lpev, HWND hwndLoseFocus);
void Edit_View_OnHScroll(LPEDITVIEW lpev, UINT nSBCode, UINT nPos, HWND hwndScrollBar);
BOOL Edit_View_OnSetCursor(LPEDITVIEW lpev, UINT nHitTest, UINT wMouseMsg);
void Edit_View_OnVScroll(LPEDITVIEW lpev, UINT nSBCode, UINT nPos, HWND hwndScrollBar);
void Edit_View_OnDestroy(LPEDITVIEW lpev);
BOOL Edit_View_OnEraseBkgnd(HDC hdc);
void Edit_View_OnSize(LPEDITVIEW lpev, UINT nType, int cx, int cy);
void Edit_View_OnPaint(LPEDITVIEW lpev);
void Edit_View_OnClear(LPEDITVIEW lpev);
void Edit_View_OnBackspace(LPEDITVIEW lpev);
void Edit_View_OnMouseWheel(LPEDITVIEW lpev, UINT nFlags, short zDelta, POINT pt);
DWORD Edit_View_GetFlags(LPEDITVIEW lpev);

/* static */
int Edit_View_GetLineActualLength(LPEDITVIEW lpev, int iLineIndex);
COLORREF Edit_View_GetColor(LPEDITVIEW lpev, int iColorIndex);
DWORD Edit_View_GetLineFlags(LPEDITVIEW lpev, int iLineIndex);
int Edit_View_GetMaxLineLength(LPEDITVIEW lpev);
int Edit_View_GetTabSize(LPEDITVIEW lpev);
void Edit_View_SetTabSize(LPEDITVIEW lpev, int iTabSize);
int Edit_View_GetScreenChars(LPEDITVIEW lpev);
void Edit_View_FindIncrementalEnd(LPEDITVIEW lpev, BOOL bReturnToOldPlace);
void Edit_View_IncrementalSearchUpdateStatusbar(LPEDITVIEW lpev);

static int Edit_View_FindStringHelper(LPCTSTR pszFindWhere, LPCTSTR pszFindWhat, DWORD dwFlags, int *nLen, RxNode *rxNode, RxMatchRes *rxMatch);

void Edit_View_ExpandChars(LPEDITVIEW lpev, LPCTSTR pszChars, int iOffset, int iCount, LPTSTR *pszLine);
void Edit_View_DrawLineHelperImpl(LPEDITVIEW lpev, HDC hdc, LPPOINT lptOrigin, const RECT rcClip, LPCTSTR pszChars, int iOffset, int iCount);
void Edit_View_DrawLineHelper(LPEDITVIEW lpev, HDC hdc, LPPOINT lptOrigin, const LPRECT lprcClip, int iColorIndex, LPCTSTR pszChars, int iOffset, int iCount, POINT ptTextPos);
void Edit_View_GetLineColors(LPEDITVIEW lpev, int iLineIndex, LPCOLORREF lpcrBkgnd, LPCOLORREF lpcrText, LPBOOL lpbDrawWhitespace);
DWORD Edit_View_GetParseCookie(LPEDITVIEW lpev, int iLineIndex);
void Edit_View_DrawSingleLine(LPEDITVIEW lpev, HDC *fdc, const LPRECT lprc, int iLineIndex);
DWORD Edit_View_ParseLine(LPEDITVIEW lpev, DWORD dwCookie, int nLineIndex, LPTEXTBLOCK pBuf, LPINT piActualItems);
void Edit_View_DrawMargin(LPEDITVIEW lpev, HDC *fdc, const LPRECT lprc, int iLineIndex);
void Edit_View_ShowCursor(LPEDITVIEW lpev);
void Edit_View_HideCursor(LPEDITVIEW lpev);
void Edit_View_RecalcVertScrollBar(LPEDITVIEW lpev, BOOL bPositionOnly);
void Edit_View_RecalcHorzScrollBar(LPEDITVIEW lpev, BOOL bPositionOnly);
void Edit_View_InvalidateLines(LPEDITVIEW lpev, int iLine1, int iLine2, BOOL bInvalidateMargin);
void Edit_View_SetFlags(LPEDITVIEW lpev, DWORD dwFlags);
BOOL Edit_View_QueryEditable(LPEDITVIEW lpev);
void Edit_View_OnEditOperation(LPEDITVIEW lpev, int nAction, LPCTSTR pszText);
void Edit_View_ScrollToChar(LPEDITVIEW lpev, int iNewOffsetChar, BOOL bNoSmoothScroll, BOOL bTrackScrollBar /*= TRUE*/);
void Edit_View_ScrollToLine(LPEDITVIEW lpev, int iNewTopLine, BOOL bNoSmoothScroll, BOOL bTrackScrollBar);
int Edit_View_GetMarginWidth(LPEDITVIEW lpev);
POINT Edit_View_ClientToText(LPEDITVIEW lpev, POINT pt);
POINT Edit_View_TextToClient(LPEDITVIEW lpev, POINT pt);
void Edit_View_CalcLineCharDim(LPEDITVIEW lpev);
int Edit_View_GetLineHeight(LPEDITVIEW lpev);
int Edit_View_GetCharWidth(LPEDITVIEW lpev);
BOOL Edit_View_GetItalic(int iColorIndex);
BOOL Edit_View_GetBold(int iColorIndex);
void Edit_View_ResetView(LPEDITVIEW lpev);
void Edit_View_PrepareSelBounds(LPEDITVIEW lpev);


LRESULT CALLBACK Pcp_Edit_View_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPEDITVIEW lpev = (LPEDITVIEW)GetWindowLong(hwnd, GWL_USERDATA);

    switch (uMsg)
    {
    case WM_NCCREATE:
        lpev = (LPEDITVIEW)Mem_Alloc(sizeof(EDITVIEW));
        ZeroMemory(lpev, sizeof(EDITVIEW));

        if (!lpev)
            return (TRUE);

        SetWindowLong(hwnd, GWL_USERDATA, (long)lpev);
    return (TRUE);
    case WM_CREATE:
        lpev->hwnd = hwnd;
        Edit_View_OnCreate(lpev, (LPCREATESTRUCT)lParam);
    return (0);
    case WM_DESTROY:
        Edit_View_OnDestroy(lpev);
    return (0);
    case WM_ERASEBKGND:
    return (Edit_View_OnEraseBkgnd((HDC)wParam));
    case WM_SIZE:
        Edit_View_OnSize(lpev, (UINT)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));
    return (0);
    case WM_VSCROLL:
        Edit_View_OnVScroll(lpev, (UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HWND)lParam);
    return (0);
    case WM_SETCURSOR:
    return Edit_View_OnSetCursor(lpev, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
    case WM_LBUTTONDOWN:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        Edit_View_OnLButtonDown(lpev, (UINT)wParam, pt);
    }
    return (0);
    case WM_SETFOCUS:
        Edit_View_OnSetFocus(lpev, (HWND)lParam);
    return (0);
    case WM_HSCROLL:
        Edit_View_OnHScroll(lpev, (UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HWND)lParam);
    return (0);
    case WM_LBUTTONUP:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        Edit_View_OnLButtonUp(lpev, (UINT)wParam, pt);
    }
    return (0);
    case WM_MOUSEMOVE:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        Edit_View_OnMouseMove(lpev, (UINT)wParam, pt);
    }
    break;
    case WM_TIMER:
        Edit_View_OnTimer(lpev, (UINT)wParam);
    return (0);
    case WM_KILLFOCUS:
        Edit_View_OnKillFocus(lpev, (HWND)wParam);
    return (0);
    case WM_LBUTTONDBLCLK:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        Edit_View_OnLButtonDblClk(lpev, (UINT)wParam, pt);
    }
    return (0);
    case WM_RBUTTONDOWN:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        Edit_View_OnRButtonDown(lpev, (UINT)wParam, pt);
    }
    return (0);
    case WM_SYSCOLORCHANGE:
        Edit_View_OnSysColorChange(lpev);
    break;
    case WM_PAINT:
        Edit_View_OnPaint(lpev);
    return (0);
    case WM_CHAR:
        Edit_View_OnChar(lpev, (UINT)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
    return (0);
    case WM_KEYDOWN:
        Edit_View_OnKeyDown(lpev, (UINT)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
    return (0);
    case WM_MOUSEACTIVATE:
        if (!lpev->bFocused)
        {
            SetFocus(hwnd);
            return (MA_ACTIVATE);
        }
    break;
    case WM_COPY:
        Edit_View_OnCopy(lpev);
    break;
    case WM_PASTE:
        Edit_View_OnPaste(lpev);
    break;
    case WM_CUT:
        Edit_View_OnCut(lpev);
    break;
    case WM_CLEAR:
        Edit_View_OnClear(lpev);
    break;
    case WM_SETFONT:
    {
        LOGFONT lf;

        GetObject((HFONT)wParam, sizeof(LOGFONT), &lf);

        Edit_View_SetFont(lpev, &lf);
    }
    return (0);
    case WM_MOUSEWHEEL:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };

        Edit_View_OnMouseWheel(lpev, (UINT)LOWORD(wParam), (short)HIWORD(wParam), pt);
    }
    break;
    default:
    break;
    }

    return (PCP_Edit_Interface_HandleMessage(lpev, uMsg, wParam, lParam));
}

void Edit_View_GetSelection(LPEDITVIEW lpev, POINT *lptStart, POINT *lptEnd)
{
    Edit_View_PrepareSelBounds(lpev);

    lptStart->x = lpev->ptDrawSelStart.x;
    lptStart->y = lpev->ptDrawSelStart.y;
    lptEnd->x   = lpev->ptDrawSelEnd.x;
    lptEnd->y   = lpev->ptDrawSelEnd.y;
}

int Edit_View_GetLineActualLength(LPEDITVIEW lpev, int iLineIndex)
{
    int iLineCount      = Edit_Buffer_GetLineCount(lpev->lpes);
    int iActualLength   = 0;
    int iLength         = Edit_Buffer_GetLineLength(lpev->lpes, iLineIndex);

    ASSERT(iLineCount > 0);
    ASSERT(iLineIndex >= 0 && iLineIndex < iLineCount);
    
    if (lpev->piActualLineLength == NULL)
    {
        lpev->piActualLineLength = (LPINT)Mem_Alloc(iLineCount * sizeof(int));
        memset(lpev->piActualLineLength, 0xFF, sizeof(int) * iLineCount);
        lpev->iActualLengthArraySize = iLineCount;
    }

    if (lpev->piActualLineLength[iLineIndex] >= 0)
        return lpev->piActualLineLength[iLineIndex];

    //  Actual line length is not determined yet, let's calculate a little
    if (iLength > 0)
    {
        LPCTSTR pszLine = Edit_Buffer_GetLineChars(lpev->lpes, iLineIndex);
        LPTSTR pszChars = (LPTSTR)_alloca(sizeof(TCHAR) * (iLength + SZ));
        LPTSTR pszCurrent;
        int iTabSize;
        memcpy(pszChars, pszLine, sizeof(TCHAR) * iLength);
        pszChars[iLength] = 0;
        pszCurrent = pszChars;

        iTabSize = Edit_View_GetTabSize(lpev);

        for (;;)
        {
            LPTSTR psz = _tcschr(pszCurrent, _T('\t'));

            if (psz == NULL)
            {
                iActualLength += (pszChars + iLength - pszCurrent);
                break;
            }

            iActualLength += (psz - pszCurrent);
            iActualLength += (iTabSize - iActualLength % iTabSize);
            pszCurrent = psz + 1;
        }
    }

    lpev->piActualLineLength[iLineIndex] = iActualLength;

    return (iActualLength);
}

void Edit_View_ScrollToChar(LPEDITVIEW lpev, int iNewOffsetChar, BOOL bNoSmoothScroll /*= FALSE*/, BOOL bTrackScrollBar /*= TRUE*/)
{
    //  For now, ignoring bNoSmoothScroll and m_bSmoothScroll
    if (lpev->iOffsetChar != iNewOffsetChar)
    {
        RECT rcScroll;
        int iScrollChars = lpev->iOffsetChar - iNewOffsetChar;

        lpev->iOffsetChar = iNewOffsetChar;
        GetClientRect(lpev->hwnd, &rcScroll);
        rcScroll.left += Edit_View_GetMarginWidth(lpev);
        
        ScrollWindow(lpev->hwnd, iScrollChars * Edit_View_GetCharWidth(lpev),
                    0, &rcScroll, &rcScroll);
        UpdateWindow(lpev->hwnd);
        
        if (bTrackScrollBar)
            Edit_View_RecalcHorzScrollBar(lpev, TRUE);
    }
}

void Edit_View_ScrollToLine(LPEDITVIEW lpev, int iNewTopLine, BOOL bNoSmoothScroll /*= FALSE*/, BOOL bTrackScrollBar /*= TRUE*/)
{
    if (lpev->iTopLine != iNewTopLine)
    {
        if (bNoSmoothScroll || !lpev->bSmoothScroll)
        {
            int iScrollLines = lpev->iTopLine - iNewTopLine;
            
            lpev->iTopLine = iNewTopLine;
            
            ScrollWindow(lpev->hwnd, 0, iScrollLines * Edit_View_GetLineHeight(lpev), NULL, NULL);
            UpdateWindow(lpev->hwnd);
            
            if (bTrackScrollBar)
                Edit_View_RecalcVertScrollBar(lpev, TRUE);
        }
        else
        {
            //  Do smooth scrolling
            int iLineHeight = Edit_View_GetLineHeight(lpev);

            if (lpev->iTopLine > iNewTopLine)
            {
                int iIncrement = (lpev->iTopLine - iNewTopLine) / SMOOTH_SCROLL_FACTOR + 1;
                
                while (lpev->iTopLine != iNewTopLine)
                {
                    int iTopLine = lpev->iTopLine - iIncrement;
                    int iScrollLines;

                    if (iTopLine < iNewTopLine)
                        iTopLine = iNewTopLine;

                    iScrollLines = iTopLine - lpev->iTopLine;
                    lpev->iTopLine = iTopLine;
                    
                    ScrollWindow(lpev->hwnd, 0, - iLineHeight * iScrollLines, NULL, NULL);
                    UpdateWindow(lpev->hwnd);
                    
                    if (bTrackScrollBar)
                        Edit_View_RecalcVertScrollBar(lpev, TRUE);
                }
            }
            else
            {
                int iIncrement = (iNewTopLine - lpev->iTopLine) / SMOOTH_SCROLL_FACTOR + 1;
                
                while (lpev->iTopLine != iNewTopLine)
                {
                    int iTopLine = lpev->iTopLine + iIncrement;
                    int iScrollLines;

                    if (iTopLine > iNewTopLine)
                        iTopLine = iNewTopLine;

                    iScrollLines = iTopLine - lpev->iTopLine;
                    lpev->iTopLine = iTopLine;

                    ScrollWindow(lpev->hwnd, 0, - iLineHeight * iScrollLines, NULL, NULL);
                    UpdateWindow(lpev->hwnd);
                    
                    if (bTrackScrollBar)
                        Edit_View_RecalcVertScrollBar(lpev, TRUE);
                }
            }
        }
    }
}

void Edit_View_ExpandChars(LPEDITVIEW lpev, LPCTSTR pszChars, int iOffset, int iCount, LPTSTR *pszLine)
{
    int iTabSize = Edit_View_GetTabSize(lpev);
    int iActualOffset = 0;
    int i;
    int iLength;
    int iTabCount;
    LPTSTR pszBuf;
    int iCurPos;
    
    if (iCount <= 0)
    {
        *pszLine = _T("");

        return;
    }

    for (i = 0; i < iOffset; i++)
    {
        if (pszChars[i] == _T('\t'))
            iActualOffset += (iTabSize - iActualOffset % iTabSize);
        else
            iActualOffset++;
    }

    pszChars += iOffset;
    iLength = iCount;

    iTabCount = 0;

    for (i = 0; i < iLength; i++)
    {
        if (pszChars[i] == _T('\t'))
            iTabCount++;
    }

    pszBuf = *pszLine = (LPTSTR)Mem_AllocStr(iLength + iTabCount * (iTabSize - 1) + 1);
    iCurPos = 0;
    
    if (iTabCount > 0 || lpev->bViewTabs)
    {
        for (i = 0; i < iLength; i++)
        {
            if (pszChars[i] == _T('\t'))
            {
                int iSpaces = iTabSize - (iActualOffset + iCurPos) % iTabSize;
                
                if (lpev->bViewTabs)
                {
                    pszBuf[iCurPos++] = TAB_CHARACTER;
                    iSpaces--;
                }

                while (iSpaces > 0)
                {
                    pszBuf[iCurPos++] = _T(' ');
                    iSpaces--;
                }
            }
            else
            {
                if (pszChars[i] == _T(' ') && lpev->bViewTabs)
                {
                    pszBuf[iCurPos] = SPACE_CHARACTER;
                }
                else
                {
                    pszBuf[iCurPos] = pszChars[i];
                }
                iCurPos++;
            }
        }
    }
    else
    {
        memcpy(pszBuf, pszChars, sizeof(TCHAR) * iLength);
        iCurPos = iLength;
    }

    pszBuf[iCurPos] = 0;
}

void Edit_View_DrawLineHelperImpl(LPEDITVIEW lpev, HDC hdc, LPPOINT lptOrigin, const RECT rcClip, LPCTSTR pszChars, int iOffset, int iCount)
{
    ASSERT(iCount >= 0);

    if (iCount > 0)
    {
        LPTSTR pszLine;
        int iWidth = rcClip.right - lptOrigin->x;

        Edit_View_ExpandChars(lpev, pszChars, iOffset, iCount, &pszLine);

        if (iWidth > 0)
        {
            int iCharWidth  = Edit_View_GetCharWidth(lpev);
            int iCount      = _tcslen(pszLine);
            int iCountFit   = iWidth / iCharWidth + 1;
            
            if (iCount > iCountFit)
                iCount = iCountFit;

            VERIFY(ExtTextOut(hdc, lptOrigin->x, lptOrigin->y,
                        ETO_CLIPPED, &rcClip, pszLine, iCount, NULL));
        }

        lptOrigin->x += Edit_View_GetCharWidth(lpev) * _tcslen(pszLine);

        Mem_Free(pszLine);
    }
}

void Edit_View_DrawLineHelper(LPEDITVIEW lpev, HDC hdc, LPPOINT lptOrigin, const LPRECT lprcClip, int iColorIndex, LPCTSTR pszChars, int iOffset, int iCount, POINT ptTextPos)
{
    if (iCount > 0)
    {
        if (lpev->bFocused || lpev->bShowInactiveSelection)
        {
            int iSelBegin = 0, iSelEnd = 0;
        
            if (lpev->ptDrawSelStart.y > ptTextPos.y)
            {
                iSelBegin = iCount;
            }
            else if (lpev->ptDrawSelStart.y == ptTextPos.y)
            {
                iSelBegin = lpev->ptDrawSelStart.x - ptTextPos.x;
                
                if (iSelBegin < 0)
                    iSelBegin = 0;
                if (iSelBegin > iCount)
                    iSelBegin = iCount;
            }

            if (lpev->ptDrawSelEnd.y > ptTextPos.y)
            {
                iSelEnd = iCount;
            }
            else if (lpev->ptDrawSelEnd.y == ptTextPos.y)
            {
                iSelEnd = lpev->ptDrawSelEnd.x - ptTextPos.x;
                
                if (iSelEnd < 0)
                    iSelEnd = 0;
                if (iSelEnd > iCount)
                    iSelEnd = iCount;
            }

            ASSERT(iSelBegin >= 0 && iSelBegin <= iCount);
            ASSERT(iSelEnd >= 0 && iSelEnd <= iCount);
            ASSERT(iSelBegin <= iSelEnd);

            //  Draw part of the text before selection
            if (iSelBegin > 0)
            {
                Edit_View_DrawLineHelperImpl(lpev, hdc, lptOrigin, *lprcClip, pszChars, iOffset, iSelBegin);
            }

            if (iSelBegin < iSelEnd)
            {
                COLORREF crOldBk = SetBkColor(hdc, Edit_View_GetColor(lpev, COLORINDEX_SELBKGND));
                COLORREF crOldText = SetTextColor(hdc, Edit_View_GetColor(lpev, COLORINDEX_SELTEXT));
                Edit_View_DrawLineHelperImpl(lpev, hdc, lptOrigin, *lprcClip, pszChars, iOffset + iSelBegin, iSelEnd - iSelBegin);
                SetBkColor(hdc, crOldBk);
                SetTextColor(hdc, crOldText);
            }
            
            if (iSelEnd < iCount)
            {
                Edit_View_DrawLineHelperImpl(lpev, hdc, lptOrigin, *lprcClip, pszChars, iOffset + iSelEnd, iCount - iSelEnd);
            }
        }
        else
        {
            Edit_View_DrawLineHelperImpl(lpev, hdc, lptOrigin, *lprcClip, pszChars, iOffset, iCount);
        }
    }
}

void Edit_View_GetLineColors(LPEDITVIEW lpev, int iLineIndex, LPCOLORREF lpcrBkgnd, LPCOLORREF lpcrText, LPBOOL lpbDrawWhitespace)
{
    DWORD dwLineFlags = Edit_Buffer_GetLineFlags(lpev->lpes, iLineIndex);

    *lpbDrawWhitespace = TRUE;
    *lpcrText = RGB(255, 255, 255);

    if (dwLineFlags & LF_EXECUTION)
    {
        *lpcrBkgnd = RGB(0, 128, 0);
        
        return;
    }
    
    if (dwLineFlags & LF_BREAKPOINT)
    {
        *lpcrBkgnd = RGB(255, 0, 0);
        
        return;
    }

    if (dwLineFlags & LF_INVALID_BREAKPOINT)
    {
        *lpcrBkgnd = RGB(128, 128, 0);
        return;
    }

    *lpcrBkgnd = CLR_NONE;
    *lpcrText = CLR_NONE;
    *lpbDrawWhitespace = FALSE;
}

DWORD Edit_View_GetParseCookie(LPEDITVIEW lpev, int iLineIndex)
{
    int iLineCount = Edit_Buffer_GetLineCount(lpev->lpes);
    int iCnt;
    int iBlocks;

    if (lpev->pdwParseCookies == NULL)
    {
        lpev->iParseArraySize = iLineCount;
        lpev->pdwParseCookies = (PDWORD)Mem_Alloc(iLineCount * sizeof(DWORD));
        memset(lpev->pdwParseCookies, 0xFF, iLineCount * sizeof(DWORD));
    }

    if (iLineIndex < 0)
        return 0;
    
    if (lpev->pdwParseCookies[iLineIndex] != (DWORD)-1)
        return lpev->pdwParseCookies[iLineIndex];

    iCnt = iLineIndex;
    
    while (iCnt >= 0 && lpev->pdwParseCookies[iCnt] == (DWORD)-1)
        iCnt--;
    iCnt++;

    while (iCnt <= iLineIndex)
    {
        DWORD dwCookie = 0;

        if (iCnt > 0)
            dwCookie = lpev->pdwParseCookies[iCnt - 1];

        ASSERT(dwCookie != (DWORD)-1);
        lpev->pdwParseCookies[iCnt] = Edit_View_ParseLine(lpev, dwCookie, iCnt, NULL, &iBlocks);
        ASSERT(lpev->pdwParseCookies[iCnt] != (DWORD)-1);
        iCnt++;
    }

    return lpev->pdwParseCookies[iLineIndex];
}

void Edit_View_DrawSingleLine(LPEDITVIEW lpev, HDC *fdc, const LPRECT lprc, int iLineIndex)
{
    BOOL bDrawWhitespace = FALSE;
    COLORREF crBkgnd, crText, crOldBkgnd, crOldText;
    int iLength;
    LPCTSTR pszChars;
    DWORD dwCookie;
    TEXTBLOCK *pBuf;
    int iBlocks = 0;
    POINT origin;
    RECT frect;

    ASSERT(iLineIndex >= -1 && iLineIndex < Edit_View_GetLineCount(lpev));

    if (iLineIndex == -1)
    {
        //  Draw line beyond the text
        Paint_FillRect(*fdc, lprc, Edit_View_GetColor(lpev, COLORINDEX_WHITESPACE));

        return;
    }

    //  Acquire the background color for the current line
    Edit_View_GetLineColors(lpev, iLineIndex, &crBkgnd, &crText, &bDrawWhitespace);
    if (crBkgnd == CLR_NONE)
        crBkgnd = Edit_View_GetColor(lpev, COLORINDEX_BKGND);

    iLength = Edit_View_GetLineLength(lpev, iLineIndex);

    if (iLength == 0)
    {
        //  Draw the empty line
        RECT rect = *lprc;
        POINT p = { 0, iLineIndex };
        
        if ((lpev->bFocused || lpev->bShowInactiveSelection) &&
            Edit_View_IsInsideSelBlock(lpev, p))
        {
            //FIXME: does this work?
            rect.right += Edit_View_GetCharWidth(lpev);
            Paint_FillRect(*fdc, &rect, Edit_View_GetColor(lpev, COLORINDEX_SELBKGND));
            rect.left += Edit_View_GetCharWidth(lpev);
        }

        Paint_FillRect(*fdc, &rect, bDrawWhitespace ? crBkgnd : Edit_View_GetColor(lpev, COLORINDEX_WHITESPACE));

        return;
    }

    // Parse the line
    pszChars = Edit_Buffer_GetLineChars(lpev->lpes, iLineIndex);
    dwCookie = Edit_View_GetParseCookie(lpev, iLineIndex - 1);
    pBuf = (LPTEXTBLOCK)_alloca(sizeof(TEXTBLOCK) * iLength * 3);

    lpev->pdwParseCookies[iLineIndex] = Edit_View_ParseLine(lpev, dwCookie, iLineIndex, pBuf, &iBlocks);
    ASSERT(lpev->pdwParseCookies[iLineIndex] != (DWORD)-1);

    //  Draw the line text
    origin.x = lprc->left - lpev->iOffsetChar * Edit_View_GetCharWidth(lpev);
    origin.y = lprc->top;

    crOldBkgnd = SetBkColor(*fdc, crBkgnd);
    if (crText != CLR_NONE)
        crOldText = SetTextColor(*fdc, crText);

    if (iBlocks > 0)
    {
        POINT pDLH = { 0, iLineIndex };
        int i;
        
        ASSERT(pBuf[0].iCharPos >= 0 && pBuf[0].iCharPos <= iLength);
        
        if (crText == CLR_NONE)
            crOldText = SetTextColor(*fdc, Edit_View_GetColor(lpev, COLORINDEX_NORMALTEXT));

        SelectObject(*fdc,
                    Edit_View_GetFont(lpev, 
                        Edit_View_GetItalic(COLORINDEX_NORMALTEXT),
                            Edit_View_GetBold(COLORINDEX_NORMALTEXT)));
        
        Edit_View_DrawLineHelper(lpev, *fdc, &origin, lprc, COLORINDEX_NORMALTEXT,
            pszChars, 0, pBuf[0].iCharPos, pDLH);
        
        for (i = 0; i < (iBlocks - 1); i++)
        {
            ASSERT(pBuf[i].iCharPos >= 0 && pBuf[i].iCharPos <= iLength);
            
            if (crText == CLR_NONE)
                SetTextColor(*fdc, Edit_View_GetColor(lpev, pBuf[i].iColorIndex));

            SelectObject(*fdc, Edit_View_GetFont(lpev, 
                                Edit_View_GetItalic(pBuf[i].iColorIndex),
                                    Edit_View_GetBold(pBuf[i].iColorIndex)));

            pDLH.x = pBuf[i].iCharPos;
            pDLH.y = iLineIndex;
            
            Edit_View_DrawLineHelper(lpev, *fdc, &origin, lprc, pBuf[i].iColorIndex, pszChars,
                            pBuf[i].iCharPos, pBuf[i + 1].iCharPos - pBuf[i].iCharPos,
                            pDLH);
        }

        ASSERT(pBuf[iBlocks - 1].iCharPos >= 0 &&
            pBuf[iBlocks - 1].iCharPos <= iLength);
        
        if (crText == CLR_NONE)
            SetTextColor(*fdc, Edit_View_GetColor(lpev, pBuf[iBlocks - 1].iColorIndex));
        
        SelectObject(*fdc, Edit_View_GetFont(lpev, 
                            Edit_View_GetItalic(pBuf[iBlocks - 1].iColorIndex),
                            Edit_View_GetBold(pBuf[iBlocks - 1].iColorIndex)));

        pDLH.x = pBuf[iBlocks - 1].iCharPos;
        pDLH.y = iLineIndex;

        Edit_View_DrawLineHelper(lpev, *fdc, &origin, lprc, pBuf[iBlocks - 1].iColorIndex, pszChars,
                            pBuf[iBlocks - 1].iCharPos, iLength - pBuf[iBlocks - 1].iCharPos,
                            pDLH);
    }
    else
    {
        POINT pDLH = { 0, iLineIndex };

        if (crText == CLR_NONE)
            SetTextColor(*fdc, Edit_View_GetColor(lpev, COLORINDEX_NORMALTEXT));

        SelectObject(*fdc, Edit_View_GetFont(lpev, 
                            Edit_View_GetItalic(COLORINDEX_NORMALTEXT),
                            Edit_View_GetBold(COLORINDEX_NORMALTEXT)));
        Edit_View_DrawLineHelper(lpev, *fdc, &origin, lprc, COLORINDEX_NORMALTEXT,
            pszChars, 0, iLength, pDLH);
    }

    //  Draw whitespaces to the left of the text
    frect = *lprc;

    if (origin.x > frect.left)
        frect.left = origin.x;

    if (frect.right > frect.left)
    {
        POINT p = { iLength, iLineIndex };

        if ((lpev->bFocused || lpev->bShowInactiveSelection) &&
            Edit_View_IsInsideSelBlock(lpev, p))
        {
            RECT rcTemp = frect;
            rcTemp.right += Edit_View_GetCharWidth(lpev);
            Paint_FillRect(*fdc, &rcTemp, Edit_View_GetColor(lpev, COLORINDEX_SELBKGND));

            frect.left += Edit_View_GetCharWidth(lpev);
        }

        if (frect.right > frect.left)
            Paint_FillRect(*fdc, &frect, bDrawWhitespace ? crBkgnd : Edit_View_GetColor(lpev, COLORINDEX_WHITESPACE));
    }
}

COLORREF Edit_View_GetColor(LPEDITVIEW lpev, int iColorIndex)
{
    switch (iColorIndex)
    {
    case COLORINDEX_WHITESPACE:
        return ((lpev->lptdCurSourceDef->crWhiteSpace == -1) ?
                    GetSysColor(COLOR_WINDOW) :
                    lpev->lptdCurSourceDef->crWhiteSpace);
    case COLORINDEX_BKGND:
        return ((lpev->lptdCurSourceDef->crBackground == -1) ?
                    GetSysColor(COLOR_WINDOW) :
                    lpev->lptdCurSourceDef->crBackground);
    case COLORINDEX_NORMALTEXT:
        return ((lpev->lptdCurSourceDef->crNormalText == -1) ?
                    GetSysColor(COLOR_WINDOWTEXT) :
                    lpev->lptdCurSourceDef->crNormalText);
    case COLORINDEX_SELMARGIN:
        return ((lpev->lptdCurSourceDef->crSelMargin == -1) ?
                    GetSysColor(COLOR_SCROLLBAR) :
                    lpev->lptdCurSourceDef->crSelMargin);
    case COLORINDEX_PREPROCESSOR:
        return ((lpev->lptdCurSourceDef->crPreprocessor == -1) ?
                    RGB(0, 128, 192) :
                    lpev->lptdCurSourceDef->crPreprocessor);
    case COLORINDEX_COMMENT:
        return ((lpev->lptdCurSourceDef->crComment == -1) ?
                    RGB(0, 128, 0) :
                    lpev->lptdCurSourceDef->crComment);
    case COLORINDEX_NUMBER:
        return ((lpev->lptdCurSourceDef->crNumber == -1) ?
                    RGB(128, 0, 0) :
                    lpev->lptdCurSourceDef->crNumber);
    case COLORINDEX_OPERATOR:
        return ((lpev->lptdCurSourceDef->crOperator == -1) ?
                    RGB(96, 96, 96) :
                    lpev->lptdCurSourceDef->crOperator);
    case COLORINDEX_PREFIXED:
        return ((lpev->lptdCurSourceDef->crPrefixed == -1) ?
                GetSysColor(COLOR_WINDOWTEXT) :
                lpev->lptdCurSourceDef->crPrefixed);
    case COLORINDEX_KEYWORD:
        return ((lpev->lptdCurSourceDef->crKeyword == -1) ?
                    RGB(0, 0, 255) :
                    lpev->lptdCurSourceDef->crKeyword);
    case COLORINDEX_FUNCNAME:
        return ((lpev->lptdCurSourceDef->crFunctionName == -1) ?
                    RGB(128, 0, 128) :
                    lpev->lptdCurSourceDef->crFunctionName);
    case COLORINDEX_USER1:
        return ((lpev->lptdCurSourceDef->crUser1 == -1) ?
                    RGB(0, 0, 128) :
                    lpev->lptdCurSourceDef->crUser1);
    case COLORINDEX_USER2:
        return ((lpev->lptdCurSourceDef->crUser2 == -1) ?
                    RGB(0, 128, 192) :
                    lpev->lptdCurSourceDef->crUser2);
    case COLORINDEX_SELBKGND:
        return ((lpev->lptdCurSourceDef->crSelBackground == -1) ?
                    GetSysColor(COLOR_HIGHLIGHT) :
                    lpev->lptdCurSourceDef->crSelBackground);
    case COLORINDEX_SELTEXT:
        return ((lpev->lptdCurSourceDef->crSelText == -1) ?
                    GetSysColor(COLOR_HIGHLIGHTTEXT) :
                    lpev->lptdCurSourceDef->crSelText);
    }

    return (RGB(128, 0, 0));
}

DWORD Edit_View_GetLineFlags(LPEDITVIEW lpev, int iLineIndex)
{
    if (lpev->lpes == NULL)
        return 0;
    return Edit_Buffer_GetLineFlags(lpev->lpes, iLineIndex);
}

void Edit_View_DrawMargin(LPEDITVIEW lpev, HDC *fdc, const LPRECT lprc, int iLineIndex)
{
    int iImageIndex = -1;

    if (!lpev->bSelMargin)
    {
        Paint_FillRect(*fdc, lprc, Edit_View_GetColor(lpev, COLORINDEX_BKGND));
        
        return;
    }

    Paint_FillRect(*fdc, lprc, Edit_View_GetColor(lpev, COLORINDEX_SELMARGIN));

    if (iLineIndex >= 0)
    {
        DWORD dwLineFlags = Edit_Buffer_GetLineFlags(lpev->lpes, iLineIndex);

        static const DWORD adwFlags[] =
        {
            LF_EXECUTION,
            LF_BREAKPOINT,
            LF_COMPILATION_ERROR,
            LF_BOOKMARK(1),
            LF_BOOKMARK(2),
            LF_BOOKMARK(3),
            LF_BOOKMARK(4),
            LF_BOOKMARK(5),
            LF_BOOKMARK(6),
            LF_BOOKMARK(7),
            LF_BOOKMARK(8),
            LF_BOOKMARK(9),
            LF_BOOKMARK(0),
            LF_BOOKMARKS,
            LF_INVALID_BREAKPOINT
        };
        int i;

        for (i = 0; i <= sizeof(adwFlags) / sizeof(adwFlags[0]); i++)
        {
            if ((dwLineFlags & adwFlags[i]) != 0)
            {
                iImageIndex = i;
                break;
            }
        }
    }

    if (iImageIndex >= 0)
    {
        POINT pt;

        if (lpev->hilIcons == NULL)
        { 
//          lpev->pIcons = (IMAGELIST)Mem_Alloc(sizeof(IMAGELIST));
            VERIFY(lpev->hilIcons = ImageList_Create(16, 12, ILC_COLOR24 | ILC_MASK, 3, 0));
            VERIFY(ImageList_AddMasked(lpev->hilIcons, LoadBitmap(PCP_Edit_GetHandle(), MAKEINTRESOURCE(IDB_MARGIN_ICONS)), RGB(255, 255, 255)) != -1);
        }

        pt.x = lprc->left + 2;
        //FIXME: Right?
//      pt.y = lprc->top + (Edit_View_GetLineHeight(lpev) - 16 / 2);
        pt.y = lprc->top + (Rect_Height(&*lprc) - 12) / 2;
        VERIFY(ImageList_Draw(lpev->hilIcons, iImageIndex, *fdc, pt.x, pt.y, ILD_TRANSPARENT));
    }
}

BOOL Edit_View_IsInsideSelBlock(LPEDITVIEW lpev, POINT ptTextPos)
{
    ASSERT_VALIDTEXTPOS(lpev, ptTextPos);

    if (ptTextPos.y < lpev->ptDrawSelStart.y)
        return FALSE;
    if (ptTextPos.y > lpev->ptDrawSelEnd.y)
        return FALSE;
    if (ptTextPos.y < lpev->ptDrawSelEnd.y && ptTextPos.y > lpev->ptDrawSelStart.y)
        return TRUE;
    if (lpev->ptDrawSelStart.y < lpev->ptDrawSelEnd.y)
    {
        if (ptTextPos.y == lpev->ptDrawSelEnd.y)
            return (ptTextPos.x < lpev->ptDrawSelEnd.x);
        ASSERT(ptTextPos.y == lpev->ptDrawSelStart.y);
        return (ptTextPos.x >= lpev->ptDrawSelStart.x);
    }
    ASSERT(lpev->ptDrawSelStart.y == lpev->ptDrawSelEnd.y);
    return (ptTextPos.x >= lpev->ptDrawSelStart.x && ptTextPos.x < lpev->ptDrawSelEnd.x);
}

BOOL Edit_View_IsInsideSelection(LPEDITVIEW lpev, const POINT ptTextPos)
{
    Edit_View_PrepareSelBounds(lpev);
    return (Edit_View_IsInsideSelBlock(lpev, ptTextPos));
}

void Edit_View_PrepareSelBounds(LPEDITVIEW lpev)
{
    if (lpev->ptSelStart.y < lpev->ptSelEnd.y ||
            (lpev->ptSelStart.y == lpev->ptSelEnd.y &&
            lpev->ptSelStart.x < lpev->ptSelEnd.x))
    {
        lpev->ptDrawSelStart = lpev->ptSelStart;
        lpev->ptDrawSelEnd = lpev->ptSelEnd;
    }
    else
    {
        lpev->ptDrawSelStart    = lpev->ptSelEnd;
        lpev->ptDrawSelEnd      = lpev->ptSelStart;
    }
}

void Edit_View_OnPaint(LPEDITVIEW lpev)
{
    int iLineCount;
    int iLineHeight;
    RECT rcClient;
    HDC cacheDC;
    HBITMAP hOldBitmap;
    RECT rcLine;
    RECT rcCacheMargin;
    RECT rcCacheLine;
    int iCurrentLine;
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(lpev->hwnd, &ps);

    GetClientRect(lpev->hwnd, &rcClient);

    iLineCount  = Edit_View_GetLineCount(lpev);
    iLineHeight = Edit_View_GetLineHeight(lpev);
    Edit_View_PrepareSelBounds(lpev);

    VERIFY(cacheDC = CreateCompatibleDC(hdc));

    if (lpev->pCacheBitmap == NULL)
    {
        lpev->pCacheBitmap = (HBITMAP *)Mem_Alloc(sizeof(HBITMAP));
        VERIFY(*lpev->pCacheBitmap = CreateCompatibleBitmap(hdc,
                                        Rect_Width(&rcClient), iLineHeight));
    }

    hOldBitmap = SelectObject(cacheDC, *lpev->pCacheBitmap);

    rcLine = ps.rcPaint; //rcClient;
    rcLine.left = rcClient.left;
    rcLine.right = rcClient.right;
    // Make sure that top isn't below a line
    rcLine.top -= (rcLine.top % iLineHeight);
    rcLine.bottom = rcLine.top + iLineHeight;
    
    SetRect(&rcCacheMargin, 0, 0, Edit_View_GetMarginWidth(lpev), iLineHeight);
    SetRect(&rcCacheLine, Edit_View_GetMarginWidth(lpev), 0, Rect_Width(&rcLine), iLineHeight);

    iCurrentLine = lpev->iTopLine + (rcLine.top / iLineHeight); //lpev->iTopLine;

    while (rcLine.top < ps.rcPaint.bottom)
    {
        if (iCurrentLine < iLineCount)
        {
            Edit_View_DrawMargin(lpev, &cacheDC, &rcCacheMargin, iCurrentLine);
            Edit_View_DrawSingleLine(lpev, &cacheDC, &rcCacheLine, iCurrentLine);
        }
        else
        {
            Edit_View_DrawMargin(lpev, &cacheDC, &rcCacheMargin, -1);
            Edit_View_DrawSingleLine(lpev, &cacheDC, &rcCacheLine, -1);
        }

        VERIFY(BitBlt(hdc, rcLine.left, rcLine.top,
                    Rect_Width(&rcLine), Rect_Height(&rcLine),
                    cacheDC, 0, 0, SRCCOPY));

        iCurrentLine++;
        OffsetRect(&rcLine, 0, iLineHeight);
    }

    SelectObject(cacheDC, hOldBitmap);
    DeleteDC(cacheDC);

    EndPaint(lpev->hwnd, &ps);
}

void Edit_View_ResetView(LPEDITVIEW lpev)
{
    int i;

    lpev->bOvrMode          = FALSE;
    lpev->iTopLine          = 0;
    lpev->iOffsetChar       = 0;
    lpev->iLineHeight       = -1;
    lpev->iCharWidth        = -1;
    lpev->iMaxLineLength    = -1;
    lpev->iScreenLines      = -1;
    lpev->iScreenChars      = -1;
    lpev->iIdealCharPos     = -1;
    lpev->ptAnchor.x        = 0;
    lpev->ptAnchor.y        = 0;
    
    if (lpev->hilIcons != NULL)
    {
        ImageList_Destroy(lpev->hilIcons);
        lpev->hilIcons = NULL;
    }

    for (i = 0; i < 4; i++)
    {
        if (lpev->ahFonts[i] != NULL)
        {
            DeleteObject(lpev->ahFonts[i]);
            lpev->ahFonts[i] = NULL;
        }
    }

    if (lpev->pdwParseCookies != NULL)
    {
        Mem_Free(lpev->pdwParseCookies);
        lpev->pdwParseCookies = NULL;
    }

    if (lpev->piActualLineLength != NULL)
    {
        Mem_Free(lpev->piActualLineLength);
        lpev->piActualLineLength = NULL;
    }

    lpev->iParseArraySize           = 0;
    lpev->iActualLengthArraySize    = 0;
    lpev->ptCursorPos.x             = 0;
    lpev->ptCursorPos.y             = 0;
    lpev->ptSelStart                = lpev->ptCursorPos;
    lpev->ptSelEnd                  = lpev->ptCursorPos;
    lpev->bDragSelection            = FALSE;
    lpev->bVertScrollBarLocked      = FALSE;
    lpev->bHorzScrollBarLocked      = FALSE;

    if (IsWindow(lpev->hwnd))
        Edit_View_UpdateCaret(lpev);

    lpev->bLastSearch               = FALSE;
    lpev->bPrintHeader              = FALSE;
    lpev->bPrintFooter              = TRUE;
    lpev->bShowInactiveSelection    = TRUE;

    lpev->bBookmarkExist            = FALSE;    // More bookmarks
    lpev->bMultipleSearch           = FALSE;    // More search

    // Unnecessary
    lpev->pszMatched                = NULL;
    lpev->pszLastFindWhat           = NULL;
    lpev->pszIncrementalSearch      = NULL;
}

#ifdef _DEBUG
void Edit_View_AssertValidTextPos(LPEDITVIEW lpev, POINT pt)
{
    if (Edit_View_GetLineCount(lpev) > 0)
    {
        ASSERT(lpev->iTopLine >= 0 && lpev->iOffsetChar >= 0);
        ASSERT(pt.y >= 0 && pt.y <= Edit_View_GetLineCount(lpev));
        ASSERT(pt.x >= 0 && pt.x <= Edit_View_GetLineLength(lpev, pt.y));
    }
}
#endif

void Edit_View_UpdateCaret(LPEDITVIEW lpev)
{
    ASSERT_VALIDTEXTPOS(lpev, lpev->ptCursorPos);

    if (lpev->bFocused && !lpev->bCursorHidden &&
        Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y,
                        lpev->ptCursorPos.x) >= lpev->iOffsetChar)
    {
        POINT pt;

        if (lpev->bOvrMode)
            CreateCaret(lpev->hwnd, NULL, Edit_View_GetCharWidth(lpev), Edit_View_GetLineHeight(lpev));
        else
            CreateCaret(lpev->hwnd, NULL, 2, Edit_View_GetLineHeight(lpev));
        
        pt = Edit_View_TextToClient(lpev, lpev->ptCursorPos);
        SetCaretPos(pt.x, pt.y);
        ShowCaret(lpev->hwnd);
    }
    else
    {
        HideCaret(lpev->hwnd);
    }
}

int Edit_View_GetTabSize(LPEDITVIEW lpev)
{
    ASSERT(lpev->iTabSize >= 0 && lpev->iTabSize <= 64);

    return (lpev->iTabSize);
}

void Edit_View_SetTabSize(LPEDITVIEW lpev, int iTabSize)
{
    ASSERT(iTabSize >= 0 && iTabSize <= 64);

    if (lpev->iTabSize != iTabSize)
    {
        lpev->iTabSize = iTabSize;
        
        if (lpev->piActualLineLength != NULL)
        {
            Mem_Free(lpev->piActualLineLength);
            lpev->piActualLineLength = NULL;
        }

        lpev->iActualLengthArraySize    = 0;
        lpev->iMaxLineLength            = -1;
        
        Edit_View_RecalcHorzScrollBar(lpev, FALSE);
        InvalidateRect(lpev->hwnd, NULL, FALSE);
        Edit_View_UpdateCaret(lpev);
    }
}

HFONT Edit_View_GetFont(LPEDITVIEW lpev, BOOL bItalic, BOOL bBold)
{
    int iIndex = 0;

    if (bBold)
        iIndex |= 1;
    if (bItalic)
        iIndex |= 2;

    if (lpev->ahFonts[iIndex] == NULL)
    {
//      lpev->ahFonts[iIndex] = new CFont;
        lpev->lfBaseFont.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
        lpev->lfBaseFont.lfItalic = (BYTE)bItalic;
        if ((lpev->ahFonts[iIndex] = CreateFontIndirect(&lpev->lfBaseFont)) == NULL)
        {
//          delete m_apFonts[nIndex];
//          m_apFonts[nIndex] = NULL;
//          return CView_GetFont();
        }

    }
    return (lpev->ahFonts[iIndex]);
}

void Edit_View_CalcLineCharDim(LPEDITVIEW lpev)
{
    HDC hdc         = GetDC(lpev->hwnd);
    HFONT hOldFont  = SelectObject(hdc, Edit_View_GetFont(lpev, FALSE, FALSE));
    SIZE sCharExt;
    
    GetTextExtentPoint(hdc, _T("X"), 1, &sCharExt);
    
    lpev->iLineHeight = sCharExt.cy;
    
    if (lpev->iLineHeight < 1)
        lpev->iLineHeight = 1;

    lpev->iCharWidth = sCharExt.cx;

    SelectObject(hdc, hOldFont);
    ReleaseDC(lpev->hwnd, hdc);
}

int Edit_View_GetLineHeight(LPEDITVIEW lpev)
{
    if (lpev->iLineHeight == -1)
        Edit_View_CalcLineCharDim(lpev);

    ASSERT(lpev->iLineHeight != 0);
    ASSERT(lpev->iCharWidth != 0);

    return (lpev->iLineHeight);
}

int Edit_View_GetCharWidth(LPEDITVIEW lpev)
{
    if (lpev->iCharWidth == -1)
        Edit_View_CalcLineCharDim(lpev);

    ASSERT(lpev->iLineHeight != 0);
    ASSERT(lpev->iCharWidth != 0);

    return lpev->iCharWidth;
}

int Edit_View_GetMaxLineLength(LPEDITVIEW lpev)
{

    if (lpev->iMaxLineLength == -1)
    {
        int iLineCount = Edit_View_GetLineCount(lpev);
        int i;
        
        lpev->iMaxLineLength = 0;

        for (i = 0; i < iLineCount; i++)
        {
            int iActualLength = Edit_View_GetLineActualLength(lpev, i);
            
            if (lpev->iMaxLineLength < iActualLength)
                lpev->iMaxLineLength = iActualLength;
        }
    }

    return (lpev->iMaxLineLength);
}

int Edit_View_GetLineCount(LPEDITVIEW lpev)
{
    if (lpev->lpes == NULL)
    {
        return (1);     //  Single empty line
    }
    else
    {
        int iLineCount = Edit_Buffer_GetLineCount(lpev->lpes);
        
        ASSERT(iLineCount > 0);

        return (iLineCount);
    }
}

int Edit_View_GetLineLength(LPEDITVIEW lpev, int iLineIndex)
{
    if (lpev->lpes == NULL)
        return (0);

    return (Edit_Buffer_GetLineLength(lpev->lpes, iLineIndex));
}

LPCTSTR Edit_View_GetLineChars(LPEDITVIEW lpev, int iLineIndex)
{
    if (lpev->lpes == NULL)
        return (NULL);

    return (Edit_Buffer_GetLineChars(lpev->lpes, iLineIndex));
}

void Edit_View_AttachToBuffer(LPEDITVIEW lpev, LPEDITSTATE lpes)
{
    lpev->lpes = lpes;
}

int Edit_View_GetScreenLines(LPEDITVIEW lpev)
{
    if (lpev->iScreenLines == -1)
    {
        RECT rc;
        GetClientRect(lpev->hwnd, &rc);
        lpev->iScreenLines = Rect_Height(&rc) / Edit_View_GetLineHeight(lpev);
    }

    return (lpev->iScreenLines);
}

BOOL Edit_View_GetItalic(int iColorIndex)
{
    return (FALSE);
}

BOOL Edit_View_GetBold(int iColorIndex)
{
    return (FALSE);
}

int Edit_View_GetScreenChars(LPEDITVIEW lpev)
{
    if (lpev->iScreenChars == -1)
    {
        RECT rc;
        GetClientRect(lpev->hwnd, &rc);
        lpev->iScreenChars = (Rect_Width(&rc) - Edit_View_GetMarginWidth(lpev)) / Edit_View_GetCharWidth(lpev);
    }

    return lpev->iScreenChars;
}

void Edit_View_OnDestroy(LPEDITVIEW lpev)
{
    int i;

    Edit_Buffer_FreeAll(lpev->lpes);

    if (lpev->pszLastFindWhat != NULL)
        Mem_Free(lpev->pszLastFindWhat);
    if (lpev->pdwParseCookies != NULL)
        Mem_Free(lpev->pdwParseCookies);
    if (lpev->piActualLineLength != NULL)
        Mem_Free(lpev->piActualLineLength);
    if (lpev->rxNode != NULL)
        Mem_Free(lpev->rxNode);
    if (lpev->pszMatched != NULL)
        Mem_Free(lpev->pszMatched);

    if (lpev->hCursor != NULL)
        DestroyCursor(lpev->hCursor);

    for (i = 0; i < 4; i++)
    {
        if (lpev->ahFonts[i] != NULL)
        {
            DeleteObject(lpev->ahFonts[i]);
            lpev->ahFonts[i] = NULL;
        }
    }

    if (lpev->pCacheBitmap != NULL)
    {
        //FIXME: Correct?
        Mem_Free(lpev->pCacheBitmap);
        lpev->pCacheBitmap = NULL;
    }

    DefWindowProc(lpev->hwnd, WM_DESTROY, 0, 0);
}

BOOL Edit_View_OnEraseBkgnd(HDC hdc) 
{
    return (TRUE);
}

void Edit_View_OnSize(LPEDITVIEW lpev, UINT nType, int cx, int cy)
{
    DefWindowProc(lpev->hwnd, WM_SIZE, nType, MAKELPARAM(cx, cy));

    if (lpev->pCacheBitmap != NULL)
    {
        DeleteObject(*lpev->pCacheBitmap);
        Mem_Free(lpev->pCacheBitmap); //FIXME: WHY?
        lpev->pCacheBitmap = NULL;
    }
    lpev->iScreenLines = -1;
    lpev->iScreenChars = -1;
    Edit_View_RecalcVertScrollBar(lpev, FALSE);
    Edit_View_RecalcHorzScrollBar(lpev, FALSE);
}

void Edit_View_RecalcVertScrollBar(LPEDITVIEW lpev, BOOL bPositionOnly)
{
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    if (bPositionOnly)
    {
        si.fMask    = SIF_POS;
        si.nPos     = lpev->iTopLine;
    }
    else
    {
        if (Edit_View_GetScreenLines(lpev) >= Edit_View_GetLineCount(lpev) &&
            lpev->iTopLine > 0)
        {
            lpev->iTopLine = 0;
            InvalidateRect(lpev->hwnd, NULL, FALSE);
            Edit_View_UpdateCaret(lpev);
        }

        si.fMask    = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
        si.nMin     = 0;
        si.nMax     = Edit_View_GetLineCount(lpev) - 1;
        si.nPage    = Edit_View_GetScreenLines(lpev);
        si.nPos     = lpev->iTopLine;
    }

    //VERIFY (?)
    SetScrollInfo(lpev->hwnd, SB_VERT, &si, TRUE);
}

void Edit_View_OnVScroll(LPEDITVIEW lpev, UINT nSBCode, UINT nPos, HWND hwndScrollBar) 
{
    SCROLLINFO si;
    int iPageLines, iLineCount, iNewTopLine;
    BOOL bDisableSmooth = TRUE;

    DefWindowProc(lpev->hwnd, WM_VSCROLL, MAKEWPARAM(nSBCode, nPos), (LPARAM)hwndScrollBar);

    //  Note we cannot use nPos because of its 16-bit nature
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    VERIFY(GetScrollInfo(lpev->hwnd, SB_VERT, &si));

    iPageLines = Edit_View_GetScreenLines(lpev);
    iLineCount = Edit_View_GetLineCount(lpev);

    switch (nSBCode)
    {
        case SB_TOP:
            iNewTopLine = 0;
            bDisableSmooth = FALSE;
        break;
        case SB_BOTTOM:
            iNewTopLine = iLineCount - iPageLines + 1;
            bDisableSmooth = FALSE;
        break;
        case SB_LINEUP:
            iNewTopLine = lpev->iTopLine - 1;
        break;
        case SB_LINEDOWN:
            iNewTopLine = lpev->iTopLine + 1;
        break;
        case SB_PAGEUP:
            iNewTopLine = lpev->iTopLine - si.nPage + 1;
            bDisableSmooth = FALSE;
        break;
        case SB_PAGEDOWN:
            iNewTopLine = lpev->iTopLine + si.nPage - 1;
            bDisableSmooth = FALSE;
        break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            iNewTopLine = si.nTrackPos;
        break;
        default:
        return;
    }

    if (iNewTopLine < 0)
        iNewTopLine = 0;
    if (iNewTopLine >= iLineCount)
        iNewTopLine = iLineCount - 1;
    
    Edit_View_ScrollToLine(lpev, iNewTopLine, bDisableSmooth, TRUE);
}

void Edit_View_RecalcHorzScrollBar(LPEDITVIEW lpev, BOOL bPositionOnly)
{
    //  Again, we cannot use nPos because it's 16-bit
    SCROLLINFO si;
    si.cbSize = sizeof(si);

    if (bPositionOnly)
    {
        si.fMask = SIF_POS;
        si.nPos = lpev->iOffsetChar;
    }
    else
    {
        if (Edit_View_GetScreenChars(lpev) >= Edit_View_GetMaxLineLength(lpev) &&
            lpev->iOffsetChar > 0)
        {
            lpev->iOffsetChar = 0;
            InvalidateRect(lpev->hwnd, NULL, FALSE);
            Edit_View_UpdateCaret(lpev);
        }

        si.fMask    = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
        si.nMin     = 0;
        si.nMax     = TB_MAX_LINE_LENGTH; // Edit_View_GetMaxLineLength(lpev) - 1;
        si.nPage    = Edit_View_GetScreenChars(lpev);
        si.nPos     = lpev->iOffsetChar;
    }

    SetScrollInfo(lpev->hwnd, SB_HORZ, &si, TRUE);
}

void Edit_View_OnHScroll(LPEDITVIEW lpev, UINT nSBCode, UINT nPos, HWND hwndScrollBar) 
{
    SCROLLINFO si;
    int iPageChars, iMaxLineLength, iNewOffset;

    DefWindowProc(lpev->hwnd, WM_HSCROLL, MAKEWPARAM(nSBCode, nPos), (LPARAM)hwndScrollBar);

    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    VERIFY(GetScrollInfo(lpev->hwnd, SB_HORZ, &si));

    iPageChars      = Edit_View_GetScreenChars(lpev);
    iMaxLineLength  = TB_MAX_LINE_LENGTH; //Edit_View_GetMaxLineLength(lpev);

    switch (nSBCode)
    {
        case SB_LEFT:
            iNewOffset = 0;
        break;
        case SB_BOTTOM:
            iNewOffset = iMaxLineLength - iPageChars + 1;
        break;
        case SB_LINEUP:
            iNewOffset = lpev->iOffsetChar - 1;
        break;
        case SB_LINEDOWN:
            iNewOffset = lpev->iOffsetChar + 1;
        break;
        case SB_PAGEUP:
            iNewOffset = lpev->iOffsetChar - si.nPage + 1;
        break;
        case SB_PAGEDOWN:
            iNewOffset = lpev->iOffsetChar + si.nPage - 1;
        break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            iNewOffset = si.nTrackPos;
        break;
        default:
        return;
    }

    if (iNewOffset >= iMaxLineLength)
        iNewOffset = iMaxLineLength - 1;
    if (iNewOffset < 0)
        iNewOffset = 0;

    Edit_View_ScrollToChar(lpev, iNewOffset, FALSE, TRUE);
    Edit_View_UpdateCaret(lpev);
}

BOOL Edit_View_OnSetCursor(LPEDITVIEW lpev, UINT nHitTest, UINT wMouseMsg) 
{
    if (nHitTest == HTCLIENT)
    {
        POINT pt;

        GetCursorPos(&pt);
        ScreenToClient(lpev->hwnd, &pt);

        if (pt.x < Edit_View_GetMarginWidth(lpev))
        {
            SetCursor(lpev->hCursor);
        }
        else
        {
            POINT ptText = Edit_View_ClientToText(lpev, pt);
            Edit_View_PrepareSelBounds(lpev);
            if (Edit_View_IsInsideSelBlock(lpev, ptText))
            {
                //  [JRT]:  Support For Disabling Drag and Drop...
                if (!lpev->bDisableDragAndDrop)             // If Drag And Drop Not Disabled
                    SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));    // Set To Arrow Cursor
            }
            else
            {
                SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
            }
        }

        return (TRUE);
    }

    return (DefWindowProc(lpev->hwnd, WM_SETCURSOR, (WPARAM)lpev->hwnd, MAKELPARAM(nHitTest, wMouseMsg)));
}

//FIXME: Needs debugging
POINT Edit_View_ClientToText(LPEDITVIEW lpev, POINT pt)
{
    int iLineCount = Edit_View_GetLineCount(lpev);
    POINT ptRet;
    int nLength = 0;
    LPCTSTR pszLine = NULL;
    int iPos;
    int iIndex = 0, iCurPos = 0;
    int iTabSize;

    ptRet.y = lpev->iTopLine + pt.y / Edit_View_GetLineHeight(lpev);

    if (ptRet.y >= iLineCount)
        ptRet.y = iLineCount - 1;
    if (ptRet.y < 0)
        ptRet.y = 0;

    if (ptRet.y >= 0 && ptRet.y < iLineCount)
    {
        nLength = Edit_View_GetLineLength(lpev, ptRet.y);
        pszLine = Edit_View_GetLineChars(lpev, ptRet.y);
    }

    iPos = lpev->iOffsetChar + (pt.x - Edit_View_GetMarginWidth(lpev)) / Edit_View_GetCharWidth(lpev);
    if (iPos < 0)
        iPos = 0;

    iTabSize = Edit_View_GetTabSize(lpev);

    while (iIndex < nLength)
    {
        if (pszLine[iIndex] == _T('\t'))
            iCurPos += (iTabSize - iCurPos % iTabSize);
        else
            iCurPos++;

        if (iCurPos > iPos)
            break;

        iIndex++;
    }

    ASSERT(iIndex >= 0 && iIndex <= nLength);
    ptRet.x = iIndex;
    
    return (ptRet);
}

POINT Edit_View_TextToClient(LPEDITVIEW lpev, POINT pt)
{
    int iLength;
    LPCTSTR pszLine;
    POINT ptRet;
    int nTabSize = Edit_View_GetTabSize(lpev);
    int iIndex;

    ASSERT_VALIDTEXTPOS(lpev, pt);
    
    iLength = Edit_View_GetLineLength(lpev, pt.y);
    pszLine = Edit_View_GetLineChars(lpev, pt.y);

    ptRet.y = (pt.y - lpev->iTopLine) * Edit_View_GetLineHeight(lpev);
    ptRet.x = 0;

    for (iIndex = 0; iIndex < pt.x; iIndex++)
    {
        if (pszLine[iIndex] == _T('\t'))
            ptRet.x += (nTabSize - ptRet.x % nTabSize);
        else
            ptRet.x++;
    }

    ptRet.x = (ptRet.x - lpev->iOffsetChar) * Edit_View_GetCharWidth(lpev) + Edit_View_GetMarginWidth(lpev);

    return (ptRet);
}

void Edit_View_InvalidateLines(LPEDITVIEW lpev, int iLine1, int iLine2, BOOL bInvalidateMargin)
{
    bInvalidateMargin = TRUE;

    if (iLine2 == -1)
    {
        RECT rcInvalid;
        
        GetClientRect(lpev->hwnd, &rcInvalid);
        if (!bInvalidateMargin)
            rcInvalid.left += Edit_View_GetMarginWidth(lpev);
        rcInvalid.top = (iLine1 - lpev->iTopLine) * Edit_View_GetLineHeight(lpev);
        InvalidateRect(lpev->hwnd, &rcInvalid, FALSE);
    }
    else
    {
        RECT rcInvalid;

        if (iLine2 < iLine1)
        {
            int iTemp = iLine1;

            iLine1 = iLine2;
            iLine2 = iTemp;
        }

        GetClientRect(lpev->hwnd, &rcInvalid);
        if (!bInvalidateMargin)
            rcInvalid.left += Edit_View_GetMarginWidth(lpev);
        rcInvalid.top = (iLine1 - lpev->iTopLine) * Edit_View_GetLineHeight(lpev);
        rcInvalid.bottom = (iLine2 - lpev->iTopLine + 1) * Edit_View_GetLineHeight(lpev);
        InvalidateRect(lpev->hwnd, &rcInvalid, FALSE);
    }
}

void Edit_View_SetSelection(LPEDITVIEW lpev, POINT ptStart, POINT ptEnd)
{
    NMHDR nmhdr;

    ASSERT_VALIDTEXTPOS(lpev, ptStart);
    ASSERT_VALIDTEXTPOS(lpev, ptEnd);

    if (Point_Equal(&lpev->ptSelStart, &ptStart))
    {
        if (!Point_Equal(&lpev->ptSelEnd, &ptEnd))
            Edit_View_InvalidateLines(lpev, ptEnd.y, lpev->ptSelEnd.y, FALSE);
    }
    else
    {
        Edit_View_InvalidateLines(lpev, ptStart.y, ptEnd.y, FALSE);
        Edit_View_InvalidateLines(lpev, lpev->ptSelStart.y, lpev->ptSelEnd.y, FALSE);
    }

    lpev->ptSelStart = ptStart;
    lpev->ptSelEnd = ptEnd;

    nmhdr.hwndFrom  = lpev->hwnd;
    nmhdr.idFrom    = GetWindowLong(lpev->hwnd, GWL_ID);
    nmhdr.code      = PEN_SELCHANGE;

    SendMessage(GetParent(lpev->hwnd), WM_NOTIFY, (WPARAM)nmhdr.idFrom, (LPARAM)&nmhdr);
}

void Edit_View_AdjustTextPoint(LPEDITVIEW lpev, POINT *lppt)
{
    //FIXME: this needs work
    lppt->x += Edit_View_GetCharWidth(lpev) / 2;
}

void Edit_View_OnSetFocus(LPEDITVIEW lpev, HWND hwndLoseFocus) 
{
    DefWindowProc(lpev->hwnd, WM_SETFOCUS, (WPARAM)hwndLoseFocus, 0);

    lpev->bFocused = TRUE;
    if (Point_Equal(&lpev->ptSelStart, &lpev->ptSelEnd))
        Edit_View_InvalidateLines(lpev, lpev->ptSelStart.y, lpev->ptSelEnd.y, TRUE);
    Edit_View_UpdateCaret(lpev);
}

DWORD Edit_View_ParseLine(LPEDITVIEW lpev, DWORD dwCookie, int nLineIndex, LPTEXTBLOCK pBuf, LPINT piActualItems)
{
    return ((lpev->lptdCurSourceDef->bPlain) ? 0 : Edit_View_ParseLineGeneric(lpev, dwCookie, nLineIndex, pBuf, piActualItems));
}

int Edit_View_CalculateActualOffset(LPEDITVIEW lpev, int iLineIndex, int iCharIndex)
{
    int iLength = Edit_View_GetLineLength(lpev, iLineIndex);
    LPCTSTR pszChars;
    int iOffset = 0;
    int iTabSize = Edit_View_GetTabSize(lpev);
    int i;

    ASSERT(iCharIndex >= 0 && iCharIndex <= iLength);
    pszChars = Edit_View_GetLineChars(lpev, iLineIndex);

    for (i = 0; i < iCharIndex; i++)
    {
        if (pszChars[i] == _T('\t'))
            iOffset += (iTabSize - iOffset % iTabSize);
        else
            iOffset++;
    }

    return (iOffset);
}

int Edit_View_ApproxActualOffset(LPEDITVIEW lpev, int iLineIndex, int iOffset)
{
    int iLength;
    LPCTSTR pszChars;
    int iCurrentOffset;
    int iTabSize = Edit_View_GetTabSize(lpev);
    int i;

    if (iOffset == 0)
        return 0;

    iLength     = Edit_View_GetLineLength(lpev, iLineIndex);
    pszChars    = Edit_View_GetLineChars(lpev, iLineIndex);
    iCurrentOffset = 0;

    for (i = 0; i < iLength; i++)
    {
        if (pszChars[i] == _T('\t'))
            iCurrentOffset += (iTabSize - iCurrentOffset % iTabSize);
        else
            iCurrentOffset++;
        
        if (iCurrentOffset >= iOffset)
        {
            //FIXME: Is this really right?
            if (iOffset <= iCurrentOffset - iTabSize / 2)
                return (i);

            return (i + 1);
        }
    }

    return (iLength);
}

void Edit_View_EnsureVisible(LPEDITVIEW lpev, POINT pt)
{
    //  Scroll vertically
    int iLineCount  = Edit_View_GetLineCount(lpev);
    int iNewTopLine = lpev->iTopLine;
    int iActualPos;
    int iNewOffset;

    if (pt.y >= iNewTopLine + Edit_View_GetScreenLines(lpev))
    {
        iNewTopLine = pt.y - Edit_View_GetScreenLines(lpev) + 1;
    }
    if (pt.y < iNewTopLine)
    {
        iNewTopLine = pt.y;
    }

    if (iNewTopLine < 0)
        iNewTopLine = 0;
    if (iNewTopLine >= iLineCount)
        iNewTopLine = iLineCount - 1;

    if (lpev->iTopLine != iNewTopLine)
    {
        //FIXME: Is the TRUE's true (right)? 
        Edit_View_ScrollToLine(lpev, iNewTopLine, FALSE, TRUE);
        //Edit_View_UpdateSiblingScrollPos(lpev, TRUE);
    }

    //  Scroll horizontally
    iActualPos = Edit_View_CalculateActualOffset(lpev, pt.y, pt.x);
    iNewOffset = lpev->iOffsetChar;

    if (iActualPos > iNewOffset + Edit_View_GetScreenChars(lpev))
    {
        iNewOffset = iActualPos - Edit_View_GetScreenChars(lpev) + 16;
    }
    if (iActualPos < iNewOffset)
    {
        iNewOffset = iActualPos - 16;
    }

    // FIXME: might need chaning
    if (iNewOffset >= Edit_View_GetMaxLineLength(lpev))
        iNewOffset = Edit_View_GetMaxLineLength(lpev) - 1;
    if (iNewOffset < 0)
        iNewOffset = 0;

    if (lpev->iOffsetChar != iNewOffset)
    {
        Edit_View_ScrollToChar(lpev, iNewOffset, FALSE, TRUE);
        Edit_View_UpdateCaret(lpev);
        //Edit_View_UpdateSiblingScrollPos(lpev, FALSE);
    }
}

void Edit_View_OnKillFocus(LPEDITVIEW lpev, HWND hwndGetFocus)
{
    DefWindowProc(lpev->hwnd, WM_KILLFOCUS, (WPARAM)hwndGetFocus, 0);

    lpev->bFocused = FALSE;
    Edit_View_UpdateCaret(lpev);
    
    if (!Point_Equal(&lpev->ptSelStart, &lpev->ptSelEnd))
        Edit_View_InvalidateLines(lpev, lpev->ptSelStart.y, lpev->ptSelEnd.y, TRUE);

    if (lpev->bDragSelection)
    {
        ReleaseCapture();
        KillTimer(lpev->hwnd, lpev->uDragSelTimer);
        lpev->bDragSelection = FALSE;
    }
}

void Edit_View_OnSysColorChange(LPEDITVIEW lpev) 
{
    DefWindowProc(lpev->hwnd, WM_SYSCOLORCHANGE, 0, 0);
    InvalidateRect(lpev->hwnd, NULL, FALSE);
}

void Edit_View_GetText(LPEDITVIEW lpev, POINT ptStart, POINT ptEnd, LPTSTR *pszText)
{
    //FIXME: check the addresses
    if (lpev->lpes != NULL)
        Edit_Buffer_GetText(lpev->lpes, ptStart.y, ptStart.x, ptEnd.y, ptEnd.x, pszText, NULL);
    else
        *pszText = _T("");
}

void Edit_View_UpdateView(LPEDITVIEW lpev, LPINSERTCONTEXT pContext, DWORD dwFlags, int iLineIndex)
{
    int iLineCount;

    if (dwFlags & UPDATE_RESET)
    {
        Edit_View_ResetView(lpev);
        Edit_View_RecalcVertScrollBar(lpev, FALSE);
        Edit_View_RecalcHorzScrollBar(lpev, FALSE);

        return;
    }

    iLineCount = Edit_View_GetLineCount(lpev);

    ASSERT(iLineCount > 0);
    ASSERT(iLineIndex >= -1 && iLineIndex < iLineCount);
    
    if ((dwFlags & UPDATE_SINGLELINE) != 0)
    {
        ASSERT(iLineIndex != -1);
        //  All text below this line should be reparsed
        if (lpev->pdwParseCookies != NULL)
        {
            ASSERT(lpev->iParseArraySize == iLineCount);
            memset(lpev->pdwParseCookies + iLineIndex, 0xFF, sizeof(DWORD) * (lpev->iParseArraySize - iLineIndex));
        }
        //  This line'th actual length must be recalculated
        if (lpev->piActualLineLength != NULL)
        {
            ASSERT(lpev->iActualLengthArraySize == iLineCount);
            lpev->piActualLineLength[iLineIndex] = -1;
        }
        //  Repaint the lines
        Edit_View_InvalidateLines(lpev, iLineIndex, -1, TRUE);
    }
    else
    {
        if (iLineIndex == -1)
            iLineIndex = 0;     //  Refresh all text
        //  All text below this line should be reparsed
        if (lpev->pdwParseCookies != NULL)
        {
            if (lpev->iParseArraySize != iLineCount)
            {
                //  Reallocate cookies array
                LPDWORD pdwNewArray = (LPDWORD)Mem_Alloc(sizeof(DWORD) * iLineCount);
                
                if (iLineIndex > 0)
                    memcpy(pdwNewArray, lpev->pdwParseCookies, sizeof(DWORD) * iLineIndex);

                Mem_Free(lpev->pdwParseCookies);
                
                lpev->iParseArraySize   = iLineCount;
                lpev->pdwParseCookies   = pdwNewArray;
            }

            memset(lpev->pdwParseCookies + iLineIndex, 0xFF, sizeof(DWORD) * (lpev->iParseArraySize - iLineIndex));
        }
        //  Recalculate actual length for all lines below this
        if (lpev->piActualLineLength != NULL)
        {
            if (lpev->iActualLengthArraySize != iLineCount)
            {
                //  Reallocate actual length array
                LPINT piNewArray = (LPINT)Mem_Alloc(sizeof(INT) * iLineCount);
                
                if (iLineIndex > 0)
                    memcpy(piNewArray, lpev->piActualLineLength, sizeof(INT) * iLineIndex);

                Mem_Free(lpev->piActualLineLength);

                lpev->iActualLengthArraySize = iLineCount;
                lpev->piActualLineLength = piNewArray;
            }

            //sizeof DWORD??!?!?! not INT?
            memset(lpev->piActualLineLength + iLineIndex, 0xFF, sizeof(INT) * (lpev->iActualLengthArraySize - iLineIndex));
        }
        //  Repaint the lines
        Edit_View_InvalidateLines(lpev, iLineIndex, -1, TRUE);
    }

    //FIXME: What here?!?!?

    //  All those points must be recalculated and validated
    if (pContext != NULL)
    {
        POINT ptTopLine;

        Edit_Buffer_RecalcPoint(*pContext, &lpev->ptCursorPos);
        Edit_Buffer_RecalcPoint(*pContext, &lpev->ptSelStart);
        Edit_Buffer_RecalcPoint(*pContext, &lpev->ptSelEnd);
        Edit_Buffer_RecalcPoint(*pContext, &lpev->ptAnchor);
        ASSERT_VALIDTEXTPOS(lpev, lpev->ptCursorPos);
        ASSERT_VALIDTEXTPOS(lpev, lpev->ptSelStart);
        ASSERT_VALIDTEXTPOS(lpev, lpev->ptSelEnd);
        ASSERT_VALIDTEXTPOS(lpev, lpev->ptAnchor);

        if (lpev->bDraggingText)
        {
            Edit_Buffer_RecalcPoint(*pContext, &lpev->ptDraggedTextBegin);
            Edit_Buffer_RecalcPoint(*pContext, &lpev->ptDraggedTextEnd);
            ASSERT_VALIDTEXTPOS(lpev, lpev->ptDraggedTextBegin);
            ASSERT_VALIDTEXTPOS(lpev, lpev->ptDraggedTextEnd);
        }
        
        ptTopLine.x = 0;
        ptTopLine.y = lpev->iTopLine;
        Edit_Buffer_RecalcPoint(*pContext, &ptTopLine);
        ASSERT_VALIDTEXTPOS(lpev, ptTopLine);
        
        lpev->iTopLine = ptTopLine.y;
        Edit_View_UpdateCaret(lpev);
    }

    //  Recalculate vertical scrollbar, if needed
    if ((dwFlags & UPDATE_VERTRANGE) != 0)
    {
        if (!lpev->bVertScrollBarLocked)
            Edit_View_RecalcVertScrollBar(lpev, FALSE);
    }

    //  Recalculate horizontal scrollbar, if needed
    if ((dwFlags & UPDATE_HORZRANGE) != 0)
    {
        lpev->iMaxLineLength = -1;
        if (!lpev->bHorzScrollBarLocked)
            Edit_View_RecalcHorzScrollBar(lpev, FALSE);
    }
}

int Edit_View_OnCreate(LPEDITVIEW lpev, LPCREATESTRUCT lpCreateStruct) 
{
    memset(&lpev->lfBaseFont, 0, sizeof(lpev->lfBaseFont));
    _tcscpy(lpev->lfBaseFont.lfFaceName, _T("FixedSys"));
    lpev->lfBaseFont.lfHeight           = 0;
    lpev->lfBaseFont.lfWeight           = FW_NORMAL;
    lpev->lfBaseFont.lfItalic           = FALSE;
    lpev->lfBaseFont.lfCharSet          = DEFAULT_CHARSET;
    lpev->lfBaseFont.lfOutPrecision     = OUT_DEFAULT_PRECIS;
    lpev->lfBaseFont.lfClipPrecision    = CLIP_DEFAULT_PRECIS;
    lpev->lfBaseFont.lfQuality          = DEFAULT_QUALITY;
    lpev->lfBaseFont.lfPitchAndFamily   = DEFAULT_PITCH;

    lpev->iCharWidth    = -1;
    lpev->iLineHeight   = -1;

    lpev->hCursor = LoadCursor(PCP_Edit_GetHandle(), MAKEINTRESOURCE(IDC_MARGIN_CURSOR));

    lpev->lpes = (LPEDITSTATE)Mem_Alloc(sizeof(EDITSTATE));

    if (lpCreateStruct->lpCreateParams == NULL || !Edit_Buffer_LoadFromFile(lpev, lpev->lpes, (LPTSTR)lpCreateStruct->lpCreateParams, CRLF_STYLE_AUTOMATIC))
        Edit_Buffer_InitNew(lpev, CRLF_STYLE_DOS);

    if (DefWindowProc(lpev->hwnd, WM_CREATE, 0, (LPARAM)lpCreateStruct) == -1)
        return (-1);

    return (TRUE);
}

void Edit_View_SetAnchor(LPEDITVIEW lpev, POINT ptNewAnchor)
{
    ASSERT_VALIDTEXTPOS(lpev, ptNewAnchor);
    lpev->ptAnchor = ptNewAnchor;
}

void Edit_View_OnEditOperation(LPEDITVIEW lpev, int nAction, LPCTSTR pszText)
{
    if (lpev->dwFlags & SRCOPT_AUTOINDENT)
    {
        //  Analyse last action...
        if (nAction == CE_ACTION_TYPING && String_Equal(pszText, _T ("\r\n"), TRUE) && !lpev->bOvrMode)
        {
            //  Enter stroke!
            POINT ptCursorPos = Edit_View_GetCursorPos(lpev);
            int nLength;
            LPCTSTR pszLineChars;
            int nPos = 0;
            int x, y;
            DWORD dwFlags = Edit_View_GetFlags(lpev);

            ASSERT(ptCursorPos.y > 0);
            
            //  Take indentation from the previos line
            nLength = Edit_View_GetLineLength(lpev, ptCursorPos.y - 1);
            pszLineChars = Edit_View_GetLineChars(lpev, ptCursorPos.y - 1);

            while (nPos < nLength && isspace(pszLineChars[nPos]))
                nPos++;
            
            if (nPos > 0)
            {
                TCHAR *pszInsertStr;
                POINT pt;

                if ((dwFlags & SRCOPT_BRACEGNU) && Edit_View_IsKeywordGeneric(lpev->lptdCurSourceDef->apszIndent, (pszLineChars + nLength - 1), _tcslen(pszLineChars + nLength - 1), lpev->lptdCurSourceDef->bCase) && ptCursorPos.y > 0 && nPos && nPos == nLength - 1)
                {
                    if (pszLineChars[nPos - 1] == _T('\t'))
                    {
                        nPos--;
                    }
                    else
                    {
                        int nTabSize    = Edit_View_GetTabSize(lpev),
                            nDelta      = nTabSize - nPos % nTabSize;

                        if (!nDelta)
                            nDelta = nTabSize;

                        nPos -= nDelta;

                        if (nPos < 0)
                            nPos = 0;
                    }
                }

                //  Insert part of the previos line
                if ((dwFlags & (SRCOPT_BRACEGNU|SRCOPT_BRACEANSI)) && Edit_View_IsKeywordGeneric(lpev->lptdCurSourceDef->apszIndent, (pszLineChars + nLength - 1), _tcslen(pszLineChars + nLength - 1), lpev->lptdCurSourceDef->bCase))
                {
                    if (dwFlags & SRCOPT_INSERTTABS)
                    {
                        pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * (nPos + 2));
                        _tcsncpy(pszInsertStr, pszLineChars, nPos);
                        pszInsertStr[nPos++] = _T('\t');
                    }
                    else
                    {
                        int nTabSize = Edit_View_GetTabSize(lpev);
                        int nChars = nTabSize - nPos % nTabSize;

                        pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * (nPos + nChars));
                        _tcsncpy(pszInsertStr, pszLineChars, nPos);

                        while (nChars--)
                        {
                            pszInsertStr[nPos++] = _T(' ');
                        }
                    }
                }
                else
                {
                    pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * (nPos + 1));
                    _tcsncpy(pszInsertStr, pszLineChars, nPos);
                }

                pszInsertStr[nPos] = _T('\0');

                Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x,
                    pszInsertStr, &y, &x, CE_ACTION_AUTOINDENT);
                
                pt.x = x;
                pt.y = y;

                Edit_View_SetCursorPos(lpev, pt);
                Edit_View_SetSelection(lpev, pt, pt);
                Edit_View_SetAnchor(lpev, pt);
                Edit_View_EnsureVisible(lpev, pt);
            }
            else
            {
                //  Insert part of the previous line
                TCHAR *pszInsertStr;

                if ((dwFlags & (SRCOPT_BRACEGNU|SRCOPT_BRACEANSI)) && Edit_View_IsKeywordGeneric(lpev->lptdCurSourceDef->apszIndent, (pszLineChars + nLength - 1), _tcslen(pszLineChars + nLength - 1), lpev->lptdCurSourceDef->bCase))
                {
                    int x, y;
                    POINT pt;

                    if (dwFlags & SRCOPT_INSERTTABS)
                    {
                        pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * 2);
                        pszInsertStr[nPos++] = _T('\t');
                    }
                    else
                    {
                        int nTabSize = Edit_View_GetTabSize(lpev);
                        int nChars = nTabSize - nPos % nTabSize;
                        
                        pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * (nChars + 1));

                        while (nChars--)
                        {
                            pszInsertStr[nPos++] = _T(' ');
                        }
                    }

                    pszInsertStr[nPos] = 0;
                    
                    Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x,
                        pszInsertStr, &y, &x, CE_ACTION_AUTOINDENT);

                    pt.x = x;
                    pt.y = y;

                    Edit_View_SetCursorPos(lpev, pt);
                    Edit_View_SetSelection(lpev, pt, pt);
                    Edit_View_SetAnchor(lpev, pt);
                    Edit_View_EnsureVisible(lpev, pt);
                }
            }
        }
        else if (nAction == CE_ACTION_TYPING && (Edit_View_GetFlags(lpev) & SRCOPT_FNBRACE) && bracetypesz(pszText) == 2)
        {
            //  Enter stroke!
            POINT ptCursorPos   = Edit_View_GetCursorPos(lpev);
            LPCTSTR pszChars    = Edit_View_GetLineChars(lpev, ptCursorPos.y);

            if (ptCursorPos.x > 1 && xisalnum(pszChars[ptCursorPos.x - 2]))
            {
                int x, y;
                LPTSTR pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * 2);

                *pszInsertStr = _T(' ');
                pszInsertStr[1] = _T('\0');

                Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x - 1,
                    pszInsertStr, &y, &x, CE_ACTION_AUTOINDENT);

                ptCursorPos.x = x + 1;
                ptCursorPos.y = y;
                
                Edit_View_SetCursorPos(lpev, ptCursorPos);
                Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
                Edit_View_SetAnchor(lpev, ptCursorPos);
                Edit_View_EnsureVisible(lpev, ptCursorPos);
            }
        }
        else if (nAction == CE_ACTION_TYPING && (Edit_View_GetFlags(lpev) & SRCOPT_BRACEGNU) && Edit_View_IsKeywordGeneric(lpev->lptdCurSourceDef->apszIndent, pszText, _tcslen(pszText), lpev->lptdCurSourceDef->bCase))
        {
            //  Enter stroke!
            POINT ptCursorPos = Edit_View_GetCursorPos(lpev);
            //  Take indentation from the previous line
            int nLength = Edit_View_GetLineLength(lpev, ptCursorPos.y);
            LPCTSTR pszLineChars = Edit_View_GetLineChars(lpev, ptCursorPos.y );
            int nPos = 0;

            while (nPos < nLength && isspace(pszLineChars[nPos]))
                nPos++;

            if (nPos == nLength - 1)
            {
                TCHAR *pszInsertStr;
                int x, y;
                POINT pt;

                if (Edit_View_GetFlags(lpev) & SRCOPT_INSERTTABS)
                {
                    pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * 2);
                    *pszInsertStr = _T('\t');
                    nPos = 1;
                }
                else
                {
                    int nTabSize = Edit_View_GetTabSize(lpev);
                    int nChars = nTabSize - nPos % nTabSize;

                    pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * (nChars + 1));
                    nPos = 0;
                    
                    while (nChars--)
                    {
                        pszInsertStr[nPos++] = _T(' ');
                    }
                }

                pszInsertStr[nPos] = 0;
                
                Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x - 1,
                    pszInsertStr, &y, &x, CE_ACTION_AUTOINDENT);
                
                pt.x = x + 1;
                pt.y = y;

                Edit_View_SetCursorPos(lpev, pt);
                Edit_View_SetSelection(lpev, pt, pt);
                Edit_View_SetAnchor(lpev, pt);
                Edit_View_EnsureVisible(lpev, pt);
            }
        }
        else if (nAction == CE_ACTION_TYPING && (Edit_View_GetFlags(lpev) & (SRCOPT_BRACEGNU|SRCOPT_BRACEANSI)) && Edit_View_IsKeywordGeneric(lpev->lptdCurSourceDef->apszUnIndent, pszText, _tcslen(pszText), lpev->lptdCurSourceDef->bCase))
        {
            POINT ptCursorPos = Edit_View_GetCursorPos(lpev);
            POINT ptMatchPos;
            int nLength;
            LPCTSTR pszLineChars;
            int nPos = 0;

//          ptMatchPos = Edit_View_MatchBrace(lpev, ptCursorPos);

//          if (ptMatchPos.x == -1 || ptMatchPos.y == -1)
            ptMatchPos = ptCursorPos;
            
            nLength = Edit_View_GetLineLength(lpev, ptMatchPos.y);
            pszLineChars = Edit_View_GetLineChars(lpev, ptMatchPos.y );

            while (nPos < nLength && _istspace(pszLineChars[nPos]))
                nPos++;

            if (ptMatchPos.y > 0 && nPos && nPos == nLength - 1)
            {
                if (pszLineChars[nPos - 1] == _T('\t'))
                {
                    nPos = 1;
                }
                else
                {
                    int nTabSize = Edit_View_GetTabSize(lpev);
                    nPos = nTabSize - (ptMatchPos.x - 1) % nTabSize;

                    if (!nPos)
                    {
                        nPos = nTabSize;
                    }
                    
                    if (nPos > nLength - 1)
                    {
                        nPos = nLength - 1;
                    }
                }

                Edit_Buffer_DeleteText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x - nPos - 1,
                    ptCursorPos.y, ptCursorPos.x - 1, CE_ACTION_AUTOINDENT);
                ptCursorPos.x -= nPos;

                Edit_View_SetCursorPos(lpev, ptCursorPos);
                Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
                Edit_View_SetAnchor(lpev, ptCursorPos);
                Edit_View_EnsureVisible(lpev, ptCursorPos);
            }
        }
    }

    SendMessage(GetParent(lpev->hwnd), WM_COMMAND, MAKEWPARAM(GetWindowLong(lpev->hwnd, GWL_ID), EN_CHANGE), (LPARAM)lpev->hwnd);
}

void Edit_View_AutoComplete(LPEDITVIEW lpev)
{
    POINT ptCursorPos = Edit_View_GetCursorPos(lpev);
    int nLength = Edit_View_GetLineLength(lpev, ptCursorPos.y);
    LPCTSTR pszText = Edit_View_GetLineChars(lpev, ptCursorPos.y);
    LPCTSTR pszEnd = pszText + ptCursorPos.x;

    if (ptCursorPos.x > 0 && ptCursorPos.y > 0 && (nLength == ptCursorPos.x || !xisalnum(*pszEnd)) && xisalnum(pszEnd[-1]))
    {
        LPCTSTR pszBegin = pszEnd - 1;
        LPTSTR pszBuffer;
        POINT ptTextPos;
        BOOL bFound;

        while (pszBegin > pszText && xisalnum(*pszBegin))
            pszBegin--;

        if (!xisalnum(*pszBegin))
            pszBegin++;

        nLength = pszEnd - pszBegin;

        pszBuffer = (LPTSTR)Mem_AllocStr(nLength + 1 + SZ);
        *pszBuffer = _T('<');
        _tcsncpy(pszBuffer + 1, pszBegin, nLength);
        pszBuffer[nLength + 1] = _T('\0');
        
        ptCursorPos.x -= nLength;
        bFound = Edit_View_FindText(lpev, pszBuffer, ptCursorPos, FIND_MATCH_CASE|FIND_REG_EXP|FIND_DIRECTION_UP, TRUE, &ptTextPos);

        if (!bFound)
        {
            ptCursorPos.x += nLength;
            bFound = Edit_View_FindText(lpev, pszBuffer, ptCursorPos, FIND_MATCH_CASE|FIND_REG_EXP, TRUE, &ptTextPos);
            ptCursorPos.x -= nLength;
        }

        if (bFound)
        {
            int nFound = Edit_View_GetLineLength(lpev, ptTextPos.y);
            LPTSTR psz;

            pszText = Edit_View_GetLineChars(lpev, ptTextPos.y) + ptTextPos.x + lpev->iLastFindWhatLen;
            nFound -= ptTextPos.x + lpev->iLastFindWhatLen;
            Mem_Free(pszBuffer);
            psz = pszBuffer = (LPTSTR)Mem_AllocStr(nFound + SZ);

            while (nFound-- && xisalnum(*pszText))
                *psz++ = *pszText++;

            *psz = _T('\0');

            if ((pszBuffer[0] != _T('\0')))
            {
                int x, y;

                Edit_Buffer_BeginUndoGroup(lpev->lpes, FALSE);
                Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x + nLength, pszBuffer, &y, &x, CE_ACTION_AUTOCOMPLETE);
                ptCursorPos.x = x;
                ptCursorPos.y = y;
                
                Edit_View_SetCursorPos(lpev, ptCursorPos);
                Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
                Edit_View_SetAnchor(lpev, ptCursorPos);
                Edit_View_EnsureVisible(lpev, ptCursorPos);
                Edit_Buffer_FlushUndoGroup(lpev, lpev->lpes);
            }
        }
    }
}

POINT Edit_View_MatchBrace(LPEDITVIEW lpev, POINT ptStartPos)
{
    int nLength = Edit_View_GetLineLength(lpev, ptStartPos.y);
    LPCTSTR pszText = Edit_View_GetLineChars(lpev, ptStartPos.y);
    LPCTSTR pszEnd = pszText + ptStartPos.x;
    BOOL bAfter;
    int nType = 0;
    POINT ptFailure = { -1, -1 };

    if (ptStartPos.x < nLength)
    {
        nType = bracetype(*pszEnd);

        if (nType)
        {
            bAfter = FALSE;
        }
        else if (!nType && ptStartPos.x > 0)
        {
            nType = bracetype(pszEnd[-1]);
            bAfter = TRUE;
        }
    }
    else if (ptStartPos.x > 0)
    {
        nType = bracetype(pszEnd[-1]);
        bAfter = TRUE;
    }

    if (nType)
    {
        int nOther, nCount = 0, nComment = 0;
        LPCTSTR pszOpenComment      = lpev->lptdCurSourceDef->szOpenComment,
                pszCloseComment     = lpev->lptdCurSourceDef->szCloseComment,
                pszLineComment      = lpev->lptdCurSourceDef->szLineComment,
                pszOpenComment2     = lpev->lptdCurSourceDef->szOpenComment2,
                pszCloseComment2    = lpev->lptdCurSourceDef->szCloseComment2,
                pszLineComment2     = lpev->lptdCurSourceDef->szLineComment2,
                pszTest;
        int nOpenComment    = lpev->lptdCurSourceDef->nOpenCommentLen,
            nCloseComment   = lpev->lptdCurSourceDef->nCloseCommentLen,
            nLineComment    = lpev->lptdCurSourceDef->nLineCommentLen,
            nOpenComment2   = lpev->lptdCurSourceDef->nOpenComment2Len,
            nCloseComment2  = lpev->lptdCurSourceDef->nCloseComment2Len,
            nLineComment2   = lpev->lptdCurSourceDef->nLineComment2Len;

        if (bAfter)
        {
            nOther = bracebuddy(*pszEnd);

            if (nOther & 1)
                pszEnd--;
        }
        else
        {
            nOther = bracebuddy(*pszEnd);

            if (!(nOther & 1))
                pszEnd++;
        }


        if (nOther & 1)
        {
            for (;;)
            {
                while (--pszEnd >= pszText)
                {
                    if (nOpenComment)
                    {
                        pszTest = pszEnd - nOpenComment + 1;

                        if (pszTest >= pszText &&
                            String_NumEqual(pszTest, pszOpenComment,
                                    nOpenComment, FALSE))
                        {
                            nComment--;
                            pszEnd = pszTest;

                            if (--pszEnd < pszText)
                                break;
                        }
                    }

                    if (nOpenComment2)
                    {
                        pszTest = pszEnd - nOpenComment2 + 1;

                        if (pszTest >= pszText &&
                            String_NumEqual(pszTest, pszOpenComment2,
                                    nOpenComment2, FALSE))
                        {
                            nComment--;
                            pszEnd = pszTest;

                            if (--pszEnd < pszText)
                                break;
                        }
                    }

                    if (nCloseComment)
                    {
                        pszTest = pszEnd - nCloseComment + 1;

                        if (pszTest >= pszText &&
                            String_NumEqual(pszTest, pszCloseComment,
                            nCloseComment, FALSE))
                        {
                            nComment++;
                            pszEnd = pszTest;

                            if (--pszEnd < pszText)
                                break;
                        }
                    }

                    if (nCloseComment2)
                    {
                        pszTest = pszEnd - nCloseComment2 + 1;

                        if (pszTest >= pszText &&
                            String_NumEqual(pszTest, pszCloseComment2,
                            nCloseComment2, FALSE))
                        {
                            nComment++;
                            pszEnd = pszTest;

                            if (--pszEnd < pszText)
                                break;
                        }
                    }

                    if (!nComment)
                    {
                        if (nLineComment)
                        {
                            pszTest = pszEnd - nLineComment + 1;

                            if (pszTest >= pszText &&
                                String_NumEqual(pszTest, pszLineComment,
                                nLineComment, FALSE))
                                break;
                        }

                        if (nLineComment2)
                        {
                            pszTest = pszEnd - nLineComment2 + 1;

                            if (pszTest >= pszText &&
                                String_NumEqual(pszTest, pszLineComment2,
                                nLineComment2, FALSE))
                                break;
                        }

                        if (bracetype(*pszEnd) == nType)
                        {
                            nCount++;
                        }
                        else if (bracetype(*pszEnd) == nOther)
                        {
                            if (!nCount--)
                            {
                                ptStartPos.x = pszEnd - pszText;

                                if (bAfter)
                                    ptStartPos.x++;

                                return (ptStartPos);
                            }
                        }
                    }
                }

                if (ptStartPos.y)
                {
                    ptStartPos.x = Edit_View_GetLineLength(lpev, --ptStartPos.y);
                    pszText = Edit_View_GetLineChars(lpev, ptStartPos.y);
                    pszEnd = pszText + ptStartPos.x;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            LPCTSTR pszBegin = pszText;
            int nLines = Edit_View_GetLineCount(lpev);

            pszText = pszEnd;
            pszEnd = pszBegin + nLength;

            for (;;)
            {
                while (pszText < pszEnd)
                {
                    if (nCloseComment)
                    {

                        pszTest = pszText + nCloseComment;

                        if (pszTest <= pszEnd &&
                            String_NumEqual(pszText, pszCloseComment,
                                    nCloseComment, FALSE))
                        {
                            nComment--;
                            pszText = pszTest;

                            if (pszText > pszEnd)
                                break;
                        }
                    }

                    if (nCloseComment2)
                    {

                        pszTest = pszText + nCloseComment2;

                        if (pszTest <= pszEnd &&
                            String_NumEqual(pszText, pszCloseComment2,
                                    nCloseComment2, FALSE))
                        {
                            nComment--;
                            pszText = pszTest;

                            if (pszText > pszEnd)
                                break;
                        }
                    }

                    if (nOpenComment)
                    {
                        pszTest = pszText + nOpenComment;

                        if (pszTest <= pszEnd &&
                            String_NumEqual(pszText, pszOpenComment,
                                    nOpenComment, FALSE))
                        {
                            nComment++;
                            pszText = pszTest;

                            if (pszText > pszEnd)
                                break;
                        }
                    }

                    if (nOpenComment2)
                    {
                        pszTest = pszText + nOpenComment2;

                        if (pszTest <= pszEnd &&
                            String_NumEqual(pszText, pszOpenComment2,
                                    nOpenComment2, FALSE))
                        {
                            nComment++;
                            pszText = pszTest;

                            if (pszText > pszEnd)
                                break;
                        }
                    }

                    if (!nComment)
                    {
                        if (nLineComment)
                        {
                            pszTest = pszText + nLineComment;
                            if (pszTest <= pszEnd &&
                                String_NumEqual(pszText, pszLineComment,
                                        nLineComment, FALSE))
                                break;
                        }
                        
                        if (nLineComment2)
                        {
                            pszTest = pszText + nLineComment2;
                            if (pszTest <= pszEnd &&
                                String_NumEqual(pszText, pszLineComment2,
                                        nLineComment2, FALSE))
                                break;
                        }

                        if (bracetype(*pszText) == nType)
                        {
                            nCount++;
                        }
                        else if (bracetype(*pszText) == nOther)
                        {
                            if (!nCount--)
                            {
                                ptStartPos.x = pszText - pszBegin;

                                if (bAfter)
                                    ptStartPos.x++;

                                return (ptStartPos);
                            }
                        }
                    }

                    pszText++;
                }

                if (ptStartPos.y + 1 < nLines)
                {
                    ptStartPos.x = 0;
                    nLength = Edit_View_GetLineLength(lpev, ++ptStartPos.y);
                    pszBegin = pszText = Edit_View_GetLineChars(lpev, ptStartPos.y);
                    pszEnd = pszBegin + nLength;
                }
                else
                {
                    break;
                }
            }
        }
    }

    return (ptFailure);
}

POINT Edit_View_GetCursorPos(LPEDITVIEW lpev)
{
    return lpev->ptCursorPos;
}

void Edit_View_SetCursorPos(LPEDITVIEW lpev, POINT ptCursorPos)
{
    ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
    lpev->ptCursorPos = ptCursorPos;
    lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);
    Edit_View_UpdateCaret(lpev);
}

void Edit_View_SetSelectionMargin(LPEDITVIEW lpev, BOOL bSelMargin)
{
    if (lpev->bSelMargin != bSelMargin)
    {
        lpev->bSelMargin = bSelMargin;
        if (IsWindow(lpev->hwnd))
        {
            lpev->iScreenChars = -1;
            InvalidateRect(lpev->hwnd, NULL, FALSE);
            Edit_View_RecalcHorzScrollBar(lpev, FALSE);
        }
    }
}

//FIXME: right?
/*
void Edit_View_GetFont(LOGFONT *lplf)
{
    lplf = lpev->lfBaseFont;
}
*/
void Edit_View_SetFont(LPEDITVIEW lpev, LPLOGFONT lplf)
{
    int i;

    lpev->lfBaseFont    = *lplf;
    lpev->iScreenLines  = -1;
    lpev->iScreenChars  = -1;
    lpev->iCharWidth    = -1;
    lpev->iLineHeight   = -1;
    
    if (lpev->pCacheBitmap != NULL)
    {
        DeleteObject(*lpev->pCacheBitmap);
        Mem_Free(lpev->pCacheBitmap);
        lpev->pCacheBitmap = NULL;
    }

    for (i = 0; i < 4; i++)
    {
        if (lpev->ahFonts[i] != NULL)
        {
            DeleteObject(lpev->ahFonts[i]);
//          Mem_Free(lpev->ahFonts[i]);
            lpev->ahFonts[i] = NULL;
        }
    }

    if (IsWindow(lpev->hwnd))
    {
        Edit_View_RecalcVertScrollBar(lpev, FALSE);
        Edit_View_RecalcHorzScrollBar(lpev, FALSE);
        Edit_View_UpdateCaret(lpev);
        InvalidateRect(lpev->hwnd, NULL, FALSE);
    }
}

void Edit_View_OnClearBookmarks(LPEDITVIEW lpev)
{
    if (lpev->lpes != NULL)
    {
        int iBookmarkID;

        for (iBookmarkID = 0; iBookmarkID <= 9; iBookmarkID++)
        {
            int iLine = Edit_Buffer_GetLineWithFlag(lpev->lpes, LF_BOOKMARK(iBookmarkID));
            if (iLine >= 0)
            {
                Edit_Buffer_SetLineFlag(lpev, lpev->lpes, iLine, LF_BOOKMARK(iBookmarkID), FALSE, TRUE);
            }
        }
        
    }
}

void Edit_View_OnClearAllBookmarks(LPEDITVIEW lpev)
{
    if (lpev->lpes != NULL)
    {
        int nLineCount = Edit_View_GetLineCount(lpev);
        int i;

        for (i = 0; i < nLineCount; i++)
        {
            if (Edit_Buffer_GetLineFlags(lpev->lpes, i) & LF_BOOKMARKS)
                Edit_Buffer_SetLineFlag(lpev, lpev->lpes, i, LF_BOOKMARKS, FALSE, TRUE);
        }

        lpev->bBookmarkExist = FALSE;
    }
}

void Edit_View_ShowCursor(LPEDITVIEW lpev)
{
    lpev->bCursorHidden = FALSE;
    Edit_View_UpdateCaret(lpev);
}

void Edit_View_HideCursor(LPEDITVIEW lpev)
{
    lpev->bCursorHidden = TRUE;
    Edit_View_UpdateCaret(lpev);
}

static int Edit_View_FindStringHelper(LPCTSTR pszFindWhere, LPCTSTR pszFindWhat, DWORD dwFlags, int *nLen, RxNode *rxNode, RxMatchRes *rxMatch)
{
    if (dwFlags & FIND_REG_EXP)
    {
        int nPos;
        
        if (rxNode)
            RxFree(rxNode);

        rxNode = RxCompile(pszFindWhat);

        if (rxNode && RxExec(rxNode, pszFindWhere, _tcslen(pszFindWhere), pszFindWhere, rxMatch, (dwFlags & FIND_MATCH_CASE) != 0 ? RX_CASE : 0))
        {
            nPos = rxMatch->Open[0];
            *nLen = rxMatch->Close[0] - rxMatch->Open[0];
        }
        else
        {
            nPos = -1;
        }

        return (nPos);
    }
    else
    {
        int iCur = 0;
        int iLength;

        ASSERT(pszFindWhere != NULL);
        ASSERT(pszFindWhat != NULL);

        iLength = _tcslen(pszFindWhat);
        *nLen = iLength;

        for (;;)
        {
            LPCTSTR pszPos = _tcsstr(pszFindWhere, pszFindWhat);
            
            if (pszPos == NULL)
                return (-1);
            
            if ((dwFlags & FIND_WHOLE_WORD) == 0)
                return (iCur + (pszPos - pszFindWhere));
            
            if (pszPos > pszFindWhere && xisalnum(pszPos[-1]))
            {
                iCur += (pszPos - pszFindWhere);
                pszFindWhere = pszPos + 1;

                continue;
            }
            
            if (xisalnum(pszPos[iLength]))
            {
                iCur += (pszPos - pszFindWhere + 1);
                pszFindWhere = pszPos + 1;
            
                continue;
            }

            return (iCur + (pszPos - pszFindWhere));
        }
    }

    ASSERT(FALSE);      // Unreachable
        
    return (-1);
}

BOOL Edit_View_GetSelectionXY(LPEDITVIEW lpev, LPDWORD lpdwStartPos, LPDWORD lpdwEndPos)
{
    POINT ptStartPos = { 0, 0 }, ptEndPos;
    DWORD dwCurrentPos = 0;
    BOOL bEdit = TRUE;
    int i;

    Edit_View_GetSelection(lpev, &ptStartPos, &ptEndPos);

    for (i = 0; i < ptStartPos.y; i++)
    {
        dwCurrentPos += Edit_View_GetLineLength(lpev, i++);
    }

    *lpdwStartPos = dwCurrentPos + ptStartPos.x;

    for (i = ptStartPos.y; i < ptEndPos.y; i++)
    {
        dwCurrentPos += Edit_View_GetLineLength(lpev, i++);
    }

    *lpdwEndPos = dwCurrentPos + ptEndPos.x;

    return (TRUE);
}

BOOL Edit_View_SetSelectionXY(LPEDITVIEW lpev, DWORD dwStartPos, DWORD dwEndPos)
{
    POINT ptStartPos = { 0, 0 }, ptEndPos;
    DWORD dwCurrentPos = 0;
    int nLineCount = Edit_View_GetLineCount(lpev);
    BOOL bEdit = TRUE;

    if (dwEndPos != -1)
        ASSERT(dwStartPos <= dwEndPos);

    while (dwCurrentPos < dwStartPos)
    {
        if (ptStartPos.y >= nLineCount)
        {
            ptStartPos.y = Edit_View_GetLineCount(lpev) - 1;
            ptStartPos.x = Edit_View_GetLineLength(lpev, ptStartPos.y);
            bEdit = FALSE;

            break;
        }

        dwCurrentPos += Edit_View_GetLineLength(lpev, ptStartPos.y++);
    }

    if (bEdit)
    {
        dwCurrentPos -= (dwCurrentPos - dwStartPos);
        ptStartPos.x = dwCurrentPos;
        ptStartPos.y -= (ptStartPos.y == 0) ? 0 : 1;
        ptEndPos = ptStartPos;
    }
    else
    {
        bEdit = TRUE;
    }

    if (dwEndPos == -1)
    {
        ptEndPos.y = Edit_View_GetLineCount(lpev) - 1;
        ptEndPos.x = Edit_View_GetLineLength(lpev, ptEndPos.y);
        bEdit = FALSE;
    }
    else
    {
        while (dwCurrentPos < dwEndPos)
        {
            if (ptEndPos.y >= nLineCount)
            {
                ptEndPos.y = Edit_View_GetLineCount(lpev) - 1;
                ptEndPos.x = Edit_View_GetLineLength(lpev, ptEndPos.y);
                bEdit = FALSE;

                break;
            }

            dwCurrentPos += Edit_View_GetLineLength(lpev, ptEndPos.y++);
        }
    }

    if (bEdit)
    {
        dwCurrentPos -= (dwCurrentPos - dwEndPos);
        ptEndPos.x = dwCurrentPos;
        ptEndPos.y -= (ptEndPos.y == 0) ? 0 : 1;
    }

    lpev->ptCursorPos   = ptEndPos;
    lpev->ptAnchor      = lpev->ptCursorPos;
    Edit_View_SetSelection(lpev, ptStartPos, ptEndPos);
    Edit_View_UpdateCaret(lpev);
    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
    
    return (TRUE);
}

BOOL Edit_View_HighlightText(LPEDITVIEW lpev, POINT ptStartPos, int iLength, BOOL bReverse)
{
    POINT ptEndPos = ptStartPos;
    int nCount = Edit_View_GetLineLength(lpev, ptEndPos.y) - ptEndPos.x;

    ASSERT_VALIDTEXTPOS(lpev, ptStartPos);
    
    if (iLength <= nCount)
    {
        ptEndPos.x += iLength;
    }
    else
    {
        while (iLength > nCount)
        {
            iLength -= nCount + 1;
            nCount = Edit_View_GetLineLength(lpev, ++ptEndPos.y);
        }

        ptEndPos.x = iLength;
    }

    ASSERT_VALIDTEXTPOS(lpev, lpev->ptCursorPos);       //  Probably 'nLength' is bigger than expected...
    lpev->ptCursorPos = (bReverse) ? ptStartPos : ptEndPos;
    lpev->ptAnchor = lpev->ptCursorPos;
    Edit_View_SetSelection(lpev, ptStartPos, ptEndPos);
    Edit_View_UpdateCaret(lpev);
    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);

    return (TRUE);
}

BOOL Edit_View_FindText(LPEDITVIEW lpev, LPCTSTR pszText, POINT ptStartPos, DWORD dwFlags, BOOL bWrapSearch, POINT *lpptFoundPos)
{
    int iLineCount = Edit_View_GetLineCount(lpev);
    POINT ptBlockBegin = { 0, 0 };
    POINT ptBlockEnd = { Edit_View_GetLineLength(lpev, iLineCount - 1), (iLineCount - 1) };
    
    return (Edit_View_FindTextInBlock(lpev, pszText, ptStartPos, ptBlockBegin,
            ptBlockEnd, dwFlags, bWrapSearch, lpptFoundPos));
}

BOOL Edit_View_FindTextInBlock(LPEDITVIEW lpev, LPCTSTR pszText, POINT ptStartPosition, 
                            POINT ptBlockBegin, POINT ptBlockEnd,
                            DWORD dwFlags, BOOL bWrapSearch, POINT *lpptFoundPos)
{
    POINT ptCurrentPos = ptStartPosition;
    LPTSTR pszWhat;
    INT nEolns;

    ASSERT(pszText != NULL && _tcslen(pszText) > 0);
    ASSERT_VALIDTEXTPOS(lpev, ptCurrentPos);
    ASSERT_VALIDTEXTPOS(lpev, ptBlockBegin);
    ASSERT_VALIDTEXTPOS(lpev, ptBlockEnd);
    ASSERT (ptBlockBegin.y < ptBlockEnd.y || ptBlockBegin.y == ptBlockEnd.y &&
            ptBlockBegin.x <= ptBlockEnd.x);

    if (Point_Equal(&ptBlockBegin, &ptBlockEnd))
        return (FALSE);

    if (ptCurrentPos.y < ptBlockBegin.y || ptCurrentPos.y == ptBlockBegin.y &&
        ptCurrentPos.x < ptBlockBegin.x)
        ptCurrentPos = ptBlockBegin;

    pszWhat = (LPTSTR)pszText;

    if (dwFlags & FIND_REG_EXP)
    {
        nEolns = String_CountSubStrings(pszWhat, _T("\\n"));
    }
    else
    {
        nEolns = 0;

        if ((dwFlags & FIND_MATCH_CASE) == 0)
        {
            /* seems we need to mess around with what
             * we're searching for so make a copy
             * as to not break the const (so that the
             * user doesn't get an UGLY surprise
             */

            pszWhat = String_Duplicate(pszText);
            _tcsupr(pszWhat);
        }
    }

    if (dwFlags & FIND_DIRECTION_UP)
    {
        //  Let's check if we deal with whole text.
        //  At this point, we cannot search *up* in selection
        ASSERT(ptBlockBegin.x == 0 && ptBlockBegin.y == 0);
        ASSERT(ptBlockEnd.x == Edit_View_GetLineLength(lpev, Edit_View_GetLineCount(lpev) - 1) &&
                ptBlockEnd.y == Edit_View_GetLineCount(lpev) - 1);

        //  Proceed as if we have whole text search.
        for (;;)
        {
            while (ptCurrentPos.y >= 0)
            {
                int iLineLength;
                LPCTSTR pszChars;
                LPTSTR pszLine = (LPTSTR)Mem_AllocStr(SZ);
                int iPos;
                int nFoundPos = -1;
                int nMatchLen;
                int nLineLen;

                if (dwFlags & FIND_REG_EXP)
                {
                    int i;

                    for (i = 0; i <= nEolns && ptCurrentPos.y >= i; i++)
                    {
                        LPCTSTR pszChars = Edit_View_GetLineChars(lpev, ptCurrentPos.y - i);

                        if (i)
                        {
                            iLineLength = Edit_View_GetLineLength(lpev, ptCurrentPos.y - i);
                            ptCurrentPos.x = 0;

                            pszLine = (LPTSTR)Mem_ReAllocStr(pszLine, Mem_SizeStr(pszLine) + 1 + SZ);

                            String_Insert(&pszLine, _T("\n"), 0);
                        }
                        else
                        {
                            iLineLength = (ptCurrentPos.x != -1) ? ptCurrentPos.x : Edit_View_GetLineLength(lpev, ptCurrentPos.y - i);
                        }

                        if (iLineLength > 0)
                        {
                            pszLine = (LPTSTR)Mem_ReAllocStr(pszLine, Mem_SizeStr(pszLine) + _tcslen(pszChars) + SZ);

                            String_Insert(&pszLine, (LPSTR)pszChars, 0);
                        }
                    }

                    iLineLength = _tcslen(pszLine);

                    if (ptCurrentPos.x == -1)
                        ptCurrentPos.x = 0;
                }
                else
                {
                    iLineLength = Edit_View_GetLineLength(lpev, ptCurrentPos.y);

                    if (ptCurrentPos.x == -1)
                    {
                        ptCurrentPos.x = iLineLength;
                    }
                    else
                    {
                        if (ptCurrentPos.x > iLineLength)
                            ptCurrentPos.x = iLineLength - 1;
                    }

                    pszChars = Edit_View_GetLineChars(lpev, ptCurrentPos.y);
                    pszLine = (LPTSTR)Mem_AllocStr(ptCurrentPos.x + 1 + SZ);
                    _tcsncpy(pszLine, pszChars, ptCurrentPos.x); // + 1

                    if ((dwFlags & FIND_MATCH_CASE) == 0)
                        _tcsupr(pszLine);
                }

                nMatchLen   = _tcslen(pszWhat);
                nLineLen    = _tcslen(pszLine);

                do
                {
                    iPos = Edit_View_FindStringHelper(pszLine, pszWhat, dwFlags, &lpev->iLastFindWhatLen, lpev->rxNode, &lpev->rxMatch);

                    if (iPos >= 0)
                    {
                        nFoundPos = (nFoundPos == -1) ? iPos : (nFoundPos + iPos);
                        nFoundPos += nMatchLen;
                        pszLine = String_Right(pszLine, nLineLen - (nMatchLen + iPos));
                        nLineLen = _tcslen(pszLine);
                    }
                }
                while (iPos >= 0);

                if (nFoundPos >= 0)
                {
                    ptCurrentPos.x = nFoundPos - nMatchLen;
                    *lpptFoundPos = ptCurrentPos;

                    if (lpev->pszMatched != NULL)
                        Mem_Free(lpev->pszMatched);

                    lpev->pszMatched = String_Duplicate(pszLine);

                    Mem_Free(pszLine);

                    return (TRUE);
                }
                else
                {
                    if (lpev->pszMatched != NULL)
                        Mem_Free(lpev->pszMatched);

                    lpev->pszMatched = NULL;
                }

                ptCurrentPos.y--;

                if (ptCurrentPos.y >= 0)
                    ptCurrentPos.x = Edit_View_GetLineLength(lpev, ptCurrentPos.y);

                Mem_Free(pszLine);
            }
            
            //  Beginning of text reached
            if (!bWrapSearch)
                return (FALSE);

            //  Start again from the end of text
            bWrapSearch = FALSE;

            ptCurrentPos.x = -1;
            ptCurrentPos.y = Edit_View_GetLineCount(lpev) - 1;
        }
    }
    else
    {
        for (;;)
        {
            while (ptCurrentPos.y <= ptBlockEnd.y)
            {
                int nLineLength;
                LPTSTR pszLine = (LPTSTR)Mem_AllocStr(SZ);
                int nPos;
                int nLines;

                if (dwFlags & FIND_REG_EXP)
                {
                    int i;

                    nLines = Edit_View_GetLineCount(lpev);

                    for (i = 0; i <= nEolns && ptCurrentPos.y + i < nLines; i++)
                    {
                        LPCTSTR pszChars = Edit_View_GetLineChars(lpev, ptCurrentPos.y + i);
                        nLineLength = Edit_View_GetLineLength(lpev, ptCurrentPos.y + i);

                        if (i)
                        {
                            pszLine = (LPTSTR)Mem_ReAllocStr(pszLine, Mem_SizeStr(pszLine) + 1 + SZ);

                            _tcscat(pszLine, _T("\n"));
                        }
                        else
                        {
                            pszChars += ptCurrentPos.x;
                            nLineLength -= ptCurrentPos.x;
                        }

                        if (nLineLength > 0)
                        {
                            pszLine = (LPTSTR)Mem_ReAllocStr(pszLine, Mem_SizeStr(pszLine) + _tcslen(pszChars) + SZ);

                            _tcscat(pszLine, pszChars);
                        }
                    }

                    nLineLength = _tcslen(pszLine);
                }
                else
                {
                    LPCTSTR pszChars;

                    nLineLength = Edit_View_GetLineLength(lpev, ptCurrentPos.y) - ptCurrentPos.x;

                    if (nLineLength <= 0)
                    {
                        ptCurrentPos.x = 0;
                        ptCurrentPos.y++;

                        continue;
                    }
                    
                    pszChars = Edit_View_GetLineChars(lpev, ptCurrentPos.y);
                    pszChars += ptCurrentPos.x;

                    pszLine = (LPTSTR)Mem_AllocStr(nLineLength + SZ);
                    //  Prepare necessary part of line
                    _tcsncpy(pszLine, pszChars, nLineLength);

                    if ((dwFlags & FIND_MATCH_CASE) == 0)
                        _tcsupr(pszLine);
                }
                
                //  Perform search in the line
                nPos = Edit_View_FindStringHelper(pszLine, pszWhat, dwFlags, &lpev->iLastFindWhatLen, lpev->rxNode, &lpev->rxMatch);

                if (nPos >= 0)
                {
                    if (lpev->pszMatched != NULL)
                        Mem_Free(lpev->pszMatched);

                    lpev->pszMatched = String_Duplicate(pszLine);

                    if (nEolns)
                    {
                        LPTSTR pszItem = (LPTSTR)Mem_AllocStr(nPos + SZ);
                        LPCTSTR pszCurrent = _tcsrchr(pszItem, _T('\n'));

                        if (pszCurrent)
                            pszCurrent++;
                        else
                            pszCurrent = pszItem;

                        nEolns = String_CountSubChars(pszItem, _T('\n'));

                        if (nEolns)
                        {
                            ptCurrentPos.y += nEolns;
                            ptCurrentPos.x = nPos - (pszCurrent - (LPCTSTR)pszItem);
                        }
                        else
                        {
                            ptCurrentPos.x += nPos - (pszCurrent - (LPCTSTR)pszItem);
                        }

                        if (ptCurrentPos.x < 0)
                            ptCurrentPos.x = 0;

                        Mem_Free(pszItem);
                    }
                    else
                    {
                        ptCurrentPos.x += nPos;
                    }

                    //  Check of the text found is outside the block.
                    if (ptCurrentPos.y == ptBlockEnd.y && ptCurrentPos.x >= ptBlockEnd.x)
                        break;
                    
                    *lpptFoundPos = ptCurrentPos;

                    Mem_Free(pszLine);

                    return (TRUE);
                }
                else
                {
                    if (lpev->pszMatched != NULL)
                        Mem_Free(lpev->pszMatched);

                    lpev->pszMatched = NULL;
                }

                Mem_Free(pszLine);
                //  Go further, text was not found
                ptCurrentPos.x = 0;
                ptCurrentPos.y++;
            }
            
            //  End of text reached
            if (!bWrapSearch)
                return (FALSE);
            
            //  Start from the beginning
            bWrapSearch = FALSE;
            ptCurrentPos = ptBlockBegin;
        }
    }
    
    ASSERT (FALSE);               // Unreachable
    
    return (FALSE);
}

void Edit_View_OnToggleBookmark(LPEDITVIEW lpev)
{
    int iLine;

    if (lpev->lpes != NULL)
    {
        DWORD dwFlags = Edit_View_GetLineFlags(lpev, lpev->ptCursorPos.y);
        DWORD dwMask  = LF_BOOKMARKS;
        Edit_Buffer_SetLineFlag(lpev, lpev->lpes, lpev->ptCursorPos.y, dwMask, (dwFlags & dwMask) == 0, FALSE);
    }

    iLine = Edit_Buffer_GetLineWithFlag(lpev->lpes, LF_BOOKMARKS);
    
    if (iLine >= 0)
        lpev->bBookmarkExist = TRUE;
    else
        lpev->bBookmarkExist = FALSE;
}

void Edit_View_OnNextBookmark(LPEDITVIEW lpev)
{
    if (lpev->lpes != NULL)
    {
        int iLine = Edit_Buffer_FindNextBookmarkLine(lpev->lpes, lpev->ptCursorPos.y);
        
        if (iLine >= 0)
        {
            POINT pt = { 0, iLine };
            ASSERT_VALIDTEXTPOS(lpev, pt);
            Edit_View_SetCursorPos(lpev, pt);
            Edit_View_SetSelection(lpev, pt, pt);
            Edit_View_SetAnchor(lpev, pt);
            Edit_View_EnsureVisible(lpev, pt);
        }
    }
}

void Edit_View_OnPrevBookmark(LPEDITVIEW lpev)
{
    if (lpev->lpes != NULL)
    {
        int iLine = Edit_Buffer_FindPrevBookmarkLine(lpev->lpes, lpev->ptCursorPos.y);

        if (iLine >= 0)
        {
            POINT pt = { 0, iLine };
            ASSERT_VALIDTEXTPOS(lpev, pt);
            Edit_View_SetCursorPos(lpev, pt);
            Edit_View_SetSelection(lpev, pt, pt);
            Edit_View_SetAnchor(lpev, pt);
            Edit_View_EnsureVisible(lpev, pt);
        }
    }
}

BOOL Edit_View_GetViewTabs(LPEDITVIEW lpev)
{
    return lpev->bViewTabs;
}

void Edit_View_SetViewTabs(LPEDITVIEW lpev, BOOL bViewTabs)
{
    if (bViewTabs != lpev->bViewTabs)
    {
        lpev->bViewTabs = bViewTabs;
        if (IsWindow(lpev->hwnd))
            InvalidateRect(lpev->hwnd, NULL, FALSE);
    }
}

BOOL Edit_View_GetSelectionMargin(LPEDITVIEW lpev)
{
    return (lpev->bSelMargin);
}

int Edit_View_GetMarginWidth(LPEDITVIEW lpev)
{
    return (lpev->bSelMargin) ? 20 : 1;
}

BOOL Edit_View_GetSmoothScroll(LPEDITVIEW lpev)
{
    return (lpev->bSmoothScroll);
}

void Edit_View_SetSmoothScroll(LPEDITVIEW lpev, BOOL bSmoothScroll)
{
    lpev->bSmoothScroll = bSmoothScroll;
}

//  [JRT]
BOOL Edit_View_GetDisableDragAndDrop(LPEDITVIEW lpev)
{
    return (lpev->bDisableDragAndDrop);
}

//  [JRT]
void Edit_View_SetDisableDragAndDrop(LPEDITVIEW lpev, BOOL bDDAD)
{
    lpev->bDisableDragAndDrop = bDDAD;
}

BOOL Edit_View_QueryEditable(LPEDITVIEW lpev)
{
    if (lpev->lpes == NULL)
        return (FALSE);

    return (!Edit_Buffer_GetReadOnly(lpev->lpes));
}

BOOL Edit_View_DeleteCurrentSelection(LPEDITVIEW lpev)
{
    if (Edit_View_IsSelection(lpev))
    {
        POINT ptSelStart, ptSelEnd, ptCursorPos;
        Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);

        ptCursorPos = ptSelStart;
        ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
        Edit_View_SetAnchor(lpev, ptCursorPos);
        Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
        Edit_View_SetCursorPos(lpev, ptCursorPos);
        Edit_View_EnsureVisible(lpev, ptCursorPos);

        // [JRT]:
        Edit_Buffer_DeleteText(lpev, lpev->lpes, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, CE_ACTION_DELSEL);

        return (TRUE);
    }

    return (FALSE);
}

void Edit_View_OnChar(LPEDITVIEW lpev, UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    DefWindowProc(lpev->hwnd, WM_CHAR, (WPARAM)nChar, MAKELPARAM(nRepCnt, nFlags));

    if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 ||
            (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0)
        return;

    if (lpev->bIncrementalSearchBackwards || lpev->bIncrementalSearchForwards)
    {
        TCHAR szChar[2] = { nChar, _T('\0') };

        if (nChar == VK_ESCAPE)
        {
            if (!lpev->bIncrementalFound && lpev->pszIncrementalSearch != NULL && _tcslen(lpev->pszIncrementalSearch) > 1)
            {
                lpev->pszIncrementalSearch[_tcslen(lpev->pszIncrementalSearch) - 1] = _T('\0');
                Edit_View_FindIncremental(lpev, FALSE);

                return;
            }

            Edit_View_FindIncrementalEnd(lpev, TRUE);

            return;
        }
        else if (nChar == VK_RETURN)
        {
            Edit_View_FindIncrementalEnd(lpev, FALSE);

            return;
        }
        else if (nChar == VK_DELETE || nChar == VK_BACK)
        {
            if (lpev->pszIncrementalSearch != NULL && _tcslen(lpev->pszIncrementalSearch) > 0)
            {
                lpev->pszIncrementalSearch[_tcslen(lpev->pszIncrementalSearch) - 1] = _T('\0');
                Edit_View_FindIncremental(lpev, FALSE);
            }

            return;
        }

        if (lpev->pszIncrementalSearch == NULL)
            lpev->pszIncrementalSearch = (LPTSTR)Mem_AllocStr(1);
        else
            lpev->pszIncrementalSearch = (LPTSTR)Mem_ReAllocStr(lpev->pszIncrementalSearch, Mem_SizeStr(lpev->pszIncrementalSearch) + 1);

        _tcscat(lpev->pszIncrementalSearch, szChar);
        Edit_View_FindIncremental(lpev, FALSE);

        return;
    }

    if (nChar == VK_RETURN)
    {
        if (lpev->bOvrMode)
        {
            POINT ptCursorPos;
            
            ptCursorPos = Edit_View_GetCursorPos(lpev);
            ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
            
            if (ptCursorPos.y < Edit_View_GetLineCount(lpev) - 1)
            {
                ptCursorPos.x = 0;
                ptCursorPos.y++;

                ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
                Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
                Edit_View_SetAnchor(lpev, ptCursorPos);
                Edit_View_SetCursorPos(lpev, ptCursorPos);
                Edit_View_EnsureVisible(lpev, ptCursorPos);

                return;
            }
        }

        Edit_Buffer_BeginUndoGroup(lpev->lpes, FALSE);

        if (Edit_View_QueryEditable(lpev) && lpev->lpes != NULL)
        {
            POINT ptCursorPos;
            const static TCHAR pszText[2] = _T("\r\n");
            int x, y;

            if (Edit_View_IsSelection(lpev))
            {
                POINT ptSelStart, ptSelEnd;

                Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);

                ptCursorPos = ptSelStart;

                Edit_Buffer_DeleteText(lpev, lpev->lpes, lpev->ptSelStart.y, lpev->ptSelStart.x, lpev->ptSelEnd.y, lpev->ptSelStart.x, CE_ACTION_TYPING);
            }
            else
            {
                ptCursorPos = Edit_View_GetCursorPos(lpev);
            }

            ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);

            Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x, pszText, &y, &x, CE_ACTION_TYPING);

            ptCursorPos.x = x;
            ptCursorPos.y = y;
            ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
            
            Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
            Edit_View_SetAnchor(lpev, ptCursorPos);
            Edit_View_SetCursorPos(lpev, ptCursorPos);
            Edit_View_EnsureVisible(lpev, ptCursorPos);
        }

        Edit_Buffer_FlushUndoGroup(lpev, lpev->lpes);

        return;
    }
    else if (nChar == VK_BACK || nChar == 0x7F) //0xF7 is some weird char put out when Ctrl+Backspace are pressed
    {
        Edit_View_OnBackspace(lpev);
    }
    else if (nChar == VK_ESCAPE)
    {
        if (lpev->bSelectCharMode)
            lpev->bSelectCharMode = FALSE;
    }
    else if (nChar == VK_TAB)
    {
        Edit_View_OnTab(lpev);
    }
    else if (nChar > 0x1F)
    {
        if (Edit_View_QueryEditable(lpev) && lpev->lpes != NULL)
        {
            POINT ptSelStart, ptSelEnd, ptCursorPos;
            TCHAR pszText[2];
            int x, y;

            Edit_Buffer_BeginUndoGroup(lpev->lpes, (nChar != _T(' ')));
            Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);

            if (!Point_Equal(&ptSelStart, &ptSelEnd))
            {
                ptCursorPos = ptSelStart;
                Edit_View_DeleteCurrentSelection(lpev);
            }
            else
            {
                ptCursorPos = Edit_View_GetCursorPos(lpev);
                if (lpev->bOvrMode && ptCursorPos.x < Edit_View_GetLineLength(lpev, ptCursorPos.y))
                    Edit_Buffer_DeleteText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x, ptCursorPos.y, ptCursorPos.x + 1, CE_ACTION_TYPING);
            }

            ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);

            pszText[0] = (TCHAR)nChar;
            pszText[1] = _T('\0');

            Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x, pszText, &y, &x, CE_ACTION_TYPING);
            
            ptCursorPos.x = x;
            ptCursorPos.y = y;
            ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
            
            Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
            Edit_View_SetAnchor(lpev, ptCursorPos);
            Edit_View_SetCursorPos(lpev, ptCursorPos);
            Edit_View_EnsureVisible(lpev, ptCursorPos);

            Edit_Buffer_FlushUndoGroup(lpev, lpev->lpes);
        }
    }
}

void Edit_View_OnKeyDown(LPEDITVIEW lpev, UINT nVirtKey, UINT cRepeat, UINT fFlags)
{
    int i;

    DefWindowProc(lpev->hwnd, WM_KEYDOWN, (WPARAM)nVirtKey, MAKELPARAM(cRepeat, fFlags));

    for (i = 0; i < (int)cRepeat; i++)
    {
        BOOL fControl   = IsCtrlDown();
        BOOL fShift     = (IsShiftDown() || lpev->bSelectCharMode) ? TRUE : FALSE;
        BOOL fAlt       = IsAltDown();

        if (nVirtKey == VK_INSERT && !fControl && !fShift && !fAlt)
        {
            NMHDR nmhdr;

            lpev->bOvrMode = !lpev->bOvrMode;

            Edit_View_UpdateCaret(lpev);

            nmhdr.hwndFrom  = lpev->hwnd;
            nmhdr.idFrom    = GetWindowLong(lpev->hwnd, GWL_ID);
            nmhdr.code      = PEN_OVERWRITEMODECHANGED;

            SendMessage(GetParent(lpev->hwnd), WM_NOTIFY, (WPARAM)nmhdr.idFrom, (LPARAM)&nmhdr);
        }
        if (nVirtKey == VK_DELETE)
        {
            if (lpev->bIncrementalSearchForwards || lpev->bIncrementalSearchBackwards)
            {
                Edit_View_OnChar(lpev, nVirtKey, cRepeat, fFlags);

                return;
            }
            else
            {
                Edit_View_OnDelete(lpev);
            }
        } // 0x21 Through 0x28 (movement keys)
        else if (nVirtKey >= VK_PRIOR && nVirtKey <= VK_DOWN)
        {
            if (nVirtKey == VK_PRIOR)
            {
                TextMove_MovePgUp(lpev, fShift);
            }
            else if (nVirtKey == VK_NEXT)
            {
                TextMove_MovePgDn(lpev, fShift);
            }
            else if (nVirtKey == VK_END)
            {
                if (fControl)
                    TextMove_MoveCtrlEnd(lpev, fShift);
                else
                    TextMove_MoveEnd(lpev, fShift);
            }
            else if (nVirtKey == VK_HOME)
            {
                if (fControl)
                    TextMove_MoveCtrlHome(lpev, fShift);
                else
                    TextMove_MoveHome(lpev, fShift);
            }
            else if (nVirtKey == VK_LEFT)
            {
                if (fControl)
                    TextMove_MoveWordLeft(lpev, fShift);
                else
                    TextMove_MoveLeft(lpev, fShift);
            }
            else if (nVirtKey == VK_UP)
            {
                if (fControl && !fShift)
                    TextMove_ScrollUp(lpev);
                else
                    TextMove_MoveUp(lpev, fShift);
            }
            if (nVirtKey == VK_RIGHT)
            {
                if (fControl)
                    TextMove_MoveWordRight(lpev, fShift);
                else
                    TextMove_MoveRight(lpev, fShift);
            }
            else if (nVirtKey == VK_DOWN)
            {
                if (fControl && !fShift)
                    TextMove_ScrollDown(lpev);
                else
                    TextMove_MoveDown(lpev, fShift);
            }
        }
    }
}

void Edit_View_OnPaste(LPEDITVIEW lpev)
{
    LPTSTR pszText;

    if (!Edit_View_QueryEditable(lpev) || lpev->lpes == NULL)
        return;

    if (Clipboard_GetFromClipboard(&pszText))
    {
        int x, y;
        POINT ptCursorPos;

        Edit_Buffer_BeginUndoGroup(lpev->lpes, FALSE);

        if (Edit_View_IsSelection(lpev))
        {
            POINT ptSelStart, ptSelEnd;

            Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);

            ptCursorPos = ptSelStart;

            Edit_Buffer_DeleteText(lpev, lpev->lpes, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, CE_ACTION_PASTE);
        }
        else
        {
             ptCursorPos = Edit_View_GetCursorPos(lpev);
        }

        ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);

        Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x, pszText, &y, &x, CE_ACTION_PASTE);
        ptCursorPos.x = x;
        ptCursorPos.y = y;

        ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
        Edit_View_SetAnchor(lpev, ptCursorPos);
        Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
        Edit_View_SetCursorPos(lpev, ptCursorPos);
        Edit_View_EnsureVisible(lpev, ptCursorPos);

        Mem_Free(pszText);
        Edit_Buffer_FlushUndoGroup(lpev, lpev->lpes);
    }
}

BOOL Edit_View_IsSelection(LPEDITVIEW lpev)
{
    return (!Point_Equal(&lpev->ptSelStart, &lpev->ptSelEnd));
}

void Edit_View_OnCopy(LPEDITVIEW lpev)
{
    LPTSTR pszText;
    
    if (Point_Equal(&lpev->ptSelStart, &lpev->ptSelEnd))
        return;

    Edit_View_PrepareSelBounds(lpev);
    Edit_View_GetText(lpev, lpev->ptDrawSelStart, lpev->ptDrawSelEnd, &pszText);
    Clipboard_SetText(pszText);

    Mem_Free(pszText);
}

void Edit_View_OnCut(LPEDITVIEW lpev)
{
    POINT ptSelStart, ptSelEnd, ptCursorPos;
    LPTSTR pszText;

    if (!Edit_View_QueryEditable(lpev) || lpev->lpes == NULL)
        return;
    if (!Edit_View_IsSelection(lpev))
        return;

    Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);
    Edit_View_GetText(lpev, ptSelStart, ptSelEnd, &pszText);
    Clipboard_SetText(pszText);

    ptCursorPos = ptSelStart;
    ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
    Edit_View_SetAnchor(lpev, ptCursorPos);
    Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
    Edit_View_SetCursorPos(lpev, ptCursorPos);
    Edit_View_EnsureVisible(lpev, ptCursorPos);

    Mem_Free(pszText);
    Edit_Buffer_DeleteText(lpev, lpev->lpes, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, CE_ACTION_CUT);
}

void Edit_View_OnClear(LPEDITVIEW lpev)
{
    POINT ptSelStart, ptSelEnd, ptCursorPos;

    if (!Edit_View_QueryEditable(lpev) || lpev->lpes == NULL)
        return;

    Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);
    
    if (Point_Equal(&ptSelStart, &ptSelEnd))
    {
        if (ptSelEnd.x == Edit_View_GetLineLength(lpev, ptSelEnd.y))
        {
            if (ptSelEnd.y == Edit_View_GetLineCount(lpev) - 1)
                return;

            ptSelEnd.y ++;
            ptSelEnd.x = 0;
        }
        else
            ptSelEnd.x ++;
    }

    ptCursorPos = ptSelStart;
    ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
    Edit_View_SetAnchor(lpev, ptCursorPos);
    Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
    Edit_View_SetCursorPos(lpev, ptCursorPos);
    Edit_View_EnsureVisible(lpev, ptCursorPos);

    Edit_Buffer_DeleteText(lpev, lpev->lpes, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, CE_ACTION_DELETE);
}

void Edit_View_OnBackspace(LPEDITVIEW lpev)
{
    POINT ptCursorPos, ptCurrentCursorPos;
    BOOL bDeleted = FALSE;
    BOOL fControl = IsCtrlDown();

    if (Edit_View_IsSelection(lpev))
    {
        Edit_View_OnClear(lpev);
        return;
    }

    if (!Edit_View_QueryEditable(lpev) || lpev->lpes == NULL)
        return;

    ptCurrentCursorPos = ptCursorPos = Edit_View_GetCursorPos(lpev);
    
    if (ptCursorPos.x == 0) // If At Start Of Line
    {
        if (lptd->dwFlags & SRCOPT_BSATBOL) // If DBSASOL Is Disabled
        {
            if (ptCursorPos.y > 0) // If Previous Lines Available
            {
                ptCursorPos.y--; // Decrement To Previous Line
                ptCursorPos.x = Edit_View_GetLineLength(lpev, ptCursorPos.y);   // Set Cursor To End Of Previous Line
                bDeleted = TRUE; // Set Deleted Flag
            }
        }
    }
    else // If Caret Not At SOL
    {
        if (fControl)
        {
            LPCTSTR pszLine = Edit_View_GetLineChars(lpev, ptCursorPos.y);
            LPCTSTR psz = pszLine + ptCursorPos.x - 1;

            // First, eat up all whitespace
            while (_istspace(*(psz)) && psz >= pszLine)
            {
                ptCursorPos.x--;

                psz--;
            }

            // Have we eaten a whole line? if so then remove it
            if (ptCursorPos.x == 0 && ptCursorPos.y > 0)
            {
                ptCursorPos.y--; // Decrement to Previous Line
                ptCursorPos.x = Edit_View_GetLineLength(lpev, ptCursorPos.y);
            }
            else
            {
                // Second, eat up a word. WACKA WACKA...
                while (xisalnum(*(psz)) && psz >= pszLine)
                {
                    ptCursorPos.x--;

                    psz--;
                }
            }
        }
        else
        {
            ptCursorPos.x--; // Decrement Position
        }

        bDeleted = TRUE; // Set Deleted Flag
    }

    ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
    Edit_View_SetAnchor(lpev, ptCursorPos);
    Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
    Edit_View_SetCursorPos(lpev, ptCursorPos);
    Edit_View_EnsureVisible(lpev, ptCursorPos);

    if (bDeleted)
        Edit_Buffer_DeleteText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x, ptCurrentCursorPos.y, ptCurrentCursorPos.x, CE_ACTION_BACKSPACE);

    return;
}

void Edit_View_OnTab(LPEDITVIEW lpev) 
{
    BOOL    bTabify = FALSE;
    POINT   ptSelStart, ptSelEnd, ptCursorPos;
    static  TCHAR szText[256];
    int x, y;
    
    if (!Edit_View_QueryEditable(lpev) || lpev->lpes == NULL)
        return;

    if (Edit_View_IsSelection(lpev))
    {
        Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);
        bTabify = TRUE; //(ptSelStart.y != ptSelEnd.y);
    }

    ptCursorPos = Edit_View_GetCursorPos(lpev);
    ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);

    if (!(Edit_View_GetFlags(lpev) & SRCOPT_INSERTTABS))
    {
        int nTabSize    = Edit_View_GetTabSize(lpev);
        int nChars      = nTabSize - ptCursorPos.x % nTabSize;

        memset(szText, _T(' '), nChars * sizeof(TCHAR));
        szText[nChars] = _T('\0');
    }

    if (bTabify)
    {
        int nStartLine, nEndLine, i;

        Edit_Buffer_BeginUndoGroup(lpev->lpes, FALSE);

        nStartLine = ptSelStart.y;
        nEndLine = ptSelEnd.y;
        ptSelStart.x = 0;

        if (ptSelEnd.x > 0)
        {
            if (ptSelEnd.y == Edit_View_GetLineCount(lpev) - 1)
            {
                ptSelEnd.x = Edit_View_GetLineLength(lpev, ptSelEnd.y);
            }
            else
            {
                ptSelEnd.x = 0;
                ptSelEnd.y++;
            }
        }
        else
        {
            nEndLine--;
        }

        Edit_View_SetSelection(lpev, ptSelStart, ptSelEnd);
        Edit_View_SetCursorPos(lpev, ptSelEnd);
        Edit_View_EnsureVisible(lpev, ptSelEnd);

        //  Shift selection to right
        lpev->bHorzScrollBarLocked = TRUE;

        for (i = nStartLine; i <= nEndLine; i++)
        {
            int x, y;

            if (Edit_View_GetFlags(lpev) & SRCOPT_INSERTTABS)
            {
                int iSpaceStart, nSpaces = 0;
                int nTabs = 0, nTabSize = Edit_View_GetTabSize(lpev), nTabOffset;
                LPCTSTR pszSpaces, pszSpacesBase;
                int nActualX;
                ptCursorPos = Edit_View_GetCursorPos(lpev);
                ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);

                pszSpacesBase   = Edit_View_GetLineChars(lpev, ptCursorPos.y);
                pszSpaces       = (pszSpacesBase - _tcslen(pszSpacesBase) + ptCursorPos.x);
                iSpaceStart     = _tcslen(pszSpaces) - 1;
                while (iSpaceStart >= 0 && pszSpaces[iSpaceStart] == _T(' '))
                {
                    nSpaces++;
                    iSpaceStart--;
                }

                // Can one use (iSpaceStart + nSpaces) instead of ptCursorPos.x?
                nActualX = Edit_View_CalculateActualOffset(lpev, ptCursorPos.y, ptCursorPos.x);
                nTabOffset = (nTabSize - (nActualX % nTabSize));
                if (nTabOffset == nTabSize)
                    nTabOffset = 0;
                nTabs = (int)ceil((double)((double)(nSpaces + nTabOffset) / (double)nTabSize));
                if ((nTabOffset == 0) || (nSpaces == 0 && nTabs == 0))
                    nTabs += 1;

                memset(szText, _T('\t'), nTabs * sizeof(TCHAR));
                szText[nTabs] = _T('\0');
                Edit_Buffer_DeleteText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x - nSpaces, ptCursorPos.y, ptCursorPos.x, CE_ACTION_TYPING);
                Edit_Buffer_InsertText(lpev, lpev->lpes, i, 0, szText, &y, &x, CE_ACTION_INDENT);
            }
            else
            {
                Edit_Buffer_InsertText(lpev, lpev->lpes, i, 0, szText, &y, &x, CE_ACTION_INDENT);
            }
        }

        lpev->bHorzScrollBarLocked = FALSE;
        Edit_View_RecalcHorzScrollBar(lpev, FALSE);

        Edit_Buffer_FlushUndoGroup(lpev, lpev->lpes);

        return;
    }

    if (lpev->bOvrMode)
    {
        int nLineLength;
        POINT ptCursorPos = Edit_View_GetCursorPos(lpev);
        LPCTSTR pszLineChars;

        ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);

        nLineLength = Edit_View_GetLineLength(lpev, ptCursorPos.y);
        pszLineChars = Edit_View_GetLineChars(lpev, ptCursorPos.y);

        if (ptCursorPos.x < nLineLength)
        {
            int nTabSize = Edit_View_GetTabSize(lpev);
            int nChars = nTabSize - Edit_View_CalculateActualOffset(lpev, ptCursorPos.y, ptCursorPos.x) % nTabSize;
            ASSERT(nChars > 0 && nChars <= nTabSize);

            while (nChars > 0)
            {
                if (ptCursorPos.x == nLineLength)
                    break;
                
                if (pszLineChars[ptCursorPos.x] == _T('\t'))
                {
                    ptCursorPos.x++;

                    break;
                }
                
                ptCursorPos.x++;
                nChars--;
            }

            ASSERT(ptCursorPos.x <= nLineLength);
            ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);

            Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
            Edit_View_SetAnchor(lpev, ptCursorPos);
            Edit_View_SetCursorPos(lpev, ptCursorPos);
            Edit_View_EnsureVisible(lpev, ptCursorPos);
            
            return;
        }
    }

    Edit_Buffer_BeginUndoGroup(lpev->lpes, FALSE);

    Edit_View_DeleteCurrentSelection(lpev);

    if (lpev->4)
    {
        int iSpaceStart, nSpaces = 0;
        int nTabs = 0, nTabSize = Edit_View_GetTabSize(lpev), nTabOffset;
        LPCTSTR pszSpaces, pszSpacesBase;
        int nActualX;
        ptCursorPos = Edit_View_GetCursorPos(lpev);
        ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);

        pszSpacesBase   = Edit_View_GetLineChars(lpev, ptCursorPos.y);
        pszSpaces       = (pszSpacesBase - _tcslen(pszSpacesBase) + ptCursorPos.x);
        iSpaceStart     = _tcslen(pszSpaces) - 1;
        while (iSpaceStart >= 0 && pszSpaces[iSpaceStart] == _T(' '))
        {
            nSpaces++;
            iSpaceStart--;
        }

        // this might perhaps be made a bit easier
        // (position * tabsize - lengthoftotalstring % nTabSize) ?

        // Can one use (iSpaceStart + nSpaces) instead of ptCursorPos.x?
        nActualX = Edit_View_CalculateActualOffset(lpev, ptCursorPos.y, ptCursorPos.x);
        nTabOffset = (nTabSize - (nActualX % nTabSize));
        if (nTabOffset == nTabSize)
            nTabOffset = 0;
        nTabs = (int)ceil((double)((double)(nSpaces + nTabOffset) / (double)nTabSize));
        if ((nTabOffset == 0) || (nSpaces == 0 && nTabs == 0))
            nTabs += 1;

        memset(szText, _T('\t'), nTabs * sizeof(TCHAR));
        szText[nTabs] = _T('\0');
        Edit_Buffer_DeleteText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x - nSpaces, ptCursorPos.y, ptCursorPos.x, CE_ACTION_TYPING);
        ptCursorPos = Edit_View_GetCursorPos(lpev);
        ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
        Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x, szText, &y, &x, CE_ACTION_TYPING);
    }
    else
    {
        ptCursorPos = Edit_View_GetCursorPos(lpev);
        ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);

        Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x, szText, &y, &x, CE_ACTION_TYPING);
    }

    ptCursorPos.x = x;
    ptCursorPos.y = y;
    ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
    Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
    Edit_View_SetAnchor(lpev, ptCursorPos);
    Edit_View_SetCursorPos(lpev, ptCursorPos);
    Edit_View_EnsureVisible(lpev, ptCursorPos);

    Edit_Buffer_FlushUndoGroup(lpev, lpev->lpes);
}

void Edit_View_OnEditUntab(LPEDITVIEW lpev)
{
    BOOL bTabify = FALSE;
    POINT ptSelStart, ptSelEnd;
    
    if (!Edit_View_QueryEditable(lpev) || lpev->lpes == NULL)
        return;

    if (Edit_View_IsSelection(lpev))
    {
        Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);
        bTabify = (ptSelStart.y != ptSelEnd.y);
    }

    if (bTabify)
    {
        POINT ptSelStart, ptSelEnd;
        int nStartLine;
        int nEndLine;
        int i;

        Edit_Buffer_BeginUndoGroup(lpev->lpes, FALSE);

        Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);

        nStartLine = ptSelStart.y;
        nEndLine = ptSelEnd.y;
        ptSelStart.x = 0;

        if (ptSelEnd.x > 0)
        {
            if (ptSelEnd.y == Edit_View_GetLineCount(lpev) - 1)
            {
                ptSelEnd.x = Edit_View_GetLineLength(lpev, ptSelEnd.y);
            }
            else
            {
                ptSelEnd.x = 0;
                ptSelEnd.y++;
            }
        }
        else
        {
            nEndLine--;
        }
        
        Edit_View_SetSelection(lpev, ptSelStart, ptSelEnd);
        Edit_View_SetCursorPos(lpev, ptSelEnd);
        Edit_View_EnsureVisible(lpev, ptSelEnd);

        //  Shift selection to left
        lpev->bHorzScrollBarLocked = TRUE;

        for (i = nStartLine; i <= nEndLine; i++)
        {
            int nLength = Edit_View_GetLineLength(lpev, i);

            if (nLength > 0)
            {
                LPCTSTR pszChars = Edit_View_GetLineChars(lpev, i);
                int nPos = 0, nOffset = 0;

                while (nPos < nLength)
                {
                    if (pszChars[nPos] == _T(' '))
                    {
                        nPos++;
                        if (++nOffset >= Edit_View_GetTabSize(lpev))
                            break;
                    }
                    else
                    {
                        if (pszChars[nPos] == _T('\t'))
                            nPos++;

                        break;
                    }
                }

                if (nPos > 0)
                    Edit_Buffer_DeleteText(lpev, lpev->lpes, i, 0, i, nPos, CE_ACTION_INDENT);
            }
        }

        lpev->bHorzScrollBarLocked = FALSE;
        Edit_View_RecalcHorzScrollBar(lpev, FALSE);

        Edit_Buffer_FlushUndoGroup(lpev, lpev->lpes);
    }
    else
    {
        POINT ptCursorPos = Edit_View_GetCursorPos(lpev);

        ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
        
        if (ptCursorPos.x > 0)
        {
            int nTabSize = Edit_View_GetTabSize(lpev);
            int nOffset = Edit_View_CalculateActualOffset(lpev, ptCursorPos.y, ptCursorPos.x);
            int nNewOffset = ((nOffset / nTabSize) * nTabSize);
            LPCTSTR pszChars;
            int nCurrentOffset;
            int i;

            if (nOffset == nNewOffset && nNewOffset > 0)
                nNewOffset -= nTabSize;
            
            ASSERT(nNewOffset >= 0);

            pszChars = Edit_View_GetLineChars(lpev, ptCursorPos.y);
            nCurrentOffset = 0;
            i = 0;
            
            while (nCurrentOffset < nNewOffset)
            {
                if (pszChars[i] == _T('\t'))
                    nCurrentOffset = (((nCurrentOffset / nTabSize) * nTabSize) + nTabSize);
                else
                    nCurrentOffset++;
                
                i++;
            }

            ASSERT(nCurrentOffset == nNewOffset);

            ptCursorPos.x = i;
            ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
            Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
            Edit_View_SetAnchor(lpev, ptCursorPos);
            Edit_View_SetCursorPos(lpev, ptCursorPos);
            Edit_View_EnsureVisible(lpev, ptCursorPos);
        }
    }
}

void Edit_View_OnUndo(LPEDITVIEW lpev)
{
    if (lpev->lpes != NULL && Edit_Buffer_CanUndo(lpev->lpes))
    {
        POINT ptCursorPos;

        if (Edit_Buffer_Undo(lpev, lpev->lpes, &ptCursorPos))
        {
            ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
            Edit_View_SetAnchor(lpev, ptCursorPos);
            Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
            Edit_View_SetCursorPos(lpev, ptCursorPos);
            Edit_View_EnsureVisible(lpev, ptCursorPos);
        }
    }

    SendMessage(GetParent(lpev->hwnd), WM_COMMAND, MAKEWPARAM(GetWindowLong(lpev->hwnd, GWL_ID), EN_CHANGE), (LPARAM)lpev->hwnd);
}

void Edit_View_OnRedo(LPEDITVIEW lpev)
{
    if (lpev->lpes != NULL && Edit_Buffer_CanRedo(lpev->lpes))
    {
        POINT ptCursorPos;

        if (Edit_Buffer_Redo(lpev, lpev->lpes, &ptCursorPos))
        {
            ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
            Edit_View_SetAnchor(lpev, ptCursorPos);
            Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
            Edit_View_SetCursorPos(lpev, ptCursorPos);
            Edit_View_EnsureVisible(lpev, ptCursorPos);
        }
    }

    SendMessage(GetParent(lpev->hwnd), WM_COMMAND, MAKEWPARAM(GetWindowLong(lpev->hwnd, GWL_ID), EN_CHANGE), (LPARAM)lpev->hwnd);
}

void Edit_View_OnDelete(LPEDITVIEW lpev)
{
    POINT ptSelStart, ptSelEnd, ptCursorPos;
    BOOL fControl   = IsCtrlDown();
    BOOL fShift     = IsShiftDown();

    if (!Edit_View_QueryEditable(lpev) || lpev->lpes == NULL)
        return;

    if (fControl && fShift)
    {
        int nLineCount = Edit_View_GetLineCount(lpev);
        ptCursorPos = Edit_View_GetCursorPos(lpev);

        Edit_Buffer_BeginUndoGroup(lpev->lpes, FALSE);

        if ((ptCursorPos.y + 1) == nLineCount && nLineCount == 1)
            Edit_Buffer_DeleteText(lpev, lpev->lpes, ptCursorPos.y, 0, ptCursorPos.y, Edit_View_GetLineLength(lpev, ptCursorPos.y), CE_ACTION_DELETE);
        else if ((ptCursorPos.y + 1) == nLineCount)
            Edit_Buffer_DeleteText(lpev, lpev->lpes, ptCursorPos.y - 1, Edit_View_GetLineLength(lpev, ptCursorPos.y - 1), ptCursorPos.y, Edit_View_GetLineLength(lpev, ptCursorPos.y), CE_ACTION_DELETE);
        else
            Edit_Buffer_DeleteText(lpev, lpev->lpes, ptCursorPos.y, 0, ptCursorPos.y + 1, 0, CE_ACTION_DELETE);

        if ((ptCursorPos.y + 1) == nLineCount && nLineCount != 1)
            ptCursorPos.y--;

        ptCursorPos.x = 0;

        ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
        Edit_View_SetAnchor(lpev, ptCursorPos);
        Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
        Edit_View_SetCursorPos(lpev, ptCursorPos);
        Edit_View_EnsureVisible(lpev, ptCursorPos);

        Edit_Buffer_FlushUndoGroup(lpev, lpev->lpes);
    }
    else if (fControl)
    {
        if (!Edit_View_IsSelection(lpev))
            TextMove_MoveWordLeft(lpev, TRUE);

        if (Edit_View_IsSelection(lpev))
        {
            Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);
            
            Edit_Buffer_BeginUndoGroup(lpev->lpes, FALSE);
            Edit_Buffer_DeleteText(lpev, lpev->lpes, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, CE_ACTION_DELETE);

            ASSERT_VALIDTEXTPOS(lpev, ptSelStart);
            Edit_View_SetAnchor(lpev, ptSelStart);
            Edit_View_SetSelection(lpev, ptSelStart, ptSelStart);
            Edit_View_SetCursorPos(lpev, ptSelStart);
            Edit_View_EnsureVisible(lpev, ptSelStart);

            Edit_Buffer_FlushUndoGroup(lpev, lpev->lpes);
        }
    }
    else
    {
        Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);

        if (Point_Equal(&ptSelStart, &ptSelEnd))
        {
            if (ptSelEnd.x == Edit_View_GetLineLength(lpev, ptSelEnd.y))
            {
                if (ptSelEnd.y == Edit_View_GetLineCount(lpev) - 1)
                    return;

                ptSelEnd.y++;
                ptSelEnd.x = 0;
            }
            else
            {
                ptSelEnd.x++;
            }
        }

        ptCursorPos = ptSelStart;
        ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
        Edit_View_SetAnchor(lpev, ptCursorPos);
        Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
        Edit_View_SetCursorPos(lpev, ptCursorPos);
        Edit_View_EnsureVisible(lpev, ptCursorPos);

        Edit_Buffer_DeleteText(lpev, lpev->lpes, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, CE_ACTION_DELETE);
    }
}

void Edit_View_SetTextType(LPEDITVIEW lpev, LPCTSTR pszExt)
{
    int nEoln;

    lpev->lpTextDef = PCP_Edit_GetTextDef(pszExt);

    Edit_View_SetTabSize(lpev, lpev->lpTextDef->dwTabSize);
    Edit_View_SetViewTabs(lpev, (lpev->lpTextDef->dwFlags & SRCOPT_SHOWTABS));
    Edit_View_SetSelectionMargin(lpev, (lpev->lpTextDef->dwFlags & SRCOPT_SELMARGIN));

    if (lpev->lpTextDef->dwFlags & SRCOPT_EOLNDOS)
        nEoln = CRLF_STYLE_DOS;
    else if (lpev->lpTextDef->dwFlags & SRCOPT_EOLNUNIX)
        nEoln = CRLF_STYLE_UNIX;
    else if (lpev->lpTextDef->dwFlags & SRCOPT_EOLNMAC)
        nEoln = CRLF_STYLE_MAC;
    else /* eoln auto */
        nEoln = CRLF_STYLE_DOS; //CRLF_STYLE_AUTOMATIC;

    Edit_Buffer_SetCRLFMode(lpev->lpes, nEoln);
    Edit_View_SetFlags(lpev, lpev->lpTextDef->dwFlags);
}

void Edit_View_SetFlags(LPEDITVIEW lpev, DWORD dwFlags)
{
    if (lpev->dwFlags != dwFlags)
    {
        lpev->dwFlags = dwFlags;

        if (IsWindow(lpev->hwnd))
            InvalidateRect(lpev->hwnd, NULL, FALSE);
    }
}

void Edit_View_OnMouseWheel(LPEDITVIEW lpev, UINT nFlags, short zDelta, POINT pt)
{
    int nLineCount  = Edit_View_GetLineCount(lpev);
    int nNewTopLine = lpev->iTopLine - zDelta / 40;

    if (nNewTopLine < 0)
    nNewTopLine = 0;
    if (nNewTopLine >= nLineCount)
        nNewTopLine = nLineCount - 1;
    
    if (lpev->iTopLine != nNewTopLine)
    {
        int nScrollLines = lpev->iTopLine - nNewTopLine;

        lpev->iTopLine = nNewTopLine;

        ScrollWindow(lpev->hwnd, 0, nScrollLines * Edit_View_GetLineHeight(lpev), NULL, NULL);
        UpdateWindow(lpev->hwnd);
    }

    Edit_View_RecalcVertScrollBar(lpev, TRUE);
}


DWORD Edit_View_GetFlags(LPEDITVIEW lpev)
{
    return (lpev->dwFlags);
}

void Edit_View_FindIncremental(LPEDITVIEW lpev, BOOL bFindNext)
{
    POINT ptMatchStart, ptMatchEnd;

    // Goto position were to start
    if (lpev->pszIncrementalSearch == NULL)
    {
        Edit_View_SetSelection(lpev, lpev->ptIncrementalSearchStartPos, lpev->ptIncrementalSearchStartPos);
        Edit_View_SetCursorPos(lpev, lpev->ptIncrementalSearchStartPos);
        Edit_View_EnsureVisible(lpev, lpev->ptIncrementalSearchStartPos);

        return;
    }

    // Start search at cursor pos
    if (bFindNext)
    {
        POINT ptSelStart, ptSelEnd;
        
        Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);
        lpev->ptIncrementalSearchStartPos = (lpev->bIncrementalSearchBackwards) ? ptSelStart : ptSelEnd;
    }

    lpev->bIncrementalFound = Edit_View_FindText(lpev, lpev->pszIncrementalSearch,
                                lpev->ptIncrementalSearchStartPos, (lpev->bIncrementalSearchBackwards) ? FIND_DIRECTION_UP : 0,
                                TRUE, &ptMatchStart);

    if (!lpev->bIncrementalFound)
    {
        MessageBeep(MB_OK);
        Edit_View_IncrementalSearchUpdateStatusbar(lpev);

        return;
    }

    // Select found text
    ptMatchEnd = ptMatchStart;
    ptMatchEnd.x += _tcslen(lpev->pszIncrementalSearch);
    Edit_View_SetSelection(lpev, ptMatchStart, ptMatchEnd);
    Edit_View_SetCursorPos(lpev, ptMatchEnd);
    Edit_View_EnsureVisible(lpev, ptMatchEnd);

    Edit_View_IncrementalSearchUpdateStatusbar(lpev);
}

void Edit_View_FindIncrementalNext(LPEDITVIEW lpev, BOOL bForward)
{
    if (!lpev->bIncrementalSearchBackwards && !lpev->bIncrementalSearchForwards)
    {
        if (lpev->pszIncrementalSearch != NULL)
            lpev->pszOldIncrementalSearch = String_Duplicate(lpev->pszIncrementalSearch);

        Mem_Free(lpev->pszIncrementalSearch);
        lpev->ptIncrementalSearchStartPos = lpev->ptCursorPosBeforeIncrementalSearch = Edit_View_GetCursorPos(lpev);
        Edit_View_GetSelection(lpev, &lpev->ptSelStartBeforeIncrementalSearch, &lpev->ptSelEndBeforeIncrementalSearch);
    }
    else if (lpev->bIncrementalSearchForwards)
    {
        if (lpev->pszIncrementalSearch == NULL)
        {
            lpev->pszIncrementalSearch = String_Duplicate(lpev->pszOldIncrementalSearch);
            Mem_Free(lpev->pszOldIncrementalSearch);
            Edit_View_FindIncremental(lpev, FALSE);
        }
        else
        {
            Edit_View_FindIncremental(lpev, TRUE);
        }

        return;
    }

    lpev->bIncrementalSearchForwards    = bForward;
    lpev->bIncrementalSearchBackwards   = !bForward;
    lpev->bIncrementalFound             = TRUE;
    Edit_View_FindIncremental(lpev, FALSE);
}

void Edit_View_FindIncrementalEnd(LPEDITVIEW lpev, BOOL bReturnToOldPlace)
{
    lpev->bIncrementalSearchBackwards = lpev->bIncrementalSearchForwards = FALSE;
    lpev->bIncrementalFound = FALSE;
    Mem_Free(lpev->pszIncrementalSearch);
    Mem_Free(lpev->pszOldIncrementalSearch);
    lpev->pszIncrementalSearch      = NULL;
    lpev->pszOldIncrementalSearch   = NULL;

    if (bReturnToOldPlace)
    {
        Edit_View_SetSelection(lpev, lpev->ptSelStartBeforeIncrementalSearch, lpev->ptSelEndBeforeIncrementalSearch);
        Edit_View_SetCursorPos(lpev, lpev->ptCursorPosBeforeIncrementalSearch);
        Edit_View_EnsureVisible(lpev, lpev->ptCursorPosBeforeIncrementalSearch);
    }

    Edit_View_IncrementalSearchUpdateStatusbar(lpev);
}

void Edit_View_IncrementalSearchUpdateStatusbar(LPEDITVIEW lpev)
{
    NMINCREMENTALSTATUS nmis;

    INITSTRUCT(nmis, FALSE);
    nmis.nmhdr.code     = PEN_INCREMENTALSEARCHUPDATE;
    nmis.nmhdr.hwndFrom = lpev->hwnd;
    nmis.nmhdr.idFrom   = GetWindowLong(lpev->hwnd, GWL_ID);
    if (lpev->bIncrementalSearchForwards)
        nmis.uStatus = PE_INCREMENTAL_STATUS_FORWARD;
    else if (lpev->bIncrementalSearchBackwards)
        nmis.uStatus = PE_INCREMENTAL_STATUS_REVERSE;
    else
        nmis.uStatus = PE_INCREMENTAL_STATUS_ENDED;

    if (!lpev->bIncrementalFound)
        nmis.uStatus |= PE_INCREMENTAL_STATUS_FAILING;

    nmis.pszSearchText = lpev->pszIncrementalSearch;

    SendMessage(GetParent(lpev->hwnd), WM_NOTIFY, (WPARAM)nmis.nmhdr.idFrom, (LPARAM)&nmis);
}

BOOL Edit_View_OnFindNext(LPEDITVIEW lpev)
{
    if (lpev->bLastSearch)
    {
        POINT ptFoundPos, ptSearchPos = lpev->ptCursorPos;

        if (Edit_View_IsSelection(lpev))
        {
            POINT ptDummy;

            if (lpev->dwLastSearchFlags & FIND_DIRECTION_UP)
                Edit_View_GetSelection(lpev, &ptSearchPos, &ptDummy);
            else
                Edit_View_GetSelection(lpev, &ptDummy, &ptSearchPos);
        }

        if (!Edit_View_FindText(lpev, lpev->pszLastFindWhat, ptSearchPos, lpev->dwLastSearchFlags, TRUE, &ptFoundPos))
            return (FALSE);

        Edit_View_HighlightText(lpev, ptFoundPos, lpev->iLastFindWhatLen, (lpev->dwLastSearchFlags & FIND_DIRECTION_UP) != 0);
        lpev->bMultipleSearch = TRUE; // More search

        return (TRUE);
    }
    else if (lpev->bIncrementalFound)
    {
        Edit_View_FindIncrementalNext(lpev, lpev->bIncrementalSearchForwards);

        return (TRUE);
    }

    return (FALSE);
}

BOOL Edit_View_OnFindPrev(LPEDITVIEW lpev)
{
    DWORD dwSaveSearchFlags = lpev->dwLastSearchFlags;
    BOOL bSuccess;
    
    if ((lpev->dwLastSearchFlags & FIND_DIRECTION_UP) != 0)
        lpev->dwLastSearchFlags &= ~FIND_DIRECTION_UP;
    else
        lpev->dwLastSearchFlags |= FIND_DIRECTION_UP;

    bSuccess = Edit_View_OnFindNext(lpev);
    lpev->dwLastSearchFlags = dwSaveSearchFlags;

    return (bSuccess);
}

BOOL Edit_View_FindReplaceSelection(LPEDITVIEW lpev, LPCTSTR pszNewText, DWORD dwFlags)
{
    POINT ptCursorPos;
    POINT ptEndOfBlock;
    int x, y;

    ASSERT (pszNewText != NULL);

    if (!Edit_View_IsSelection(lpev))
        return (FALSE);

    Edit_Buffer_BeginUndoGroup(lpev->lpes, FALSE);
    
    Edit_View_DeleteCurrentSelection(lpev);
    
    ptCursorPos = Edit_View_GetCursorPos(lpev);
    ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);

    if (dwFlags & FIND_REG_EXP)
    {
        LPTSTR lpszNewStr;

        if (lpev->pszMatched != NULL && !RxReplace(pszNewText, lpev->pszMatched, lpev->iLastFindWhatLen, lpev->rxMatch, &lpszNewStr, &lpev->iLastReplaceLen))
        {
            LPTSTR pszText;

            if (lpszNewStr && lpev->iLastReplaceLen > 0)
            {
                pszText = (LPTSTR)Mem_Alloc(lpev->iLastReplaceLen + 1);
                _tcscpy(pszText, lpszNewStr);
            }
            else
            {
                pszText = _T("");
            }

            Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x, pszText, &y, &x, CE_ACTION_REPLACE);

            if (lpszNewStr != NULL)
                free(lpszNewStr);
        }
    }
    else
    {
        Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x, pszNewText, &y, &x, CE_ACTION_REPLACE);
        lpev->iLastReplaceLen = _tcslen(pszNewText);
    }

    ptEndOfBlock.x = x;
    ptEndOfBlock.y = y;

    ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
    ASSERT_VALIDTEXTPOS(lpev, ptEndOfBlock);
    Edit_View_SetAnchor(lpev, ptEndOfBlock);
    Edit_View_SetSelection(lpev, ptCursorPos, ptEndOfBlock);
    Edit_View_SetCursorPos(lpev, ptEndOfBlock);
    Edit_View_EnsureVisible(lpev, ptEndOfBlock);
    Edit_Buffer_FlushUndoGroup(lpev, lpev->lpes);

    return (TRUE);
}

void Edit_View_ReplaceSelection(LPEDITVIEW lpev, LPTSTR pszText)
{
    int x, y;
    POINT ptCursorPos;

    if (!Edit_View_QueryEditable(lpev) || lpev->lpes == NULL)
        return;

    Edit_Buffer_BeginUndoGroup(lpev->lpes, FALSE);

    Edit_View_DeleteCurrentSelection(lpev);

    ptCursorPos = Edit_View_GetCursorPos(lpev);

    ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);

    Edit_Buffer_InsertText(lpev, lpev->lpes, ptCursorPos.y, ptCursorPos.x, pszText, &y, &x, CE_ACTION_PASTE);
    ptCursorPos.x = x;
    ptCursorPos.y = y;

    ASSERT_VALIDTEXTPOS(lpev, ptCursorPos);
    Edit_View_SetAnchor(lpev, ptCursorPos);
    Edit_View_SetSelection(lpev, ptCursorPos, ptCursorPos);
    Edit_View_SetCursorPos(lpev, ptCursorPos);
    Edit_View_EnsureVisible(lpev, ptCursorPos);

    Edit_Buffer_FlushUndoGroup(lpev, lpev->lpes);
}

BOOL Edit_View_FindSelection(LPEDITVIEW lpev, BOOL fForward)
{
    LPTSTR pszSearchText;
    int nLineCount;
    POINT ptTextPos;
    POINT ptSearchPos;
    POINT ptBlockStart = { 0, 0 }, ptBlockEnd;
    POINT ptDummy;
    DWORD dwSearchFlags;
    
    if (!Edit_View_IsSelection(lpev))
    {
        // Section: select current word
        POINT ptStart, ptEnd, ptAnchor, pt;
        ptAnchor = ptStart = pt = Edit_View_GetCursorPos(lpev);

        if (pt.y < ptAnchor.y || pt.y == ptAnchor.y && pt.x < ptAnchor.x)
        {
            ptStart = TextMove_WordToLeft(lpev, pt);
            ptEnd = TextMove_WordToRight(lpev, ptAnchor);
        }
        else
        {
            ptStart = TextMove_WordToLeft(lpev, ptAnchor);
            ptEnd = TextMove_WordToRight(lpev, pt);
        }

        pt = ptEnd;

        Edit_View_SetCursorPos(lpev, pt);
        Edit_View_SetSelection(lpev, ptStart, ptEnd);
        Edit_View_SetAnchor(lpev, ptAnchor);
        Edit_View_EnsureVisible(lpev, pt);
    }
    
    if (Edit_View_IsSelection(lpev))
    {
        POINT ptSelStart, ptSelEnd;

        Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);          

        if (ptSelStart.y == ptSelEnd.y)
        {
            LPCTSTR pszChars = Edit_View_GetLineChars(lpev, ptSelStart.y);
            int nChars = ptSelEnd.x - ptSelStart.x;

            pszSearchText = (LPTSTR)Mem_AllocStr(nChars + SZ);
            _tcsncpy(pszSearchText, (pszChars + ptSelStart.x), nChars);
        }
    }
    else
    {
        return (FALSE); // cannot auto-locate any selection
    }

    nLineCount = Edit_View_GetLineCount(lpev);

    ptBlockEnd.x = Edit_View_GetLineLength(lpev, nLineCount - 1);
    ptBlockEnd.y = nLineCount - 1;

    dwSearchFlags = lpev->dwLastSearchFlags;

    if (fForward) // Find Previous
    {
        dwSearchFlags = 0; // Search downwards
        Edit_View_GetSelection(lpev, &ptDummy, &ptSearchPos);
    }
    else
    {
        dwSearchFlags |= FIND_DIRECTION_UP; // Search upwards
        Edit_View_GetSelection(lpev, &ptSearchPos, &ptDummy);
    }

    if (!Edit_View_FindTextInBlock(lpev, pszSearchText, ptSearchPos,
        ptBlockStart, ptBlockEnd, dwSearchFlags, TRUE, &ptTextPos))
    {
        return (FALSE);
    }

    lpev->bLastSearch = TRUE;
    
    if (lpev->pszLastFindWhat != NULL)
        Mem_Free(lpev->pszLastFindWhat);
    
    lpev->pszLastFindWhat = String_Duplicate(pszSearchText);
    Edit_View_HighlightText(lpev, ptTextPos, _tcslen(pszSearchText), FALSE);
    
    Mem_Free(pszSearchText);

    return (TRUE);
}

void Edit_View_AddSourceDefinition(LPTEXTDEF lptd)
{
    EnterCriticalSection(&s_SyntaxCriticalSection);

    List_AddNodeAtTail(s_pSyntaxList, lptd);

    LeaveCriticalSection(&s_SyntaxCriticalSection);
}

void Edit_View_GotoLastChange(LPEDITVIEW lpev)
{
    POINT ptLastChangePos = Edit_Buffer_GetLastChangePos(lpev->lpes);

    if (ptLastChangePos.x < 0 || ptLastChangePos.y < 0)
        return;

    Edit_View_SetCursorPos(lpev, ptLastChangePos);
    Edit_View_SetSelection(lpev, ptLastChangePos, ptLastChangePos);
    Edit_View_EnsureVisible(lpev, ptLastChangePos);
}

/*
    if (Edit_Buffer_GetLastActionDescription(lpev->lpes) == CE_ACTION_AUTOINDENT)
    {
        POINT ptLastAction = Edit_Buffer_GetLastChangePos(lpev->lpes);

        if (ptLastAction.y != lpev->ptCursorPos.y)
        {
            LPCTSTR pszLine = Edit_View_GetLineChars(lpev, ptLastAction.y);
            BOOL bSpaceOnly = TRUE;

            while (*pszLine != _T('\0'))
            {
                if (!_istspace(*pszLine))
                {
                    bSpaceOnly = FALSE;
                    break;
                }

                pszLine++;
            }

            if (bSpaceOnly)
                Edit_Buffer_DeleteText(lpev, lpev->lpes, ptLastAction.y, 0, ptLastAction.y, Edit_View_GetLineLength(lpev, ptLastAction.y), CE_ACTION_DELETE);
        }
    }
*/

void Edit_View_LoadFile(LPEDITVIEW lpev, LPCTSTR pszFileName)
{
    Edit_Buffer_FreeAll(lpev->lpes);
    Edit_Buffer_LoadFromFile(lpev, lpev->lpes, pszFileName, lpev->lpes->iCRLFMode);
    RedrawWindow(lpev->hwnd, NULL, NULL, RDW_INVALIDATE|RDW_INTERNALPAINT|RDW_ERASE|RDW_ERASENOW|RDW_UPDATENOW|RDW_NOFRAME);
}

HCURSOR Edit_View_SetMarginCursor(LPEDITVIEW lpev, HCURSOR hCursor)
{
    HCURSOR hOldCursor = lpev->hCursor;

    lpev->hCursor = hCursor;

    return (hOldCursor);
}

HCURSOR Edit_View_GetMarginCursor(LPEDITVIEW lpev)
{
    return (lpev->hCursor);
}

BOOL Edit_View_GotoLine(LPEDITVIEW lpev, int nLine, BOOL bExtendSelection)
{
    int nLineCount = Edit_View_GetLineCount(lpev);
    POINT ptSelStart, ptSelEnd;

    if (nLine <= 0)
        nLine = 1;

    if (--nLine < nLineCount)
    {
        if (bExtendSelection)
        {
            Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);
            ptSelEnd.y = nLine;
            ptSelEnd.x = 0;
            Edit_View_SetSelection(lpev, ptSelStart, ptSelEnd);
        }
        else
        {
            ptSelStart.y = ptSelEnd.y = nLine;
            ptSelStart.x = ptSelEnd.x = 0;
            Edit_View_SetSelection(lpev, ptSelStart, ptSelEnd);
        }

        return (TRUE);
    }

    return (FALSE);
}

/*
DROPEFFECT Edit_View_OnDragEnter(HWND hwnd, IDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
  if (!pDataObject->IsDataAvailable(pDataObject(CF_TEXT))
    {
      m_pOwner->HideDropIndicator ();
      return DROPEFFECT_NONE;
    }
  m_pOwner->ShowDropIndicator (point);
  if (dwKeyState & MK_CONTROL)
    return DROPEFFECT_COPY;
  return DROPEFFECT_MOVE;
}

void CEditDropTargetImpl::
OnDragLeave (CWnd * pWnd)
{
  m_pOwner->HideDropIndicator ();
}

DROPEFFECT CEditDropTargetImpl::
OnDragOver (CWnd * pWnd, COleDataObject * pDataObject, DWORD dwKeyState, CPoint point)
{
  //
  // [JRT]
  //
  bool bDataSupported = false;

  if ((!m_pOwner) ||            // If No Owner
        (!(m_pOwner->QueryEditable ())) ||   // Or Not Editable
        (m_pOwner->GetDisableDragAndDrop ()))    // Or Drag And Drop Disabled

    {
      m_pOwner->HideDropIndicator ();   // Hide Drop Caret

      return DROPEFFECT_NONE;   // Return DE_NONE

    }
  //  if ((pDataObject->IsDataAvailable( CF_TEXT ) ) ||       // If Text Available
  //          ( pDataObject -> IsDataAvailable( xxx ) ) ||    // Or xxx Available
  //          ( pDataObject -> IsDataAvailable( yyy ) ) )     // Or yyy Available
  if (pDataObject->IsDataAvailable (CF_TEXT))   // If Text Available

    {
      bDataSupported = true;    // Set Flag

    }
  if (!bDataSupported)          // If No Supported Formats Available

    {
      m_pOwner->HideDropIndicator ();   // Hide Drop Caret

      return DROPEFFECT_NONE;   // Return DE_NONE

    }
  m_pOwner->ShowDropIndicator (point);
  if (dwKeyState & MK_CONTROL)
    return DROPEFFECT_COPY;
  return DROPEFFECT_MOVE;
}

BOOL CEditDropTargetImpl::
OnDrop (CWnd * pWnd, COleDataObject * pDataObject, DROPEFFECT dropEffect, CPoint point)
{
  //
  // [JRT]            ( m_pOwner -> GetDisableDragAndDrop() ) )       // Or Drag And Drop Disabled
  //
  bool bDataSupported = false;

  m_pOwner->HideDropIndicator ();   // Hide Drop Caret

  if ((!m_pOwner) ||            // If No Owner
        (!(m_pOwner->QueryEditable ())) ||   // Or Not Editable
        (m_pOwner->GetDisableDragAndDrop ()))    // Or Drag And Drop Disabled

    {
      return DROPEFFECT_NONE;   // Return DE_NONE

    }
  //  if( ( pDataObject -> IsDataAvailable( CF_TEXT ) ) ||    // If Text Available
  //          ( pDataObject -> IsDataAvailable( xxx ) ) ||    // Or xxx Available
  //          ( pDataObject -> IsDataAvailable( yyy ) ) )     // Or yyy Available
  if (pDataObject->IsDataAvailable (CF_TEXT))   // If Text Available

    {
      bDataSupported = true;    // Set Flag

    }
  if (!bDataSupported)          // If No Supported Formats Available

    {
      return DROPEFFECT_NONE;   // Return DE_NONE

    }
  return (m_pOwner->DoDropText (pDataObject, point));   // Return Result Of Drop

}

DROPEFFECT CEditDropTargetImpl::
OnDragScroll (CWnd * pWnd, DWORD dwKeyState, CPoint point)
{
  ASSERT (m_pOwner == pWnd);
  m_pOwner->DoDragScroll (point);

  if (dwKeyState & MK_CONTROL)
    return DROPEFFECT_COPY;
  return DROPEFFECT_MOVE;
}

void CCrystalEditView::
DoDragScroll (const CPoint & point)
{
  CRect rcClientRect;
  GetClientRect (rcClientRect);
  if (point.y < rcClientRect.top + DRAG_BORDER_Y)
    {
      HideDropIndicator ();
      ScrollUp ();
      UpdateWindow ();
      ShowDropIndicator (point);
      return;
    }
  if (point.y >= rcClientRect.bottom - DRAG_BORDER_Y)
    {
      HideDropIndicator ();
      ScrollDown ();
      UpdateWindow ();
      ShowDropIndicator (point);
      return;
    }
  if (point.x < rcClientRect.left + GetMarginWidth () + DRAG_BORDER_X)
    {
      HideDropIndicator ();
      ScrollLeft ();
      UpdateWindow ();
      ShowDropIndicator (point);
      return;
    }
  if (point.x >= rcClientRect.right - DRAG_BORDER_X)
    {
      HideDropIndicator ();
      ScrollRight ();
      UpdateWindow ();
      ShowDropIndicator (point);
      return;
    }
}

BOOL CCrystalEditView::
DoDropText (COleDataObject * pDataObject, const CPoint & ptClient)
{
  HGLOBAL hData = pDataObject->GetGlobalData (CF_TEXT);
  if (hData == NULL)
    return FALSE;

  CPoint ptDropPos = ClientToText (ptClient);
  if (IsDraggingText () && IsInsideSelection (ptDropPos))
    {
      SetAnchor (ptDropPos);
      SetSelection (ptDropPos, ptDropPos);
      SetCursorPos (ptDropPos);
      EnsureVisible (ptDropPos);
      return FALSE;
    }

  LPTSTR pszText = (LPTSTR)::GlobalLock (hData);
  if (pszText == NULL)
    return FALSE;

  int x, y;
  m_pTextBuffer->InsertText (this, ptDropPos.y, ptDropPos.x, pszText, y, x, CE_ACTION_DRAGDROP);  //   [JRT]

  CPoint ptCurPos (x, y);
  ASSERT_VALIDTEXTPOS (ptCurPos);
  SetAnchor (ptDropPos);
  SetSelection (ptDropPos, ptCurPos);
  SetCursorPos (ptCurPos);
  EnsureVisible (ptCurPos);

  ::GlobalUnlock (hData);
  return TRUE;
}
*/
