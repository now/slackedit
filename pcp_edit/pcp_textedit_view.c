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
#include "pcp_textedit_view.h"
#include "pcp_textedit_interface.h"
#include "pcp_textedit_move.h"
#include "pcp_textedit_buffer.h"
#include "pcp_textedit_syntax.h"

/* pcp_paint */
#include <pcp_paint.h>

/* pcp_generic */
#include <pcp_linkedlist.h>
#include <pcp_mem.h>
#include <pcp_point.h>
#include <pcp_rect.h>
#include <pcp_string.h>
#include <pcp_clipboard.h>

#define TAB_CHARACTER           _T('\xBB');
#define SPACE_CHARACTER         _T('\x95');
#define SMOOTH_SCROLL_FACTOR    6

void TextEdit_View_OnChar(LPTEXTEDITVIEW lpte, UINT nChar, UINT nRepCnt, UINT nFlags);
void TextEdit_View_OnKeyDown(LPTEXTEDITVIEW lpte, UINT nVirtKey, UINT cRepeat, UINT fFlags);
void TextEdit_View_OnCut(LPTEXTEDITVIEW lpte);
void TextEdit_View_OnPaste(LPTEXTEDITVIEW lpte);
int TextEdit_View_OnCreate(LPTEXTEDITVIEW lpte, LPCREATESTRUCT lpCreateStruct);
void TextEdit_View_OnKillFocus(LPTEXTEDITVIEW lpte, HWND hwndGetFocus);
void TextEdit_View_OnSysColorChange(LPTEXTEDITVIEW lpte);
void TextEdit_View_OnSetFocus(LPTEXTEDITVIEW lpte, HWND hwndLoseFocus);
void TextEdit_View_OnHScroll(LPTEXTEDITVIEW lpte, UINT nSBCode, UINT nPos, HWND hwndScrollBar);
BOOL TextEdit_View_OnSetCursor(LPTEXTEDITVIEW lpte, UINT nHitTest, UINT wMouseMsg);
void TextEdit_View_OnVScroll(LPTEXTEDITVIEW lpte, UINT nSBCode, UINT nPos, HWND hwndScrollBar);
void TextEdit_View_OnDestroy(LPTEXTEDITVIEW lpte);
BOOL TextEdit_View_OnEraseBkgnd(HDC hdc);
void TextEdit_View_OnSize(LPTEXTEDITVIEW lpte, UINT nType, int cx, int cy);
void TextEdit_View_OnPaint(LPTEXTEDITVIEW lpte);
void TextEdit_View_OnClear(LPTEXTEDITVIEW lpte);
void TextEdit_View_OnBackspace(LPTEXTEDITVIEW lpte);
void TextEdit_View_OnMouseWheel(LPTEXTEDITVIEW lpte, UINT nFlags, short zDelta, POINT pt);
DWORD TextEdit_View_GetFlags(LPTEXTEDITVIEW lpte);

/* static */
COLORREF TextEdit_View_GetColor(LPTEXTEDITVIEW lpte, int iColorIndex);
DWORD TextEdit_View_GetLineFlags(LPTEXTEDITVIEW lpte, int iLineIndex);
int TextEdit_View_GetTabSize(LPTEXTEDITVIEW lpte);
void TextEdit_View_SetTabSize(LPTEXTEDITVIEW lpte, int iTabSize);
int TextEdit_View_GetScreenChars(LPTEXTEDITVIEW lpte);

void TextEdit_View_ExpandChars(LPTEXTEDITVIEW lpte, LPCTSTR pszChars, int iOffset, int iCount, LPTSTR *pszLine);
void TextEdit_View_DrawLineHelperImpl(LPTEXTEDITVIEW lpte, HDC hdc, LPPOINT lptOrigin, const RECT rcClip, LPCTSTR pszChars, int iOffset, int iCount);
void TextEdit_View_DrawLineHelper(LPTEXTEDITVIEW lpte, HDC hdc, LPPOINT lptOrigin, const LPRECT lprcClip, int iColorIndex, LPCTSTR pszChars, int iOffset, int iCount, POINT ptTextPos);
void TextEdit_View_GetLineColors(LPTEXTEDITVIEW lpte, int iLineIndex, LPCOLORREF lpcrBkgnd, LPCOLORREF lpcrText, LPBOOL lpbDrawWhitespace);
DWORD TextEdit_View_GetParseCookie(LPTEXTEDITVIEW lpte, int iLineIndex);
void TextEdit_View_DrawSingleLine(LPTEXTEDITVIEW lpte, HDC *fdc, const LPRECT lprc, int iLineIndex);
DWORD TextEdit_View_ParseLine(LPTEXTEDITVIEW lpte, DWORD dwCookie, int nLineIndex, LPTEXTBLOCK pBuf, LPINT piActualItems);
void TextEdit_View_DrawMargin(LPTEXTEDITVIEW lpte, HDC *fdc, const LPRECT lprc, int iLineIndex);
void TextEdit_View_ShowCursor(LPTEXTEDITVIEW lpte);
void TextEdit_View_HideCursor(LPTEXTEDITVIEW lpte);
void TextEdit_View_RecalcVertScrollBar(LPTEXTEDITVIEW lpte, BOOL bPositionOnly);
void TextEdit_View_RecalcHorzScrollBar(LPTEXTEDITVIEW lpte, BOOL bPositionOnly);
void TextEdit_View_InvalidateLines(LPTEXTEDITVIEW lpte, int iLine1, int iLine2, BOOL bInvalidateMargin);
void TextEdit_View_SetFlags(LPTEXTEDITVIEW lpte, DWORD dwFlags);
BOOL TextEdit_View_QueryEditable(LPTEXTEDITVIEW lpte);
POINT TextEdit_View_TextToClient(LPTEXTEDITVIEW lpte, POINT pt);
void TextEdit_View_CalcLineCharDim(LPTEXTEDITVIEW lpte);
int TextEdit_View_GetCharWidth(LPTEXTEDITVIEW lpte);
BOOL TextEdit_View_GetItalic(int iColorIndex);
BOOL TextEdit_View_GetBold(int iColorIndex);
void TextEdit_View_ResetView(LPTEXTEDITVIEW lpte);

void TextEdit_View_OnCopy(LPTEXTEDITVIEW lpte);

LRESULT CALLBACK TextEdit_View_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPTEXTEDITVIEW lpte = TextEdit_Internal_GetInterface(hwnd)->lpte;

    switch (uMsg)
    {
    case WM_NCCREATE:
        if (TextEdit_Internal_CreateInterface(hwnd))
            return (TRUE);
    return (FALSE);
    case WM_CREATE:
        lpte->hwnd = hwnd;
        TextEdit_View_OnCreate(lpte, (LPCREATESTRUCT)lParam);
    return (0);
    case WM_DESTROY:
        TextEdit_View_OnDestroy(lpte);
    return (0);
    case WM_NCDESTROY:
        TextEdit_Internal_DestroyInterface(hwnd);
    return (0);
    case WM_ERASEBKGND:
    return (TextEdit_View_OnEraseBkgnd((HDC)wParam));
    case WM_SIZE:
        TextEdit_View_OnSize(lpte, (UINT)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));
    return (0);
    case WM_VSCROLL:
        TextEdit_View_OnVScroll(lpte, (UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HWND)lParam);
    return (0);
    case WM_SETCURSOR:
    return TextEdit_View_OnSetCursor(lpte, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
    case WM_LBUTTONDOWN:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        TextEdit_View_OnLButtonDown(lpte, (UINT)wParam, pt);
    }
    return (0);
    case WM_SETFOCUS:
        TextEdit_View_OnSetFocus(lpte, (HWND)lParam);
    return (0);
    case WM_HSCROLL:
        TextEdit_View_OnHScroll(lpte, (UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HWND)lParam);
    return (0);
    case WM_LBUTTONUP:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        TextEdit_View_OnLButtonUp(lpte, (UINT)wParam, pt);
    }
    return (0);
    case WM_MOUSEMOVE:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        TextEdit_View_OnMouseMove(lpte, (UINT)wParam, pt);
    }
    break;
    case WM_TIMER:
        TextEdit_View_OnTimer(lpte, (UINT)wParam);
    return (0);
    case WM_KILLFOCUS:
        TextEdit_View_OnKillFocus(lpte, (HWND)wParam);
    return (0);
    case WM_LBUTTONDBLCLK:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        TextEdit_View_OnLButtonDblClk(lpte, (UINT)wParam, pt);
    }
    return (0);
    case WM_RBUTTONDOWN:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        TextEdit_View_OnRButtonDown(lpte, (UINT)wParam, pt);
    }
    return (0);
    case WM_SYSCOLORCHANGE:
        TextEdit_View_OnSysColorChange(lpte);
    break;
    case WM_PAINT:
        TextEdit_View_OnPaint(lpte);
    return (0);
    case WM_CHAR:
        TextEdit_View_OnChar(lpte, (UINT)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
    return (0);
    case WM_KEYDOWN:
        TextEdit_View_OnKeyDown(lpte, (UINT)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
    return (0);
    case WM_MOUSEACTIVATE:
        if (!lpte->bFocused)
        {
            SetFocus(hwnd);
            return (MA_ACTIVATE);
        }
    break;
    case WM_COPY:
        TextEdit_View_OnCopy(lpte);
    break;
    case WM_PASTE:
        TextEdit_View_OnPaste(lpte);
    break;
    case WM_CUT:
        TextEdit_View_OnCut(lpte);
    break;
    case WM_CLEAR:
        TextEdit_View_OnClear(lpte);
    break;
    case WM_SETFONT:
    {
        LOGFONT lf;

        GetObject((HFONT)wParam, sizeof(LOGFONT), &lf);

        TextEdit_View_SetFont(lpte, &lf);
    }
    return (0);
    case WM_MOUSEWHEEL:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };

        TextEdit_View_OnMouseWheel(lpte, (UINT)LOWORD(wParam), (short)HIWORD(wParam), pt);
    }
    break;
    default:
    break;
    }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}

void TextEdit_View_GetSelection(LPTEXTEDITVIEW lpte, POINT *lptStart, POINT *lptEnd)
{
    TextEdit_View_PrepareSelBounds(lpte);

    lptStart->x = lpte->ptDrawSelStart.x;
    lptStart->y = lpte->ptDrawSelStart.y;
    lptEnd->x   = lpte->ptDrawSelEnd.x;
    lptEnd->y   = lpte->ptDrawSelEnd.y;
}

int TextEdit_View_GetLineActualLength(LPTEXTEDITVIEW lpte, int iLineIndex)
{
    int iLineCount      = TextEdit_Buffer_GetLineCount(lpte->lpes);
    int iActualLength   = 0;
    int iLength         = TextEdit_Buffer_GetLineLength(lpte->lpes, iLineIndex);

    ASSERT(iLineCount > 0);
    ASSERT(iLineIndex >= 0 && iLineIndex < iLineCount);
    
    if (lpte->piActualLineLength == NULL)
    {
        lpte->piActualLineLength = (LPINT)Mem_Alloc(iLineCount * sizeof(int));
        memset(lpte->piActualLineLength, 0xFF, sizeof(int) * iLineCount);
        lpte->iActualLengthArraySize = iLineCount;
    }

    if (lpte->piActualLineLength[iLineIndex] >= 0)
        return lpte->piActualLineLength[iLineIndex];

    //  Actual line length is not determined yet, let's calculate a little
    if (iLength > 0)
    {
        LPCTSTR pszLine = TextEdit_Buffer_GetLineChars(lpte->lpes, iLineIndex);
        LPTSTR pszChars = (LPTSTR)_alloca(sizeof(TCHAR) * (iLength + SZ));
        LPTSTR pszCurrent;
        int iTabSize;
        memcpy(pszChars, pszLine, sizeof(TCHAR) * iLength);
        pszChars[iLength] = 0;
        pszCurrent = pszChars;

        iTabSize = TextEdit_View_GetTabSize(lpte);

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

    lpte->piActualLineLength[iLineIndex] = iActualLength;

    return (iActualLength);
}

void TextEdit_View_ScrollToChar(LPTEXTEDITVIEW lpte, int iNewOffsetChar, BOOL bNoSmoothScroll /*= FALSE*/, BOOL bTrackScrollBar /*= TRUE*/)
{
    //  For now, ignoring bNoSmoothScroll and m_bSmoothScroll
    if (lpte->iOffsetChar != iNewOffsetChar)
    {
        RECT rcScroll;
        int iScrollChars = lpte->iOffsetChar - iNewOffsetChar;

        lpte->iOffsetChar = iNewOffsetChar;
        GetClientRect(lpte->hwnd, &rcScroll);
        rcScroll.left += TextEdit_View_GetMarginWidth(lpte);
        
        ScrollWindow(lpte->hwnd, iScrollChars * TextEdit_View_GetCharWidth(lpte),
                    0, &rcScroll, &rcScroll);
        UpdateWindow(lpte->hwnd);
        
        if (bTrackScrollBar)
            TextEdit_View_RecalcHorzScrollBar(lpte, TRUE);
    }
}

void TextEdit_View_ScrollToLine(LPTEXTEDITVIEW lpte, int iNewTopLine, BOOL bNoSmoothScroll /*= FALSE*/, BOOL bTrackScrollBar /*= TRUE*/)
{
    if (lpte->iTopLine != iNewTopLine)
    {
        if (bNoSmoothScroll || !lpte->bSmoothScroll)
        {
            int iScrollLines = lpte->iTopLine - iNewTopLine;
            
            lpte->iTopLine = iNewTopLine;
            
            ScrollWindow(lpte->hwnd, 0, iScrollLines * TextEdit_View_GetLineHeight(lpte), NULL, NULL);
            UpdateWindow(lpte->hwnd);
            
            if (bTrackScrollBar)
                TextEdit_View_RecalcVertScrollBar(lpte, TRUE);
        }
        else
        {
            //  Do smooth scrolling
            int iLineHeight = TextEdit_View_GetLineHeight(lpte);

            if (lpte->iTopLine > iNewTopLine)
            {
                int iIncrement = (lpte->iTopLine - iNewTopLine) / SMOOTH_SCROLL_FACTOR + 1;
                
                while (lpte->iTopLine != iNewTopLine)
                {
                    int iTopLine = lpte->iTopLine - iIncrement;
                    int iScrollLines;

                    if (iTopLine < iNewTopLine)
                        iTopLine = iNewTopLine;

                    iScrollLines = iTopLine - lpte->iTopLine;
                    lpte->iTopLine = iTopLine;
                    
                    ScrollWindow(lpte->hwnd, 0, - iLineHeight * iScrollLines, NULL, NULL);
                    UpdateWindow(lpte->hwnd);
                    
                    if (bTrackScrollBar)
                        TextEdit_View_RecalcVertScrollBar(lpte, TRUE);
                }
            }
            else
            {
                int iIncrement = (iNewTopLine - lpte->iTopLine) / SMOOTH_SCROLL_FACTOR + 1;
                
                while (lpte->iTopLine != iNewTopLine)
                {
                    int iTopLine = lpte->iTopLine + iIncrement;
                    int iScrollLines;

                    if (iTopLine > iNewTopLine)
                        iTopLine = iNewTopLine;

                    iScrollLines = iTopLine - lpte->iTopLine;
                    lpte->iTopLine = iTopLine;

                    ScrollWindow(lpte->hwnd, 0, - iLineHeight * iScrollLines, NULL, NULL);
                    UpdateWindow(lpte->hwnd);
                    
                    if (bTrackScrollBar)
                        TextEdit_View_RecalcVertScrollBar(lpte, TRUE);
                }
            }
        }
    }
}

void TextEdit_View_ExpandChars(LPTEXTEDITVIEW lpte, LPCTSTR pszChars, int iOffset, int iCount, LPTSTR *pszLine)
{
    int iTabSize = TextEdit_View_GetTabSize(lpte);
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
    
    if (iTabCount > 0 || lpte->bViewTabs)
    {
        for (i = 0; i < iLength; i++)
        {
            if (pszChars[i] == _T('\t'))
            {
                int iSpaces = iTabSize - (iActualOffset + iCurPos) % iTabSize;
                
                if (lpte->bViewTabs)
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
                if (pszChars[i] == _T(' ') && lpte->bViewTabs)
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

void TextEdit_View_DrawLineHelperImpl(LPTEXTEDITVIEW lpte, HDC hdc, LPPOINT lptOrigin, const RECT rcClip, LPCTSTR pszChars, int iOffset, int iCount)
{
    ASSERT(iCount >= 0);

    if (iCount > 0)
    {
        LPTSTR pszLine;
        int iWidth = rcClip.right - lptOrigin->x;

        TextEdit_View_ExpandChars(lpte, pszChars, iOffset, iCount, &pszLine);

        if (iWidth > 0)
        {
            int iCharWidth  = TextEdit_View_GetCharWidth(lpte);
            int iCount      = _tcslen(pszLine);
            int iCountFit   = iWidth / iCharWidth + 1;
            
            if (iCount > iCountFit)
                iCount = iCountFit;

            VERIFY(ExtTextOut(hdc, lptOrigin->x, lptOrigin->y,
                        ETO_CLIPPED, &rcClip, pszLine, iCount, NULL));
        }

        lptOrigin->x += TextEdit_View_GetCharWidth(lpte) * _tcslen(pszLine);

        Mem_Free(pszLine);
    }
}

void TextEdit_View_DrawLineHelper(LPTEXTEDITVIEW lpte, HDC hdc, LPPOINT lptOrigin, const LPRECT lprcClip, int iColorIndex, LPCTSTR pszChars, int iOffset, int iCount, POINT ptTextPos)
{
    if (iCount > 0)
    {
        if (lpte->bFocused || lpte->bShowInactiveSelection)
        {
            int iSelBegin = 0, iSelEnd = 0;
        
            if (lpte->ptDrawSelStart.y > ptTextPos.y)
            {
                iSelBegin = iCount;
            }
            else if (lpte->ptDrawSelStart.y == ptTextPos.y)
            {
                iSelBegin = lpte->ptDrawSelStart.x - ptTextPos.x;
                
                if (iSelBegin < 0)
                    iSelBegin = 0;
                if (iSelBegin > iCount)
                    iSelBegin = iCount;
            }

            if (lpte->ptDrawSelEnd.y > ptTextPos.y)
            {
                iSelEnd = iCount;
            }
            else if (lpte->ptDrawSelEnd.y == ptTextPos.y)
            {
                iSelEnd = lpte->ptDrawSelEnd.x - ptTextPos.x;
                
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
                TextEdit_View_DrawLineHelperImpl(lpte, hdc, lptOrigin, *lprcClip, pszChars, iOffset, iSelBegin);
            }

            if (iSelBegin < iSelEnd)
            {
                COLORREF crOldBk = SetBkColor(hdc, TextEdit_View_GetColor(lpte, COLORINDEX_SELBKGND));
                COLORREF crOldText = SetTextColor(hdc, TextEdit_View_GetColor(lpte, COLORINDEX_SELTEXT));
                TextEdit_View_DrawLineHelperImpl(lpte, hdc, lptOrigin, *lprcClip, pszChars, iOffset + iSelBegin, iSelEnd - iSelBegin);
                SetBkColor(hdc, crOldBk);
                SetTextColor(hdc, crOldText);
            }
            
            if (iSelEnd < iCount)
            {
                TextEdit_View_DrawLineHelperImpl(lpte, hdc, lptOrigin, *lprcClip, pszChars, iOffset + iSelEnd, iCount - iSelEnd);
            }
        }
        else
        {
            TextEdit_View_DrawLineHelperImpl(lpte, hdc, lptOrigin, *lprcClip, pszChars, iOffset, iCount);
        }
    }
}

void TextEdit_View_GetLineColors(LPTEXTEDITVIEW lpte, int iLineIndex, LPCOLORREF lpcrBkgnd, LPCOLORREF lpcrText, LPBOOL lpbDrawWhitespace)
{
    DWORD dwLineFlags = TextEdit_Buffer_GetLineFlags(lpte->lpes, iLineIndex);

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

DWORD TextEdit_View_GetParseCookie(LPTEXTEDITVIEW lpte, int iLineIndex)
{
    int iLineCount = TextEdit_Buffer_GetLineCount(lpte->lpes);
    int iCnt;
    int iBlocks;

    if (lpte->pdwParseCookies == NULL)
    {
        lpte->iParseArraySize = iLineCount;
        lpte->pdwParseCookies = (PDWORD)Mem_Alloc(iLineCount * sizeof(DWORD));
        memset(lpte->pdwParseCookies, 0xFF, iLineCount * sizeof(DWORD));
    }

    if (iLineIndex < 0)
        return 0;
    
    if (lpte->pdwParseCookies[iLineIndex] != (DWORD)-1)
        return lpte->pdwParseCookies[iLineIndex];

    iCnt = iLineIndex;
    
    while (iCnt >= 0 && lpte->pdwParseCookies[iCnt] == (DWORD)-1)
        iCnt--;
    iCnt++;

    while (iCnt <= iLineIndex)
    {
        DWORD dwCookie = 0;

        if (iCnt > 0)
            dwCookie = lpte->pdwParseCookies[iCnt - 1];

        ASSERT(dwCookie != (DWORD)-1);
        lpte->pdwParseCookies[iCnt] = TextEdit_View_ParseLine(lpte, dwCookie, iCnt, NULL, &iBlocks);
        ASSERT(lpte->pdwParseCookies[iCnt] != (DWORD)-1);
        iCnt++;
    }

    return lpte->pdwParseCookies[iLineIndex];
}

void TextEdit_View_DrawSingleLine(LPTEXTEDITVIEW lpte, HDC *fdc, const LPRECT lprc, int iLineIndex)
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

    ASSERT(iLineIndex >= -1 && iLineIndex < TextEdit_View_GetLineCount(lpte));

    if (iLineIndex == -1)
    {
        //  Draw line beyond the text
        Paint_FillRect(*fdc, lprc, TextEdit_View_GetColor(lpte, COLORINDEX_WHITESPACE));

        return;
    }

    //  Acquire the background color for the current line
    TextEdit_View_GetLineColors(lpte, iLineIndex, &crBkgnd, &crText, &bDrawWhitespace);
    if (crBkgnd == CLR_NONE)
        crBkgnd = TextEdit_View_GetColor(lpte, COLORINDEX_BKGND);

    iLength = TextEdit_View_GetLineLength(lpte, iLineIndex);

    if (iLength == 0)
    {
        //  Draw the empty line
        RECT rect = *lprc;
        POINT p = { 0, iLineIndex };
        
        if ((lpte->bFocused || lpte->bShowInactiveSelection) &&
            TextEdit_View_IsInsideSelBlock(lpte, p))
        {
            //FIXME: does this work?
            rect.right += TextEdit_View_GetCharWidth(lpte);
            Paint_FillRect(*fdc, &rect, TextEdit_View_GetColor(lpte, COLORINDEX_SELBKGND));
            rect.left += TextEdit_View_GetCharWidth(lpte);
        }

        Paint_FillRect(*fdc, &rect, bDrawWhitespace ? crBkgnd : TextEdit_View_GetColor(lpte, COLORINDEX_WHITESPACE));

        return;
    }

    // Parse the line
    pszChars = TextEdit_Buffer_GetLineChars(lpte->lpes, iLineIndex);
    dwCookie = TextEdit_View_GetParseCookie(lpte, iLineIndex - 1);
    pBuf = (LPTEXTBLOCK)_alloca(sizeof(TEXTBLOCK) * iLength * 3);

    lpte->pdwParseCookies[iLineIndex] = TextEdit_View_ParseLine(lpte, dwCookie, iLineIndex, pBuf, &iBlocks);
    ASSERT(lpte->pdwParseCookies[iLineIndex] != (DWORD)-1);

    //  Draw the line text
    origin.x = lprc->left - lpte->iOffsetChar * TextEdit_View_GetCharWidth(lpte);
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
            crOldText = SetTextColor(*fdc, TextEdit_View_GetColor(lpte, COLORINDEX_NORMALTEXT));

        SelectObject(*fdc,
                    TextEdit_View_GetFont(lpte, 
                        TextEdit_View_GetItalic(COLORINDEX_NORMALTEXT),
                            TextEdit_View_GetBold(COLORINDEX_NORMALTEXT)));
        
        TextEdit_View_DrawLineHelper(lpte, *fdc, &origin, lprc, COLORINDEX_NORMALTEXT,
            pszChars, 0, pBuf[0].iCharPos, pDLH);
        
        for (i = 0; i < (iBlocks - 1); i++)
        {
            ASSERT(pBuf[i].iCharPos >= 0 && pBuf[i].iCharPos <= iLength);
            
            if (crText == CLR_NONE)
                SetTextColor(*fdc, TextEdit_View_GetColor(lpte, pBuf[i].iColorIndex));

            SelectObject(*fdc, TextEdit_View_GetFont(lpte, 
                                TextEdit_View_GetItalic(pBuf[i].iColorIndex),
                                    TextEdit_View_GetBold(pBuf[i].iColorIndex)));

            pDLH.x = pBuf[i].iCharPos;
            pDLH.y = iLineIndex;
            
            TextEdit_View_DrawLineHelper(lpte, *fdc, &origin, lprc, pBuf[i].iColorIndex, pszChars,
                            pBuf[i].iCharPos, pBuf[i + 1].iCharPos - pBuf[i].iCharPos,
                            pDLH);
        }

        ASSERT(pBuf[iBlocks - 1].iCharPos >= 0 &&
            pBuf[iBlocks - 1].iCharPos <= iLength);
        
        if (crText == CLR_NONE)
            SetTextColor(*fdc, TextEdit_View_GetColor(lpte, pBuf[iBlocks - 1].iColorIndex));
        
        SelectObject(*fdc, TextEdit_View_GetFont(lpte, 
                            TextEdit_View_GetItalic(pBuf[iBlocks - 1].iColorIndex),
                            TextEdit_View_GetBold(pBuf[iBlocks - 1].iColorIndex)));

        pDLH.x = pBuf[iBlocks - 1].iCharPos;
        pDLH.y = iLineIndex;

        TextEdit_View_DrawLineHelper(lpte, *fdc, &origin, lprc, pBuf[iBlocks - 1].iColorIndex, pszChars,
                            pBuf[iBlocks - 1].iCharPos, iLength - pBuf[iBlocks - 1].iCharPos,
                            pDLH);
    }
    else
    {
        POINT pDLH = { 0, iLineIndex };

        if (crText == CLR_NONE)
            SetTextColor(*fdc, TextEdit_View_GetColor(lpte, COLORINDEX_NORMALTEXT));

        SelectObject(*fdc, TextEdit_View_GetFont(lpte, 
                            TextEdit_View_GetItalic(COLORINDEX_NORMALTEXT),
                            TextEdit_View_GetBold(COLORINDEX_NORMALTEXT)));
        TextEdit_View_DrawLineHelper(lpte, *fdc, &origin, lprc, COLORINDEX_NORMALTEXT,
            pszChars, 0, iLength, pDLH);
    }

    //  Draw whitespaces to the left of the text
    frect = *lprc;

    if (origin.x > frect.left)
        frect.left = origin.x;

    if (frect.right > frect.left)
    {
        POINT p = { iLength, iLineIndex };

        if ((lpte->bFocused || lpte->bShowInactiveSelection) &&
            TextEdit_View_IsInsideSelBlock(lpte, p))
        {
            RECT rcTemp = frect;
            rcTemp.right += TextEdit_View_GetCharWidth(lpte);
            Paint_FillRect(*fdc, &rcTemp, TextEdit_View_GetColor(lpte, COLORINDEX_SELBKGND));

            frect.left += TextEdit_View_GetCharWidth(lpte);
        }

        if (frect.right > frect.left)
            Paint_FillRect(*fdc, &frect, bDrawWhitespace ? crBkgnd : TextEdit_View_GetColor(lpte, COLORINDEX_WHITESPACE));
    }
}

COLORREF TextEdit_View_GetColor(LPTEXTEDITVIEW lpte, int iColorIndex)
{
    switch (iColorIndex)
    {
    case COLORINDEX_WHITESPACE:
        return ((lpte->lpsd->crWhiteSpace == -1) ?
                    GetSysColor(COLOR_WINDOW) :
                    lpte->lpsd->crWhiteSpace);
    case COLORINDEX_BKGND:
        return ((lpte->lpsd->crBackground == -1) ?
                    GetSysColor(COLOR_WINDOW) :
                    lpte->lpsd->crBackground);
    case COLORINDEX_NORMALTEXT:
        return ((lpte->lpsd->crNormalText == -1) ?
                    GetSysColor(COLOR_WINDOWTEXT) :
                    lpte->lpsd->crNormalText);
    case COLORINDEX_SELMARGIN:
        return ((lpte->lpsd->crSelMargin == -1) ?
                    GetSysColor(COLOR_SCROLLBAR) :
                    lpte->lpsd->crSelMargin);
    case COLORINDEX_PREPROCESSOR:
        return ((lpte->lpsd->crPreprocessor == -1) ?
                    RGB(0, 128, 192) :
                    lpte->lpsd->crPreprocessor);
    case COLORINDEX_COMMENT:
        return ((lpte->lpsd->crComment == -1) ?
                    RGB(0, 128, 0) :
                    lpte->lpsd->crComment);
    case COLORINDEX_NUMBER:
        return ((lpte->lpsd->crNumber == -1) ?
                    RGB(128, 0, 0) :
                    lpte->lpsd->crNumber);
    case COLORINDEX_OPERATOR:
        return ((lpte->lpsd->crOperator == -1) ?
                    RGB(96, 96, 96) :
                    lpte->lpsd->crOperator);
    case COLORINDEX_PREFIXED:
        return ((lpte->lpsd->crPrefixed == -1) ?
                GetSysColor(COLOR_WINDOWTEXT) :
                lpte->lpsd->crPrefixed);
    case COLORINDEX_KEYWORD:
        return ((lpte->lpsd->crKeyword == -1) ?
                    RGB(0, 0, 255) :
                    lpte->lpsd->crKeyword);
    case COLORINDEX_FUNCNAME:
        return ((lpte->lpsd->crFunctionName == -1) ?
                    RGB(128, 0, 128) :
                    lpte->lpsd->crFunctionName);
    case COLORINDEX_USER1:
        return ((lpte->lpsd->crUser1 == -1) ?
                    RGB(0, 0, 128) :
                    lpte->lpsd->crUser1);
    case COLORINDEX_USER2:
        return ((lpte->lpsd->crUser2 == -1) ?
                    RGB(0, 128, 192) :
                    lpte->lpsd->crUser2);
    case COLORINDEX_SELBKGND:
        return ((lpte->lpsd->crSelBackground == -1) ?
                    GetSysColor(COLOR_HIGHLIGHT) :
                    lpte->lpsd->crSelBackground);
    case COLORINDEX_SELTEXT:
        return ((lpte->lpsd->crSelText == -1) ?
                    GetSysColor(COLOR_HIGHLIGHTTEXT) :
                    lpte->lpsd->crSelText);
    }

    return (RGB(128, 0, 0));
}

DWORD TextEdit_View_GetLineFlags(LPTEXTEDITVIEW lpte, int iLineIndex)
{
    if (lpte->lpes == NULL)
        return 0;

    return (TextEdit_Buffer_GetLineFlags(lpte->lpes, iLineIndex));
}

void TextEdit_View_DrawMargin(LPTEXTEDITVIEW lpte, HDC *fdc, const LPRECT lprc, int iLineIndex)
{
    int iImageIndex = -1;

    if (!lpte->bSelMargin)
    {
        Paint_FillRect(*fdc, lprc, TextEdit_View_GetColor(lpte, COLORINDEX_BKGND));
        
        return;
    }

    Paint_FillRect(*fdc, lprc, TextEdit_View_GetColor(lpte, COLORINDEX_SELMARGIN));

    if (iLineIndex >= 0)
    {
        DWORD dwLineFlags = TextEdit_Buffer_GetLineFlags(lpte->lpes, iLineIndex);

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

        if (lpte->hilIcons == NULL)
        { 
//          lpte->pIcons = (IMAGELIST)Mem_Alloc(sizeof(IMAGELIST));
            VERIFY(lpte->hilIcons = ImageList_Create(16, 12, ILC_COLOR24 | ILC_MASK, 3, 0));
            VERIFY(ImageList_AddMasked(lpte->hilIcons, LoadBitmap(PCP_Edit_Dll_GetHandle(), MAKEINTRESOURCE(IDB_MARGIN_ICONS)), RGB(255, 255, 255)) != -1);
        }

        pt.x = lprc->left + 2;
        //FIXME: Right?
//      pt.y = lprc->top + (TextEdit_View_GetLineHeight(lpte) - 16 / 2);
        pt.y = lprc->top + (Rect_Height(&*lprc) - 12) / 2;
        VERIFY(ImageList_Draw(lpte->hilIcons, iImageIndex, *fdc, pt.x, pt.y, ILD_TRANSPARENT));
    }
}

BOOL TextEdit_View_IsInsideSelBlock(LPTEXTEDITVIEW lpte, POINT ptTextPos)
{
    ASSERT_VALIDTEXTPOS(lpte, ptTextPos);

    if (ptTextPos.y < lpte->ptDrawSelStart.y)
        return FALSE;
    if (ptTextPos.y > lpte->ptDrawSelEnd.y)
        return FALSE;
    if (ptTextPos.y < lpte->ptDrawSelEnd.y && ptTextPos.y > lpte->ptDrawSelStart.y)
        return TRUE;
    if (lpte->ptDrawSelStart.y < lpte->ptDrawSelEnd.y)
    {
        if (ptTextPos.y == lpte->ptDrawSelEnd.y)
            return (ptTextPos.x < lpte->ptDrawSelEnd.x);
        ASSERT(ptTextPos.y == lpte->ptDrawSelStart.y);
        return (ptTextPos.x >= lpte->ptDrawSelStart.x);
    }
    ASSERT(lpte->ptDrawSelStart.y == lpte->ptDrawSelEnd.y);
    return (ptTextPos.x >= lpte->ptDrawSelStart.x && ptTextPos.x < lpte->ptDrawSelEnd.x);
}

BOOL TextEdit_View_IsInsideSelection(LPTEXTEDITVIEW lpte, const POINT ptTextPos)
{
    TextEdit_View_PrepareSelBounds(lpte);
    return (TextEdit_View_IsInsideSelBlock(lpte, ptTextPos));
}

void TextEdit_View_PrepareSelBounds(LPTEXTEDITVIEW lpte)
{
    if (lpte->ptSelStart.y < lpte->ptSelEnd.y ||
            (lpte->ptSelStart.y == lpte->ptSelEnd.y &&
            lpte->ptSelStart.x < lpte->ptSelEnd.x))
    {
        lpte->ptDrawSelStart = lpte->ptSelStart;
        lpte->ptDrawSelEnd = lpte->ptSelEnd;
    }
    else
    {
        lpte->ptDrawSelStart    = lpte->ptSelEnd;
        lpte->ptDrawSelEnd      = lpte->ptSelStart;
    }
}

void TextEdit_View_OnPaint(LPTEXTEDITVIEW lpte)
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
    HDC hdc = BeginPaint(lpte->hwnd, &ps);

    GetClientRect(lpte->hwnd, &rcClient);

    iLineCount  = TextEdit_View_GetLineCount(lpte);
    iLineHeight = TextEdit_View_GetLineHeight(lpte);
    TextEdit_View_PrepareSelBounds(lpte);

    VERIFY(cacheDC = CreateCompatibleDC(hdc));

    if (lpte->pCacheBitmap == NULL)
    {
        lpte->pCacheBitmap = (HBITMAP *)Mem_Alloc(sizeof(HBITMAP));
        VERIFY(*lpte->pCacheBitmap = CreateCompatibleBitmap(hdc,
                                        Rect_Width(&rcClient), iLineHeight));
    }

    hOldBitmap = SelectObject(cacheDC, *lpte->pCacheBitmap);

    rcLine = ps.rcPaint; //rcClient;
    rcLine.left = rcClient.left;
    rcLine.right = rcClient.right;
    // Make sure that top isn't below a line
    rcLine.top -= (rcLine.top % iLineHeight);
    rcLine.bottom = rcLine.top + iLineHeight;
    
    SetRect(&rcCacheMargin, 0, 0, TextEdit_View_GetMarginWidth(lpte), iLineHeight);
    SetRect(&rcCacheLine, TextEdit_View_GetMarginWidth(lpte), 0, Rect_Width(&rcLine), iLineHeight);

    iCurrentLine = lpte->iTopLine + (rcLine.top / iLineHeight); //lpte->iTopLine;

    while (rcLine.top < ps.rcPaint.bottom)
    {
        if (iCurrentLine < iLineCount)
        {
            TextEdit_View_DrawMargin(lpte, &cacheDC, &rcCacheMargin, iCurrentLine);
            TextEdit_View_DrawSingleLine(lpte, &cacheDC, &rcCacheLine, iCurrentLine);
        }
        else
        {
            TextEdit_View_DrawMargin(lpte, &cacheDC, &rcCacheMargin, -1);
            TextEdit_View_DrawSingleLine(lpte, &cacheDC, &rcCacheLine, -1);
        }

        VERIFY(BitBlt(hdc, rcLine.left, rcLine.top,
                    Rect_Width(&rcLine), Rect_Height(&rcLine),
                    cacheDC, 0, 0, SRCCOPY));

        iCurrentLine++;
        OffsetRect(&rcLine, 0, iLineHeight);
    }

    SelectObject(cacheDC, hOldBitmap);
    DeleteDC(cacheDC);

    EndPaint(lpte->hwnd, &ps);
}

void TextEdit_View_ResetView(LPTEXTEDITVIEW lpte)
{
    int i;

    lpte->bOvrMode          = FALSE;
    lpte->iTopLine          = 0;
    lpte->iOffsetChar       = 0;
    lpte->iLineHeight       = -1;
    lpte->iCharWidth        = -1;
    lpte->iMaxLineLength    = -1;
    lpte->iScreenLines      = -1;
    lpte->iScreenChars      = -1;
    lpte->iIdealCharPos     = -1;
    lpte->ptAnchor.x        = 0;
    lpte->ptAnchor.y        = 0;
    
    if (lpte->hilIcons != NULL)
    {
        ImageList_Destroy(lpte->hilIcons);
        lpte->hilIcons = NULL;
    }

    for (i = 0; i < 4; i++)
    {
        if (lpte->ahFonts[i] != NULL)
        {
            DeleteObject(lpte->ahFonts[i]);
            lpte->ahFonts[i] = NULL;
        }
    }

    if (lpte->pdwParseCookies != NULL)
    {
        Mem_Free(lpte->pdwParseCookies);
        lpte->pdwParseCookies = NULL;
    }

    if (lpte->piActualLineLength != NULL)
    {
        Mem_Free(lpte->piActualLineLength);
        lpte->piActualLineLength = NULL;
    }

    lpte->iParseArraySize           = 0;
    lpte->iActualLengthArraySize    = 0;
    lpte->ptCursorPos.x             = 0;
    lpte->ptCursorPos.y             = 0;
    lpte->ptSelStart                = lpte->ptCursorPos;
    lpte->ptSelEnd                  = lpte->ptCursorPos;
    lpte->bDragSelection            = FALSE;
    lpte->bVertScrollBarLocked      = FALSE;
    lpte->bHorzScrollBarLocked      = FALSE;

    if (IsWindow(lpte->hwnd))
        TextEdit_View_UpdateCaret(lpte);

    lpte->bPrintHeader              = FALSE;
    lpte->bPrintFooter              = TRUE;
    lpte->bShowInactiveSelection    = TRUE;

    lpte->bBookmarkExist            = FALSE;    // More bookmarks

    // Unnecessary
    lpte->pszMatched                = NULL;
}

#ifdef _DEBUG
void TextEdit_View_AssertValidTextPos(LPTEXTEDITVIEW lpte, POINT pt)
{
    if (TextEdit_View_GetLineCount(lpte) > 0)
    {
        ASSERT(lpte->iTopLine >= 0 && lpte->iOffsetChar >= 0);
        ASSERT(pt.y >= 0 && pt.y <= TextEdit_View_GetLineCount(lpte));
        ASSERT(pt.x >= 0 && pt.x <= TextEdit_View_GetLineLength(lpte, pt.y));
    }
}
#endif

void TextEdit_View_UpdateCaret(LPTEXTEDITVIEW lpte)
{
    ASSERT_VALIDTEXTPOS(lpte, lpte->ptCursorPos);

    if (lpte->bFocused && !lpte->bCursorHidden &&
        TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y,
                        lpte->ptCursorPos.x) >= lpte->iOffsetChar)
    {
        POINT pt;

        if (lpte->bOvrMode)
            CreateCaret(lpte->hwnd, NULL, TextEdit_View_GetCharWidth(lpte), TextEdit_View_GetLineHeight(lpte));
        else
            CreateCaret(lpte->hwnd, NULL, 2, TextEdit_View_GetLineHeight(lpte));
        
        pt = TextEdit_View_TextToClient(lpte, lpte->ptCursorPos);
        SetCaretPos(pt.x, pt.y);
        ShowCaret(lpte->hwnd);
    }
    else
    {
        HideCaret(lpte->hwnd);
    }
}

int TextEdit_View_GetTabSize(LPTEXTEDITVIEW lpte)
{
    ASSERT(lpte->iTabSize >= 0 && lpte->iTabSize <= 64);

    return (lpte->iTabSize);
}

void TextEdit_View_SetTabSize(LPTEXTEDITVIEW lpte, int iTabSize)
{
    ASSERT(iTabSize >= 0 && iTabSize <= 64);

    if (lpte->iTabSize != iTabSize)
    {
        lpte->iTabSize = iTabSize;
        
        if (lpte->piActualLineLength != NULL)
        {
            Mem_Free(lpte->piActualLineLength);
            lpte->piActualLineLength = NULL;
        }

        lpte->iActualLengthArraySize    = 0;
        lpte->iMaxLineLength            = -1;
        
        TextEdit_View_RecalcHorzScrollBar(lpte, FALSE);
        InvalidateRect(lpte->hwnd, NULL, FALSE);
        TextEdit_View_UpdateCaret(lpte);
    }
}

HFONT TextEdit_View_GetFont(LPTEXTEDITVIEW lpte, BOOL bItalic, BOOL bBold)
{
    int iIndex = 0;

    if (bBold)
        iIndex |= 1;
    if (bItalic)
        iIndex |= 2;

    if (lpte->ahFonts[iIndex] == NULL)
    {
//      lpte->ahFonts[iIndex] = new CFont;
        lpte->lfBaseFont.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
        lpte->lfBaseFont.lfItalic = (BYTE)bItalic;
        if ((lpte->ahFonts[iIndex] = CreateFontIndirect(&lpte->lfBaseFont)) == NULL)
        {
//          delete m_apFonts[nIndex];
//          m_apFonts[nIndex] = NULL;
//          return CView_GetFont();
        }

    }
    return (lpte->ahFonts[iIndex]);
}

void TextEdit_View_CalcLineCharDim(LPTEXTEDITVIEW lpte)
{
    HDC hdc         = GetDC(lpte->hwnd);
    HFONT hOldFont  = SelectObject(hdc, TextEdit_View_GetFont(lpte, FALSE, FALSE));
    SIZE sCharExt;
    
    GetTextExtentPoint(hdc, _T("X"), 1, &sCharExt);
    
    lpte->iLineHeight = sCharExt.cy;
    
    if (lpte->iLineHeight < 1)
        lpte->iLineHeight = 1;

    lpte->iCharWidth = sCharExt.cx;

    SelectObject(hdc, hOldFont);
    ReleaseDC(lpte->hwnd, hdc);
}

int TextEdit_View_GetLineHeight(LPTEXTEDITVIEW lpte)
{
    if (lpte->iLineHeight == -1)
        TextEdit_View_CalcLineCharDim(lpte);

    ASSERT(lpte->iLineHeight != 0);
    ASSERT(lpte->iCharWidth != 0);

    return (lpte->iLineHeight);
}

int TextEdit_View_GetCharWidth(LPTEXTEDITVIEW lpte)
{
    if (lpte->iCharWidth == -1)
        TextEdit_View_CalcLineCharDim(lpte);

    ASSERT(lpte->iLineHeight != 0);
    ASSERT(lpte->iCharWidth != 0);

    return lpte->iCharWidth;
}

int TextEdit_View_GetMaxLineLength(LPTEXTEDITVIEW lpte)
{

    if (lpte->iMaxLineLength == -1)
    {
        int iLineCount = TextEdit_View_GetLineCount(lpte);
        int i;
        
        lpte->iMaxLineLength = 0;

        for (i = 0; i < iLineCount; i++)
        {
            int iActualLength = TextEdit_View_GetLineActualLength(lpte, i);
            
            if (lpte->iMaxLineLength < iActualLength)
                lpte->iMaxLineLength = iActualLength;
        }
    }

    return (lpte->iMaxLineLength);
}

int TextEdit_View_GetLineCount(LPTEXTEDITVIEW lpte)
{
    if (lpte->lpes == NULL)
    {
        return (1);     //  Single empty line
    }
    else
    {
        int iLineCount = TextEdit_Buffer_GetLineCount(lpte->lpes);
        
        ASSERT(iLineCount > 0);

        return (iLineCount);
    }
}

int TextEdit_View_GetLineLength(LPTEXTEDITVIEW lpte, int iLineIndex)
{
    if (lpte->lpes == NULL)
        return (0);

    return (TextEdit_Buffer_GetLineLength(lpte->lpes, iLineIndex));
}

LPCTSTR TextEdit_View_GetLineChars(LPTEXTEDITVIEW lpte, int iLineIndex)
{
    if (lpte->lpes == NULL)
        return (NULL);

    return (TextEdit_Buffer_GetLineChars(lpte->lpes, iLineIndex));
}

void TextEdit_View_AttachToBuffer(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes)
{
    lpte->lpes = lpes;
}

int TextEdit_View_GetScreenLines(LPTEXTEDITVIEW lpte)
{
    if (lpte->iScreenLines == -1)
    {
        RECT rc;
        GetClientRect(lpte->hwnd, &rc);
        lpte->iScreenLines = Rect_Height(&rc) / TextEdit_View_GetLineHeight(lpte);
    }

    return (lpte->iScreenLines);
}

BOOL TextEdit_View_GetItalic(int iColorIndex)
{
    return (FALSE);
}

BOOL TextEdit_View_GetBold(int iColorIndex)
{
    return (FALSE);
}

int TextEdit_View_GetScreenChars(LPTEXTEDITVIEW lpte)
{
    if (lpte->iScreenChars == -1)
    {
        RECT rc;
        GetClientRect(lpte->hwnd, &rc);
        lpte->iScreenChars = (Rect_Width(&rc) - TextEdit_View_GetMarginWidth(lpte)) / TextEdit_View_GetCharWidth(lpte);
    }

    return lpte->iScreenChars;
}

void TextEdit_View_OnDestroy(LPTEXTEDITVIEW lpte)
{
    int i;

    TextEdit_Buffer_FreeAll(lpte->lpes);

    if (lpte->pdwParseCookies != NULL)
        Mem_Free(lpte->pdwParseCookies);
    if (lpte->piActualLineLength != NULL)
        Mem_Free(lpte->piActualLineLength);
    if (lpte->rxNode != NULL)
        Mem_Free(lpte->rxNode);
    if (lpte->pszMatched != NULL)
        Mem_Free(lpte->pszMatched);

    if (lpte->hCursor != NULL)
        DestroyCursor(lpte->hCursor);

    for (i = 0; i < 4; i++)
    {
        if (lpte->ahFonts[i] != NULL)
        {
            DeleteObject(lpte->ahFonts[i]);
            lpte->ahFonts[i] = NULL;
        }
    }

    if (lpte->pCacheBitmap != NULL)
    {
        //FIXME: Correct?
        Mem_Free(lpte->pCacheBitmap);
        lpte->pCacheBitmap = NULL;
    }

    DefWindowProc(lpte->hwnd, WM_DESTROY, 0, 0);
}

BOOL TextEdit_View_OnEraseBkgnd(HDC hdc) 
{
    return (TRUE);
}

void TextEdit_View_OnSize(LPTEXTEDITVIEW lpte, UINT nType, int cx, int cy)
{
    DefWindowProc(lpte->hwnd, WM_SIZE, nType, MAKELPARAM(cx, cy));

    if (lpte->pCacheBitmap != NULL)
    {
        DeleteObject(*lpte->pCacheBitmap);
        Mem_Free(lpte->pCacheBitmap); //FIXME: WHY?
        lpte->pCacheBitmap = NULL;
    }
    lpte->iScreenLines = -1;
    lpte->iScreenChars = -1;
    TextEdit_View_RecalcVertScrollBar(lpte, FALSE);
    TextEdit_View_RecalcHorzScrollBar(lpte, FALSE);
}

void TextEdit_View_RecalcVertScrollBar(LPTEXTEDITVIEW lpte, BOOL bPositionOnly)
{
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    if (bPositionOnly)
    {
        si.fMask    = SIF_POS;
        si.nPos     = lpte->iTopLine;
    }
    else
    {
        if (TextEdit_View_GetScreenLines(lpte) >= TextEdit_View_GetLineCount(lpte) &&
            lpte->iTopLine > 0)
        {
            lpte->iTopLine = 0;
            InvalidateRect(lpte->hwnd, NULL, FALSE);
            TextEdit_View_UpdateCaret(lpte);
        }

        si.fMask    = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
        si.nMin     = 0;
        si.nMax     = TextEdit_View_GetLineCount(lpte) - 1;
        si.nPage    = TextEdit_View_GetScreenLines(lpte);
        si.nPos     = lpte->iTopLine;
    }

    //VERIFY (?)
    SetScrollInfo(lpte->hwnd, SB_VERT, &si, TRUE);
}

void TextEdit_View_OnVScroll(LPTEXTEDITVIEW lpte, UINT nSBCode, UINT nPos, HWND hwndScrollBar) 
{
    SCROLLINFO si;
    int iPageLines, iLineCount, iNewTopLine;
    BOOL bDisableSmooth = TRUE;

    DefWindowProc(lpte->hwnd, WM_VSCROLL, MAKEWPARAM(nSBCode, nPos), (LPARAM)hwndScrollBar);

    //  Note we cannot use nPos because of its 16-bit nature
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    VERIFY(GetScrollInfo(lpte->hwnd, SB_VERT, &si));

    iPageLines = TextEdit_View_GetScreenLines(lpte);
    iLineCount = TextEdit_View_GetLineCount(lpte);

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
            iNewTopLine = lpte->iTopLine - 1;
        break;
        case SB_LINEDOWN:
            iNewTopLine = lpte->iTopLine + 1;
        break;
        case SB_PAGEUP:
            iNewTopLine = lpte->iTopLine - si.nPage + 1;
            bDisableSmooth = FALSE;
        break;
        case SB_PAGEDOWN:
            iNewTopLine = lpte->iTopLine + si.nPage - 1;
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
    
    TextEdit_View_ScrollToLine(lpte, iNewTopLine, bDisableSmooth, TRUE);
}

void TextEdit_View_RecalcHorzScrollBar(LPTEXTEDITVIEW lpte, BOOL bPositionOnly)
{
    //  Again, we cannot use nPos because it's 16-bit
    SCROLLINFO si;
    si.cbSize = sizeof(si);

    if (bPositionOnly)
    {
        si.fMask = SIF_POS;
        si.nPos = lpte->iOffsetChar;
    }
    else
    {
        if (TextEdit_View_GetScreenChars(lpte) >= TextEdit_View_GetMaxLineLength(lpte) &&
            lpte->iOffsetChar > 0)
        {
            lpte->iOffsetChar = 0;
            InvalidateRect(lpte->hwnd, NULL, FALSE);
            TextEdit_View_UpdateCaret(lpte);
        }

        si.fMask    = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
        si.nMin     = 0;
        si.nMax     = TE_MAX_LINE_LENGTH; // TextEdit_View_GetMaxLineLength(lpte) - 1;
        si.nPage    = TextEdit_View_GetScreenChars(lpte);
        si.nPos     = lpte->iOffsetChar;
    }

    SetScrollInfo(lpte->hwnd, SB_HORZ, &si, TRUE);
}

void TextEdit_View_OnHScroll(LPTEXTEDITVIEW lpte, UINT nSBCode, UINT nPos, HWND hwndScrollBar) 
{
    SCROLLINFO si;
    int iPageChars, iMaxLineLength, iNewOffset;

    DefWindowProc(lpte->hwnd, WM_HSCROLL, MAKEWPARAM(nSBCode, nPos), (LPARAM)hwndScrollBar);

    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    VERIFY(GetScrollInfo(lpte->hwnd, SB_HORZ, &si));

    iPageChars      = TextEdit_View_GetScreenChars(lpte);
    iMaxLineLength  = TE_MAX_LINE_LENGTH; //TextEdit_View_GetMaxLineLength(lpte);

    switch (nSBCode)
    {
        case SB_LEFT:
            iNewOffset = 0;
        break;
        case SB_BOTTOM:
            iNewOffset = iMaxLineLength - iPageChars + 1;
        break;
        case SB_LINEUP:
            iNewOffset = lpte->iOffsetChar - 1;
        break;
        case SB_LINEDOWN:
            iNewOffset = lpte->iOffsetChar + 1;
        break;
        case SB_PAGEUP:
            iNewOffset = lpte->iOffsetChar - si.nPage + 1;
        break;
        case SB_PAGEDOWN:
            iNewOffset = lpte->iOffsetChar + si.nPage - 1;
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

    TextEdit_View_ScrollToChar(lpte, iNewOffset, FALSE, TRUE);
    TextEdit_View_UpdateCaret(lpte);
}

BOOL TextEdit_View_OnSetCursor(LPTEXTEDITVIEW lpte, UINT nHitTest, UINT wMouseMsg) 
{
    if (nHitTest == HTCLIENT)
    {
        POINT pt;

        GetCursorPos(&pt);
        ScreenToClient(lpte->hwnd, &pt);

        if (pt.x < TextEdit_View_GetMarginWidth(lpte))
        {
            SetCursor(lpte->hCursor);
        }
        else
        {
            POINT ptText = TextEdit_View_ClientToText(lpte, pt);
            TextEdit_View_PrepareSelBounds(lpte);
            if (TextEdit_View_IsInsideSelBlock(lpte, ptText))
            {
                //  [JRT]:  Support For Disabling Drag and Drop...
                if (!lpte->bDisableDragAndDrop)             // If Drag And Drop Not Disabled
                    SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));    // Set To Arrow Cursor
            }
            else
            {
                SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
            }
        }

        return (TRUE);
    }

    return (DefWindowProc(lpte->hwnd, WM_SETCURSOR, (WPARAM)lpte->hwnd, MAKELPARAM(nHitTest, wMouseMsg)));
}

//FIXME: Needs debugging
POINT TextEdit_View_ClientToText(LPTEXTEDITVIEW lpte, POINT pt)
{
    int iLineCount = TextEdit_View_GetLineCount(lpte);
    POINT ptRet;
    int nLength = 0;
    LPCTSTR pszLine = NULL;
    int iPos;
    int iIndex = 0, iCurPos = 0;
    int iTabSize;

    ptRet.y = lpte->iTopLine + pt.y / TextEdit_View_GetLineHeight(lpte);

    if (ptRet.y >= iLineCount)
        ptRet.y = iLineCount - 1;
    if (ptRet.y < 0)
        ptRet.y = 0;

    if (ptRet.y >= 0 && ptRet.y < iLineCount)
    {
        nLength = TextEdit_View_GetLineLength(lpte, ptRet.y);
        pszLine = TextEdit_View_GetLineChars(lpte, ptRet.y);
    }

    iPos = lpte->iOffsetChar + (pt.x - TextEdit_View_GetMarginWidth(lpte)) / TextEdit_View_GetCharWidth(lpte);
    if (iPos < 0)
        iPos = 0;

    iTabSize = TextEdit_View_GetTabSize(lpte);

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

POINT TextEdit_View_TextToClient(LPTEXTEDITVIEW lpte, POINT pt)
{
    int iLength;
    LPCTSTR pszLine;
    POINT ptRet;
    int nTabSize = TextEdit_View_GetTabSize(lpte);
    int iIndex;

    ASSERT_VALIDTEXTPOS(lpte, pt);
    
    iLength = TextEdit_View_GetLineLength(lpte, pt.y);
    pszLine = TextEdit_View_GetLineChars(lpte, pt.y);

    ptRet.y = (pt.y - lpte->iTopLine) * TextEdit_View_GetLineHeight(lpte);
    ptRet.x = 0;

    for (iIndex = 0; iIndex < pt.x; iIndex++)
    {
        if (pszLine[iIndex] == _T('\t'))
            ptRet.x += (nTabSize - ptRet.x % nTabSize);
        else
            ptRet.x++;
    }

    ptRet.x = (ptRet.x - lpte->iOffsetChar) * TextEdit_View_GetCharWidth(lpte) + TextEdit_View_GetMarginWidth(lpte);

    return (ptRet);
}

void TextEdit_View_InvalidateLines(LPTEXTEDITVIEW lpte, int iLine1, int iLine2, BOOL bInvalidateMargin)
{
    bInvalidateMargin = TRUE;

    if (iLine2 == -1)
    {
        RECT rcInvalid;
        
        GetClientRect(lpte->hwnd, &rcInvalid);
        if (!bInvalidateMargin)
            rcInvalid.left += TextEdit_View_GetMarginWidth(lpte);
        rcInvalid.top = (iLine1 - lpte->iTopLine) * TextEdit_View_GetLineHeight(lpte);
        InvalidateRect(lpte->hwnd, &rcInvalid, FALSE);
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

        GetClientRect(lpte->hwnd, &rcInvalid);
        if (!bInvalidateMargin)
            rcInvalid.left += TextEdit_View_GetMarginWidth(lpte);
        rcInvalid.top = (iLine1 - lpte->iTopLine) * TextEdit_View_GetLineHeight(lpte);
        rcInvalid.bottom = (iLine2 - lpte->iTopLine + 1) * TextEdit_View_GetLineHeight(lpte);
        InvalidateRect(lpte->hwnd, &rcInvalid, FALSE);
    }
}

void TextEdit_View_SetSelection(LPTEXTEDITVIEW lpte, POINT ptStart, POINT ptEnd)
{
    NMHDR nmhdr;

    ASSERT_VALIDTEXTPOS(lpte, ptStart);
    ASSERT_VALIDTEXTPOS(lpte, ptEnd);

    if (Point_Equal(&lpte->ptSelStart, &ptStart))
    {
        if (!Point_Equal(&lpte->ptSelEnd, &ptEnd))
            TextEdit_View_InvalidateLines(lpte, ptEnd.y, lpte->ptSelEnd.y, FALSE);
    }
    else
    {
        TextEdit_View_InvalidateLines(lpte, ptStart.y, ptEnd.y, FALSE);
        TextEdit_View_InvalidateLines(lpte, lpte->ptSelStart.y, lpte->ptSelEnd.y, FALSE);
    }

    lpte->ptSelStart = ptStart;
    lpte->ptSelEnd = ptEnd;

    nmhdr.hwndFrom  = lpte->hwnd;
    nmhdr.idFrom    = GetWindowLong(lpte->hwnd, GWL_ID);
    nmhdr.code      = PEN_SELCHANGE;

    SendMessage(GetParent(lpte->hwnd), WM_NOTIFY, (WPARAM)nmhdr.idFrom, (LPARAM)&nmhdr);
}

void TextEdit_View_AdjustTextPoint(LPTEXTEDITVIEW lpte, POINT *lppt)
{
    //FIXME: this needs work
    lppt->x += TextEdit_View_GetCharWidth(lpte) / 2;
}

void TextEdit_View_OnSetFocus(LPTEXTEDITVIEW lpte, HWND hwndLoseFocus) 
{
    DefWindowProc(lpte->hwnd, WM_SETFOCUS, (WPARAM)hwndLoseFocus, 0);

    lpte->bFocused = TRUE;
    if (Point_Equal(&lpte->ptSelStart, &lpte->ptSelEnd))
        TextEdit_View_InvalidateLines(lpte, lpte->ptSelStart.y, lpte->ptSelEnd.y, TRUE);
    TextEdit_View_UpdateCaret(lpte);
}

DWORD TextEdit_View_ParseLine(LPTEXTEDITVIEW lpte, DWORD dwCookie, int nLineIndex, LPTEXTBLOCK pBuf, LPINT piActualItems)
{
    return ((lpte->lpsd->bPlain) ? 0 : TextEdit_Syntax_ParseLineGeneric(lpte, dwCookie, nLineIndex, pBuf, piActualItems));
}

int TextEdit_View_CalculateActualOffset(LPTEXTEDITVIEW lpte, int iLineIndex, int iCharIndex)
{
    int iLength = TextEdit_View_GetLineLength(lpte, iLineIndex);
    LPCTSTR pszChars;
    int iOffset = 0;
    int iTabSize = TextEdit_View_GetTabSize(lpte);
    int i;

    ASSERT(iCharIndex >= 0 && iCharIndex <= iLength);
    pszChars = TextEdit_View_GetLineChars(lpte, iLineIndex);

    for (i = 0; i < iCharIndex; i++)
    {
        if (pszChars[i] == _T('\t'))
            iOffset += (iTabSize - iOffset % iTabSize);
        else
            iOffset++;
    }

    return (iOffset);
}

int TextEdit_View_ApproxActualOffset(LPTEXTEDITVIEW lpte, int iLineIndex, int iOffset)
{
    int iLength;
    LPCTSTR pszChars;
    int iCurrentOffset;
    int iTabSize = TextEdit_View_GetTabSize(lpte);
    int i;

    if (iOffset == 0)
        return 0;

    iLength     = TextEdit_View_GetLineLength(lpte, iLineIndex);
    pszChars    = TextEdit_View_GetLineChars(lpte, iLineIndex);
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

void TextEdit_View_EnsureVisible(LPTEXTEDITVIEW lpte, POINT pt)
{
    //  Scroll vertically
    int iLineCount  = TextEdit_View_GetLineCount(lpte);
    int iNewTopLine = lpte->iTopLine;
    int iActualPos;
    int iNewOffset;

    if (pt.y >= iNewTopLine + TextEdit_View_GetScreenLines(lpte))
    {
        iNewTopLine = pt.y - TextEdit_View_GetScreenLines(lpte) + 1;
    }
    if (pt.y < iNewTopLine)
    {
        iNewTopLine = pt.y;
    }

    if (iNewTopLine < 0)
        iNewTopLine = 0;
    if (iNewTopLine >= iLineCount)
        iNewTopLine = iLineCount - 1;

    if (lpte->iTopLine != iNewTopLine)
    {
        //FIXME: Is the TRUE's true (right)? 
        TextEdit_View_ScrollToLine(lpte, iNewTopLine, FALSE, TRUE);
        //TextEdit_View_UpdateSiblingScrollPos(lpte, TRUE);
    }

    //  Scroll horizontally
    iActualPos = TextEdit_View_CalculateActualOffset(lpte, pt.y, pt.x);
    iNewOffset = lpte->iOffsetChar;

    if (iActualPos > iNewOffset + TextEdit_View_GetScreenChars(lpte))
    {
        iNewOffset = iActualPos - TextEdit_View_GetScreenChars(lpte) + 16;
    }
    if (iActualPos < iNewOffset)
    {
        iNewOffset = iActualPos - 16;
    }

    // FIXME: might need chaning
    if (iNewOffset >= TextEdit_View_GetMaxLineLength(lpte))
        iNewOffset = TextEdit_View_GetMaxLineLength(lpte) - 1;
    if (iNewOffset < 0)
        iNewOffset = 0;

    if (lpte->iOffsetChar != iNewOffset)
    {
        TextEdit_View_ScrollToChar(lpte, iNewOffset, FALSE, TRUE);
        TextEdit_View_UpdateCaret(lpte);
        //TextEdit_View_UpdateSiblingScrollPos(lpte, FALSE);
    }
}

void TextEdit_View_OnKillFocus(LPTEXTEDITVIEW lpte, HWND hwndGetFocus)
{
    DefWindowProc(lpte->hwnd, WM_KILLFOCUS, (WPARAM)hwndGetFocus, 0);

    lpte->bFocused = FALSE;
    TextEdit_View_UpdateCaret(lpte);
    
    if (!Point_Equal(&lpte->ptSelStart, &lpte->ptSelEnd))
        TextEdit_View_InvalidateLines(lpte, lpte->ptSelStart.y, lpte->ptSelEnd.y, TRUE);

    if (lpte->bDragSelection)
    {
        ReleaseCapture();
        KillTimer(lpte->hwnd, lpte->uDragSelTimer);
        lpte->bDragSelection = FALSE;
    }
}

void TextEdit_View_OnSysColorChange(LPTEXTEDITVIEW lpte) 
{
    DefWindowProc(lpte->hwnd, WM_SYSCOLORCHANGE, 0, 0);
    InvalidateRect(lpte->hwnd, NULL, FALSE);
}

void TextEdit_View_GetText(LPTEXTEDITVIEW lpte, POINT ptStart, POINT ptEnd, LPTSTR *pszText)
{
    //FIXME: check the addresses
    if (lpte->lpes != NULL)
        TextEdit_Buffer_GetText(lpte->lpes, ptStart.y, ptStart.x, ptEnd.y, ptEnd.x, pszText, NULL);
    else
        *pszText = _T("");
}

void TextEdit_View_UpdateView(LPTEXTEDITVIEW lpte, LPINSERTCONTEXT pContext, DWORD dwFlags, int iLineIndex)
{
    int iLineCount;

    if (dwFlags & UPDATE_RESET)
    {
        TextEdit_View_ResetView(lpte);
        TextEdit_View_RecalcVertScrollBar(lpte, FALSE);
        TextEdit_View_RecalcHorzScrollBar(lpte, FALSE);

        return;
    }

    iLineCount = TextEdit_View_GetLineCount(lpte);

    ASSERT(iLineCount > 0);
    ASSERT(iLineIndex >= -1 && iLineIndex < iLineCount);
    
    if ((dwFlags & UPDATE_SINGLELINE) != 0)
    {
        ASSERT(iLineIndex != -1);
        //  All text below this line should be reparsed
        if (lpte->pdwParseCookies != NULL)
        {
            ASSERT(lpte->iParseArraySize == iLineCount);
            memset(lpte->pdwParseCookies + iLineIndex, 0xFF, sizeof(DWORD) * (lpte->iParseArraySize - iLineIndex));
        }
        //  This line'th actual length must be recalculated
        if (lpte->piActualLineLength != NULL)
        {
            ASSERT(lpte->iActualLengthArraySize == iLineCount);
            lpte->piActualLineLength[iLineIndex] = -1;
        }
        //  Repaint the lines
        TextEdit_View_InvalidateLines(lpte, iLineIndex, -1, TRUE);
    }
    else
    {
        if (iLineIndex == -1)
            iLineIndex = 0;     //  Refresh all text
        //  All text below this line should be reparsed
        if (lpte->pdwParseCookies != NULL)
        {
            if (lpte->iParseArraySize != iLineCount)
            {
                //  Reallocate cookies array
                LPDWORD pdwNewArray = (LPDWORD)Mem_Alloc(sizeof(DWORD) * iLineCount);
                
                if (iLineIndex > 0)
                    memcpy(pdwNewArray, lpte->pdwParseCookies, sizeof(DWORD) * iLineIndex);

                Mem_Free(lpte->pdwParseCookies);
                
                lpte->iParseArraySize   = iLineCount;
                lpte->pdwParseCookies   = pdwNewArray;
            }

            memset(lpte->pdwParseCookies + iLineIndex, 0xFF, sizeof(DWORD) * (lpte->iParseArraySize - iLineIndex));
        }
        //  Recalculate actual length for all lines below this
        if (lpte->piActualLineLength != NULL)
        {
            if (lpte->iActualLengthArraySize != iLineCount)
            {
                //  Reallocate actual length array
                LPINT piNewArray = (LPINT)Mem_Alloc(sizeof(INT) * iLineCount);
                
                if (iLineIndex > 0)
                    memcpy(piNewArray, lpte->piActualLineLength, sizeof(INT) * iLineIndex);

                Mem_Free(lpte->piActualLineLength);

                lpte->iActualLengthArraySize = iLineCount;
                lpte->piActualLineLength = piNewArray;
            }

            //sizeof DWORD??!?!?! not INT?
            memset(lpte->piActualLineLength + iLineIndex, 0xFF, sizeof(INT) * (lpte->iActualLengthArraySize - iLineIndex));
        }
        //  Repaint the lines
        TextEdit_View_InvalidateLines(lpte, iLineIndex, -1, TRUE);
    }

    //FIXME: What here?!?!?

    //  All those points must be recalculated and validated
    if (pContext != NULL)
    {
        POINT ptTopLine;

        TextEdit_Buffer_RecalcPoint(*pContext, &lpte->ptCursorPos);
        TextEdit_Buffer_RecalcPoint(*pContext, &lpte->ptSelStart);
        TextEdit_Buffer_RecalcPoint(*pContext, &lpte->ptSelEnd);
        TextEdit_Buffer_RecalcPoint(*pContext, &lpte->ptAnchor);
        ASSERT_VALIDTEXTPOS(lpte, lpte->ptCursorPos);
        ASSERT_VALIDTEXTPOS(lpte, lpte->ptSelStart);
        ASSERT_VALIDTEXTPOS(lpte, lpte->ptSelEnd);
        ASSERT_VALIDTEXTPOS(lpte, lpte->ptAnchor);

        if (lpte->bDraggingText)
        {
            TextEdit_Buffer_RecalcPoint(*pContext, &lpte->ptDraggedTextBegin);
            TextEdit_Buffer_RecalcPoint(*pContext, &lpte->ptDraggedTextEnd);
            ASSERT_VALIDTEXTPOS(lpte, lpte->ptDraggedTextBegin);
            ASSERT_VALIDTEXTPOS(lpte, lpte->ptDraggedTextEnd);
        }
        
        ptTopLine.x = 0;
        ptTopLine.y = lpte->iTopLine;
        TextEdit_Buffer_RecalcPoint(*pContext, &ptTopLine);
        ASSERT_VALIDTEXTPOS(lpte, ptTopLine);
        
        lpte->iTopLine = ptTopLine.y;
        TextEdit_View_UpdateCaret(lpte);
    }

    //  Recalculate vertical scrollbar, if needed
    if ((dwFlags & UPDATE_VERTRANGE) != 0)
    {
        if (!lpte->bVertScrollBarLocked)
            TextEdit_View_RecalcVertScrollBar(lpte, FALSE);
    }

    //  Recalculate horizontal scrollbar, if needed
    if ((dwFlags & UPDATE_HORZRANGE) != 0)
    {
        lpte->iMaxLineLength = -1;
        if (!lpte->bHorzScrollBarLocked)
            TextEdit_View_RecalcHorzScrollBar(lpte, FALSE);
    }
}

int TextEdit_View_OnCreate(LPTEXTEDITVIEW lpte, LPCREATESTRUCT lpCreateStruct) 
{
    memset(&lpte->lfBaseFont, 0, sizeof(lpte->lfBaseFont));
    _tcscpy(lpte->lfBaseFont.lfFaceName, _T("FixedSys"));
    lpte->lfBaseFont.lfHeight           = 0;
    lpte->lfBaseFont.lfWeight           = FW_NORMAL;
    lpte->lfBaseFont.lfItalic           = FALSE;
    lpte->lfBaseFont.lfCharSet          = DEFAULT_CHARSET;
    lpte->lfBaseFont.lfOutPrecision     = OUT_DEFAULT_PRECIS;
    lpte->lfBaseFont.lfClipPrecision    = CLIP_DEFAULT_PRECIS;
    lpte->lfBaseFont.lfQuality          = DEFAULT_QUALITY;
    lpte->lfBaseFont.lfPitchAndFamily   = DEFAULT_PITCH;

    lpte->iCharWidth    = -1;
    lpte->iLineHeight   = -1;

    lpte->hCursor = LoadCursor(PCP_Edit_Dll_GetHandle(), MAKEINTRESOURCE(IDC_MARGIN_CURSOR));

    lpte->lpes = (LPTEXTEDITSTATE)Mem_Alloc(sizeof(TEXTEDITSTATE));

    if (lpCreateStruct->lpCreateParams == NULL || !TextEdit_Buffer_LoadFromFile(lpte, lpte->lpes, (LPTSTR)lpCreateStruct->lpCreateParams, CRLF_STYLE_AUTOMATIC))
        TextEdit_Buffer_InitNew(lpte, CRLF_STYLE_DOS);

    if (DefWindowProc(lpte->hwnd, WM_CREATE, 0, (LPARAM)lpCreateStruct) == -1)
        return (-1);

    return (TRUE);
}

void TextEdit_View_SetAnchor(LPTEXTEDITVIEW lpte, POINT ptNewAnchor)
{
    ASSERT_VALIDTEXTPOS(lpte, ptNewAnchor);
    lpte->ptAnchor = ptNewAnchor;
}

void TextEdit_View_OnEditOperation(LPTEXTEDITVIEW lpte, int nAction, LPCTSTR pszText)
{
    if (lpte->dwFlags & SRCOPT_AUTOINDENT)
    {
        //  Analyse last action...
        if (nAction == PE_ACTION_TYPING && String_Equal(pszText, _T ("\r\n"), TRUE) && !lpte->bOvrMode)
        {
            //  Enter stroke!
            POINT ptCursorPos = TextEdit_View_GetCursorPos(lpte);
            int nLength;
            LPCTSTR pszLineChars;
            int nPos = 0;
            int x, y;
            DWORD dwFlags = TextEdit_View_GetFlags(lpte);

            ASSERT(ptCursorPos.y > 0);
            
            //  Take indentation from the previos line
            nLength = TextEdit_View_GetLineLength(lpte, ptCursorPos.y - 1);
            pszLineChars = TextEdit_View_GetLineChars(lpte, ptCursorPos.y - 1);

            while (nPos < nLength && isspace(pszLineChars[nPos]))
                nPos++;
            
            if (nPos > 0)
            {
                TCHAR *pszInsertStr;
                POINT pt;

                if ((dwFlags & SRCOPT_BRACEGNU) && TextEdit_Syntax_IsKeywordGeneric(lpte->lpsd->apszIndent, (pszLineChars + nLength - 1), _tcslen(pszLineChars + nLength - 1), lpte->lpsd->bCase) && ptCursorPos.y > 0 && nPos && nPos == nLength - 1)
                {
                    if (pszLineChars[nPos - 1] == _T('\t'))
                    {
                        nPos--;
                    }
                    else
                    {
                        int nTabSize    = TextEdit_View_GetTabSize(lpte),
                            nDelta      = nTabSize - nPos % nTabSize;

                        if (!nDelta)
                            nDelta = nTabSize;

                        nPos -= nDelta;

                        if (nPos < 0)
                            nPos = 0;
                    }
                }

                //  Insert part of the previos line
                if ((dwFlags & (SRCOPT_BRACEGNU|SRCOPT_BRACEANSI)) && TextEdit_Syntax_IsKeywordGeneric(lpte->lpsd->apszIndent, (pszLineChars + nLength - 1), _tcslen(pszLineChars + nLength - 1), lpte->lpsd->bCase))
                {
                    if (dwFlags & SRCOPT_INSERTTABS)
                    {
                        pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * (nPos + 2));
                        _tcsncpy(pszInsertStr, pszLineChars, nPos);
                        pszInsertStr[nPos++] = _T('\t');
                    }
                    else
                    {
                        int nTabSize = TextEdit_View_GetTabSize(lpte);
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

                TextEdit_Buffer_InsertText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x,
                    pszInsertStr, &y, &x, PE_ACTION_AUTOINDENT);
                
                pt.x = x;
                pt.y = y;

                TextEdit_View_SetCursorPos(lpte, pt);
                TextEdit_View_SetSelection(lpte, pt, pt);
                TextEdit_View_SetAnchor(lpte, pt);
                TextEdit_View_EnsureVisible(lpte, pt);
            }
            else
            {
                //  Insert part of the previous line
                TCHAR *pszInsertStr;

                if ((dwFlags & (SRCOPT_BRACEGNU|SRCOPT_BRACEANSI)) && TextEdit_Syntax_IsKeywordGeneric(lpte->lpsd->apszIndent, (pszLineChars + nLength - 1), _tcslen(pszLineChars + nLength - 1), lpte->lpsd->bCase))
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
                        int nTabSize = TextEdit_View_GetTabSize(lpte);
                        int nChars = nTabSize - nPos % nTabSize;
                        
                        pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * (nChars + 1));

                        while (nChars--)
                        {
                            pszInsertStr[nPos++] = _T(' ');
                        }
                    }

                    pszInsertStr[nPos] = 0;
                    
                    TextEdit_Buffer_InsertText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x,
                        pszInsertStr, &y, &x, PE_ACTION_AUTOINDENT);

                    pt.x = x;
                    pt.y = y;

                    TextEdit_View_SetCursorPos(lpte, pt);
                    TextEdit_View_SetSelection(lpte, pt, pt);
                    TextEdit_View_SetAnchor(lpte, pt);
                    TextEdit_View_EnsureVisible(lpte, pt);
                }
            }
        }
        else if (nAction == PE_ACTION_TYPING && (TextEdit_View_GetFlags(lpte) & SRCOPT_FNBRACE) && bracetypesz(pszText) == 2)
        {
            //  Enter stroke!
            POINT ptCursorPos   = TextEdit_View_GetCursorPos(lpte);
            LPCTSTR pszChars    = TextEdit_View_GetLineChars(lpte, ptCursorPos.y);

            if (ptCursorPos.x > 1 && xisalnum(pszChars[ptCursorPos.x - 2]))
            {
                int x, y;
                LPTSTR pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * 2);

                *pszInsertStr = _T(' ');
                pszInsertStr[1] = _T('\0');

                TextEdit_Buffer_InsertText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x - 1,
                    pszInsertStr, &y, &x, PE_ACTION_AUTOINDENT);

                ptCursorPos.x = x + 1;
                ptCursorPos.y = y;
                
                TextEdit_View_SetCursorPos(lpte, ptCursorPos);
                TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
                TextEdit_View_SetAnchor(lpte, ptCursorPos);
                TextEdit_View_EnsureVisible(lpte, ptCursorPos);
            }
        }
        else if (nAction == PE_ACTION_TYPING && (TextEdit_View_GetFlags(lpte) & SRCOPT_BRACEGNU) && TextEdit_Syntax_IsKeywordGeneric(lpte->lpsd->apszIndent, pszText, _tcslen(pszText), lpte->lpsd->bCase))
        {
            //  Enter stroke!
            POINT ptCursorPos = TextEdit_View_GetCursorPos(lpte);
            //  Take indentation from the previous line
            int nLength = TextEdit_View_GetLineLength(lpte, ptCursorPos.y);
            LPCTSTR pszLineChars = TextEdit_View_GetLineChars(lpte, ptCursorPos.y );
            int nPos = 0;

            while (nPos < nLength && isspace(pszLineChars[nPos]))
                nPos++;

            if (nPos == nLength - 1)
            {
                TCHAR *pszInsertStr;
                int x, y;
                POINT pt;

                if (TextEdit_View_GetFlags(lpte) & SRCOPT_INSERTTABS)
                {
                    pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * 2);
                    *pszInsertStr = _T('\t');
                    nPos = 1;
                }
                else
                {
                    int nTabSize = TextEdit_View_GetTabSize(lpte);
                    int nChars = nTabSize - nPos % nTabSize;

                    pszInsertStr = (TCHAR *)_alloca(sizeof(TCHAR) * (nChars + 1));
                    nPos = 0;
                    
                    while (nChars--)
                    {
                        pszInsertStr[nPos++] = _T(' ');
                    }
                }

                pszInsertStr[nPos] = 0;
                
                TextEdit_Buffer_InsertText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x - 1,
                    pszInsertStr, &y, &x, PE_ACTION_AUTOINDENT);
                
                pt.x = x + 1;
                pt.y = y;

                TextEdit_View_SetCursorPos(lpte, pt);
                TextEdit_View_SetSelection(lpte, pt, pt);
                TextEdit_View_SetAnchor(lpte, pt);
                TextEdit_View_EnsureVisible(lpte, pt);
            }
        }
        else if (nAction == PE_ACTION_TYPING && (TextEdit_View_GetFlags(lpte) & (SRCOPT_BRACEGNU|SRCOPT_BRACEANSI)) && TextEdit_Syntax_IsKeywordGeneric(lpte->lpsd->apszUnIndent, pszText, _tcslen(pszText), lpte->lpsd->bCase))
        {
            POINT ptCursorPos = TextEdit_View_GetCursorPos(lpte);
            POINT ptMatchPos;
            int nLength;
            LPCTSTR pszLineChars;
            int nPos = 0;

//          ptMatchPos = TextEdit_View_MatchBrace(lpte, ptCursorPos);

//          if (ptMatchPos.x == -1 || ptMatchPos.y == -1)
            ptMatchPos = ptCursorPos;
            
            nLength = TextEdit_View_GetLineLength(lpte, ptMatchPos.y);
            pszLineChars = TextEdit_View_GetLineChars(lpte, ptMatchPos.y );

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
                    int nTabSize = TextEdit_View_GetTabSize(lpte);
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

                TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x - nPos - 1,
                    ptCursorPos.y, ptCursorPos.x - 1, PE_ACTION_AUTOINDENT);
                ptCursorPos.x -= nPos;

                TextEdit_View_SetCursorPos(lpte, ptCursorPos);
                TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
                TextEdit_View_SetAnchor(lpte, ptCursorPos);
                TextEdit_View_EnsureVisible(lpte, ptCursorPos);
            }
        }
    }

    SendMessage(GetParent(lpte->hwnd), WM_COMMAND, MAKEWPARAM(GetWindowLong(lpte->hwnd, GWL_ID), EN_CHANGE), (LPARAM)lpte->hwnd);
}

POINT TextEdit_View_MatchBrace(LPTEXTEDITVIEW lpte, POINT ptStartPos)
{
    int nLength = TextEdit_View_GetLineLength(lpte, ptStartPos.y);
    LPCTSTR pszText = TextEdit_View_GetLineChars(lpte, ptStartPos.y);
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
        LPCTSTR pszOpenComment      = lpte->lpsd->szOpenComment,
                pszCloseComment     = lpte->lpsd->szCloseComment,
                pszLineComment      = lpte->lpsd->szLineComment,
                pszOpenComment2     = lpte->lpsd->szOpenComment2,
                pszCloseComment2    = lpte->lpsd->szCloseComment2,
                pszLineComment2     = lpte->lpsd->szLineComment2,
                pszTest;
        int nOpenComment    = lpte->lpsd->nOpenCommentLen,
            nCloseComment   = lpte->lpsd->nCloseCommentLen,
            nLineComment    = lpte->lpsd->nLineCommentLen,
            nOpenComment2   = lpte->lpsd->nOpenComment2Len,
            nCloseComment2  = lpte->lpsd->nCloseComment2Len,
            nLineComment2   = lpte->lpsd->nLineComment2Len;

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
                    ptStartPos.x = TextEdit_View_GetLineLength(lpte, --ptStartPos.y);
                    pszText = TextEdit_View_GetLineChars(lpte, ptStartPos.y);
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
            int nLines = TextEdit_View_GetLineCount(lpte);

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
                    nLength = TextEdit_View_GetLineLength(lpte, ++ptStartPos.y);
                    pszBegin = pszText = TextEdit_View_GetLineChars(lpte, ptStartPos.y);
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

POINT TextEdit_View_GetCursorPos(LPTEXTEDITVIEW lpte)
{
    return lpte->ptCursorPos;
}

void TextEdit_View_SetCursorPos(LPTEXTEDITVIEW lpte, POINT ptCursorPos)
{
    ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
    lpte->ptCursorPos = ptCursorPos;
    lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);
    TextEdit_View_UpdateCaret(lpte);
}

void TextEdit_View_SetSelectionMargin(LPTEXTEDITVIEW lpte, BOOL bSelMargin)
{
    if (lpte->bSelMargin != bSelMargin)
    {
        lpte->bSelMargin = bSelMargin;
        if (IsWindow(lpte->hwnd))
        {
            lpte->iScreenChars = -1;
            InvalidateRect(lpte->hwnd, NULL, FALSE);
            TextEdit_View_RecalcHorzScrollBar(lpte, FALSE);
        }
    }
}

//FIXME: right?
/*
void TextEdit_View_GetFont(LOGFONT *lplf)
{
    lplf = lpte->lfBaseFont;
}
*/
void TextEdit_View_SetFont(LPTEXTEDITVIEW lpte, LPLOGFONT lplf)
{
    int i;

    lpte->lfBaseFont    = *lplf;
    lpte->iScreenLines  = -1;
    lpte->iScreenChars  = -1;
    lpte->iCharWidth    = -1;
    lpte->iLineHeight   = -1;
    
    if (lpte->pCacheBitmap != NULL)
    {
        DeleteObject(*lpte->pCacheBitmap);
        Mem_Free(lpte->pCacheBitmap);
        lpte->pCacheBitmap = NULL;
    }

    for (i = 0; i < 4; i++)
    {
        if (lpte->ahFonts[i] != NULL)
        {
            DeleteObject(lpte->ahFonts[i]);
//          Mem_Free(lpte->ahFonts[i]);
            lpte->ahFonts[i] = NULL;
        }
    }

    if (IsWindow(lpte->hwnd))
    {
        TextEdit_View_RecalcVertScrollBar(lpte, FALSE);
        TextEdit_View_RecalcHorzScrollBar(lpte, FALSE);
        TextEdit_View_UpdateCaret(lpte);
        InvalidateRect(lpte->hwnd, NULL, FALSE);
    }
}

void TextEdit_View_ClearAllNumBookmarks(LPTEXTEDITVIEW lpte)
{
    if (lpte->lpes != NULL)
    {
        int iBookmarkID;

        for (iBookmarkID = 0; iBookmarkID <= 9; iBookmarkID++)
        {
            int iLine = TextEdit_Buffer_GetLineWithFlag(lpte->lpes, LF_BOOKMARK(iBookmarkID));
            if (iLine >= 0)
            {
                TextEdit_Buffer_SetLineFlag(lpte, lpte->lpes, iLine, LF_BOOKMARK(iBookmarkID), FALSE, TRUE);
            }
        }
    }
}

void TextView_ToggleNumBookmark(LPTEXTEDITVIEW lpte, int nBookmarkID)
{
    ASSERT(nBookmarkID >= 0 && nBookmarkID <= 9);
    
    if (lpte->lpes != NULL)
    {
        DWORD dwFlags = TextEdit_View_GetLineFlags(lpte, lpte->ptCursorPos.y);
        DWORD dwMask = LF_BOOKMARK(nBookmarkID);

        TextEdit_Buffer_SetLineFlag(lpte, lpte->lpes, lpte->ptCursorPos.y, dwMask, (dwFlags & dwMask) == 0, TRUE);
    }
}

void TextView_GotoNumBookmark(LPTEXTEDITVIEW lpte, int nBookmarkID)
{
    ASSERT(nBookmarkID >= 0 && nBookmarkID <= 9);

    if (lpte->lpes != NULL)
    {
        int iLine = TextEdit_Buffer_GetLineWithFlag(lpte->lpes, LF_BOOKMARK(nBookmarkID));

        if (iLine >= 0)
        {
            POINT pt = { 0, iLine };

            ASSERT_VALIDTEXTPOS(lpte, pt);
            TextEdit_View_SetCursorPos(lpte, pt);
            TextEdit_View_SetSelection(lpte, pt, pt);
            TextEdit_View_SetAnchor(lpte, pt);
            TextEdit_View_EnsureVisible(lpte, pt);
        }
    }
}

void TextEdit_View_ClearAllBookmarks(LPTEXTEDITVIEW lpte)
{
    if (lpte->lpes != NULL)
    {
        int nLineCount = TextEdit_View_GetLineCount(lpte);
        int i;

        for (i = 0; i < nLineCount; i++)
        {
            if (TextEdit_Buffer_GetLineFlags(lpte->lpes, i) & LF_BOOKMARKS)
                TextEdit_Buffer_SetLineFlag(lpte, lpte->lpes, i, LF_BOOKMARKS, FALSE, TRUE);
        }

        lpte->bBookmarkExist = FALSE;
    }
}

void TextEdit_View_ShowCursor(LPTEXTEDITVIEW lpte)
{
    lpte->bCursorHidden = FALSE;
    TextEdit_View_UpdateCaret(lpte);
}

void TextEdit_View_HideCursor(LPTEXTEDITVIEW lpte)
{
    lpte->bCursorHidden = TRUE;
    TextEdit_View_UpdateCaret(lpte);
}

BOOL TextEdit_View_GetSelectionPos(LPTEXTEDITVIEW lpte, LPDWORD lpdwStartPos, LPDWORD lpdwEndPos)
{
    POINT ptStartPos = { 0, 0 }, ptEndPos;
    DWORD dwCurrentPos = 0;
    BOOL bEdit = TRUE;
    int i;

    TextEdit_View_GetSelection(lpte, &ptStartPos, &ptEndPos);

    for (i = 0; i < ptStartPos.y; i++)
    {
        dwCurrentPos += TextEdit_View_GetLineLength(lpte, i++);
    }

    *lpdwStartPos = dwCurrentPos + ptStartPos.x;

    for (i = ptStartPos.y; i < ptEndPos.y; i++)
    {
        dwCurrentPos += TextEdit_View_GetLineLength(lpte, i++);
    }

    *lpdwEndPos = dwCurrentPos + ptEndPos.x;

    return (TRUE);
}

BOOL TextEdit_View_SetSelectionPos(LPTEXTEDITVIEW lpte, DWORD dwStartPos, DWORD dwEndPos)
{
    POINT ptStartPos = { 0, 0 }, ptEndPos;
    DWORD dwCurrentPos = 0;
    int nLineCount = TextEdit_View_GetLineCount(lpte);
    BOOL bEdit = TRUE;

    if (dwEndPos != -1)
        ASSERT(dwStartPos <= dwEndPos);

    while (dwCurrentPos < dwStartPos)
    {
        if (ptStartPos.y >= nLineCount)
        {
            ptStartPos.y = nLineCount - 1;
            ptStartPos.x = TextEdit_View_GetLineLength(lpte, ptStartPos.y);
            bEdit = FALSE;

            break;
        }

        dwCurrentPos += TextEdit_View_GetLineLength(lpte, ptStartPos.y++);
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
        ptEndPos.y = nLineCount - 1;
        ptEndPos.x = TextEdit_View_GetLineLength(lpte, ptEndPos.y);
        bEdit = FALSE;
    }
    else
    {
        while (dwCurrentPos < dwEndPos)
        {
            if (ptEndPos.y >= nLineCount)
            {
                ptEndPos.y = nLineCount - 1;
                ptEndPos.x = TextEdit_View_GetLineLength(lpte, ptEndPos.y);
                bEdit = FALSE;

                break;
            }

            dwCurrentPos += TextEdit_View_GetLineLength(lpte, ptEndPos.y++);
        }
    }

    if (bEdit)
    {
        dwCurrentPos -= (dwCurrentPos - dwEndPos);
        ptEndPos.x = dwCurrentPos;
        ptEndPos.y -= (ptEndPos.y == 0) ? 0 : 1;
    }

    lpte->ptCursorPos   = ptEndPos;
    lpte->ptAnchor      = lpte->ptCursorPos;
    TextEdit_View_SetSelection(lpte, ptStartPos, ptEndPos);
    TextEdit_View_UpdateCaret(lpte);
    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
    
    return (TRUE);
}

BOOL TextEdit_View_HighlightText(LPTEXTEDITVIEW lpte, POINT ptStartPos, int iLength, BOOL bReverse)
{
    POINT ptEndPos = ptStartPos;
    int nCount = TextEdit_View_GetLineLength(lpte, ptEndPos.y) - ptEndPos.x;

    ASSERT_VALIDTEXTPOS(lpte, ptStartPos);
    
    if (iLength <= nCount)
    {
        ptEndPos.x += iLength;
    }
    else
    {
        while (iLength > nCount)
        {
            iLength -= nCount + 1;
            nCount = TextEdit_View_GetLineLength(lpte, ++ptEndPos.y);
        }

        ptEndPos.x = iLength;
    }

    ASSERT_VALIDTEXTPOS(lpte, lpte->ptCursorPos);       //  Probably 'nLength' is bigger than expected...
    lpte->ptCursorPos = (bReverse) ? ptStartPos : ptEndPos;
    lpte->ptAnchor = lpte->ptCursorPos;
    TextEdit_View_SetSelection(lpte, ptStartPos, ptEndPos);
    TextEdit_View_UpdateCaret(lpte);
    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);

    return (TRUE);
}

void TextEdit_View_ToggleBookmark(LPTEXTEDITVIEW lpte)
{
    int iLine;

    if (lpte->lpes != NULL)
    {
        DWORD dwFlags = TextEdit_View_GetLineFlags(lpte, lpte->ptCursorPos.y);
        DWORD dwMask  = LF_BOOKMARKS;
        TextEdit_Buffer_SetLineFlag(lpte, lpte->lpes, lpte->ptCursorPos.y, dwMask, (dwFlags & dwMask) == 0, FALSE);
    }

    iLine = TextEdit_Buffer_GetLineWithFlag(lpte->lpes, LF_BOOKMARKS);
    
    if (iLine >= 0)
        lpte->bBookmarkExist = TRUE;
    else
        lpte->bBookmarkExist = FALSE;
}

void TextEdit_View_NextBookmark(LPTEXTEDITVIEW lpte)
{
    if (lpte->lpes != NULL)
    {
        int iLine = TextEdit_Buffer_FindNextBookmarkLine(lpte->lpes, lpte->ptCursorPos.y);
        
        if (iLine >= 0)
        {
            POINT pt = { 0, iLine };
            ASSERT_VALIDTEXTPOS(lpte, pt);
            TextEdit_View_SetCursorPos(lpte, pt);
            TextEdit_View_SetSelection(lpte, pt, pt);
            TextEdit_View_SetAnchor(lpte, pt);
            TextEdit_View_EnsureVisible(lpte, pt);
        }
    }
}

void TextEdit_View_PrevBookmark(LPTEXTEDITVIEW lpte)
{
    if (lpte->lpes != NULL)
    {
        int iLine = TextEdit_Buffer_FindPrevBookmarkLine(lpte->lpes, lpte->ptCursorPos.y);

        if (iLine >= 0)
        {
            POINT pt = { 0, iLine };
            ASSERT_VALIDTEXTPOS(lpte, pt);
            TextEdit_View_SetCursorPos(lpte, pt);
            TextEdit_View_SetSelection(lpte, pt, pt);
            TextEdit_View_SetAnchor(lpte, pt);
            TextEdit_View_EnsureVisible(lpte, pt);
        }
    }
}

BOOL TextEdit_View_GetViewTabs(LPTEXTEDITVIEW lpte)
{
    return lpte->bViewTabs;
}

void TextEdit_View_SetViewTabs(LPTEXTEDITVIEW lpte, BOOL bViewTabs)
{
    if (bViewTabs != lpte->bViewTabs)
    {
        lpte->bViewTabs = bViewTabs;
        if (IsWindow(lpte->hwnd))
            InvalidateRect(lpte->hwnd, NULL, FALSE);
    }
}

BOOL TextEdit_View_GetSelectionMargin(LPTEXTEDITVIEW lpte)
{
    return (lpte->bSelMargin);
}

int TextEdit_View_GetMarginWidth(LPTEXTEDITVIEW lpte)
{
    return (lpte->bSelMargin) ? 20 : 1;
}

BOOL TextEdit_View_GetSmoothScroll(LPTEXTEDITVIEW lpte)
{
    return (lpte->bSmoothScroll);
}

void TextEdit_View_SetSmoothScroll(LPTEXTEDITVIEW lpte, BOOL bSmoothScroll)
{
    lpte->bSmoothScroll = bSmoothScroll;
}

//  [JRT]
BOOL TextEdit_View_GetDisableDragAndDrop(LPTEXTEDITVIEW lpte)
{
    return (lpte->bDisableDragAndDrop);
}

//  [JRT]
void TextEdit_View_SetDisableDragAndDrop(LPTEXTEDITVIEW lpte, BOOL bDDAD)
{
    lpte->bDisableDragAndDrop = bDDAD;
}

BOOL TextEdit_View_QueryEditable(LPTEXTEDITVIEW lpte)
{
    if (lpte->lpes == NULL)
        return (FALSE);

    return (!TextEdit_Buffer_GetReadOnly(lpte->lpes));
}

BOOL TextEdit_View_DeleteCurrentSelection(LPTEXTEDITVIEW lpte)
{
    if (TextEdit_View_IsSelection(lpte))
    {
        POINT ptSelStart, ptSelEnd, ptCursorPos;
        TextEdit_View_GetSelection(lpte, &ptSelStart, &ptSelEnd);

        ptCursorPos = ptSelStart;
        ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
        TextEdit_View_SetAnchor(lpte, ptCursorPos);
        TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
        TextEdit_View_SetCursorPos(lpte, ptCursorPos);
        TextEdit_View_EnsureVisible(lpte, ptCursorPos);

        // [JRT]:
        TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, PE_ACTION_DELSEL);

        return (TRUE);
    }

    return (FALSE);
}

void TextEdit_View_OnChar(LPTEXTEDITVIEW lpte, UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    DefWindowProc(lpte->hwnd, WM_CHAR, (WPARAM)nChar, MAKELPARAM(nRepCnt, nFlags));

    if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 ||
            (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0)
        return;

    if (nChar == VK_RETURN)
    {
        if (lpte->bOvrMode)
        {
            POINT ptCursorPos;
            
            ptCursorPos = TextEdit_View_GetCursorPos(lpte);
            ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
            
            if (ptCursorPos.y < TextEdit_View_GetLineCount(lpte) - 1)
            {
                ptCursorPos.x = 0;
                ptCursorPos.y++;

                ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
                TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
                TextEdit_View_SetAnchor(lpte, ptCursorPos);
                TextEdit_View_SetCursorPos(lpte, ptCursorPos);
                TextEdit_View_EnsureVisible(lpte, ptCursorPos);

                return;
            }
        }

        TextEdit_Buffer_BeginUndoGroup(lpte->lpes, FALSE);

        if (TextEdit_View_QueryEditable(lpte) && lpte->lpes != NULL)
        {
            POINT ptCursorPos;
            const static TCHAR pszText[2] = _T("\r\n");
            int x, y;

            if (TextEdit_View_IsSelection(lpte))
            {
                POINT ptSelStart, ptSelEnd;

                TextEdit_View_GetSelection(lpte, &ptSelStart, &ptSelEnd);

                ptCursorPos = ptSelStart;

                TextEdit_Buffer_DeleteText(lpte, lpte->lpes, lpte->ptSelStart.y, lpte->ptSelStart.x, lpte->ptSelEnd.y, lpte->ptSelStart.x, PE_ACTION_TYPING);
            }
            else
            {
                ptCursorPos = TextEdit_View_GetCursorPos(lpte);
            }

            ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);

            TextEdit_Buffer_InsertText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x, pszText, &y, &x, PE_ACTION_TYPING);

            ptCursorPos.x = x;
            ptCursorPos.y = y;
            ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
            
            TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
            TextEdit_View_SetAnchor(lpte, ptCursorPos);
            TextEdit_View_SetCursorPos(lpte, ptCursorPos);
            TextEdit_View_EnsureVisible(lpte, ptCursorPos);
        }

        TextEdit_Buffer_FlushUndoGroup(lpte, lpte->lpes);

        return;
    }
    else if (nChar == VK_BACK || nChar == 0x7F) //0xF7 is some weird char put out when Ctrl+Backspace are pressed
    {
        TextEdit_View_OnBackspace(lpte);
    }
    else if (nChar == VK_ESCAPE)
    {
        if (lpte->nSelectionMode != PE_SELECTION_MODE_NONE)
            lpte->nSelectionMode = PE_SELECTION_MODE_NONE;
    }
    else if (nChar == VK_TAB)
    {
        TextEdit_View_Tab(lpte);
    }
    else if (nChar > 0x1F)
    {
        if (TextEdit_View_QueryEditable(lpte) && lpte->lpes != NULL)
        {
            POINT ptSelStart, ptSelEnd, ptCursorPos;
            TCHAR pszText[2];
            int x, y;

            TextEdit_Buffer_BeginUndoGroup(lpte->lpes, (nChar != _T(' ')));
            TextEdit_View_GetSelection(lpte, &ptSelStart, &ptSelEnd);

            if (!Point_Equal(&ptSelStart, &ptSelEnd))
            {
                ptCursorPos = ptSelStart;
                TextEdit_View_DeleteCurrentSelection(lpte);
            }
            else
            {
                ptCursorPos = TextEdit_View_GetCursorPos(lpte);
                if (lpte->bOvrMode && ptCursorPos.x < TextEdit_View_GetLineLength(lpte, ptCursorPos.y))
                    TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x, ptCursorPos.y, ptCursorPos.x + 1, PE_ACTION_TYPING);
            }

            ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);

            pszText[0] = (TCHAR)nChar;
            pszText[1] = _T('\0');

            TextEdit_Buffer_InsertText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x, pszText, &y, &x, PE_ACTION_TYPING);
            
            ptCursorPos.x = x;
            ptCursorPos.y = y;
            ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
            
            TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
            TextEdit_View_SetAnchor(lpte, ptCursorPos);
            TextEdit_View_SetCursorPos(lpte, ptCursorPos);
            TextEdit_View_EnsureVisible(lpte, ptCursorPos);

            TextEdit_Buffer_FlushUndoGroup(lpte, lpte->lpes);
        }
    }
}

void TextEdit_View_OnKeyDown(LPTEXTEDITVIEW lpte, UINT nVirtKey, UINT cRepeat, UINT fFlags)
{
    int i;

    DefWindowProc(lpte->hwnd, WM_KEYDOWN, (WPARAM)nVirtKey, MAKELPARAM(cRepeat, fFlags));

    for (i = 0; i < (int)cRepeat; i++)
    {
        BOOL fControl   = IsCtrlDown();
        BOOL fShift     = (IsShiftDown() ||
                            lpte->nSelectionMode == PE_SELECTION_MODE_CHAR ||
                            lpte->nSelectionMode == PE_SELECTION_MODE_LINE)
                            ? TRUE : FALSE;
        BOOL fAlt       = IsAltDown();

        if (nVirtKey == VK_INSERT && !fControl && !fShift && !fAlt)
        {
            NMHDR nmhdr;

            lpte->bOvrMode = !lpte->bOvrMode;

            TextEdit_View_UpdateCaret(lpte);

            nmhdr.hwndFrom  = lpte->hwnd;
            nmhdr.idFrom    = GetWindowLong(lpte->hwnd, GWL_ID);
            nmhdr.code      = PEN_OVERWRITEMODECHANGED;

            SendMessage(GetParent(lpte->hwnd), WM_NOTIFY, (WPARAM)nmhdr.idFrom, (LPARAM)&nmhdr);
        }
        if (nVirtKey == VK_DELETE)
        {
                TextEdit_View_OnDelete(lpte);
        } // 0x21 Through 0x28 (movement keys)
        else if (nVirtKey >= VK_PRIOR && nVirtKey <= VK_DOWN)
        {
            if (nVirtKey == VK_PRIOR)
            {
                TextEdit_Move_MovePgUp(lpte, fShift);
            }
            else if (nVirtKey == VK_NEXT)
            {
                TextEdit_Move_MovePgDn(lpte, fShift);
            }
            else if (nVirtKey == VK_END)
            {
                if (fControl)
                    TextEdit_Move_MoveCtrlEnd(lpte, fShift);
                else
                    TextEdit_Move_MoveEnd(lpte, fShift);
            }
            else if (nVirtKey == VK_HOME)
            {
                if (fControl)
                    TextEdit_Move_MoveCtrlHome(lpte, fShift);
                else
                    TextEdit_Move_MoveHome(lpte, fShift);
            }
            else if (nVirtKey == VK_LEFT)
            {
                if (fControl)
                    TextEdit_Move_MoveWordLeft(lpte, fShift);
                else
                    TextEdit_Move_MoveLeft(lpte, fShift);
            }
            else if (nVirtKey == VK_UP)
            {
                if (fControl && !fShift)
                    TextEdit_Move_ScrollUp(lpte);
                else
                    TextEdit_Move_MoveUp(lpte, fShift);
            }
            if (nVirtKey == VK_RIGHT)
            {
                if (fControl)
                    TextEdit_Move_MoveWordRight(lpte, fShift);
                else
                    TextEdit_Move_MoveRight(lpte, fShift);
            }
            else if (nVirtKey == VK_DOWN)
            {
                if (fControl && !fShift)
                    TextEdit_Move_ScrollDown(lpte);
                else
                    TextEdit_Move_MoveDown(lpte, fShift);
            }
        }
    }
}

void TextEdit_View_OnPaste(LPTEXTEDITVIEW lpte)
{
    LPTSTR pszText;

    if (!TextEdit_View_QueryEditable(lpte) || lpte->lpes == NULL)
        return;

    if (Clipboard_GetText(&pszText))
    {
        int x, y;
        POINT ptCursorPos;

        TextEdit_Buffer_BeginUndoGroup(lpte->lpes, FALSE);

        if (TextEdit_View_IsSelection(lpte))
        {
            POINT ptSelStart, ptSelEnd;

            TextEdit_View_GetSelection(lpte, &ptSelStart, &ptSelEnd);

            ptCursorPos = ptSelStart;

            TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, PE_ACTION_PASTE);
        }
        else
        {
             ptCursorPos = TextEdit_View_GetCursorPos(lpte);
        }

        ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);

        TextEdit_Buffer_InsertText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x, pszText, &y, &x, PE_ACTION_PASTE);
        ptCursorPos.x = x;
        ptCursorPos.y = y;

        ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
        TextEdit_View_SetAnchor(lpte, ptCursorPos);
        TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
        TextEdit_View_SetCursorPos(lpte, ptCursorPos);
        TextEdit_View_EnsureVisible(lpte, ptCursorPos);

        Mem_Free(pszText);
        TextEdit_Buffer_FlushUndoGroup(lpte, lpte->lpes);
    }
}

BOOL TextEdit_View_IsSelection(LPTEXTEDITVIEW lpte)
{
    return (!Point_Equal(&lpte->ptSelStart, &lpte->ptSelEnd));
}

void TextEdit_View_OnCopy(LPTEXTEDITVIEW lpte)
{
    LPTSTR pszText;
    
    if (Point_Equal(&lpte->ptSelStart, &lpte->ptSelEnd))
        return;

    TextEdit_View_PrepareSelBounds(lpte);
    TextEdit_View_GetText(lpte, lpte->ptDrawSelStart, lpte->ptDrawSelEnd, &pszText);
    Clipboard_SetText(pszText);

    Mem_Free(pszText);
}

void TextEdit_View_OnCut(LPTEXTEDITVIEW lpte)
{
    POINT ptSelStart, ptSelEnd, ptCursorPos;
    LPTSTR pszText;

    if (!TextEdit_View_QueryEditable(lpte) || lpte->lpes == NULL)
        return;
    if (!TextEdit_View_IsSelection(lpte))
        return;

    TextEdit_View_GetSelection(lpte, &ptSelStart, &ptSelEnd);
    TextEdit_View_GetText(lpte, ptSelStart, ptSelEnd, &pszText);
    Clipboard_SetText(pszText);

    ptCursorPos = ptSelStart;
    ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
    TextEdit_View_SetAnchor(lpte, ptCursorPos);
    TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
    TextEdit_View_SetCursorPos(lpte, ptCursorPos);
    TextEdit_View_EnsureVisible(lpte, ptCursorPos);

    Mem_Free(pszText);
    TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, PE_ACTION_CUT);
}

void TextEdit_View_OnClear(LPTEXTEDITVIEW lpte)
{
    POINT ptSelStart, ptSelEnd, ptCursorPos;

    if (!TextEdit_View_QueryEditable(lpte) || lpte->lpes == NULL)
        return;

    TextEdit_View_GetSelection(lpte, &ptSelStart, &ptSelEnd);
    
    if (Point_Equal(&ptSelStart, &ptSelEnd))
    {
        if (ptSelEnd.x == TextEdit_View_GetLineLength(lpte, ptSelEnd.y))
        {
            if (ptSelEnd.y == TextEdit_View_GetLineCount(lpte) - 1)
                return;

            ptSelEnd.y ++;
            ptSelEnd.x = 0;
        }
        else
            ptSelEnd.x ++;
    }

    ptCursorPos = ptSelStart;
    ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
    TextEdit_View_SetAnchor(lpte, ptCursorPos);
    TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
    TextEdit_View_SetCursorPos(lpte, ptCursorPos);
    TextEdit_View_EnsureVisible(lpte, ptCursorPos);

    TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, PE_ACTION_DELETE);
}

void TextEdit_View_OnBackspace(LPTEXTEDITVIEW lpte)
{
    POINT ptCursorPos, ptCurrentCursorPos;
    BOOL bDeleted = FALSE;
    BOOL fControl = IsCtrlDown();

    if (TextEdit_View_IsSelection(lpte))
    {
        TextEdit_View_OnClear(lpte);
        return;
    }

    if (!TextEdit_View_QueryEditable(lpte) || lpte->lpes == NULL)
        return;

    ptCurrentCursorPos = ptCursorPos = TextEdit_View_GetCursorPos(lpte);
    
    if (ptCursorPos.x == 0) // If At Start Of Line
    {
        if (lpte->dwFlags & SRCOPT_BSATBOL) // If DBSASOL Is Disabled
        {
            if (ptCursorPos.y > 0) // If Previous Lines Available
            {
                ptCursorPos.y--; // Decrement To Previous Line
                ptCursorPos.x = TextEdit_View_GetLineLength(lpte, ptCursorPos.y);   // Set Cursor To End Of Previous Line
                bDeleted = TRUE; // Set Deleted Flag
            }
        }
    }
    else // If Caret Not At SOL
    {
        if (fControl)
        {
            LPCTSTR pszLine = TextEdit_View_GetLineChars(lpte, ptCursorPos.y);
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
                ptCursorPos.x = TextEdit_View_GetLineLength(lpte, ptCursorPos.y);
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

    ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
    TextEdit_View_SetAnchor(lpte, ptCursorPos);
    TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
    TextEdit_View_SetCursorPos(lpte, ptCursorPos);
    TextEdit_View_EnsureVisible(lpte, ptCursorPos);

    if (bDeleted)
        TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x, ptCurrentCursorPos.y, ptCurrentCursorPos.x, PE_ACTION_BACKSPACE);

    return;
}

void TextEdit_View_Tab(LPTEXTEDITVIEW lpte) 
{
    BOOL    bTabify = FALSE;
    POINT   ptSelStart, ptSelEnd, ptCursorPos;
    static  TCHAR szText[256];
    int x, y;
    
    if (!TextEdit_View_QueryEditable(lpte) || lpte->lpes == NULL)
        return;

    if (TextEdit_View_IsSelection(lpte))
    {
        TextEdit_View_GetSelection(lpte, &ptSelStart, &ptSelEnd);
        bTabify = TRUE; //(ptSelStart.y != ptSelEnd.y);
    }

    ptCursorPos = TextEdit_View_GetCursorPos(lpte);
    ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);

    if (!(TextEdit_View_GetFlags(lpte) & SRCOPT_INSERTTABS))
    {
        int nTabSize    = TextEdit_View_GetTabSize(lpte);
        int nChars      = nTabSize - ptCursorPos.x % nTabSize;

        memset(szText, _T(' '), nChars * sizeof(TCHAR));
        szText[nChars] = _T('\0');
    }

    if (bTabify)
    {
        int nStartLine, nEndLine, i;

        TextEdit_Buffer_BeginUndoGroup(lpte->lpes, FALSE);

        nStartLine = ptSelStart.y;
        nEndLine = ptSelEnd.y;
        ptSelStart.x = 0;

        if (ptSelEnd.x > 0)
        {
            if (ptSelEnd.y == TextEdit_View_GetLineCount(lpte) - 1)
            {
                ptSelEnd.x = TextEdit_View_GetLineLength(lpte, ptSelEnd.y);
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

        TextEdit_View_SetSelection(lpte, ptSelStart, ptSelEnd);
        TextEdit_View_SetCursorPos(lpte, ptSelEnd);
        TextEdit_View_EnsureVisible(lpte, ptSelEnd);

        //  Shift selection to right
        lpte->bHorzScrollBarLocked = TRUE;

        for (i = nStartLine; i <= nEndLine; i++)
        {
            int x, y;

            if (TextEdit_View_GetFlags(lpte) & SRCOPT_INSERTTABS)
            {
                int iSpaceStart, nSpaces = 0;
                int nTabs = 0, nTabSize = TextEdit_View_GetTabSize(lpte), nTabOffset;
                LPCTSTR pszSpaces, pszSpacesBase;
                int nActualX;
                ptCursorPos = TextEdit_View_GetCursorPos(lpte);
                ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);

                pszSpacesBase   = TextEdit_View_GetLineChars(lpte, ptCursorPos.y);
                pszSpaces       = (pszSpacesBase - _tcslen(pszSpacesBase) + ptCursorPos.x);
                iSpaceStart     = _tcslen(pszSpaces) - 1;
                while (iSpaceStart >= 0 && pszSpaces[iSpaceStart] == _T(' '))
                {
                    nSpaces++;
                    iSpaceStart--;
                }

                // Can one use (iSpaceStart + nSpaces) instead of ptCursorPos.x?
                nActualX = TextEdit_View_CalculateActualOffset(lpte, ptCursorPos.y, ptCursorPos.x);
                nTabOffset = (nTabSize - (nActualX % nTabSize));
                if (nTabOffset == nTabSize)
                    nTabOffset = 0;
                nTabs = (int)ceil((double)((double)(nSpaces + nTabOffset) / (double)nTabSize));
                if ((nTabOffset == 0) || (nSpaces == 0 && nTabs == 0))
                    nTabs += 1;

                memset(szText, _T('\t'), nTabs * sizeof(TCHAR));
                szText[nTabs] = _T('\0');
                TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x - nSpaces, ptCursorPos.y, ptCursorPos.x, PE_ACTION_TYPING);
                TextEdit_Buffer_InsertText(lpte, lpte->lpes, i, 0, szText, &y, &x, PE_ACTION_INDENT);
            }
            else
            {
                TextEdit_Buffer_InsertText(lpte, lpte->lpes, i, 0, szText, &y, &x, PE_ACTION_INDENT);
            }
        }

        lpte->bHorzScrollBarLocked = FALSE;
        TextEdit_View_RecalcHorzScrollBar(lpte, FALSE);

        TextEdit_Buffer_FlushUndoGroup(lpte, lpte->lpes);

        return;
    }

    if (lpte->bOvrMode)
    {
        int nLineLength;
        POINT ptCursorPos = TextEdit_View_GetCursorPos(lpte);
        LPCTSTR pszLineChars;

        ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);

        nLineLength = TextEdit_View_GetLineLength(lpte, ptCursorPos.y);
        pszLineChars = TextEdit_View_GetLineChars(lpte, ptCursorPos.y);

        if (ptCursorPos.x < nLineLength)
        {
            int nTabSize = TextEdit_View_GetTabSize(lpte);
            int nChars = nTabSize - TextEdit_View_CalculateActualOffset(lpte, ptCursorPos.y, ptCursorPos.x) % nTabSize;
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
            ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);

            TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
            TextEdit_View_SetAnchor(lpte, ptCursorPos);
            TextEdit_View_SetCursorPos(lpte, ptCursorPos);
            TextEdit_View_EnsureVisible(lpte, ptCursorPos);
            
            return;
        }
    }

    TextEdit_Buffer_BeginUndoGroup(lpte->lpes, FALSE);

    TextEdit_View_DeleteCurrentSelection(lpte);

    if (lpte->dwFlags & SRCOPT_INSERTTABS)
    {
        int iSpaceStart, nSpaces = 0;
        int nTabs = 0, nTabSize = TextEdit_View_GetTabSize(lpte), nTabOffset;
        LPCTSTR pszSpaces, pszSpacesBase;
        int nActualX;
        ptCursorPos = TextEdit_View_GetCursorPos(lpte);
        ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);

        pszSpacesBase   = TextEdit_View_GetLineChars(lpte, ptCursorPos.y);
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
        nActualX = TextEdit_View_CalculateActualOffset(lpte, ptCursorPos.y, ptCursorPos.x);
        nTabOffset = (nTabSize - (nActualX % nTabSize));
        if (nTabOffset == nTabSize)
            nTabOffset = 0;
        nTabs = (int)ceil((double)((double)(nSpaces + nTabOffset) / (double)nTabSize));
        if ((nTabOffset == 0) || (nSpaces == 0 && nTabs == 0))
            nTabs += 1;

        memset(szText, _T('\t'), nTabs * sizeof(TCHAR));
        szText[nTabs] = _T('\0');
        TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x - nSpaces, ptCursorPos.y, ptCursorPos.x, PE_ACTION_TYPING);
        ptCursorPos = TextEdit_View_GetCursorPos(lpte);
        ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
        TextEdit_Buffer_InsertText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x, szText, &y, &x, PE_ACTION_TYPING);
    }
    else
    {
        ptCursorPos = TextEdit_View_GetCursorPos(lpte);
        ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);

        TextEdit_Buffer_InsertText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x, szText, &y, &x, PE_ACTION_TYPING);
    }

    ptCursorPos.x = x;
    ptCursorPos.y = y;
    ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
    TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
    TextEdit_View_SetAnchor(lpte, ptCursorPos);
    TextEdit_View_SetCursorPos(lpte, ptCursorPos);
    TextEdit_View_EnsureVisible(lpte, ptCursorPos);

    TextEdit_Buffer_FlushUndoGroup(lpte, lpte->lpes);
}

void TextEdit_View_Untab(LPTEXTEDITVIEW lpte)
{
    BOOL bTabify = FALSE;
    POINT ptSelStart, ptSelEnd;
    
    if (!TextEdit_View_QueryEditable(lpte) || lpte->lpes == NULL)
        return;

    if (TextEdit_View_IsSelection(lpte))
    {
        TextEdit_View_GetSelection(lpte, &ptSelStart, &ptSelEnd);
        bTabify = (ptSelStart.y != ptSelEnd.y);
    }

    if (bTabify)
    {
        POINT ptSelStart, ptSelEnd;
        int nStartLine;
        int nEndLine;
        int i;

        TextEdit_Buffer_BeginUndoGroup(lpte->lpes, FALSE);

        TextEdit_View_GetSelection(lpte, &ptSelStart, &ptSelEnd);

        nStartLine = ptSelStart.y;
        nEndLine = ptSelEnd.y;
        ptSelStart.x = 0;

        if (ptSelEnd.x > 0)
        {
            if (ptSelEnd.y == TextEdit_View_GetLineCount(lpte) - 1)
            {
                ptSelEnd.x = TextEdit_View_GetLineLength(lpte, ptSelEnd.y);
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
        
        TextEdit_View_SetSelection(lpte, ptSelStart, ptSelEnd);
        TextEdit_View_SetCursorPos(lpte, ptSelEnd);
        TextEdit_View_EnsureVisible(lpte, ptSelEnd);

        //  Shift selection to left
        lpte->bHorzScrollBarLocked = TRUE;

        for (i = nStartLine; i <= nEndLine; i++)
        {
            int nLength = TextEdit_View_GetLineLength(lpte, i);

            if (nLength > 0)
            {
                LPCTSTR pszChars = TextEdit_View_GetLineChars(lpte, i);
                int nPos = 0, nOffset = 0;

                while (nPos < nLength)
                {
                    if (pszChars[nPos] == _T(' '))
                    {
                        nPos++;
                        if (++nOffset >= TextEdit_View_GetTabSize(lpte))
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
                    TextEdit_Buffer_DeleteText(lpte, lpte->lpes, i, 0, i, nPos, PE_ACTION_INDENT);
            }
        }

        lpte->bHorzScrollBarLocked = FALSE;
        TextEdit_View_RecalcHorzScrollBar(lpte, FALSE);

        TextEdit_Buffer_FlushUndoGroup(lpte, lpte->lpes);
    }
    else
    {
        POINT ptCursorPos = TextEdit_View_GetCursorPos(lpte);

        ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
        
        if (ptCursorPos.x > 0)
        {
            int nTabSize = TextEdit_View_GetTabSize(lpte);
            int nOffset = TextEdit_View_CalculateActualOffset(lpte, ptCursorPos.y, ptCursorPos.x);
            int nNewOffset = ((nOffset / nTabSize) * nTabSize);
            LPCTSTR pszChars;
            int nCurrentOffset;
            int i;

            if (nOffset == nNewOffset && nNewOffset > 0)
                nNewOffset -= nTabSize;
            
            ASSERT(nNewOffset >= 0);

            pszChars = TextEdit_View_GetLineChars(lpte, ptCursorPos.y);
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
            ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
            TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
            TextEdit_View_SetAnchor(lpte, ptCursorPos);
            TextEdit_View_SetCursorPos(lpte, ptCursorPos);
            TextEdit_View_EnsureVisible(lpte, ptCursorPos);
        }
    }
}

void TextEdit_View_Undo(LPTEXTEDITVIEW lpte)
{
    if (lpte->lpes != NULL && TextEdit_Buffer_CanUndo(lpte->lpes))
    {
        POINT ptCursorPos;

        if (TextEdit_Buffer_Undo(lpte, lpte->lpes, &ptCursorPos))
        {
            ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
            TextEdit_View_SetAnchor(lpte, ptCursorPos);
            TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
            TextEdit_View_SetCursorPos(lpte, ptCursorPos);
            TextEdit_View_EnsureVisible(lpte, ptCursorPos);
        }
    }

    SendMessage(GetParent(lpte->hwnd), WM_COMMAND, MAKEWPARAM(GetWindowLong(lpte->hwnd, GWL_ID), EN_CHANGE), (LPARAM)lpte->hwnd);
}

void TextEdit_View_Redo(LPTEXTEDITVIEW lpte)
{
    if (lpte->lpes != NULL && TextEdit_Buffer_CanRedo(lpte->lpes))
    {
        POINT ptCursorPos;

        if (TextEdit_Buffer_Redo(lpte, lpte->lpes, &ptCursorPos))
        {
            ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
            TextEdit_View_SetAnchor(lpte, ptCursorPos);
            TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
            TextEdit_View_SetCursorPos(lpte, ptCursorPos);
            TextEdit_View_EnsureVisible(lpte, ptCursorPos);
        }
    }

    SendMessage(GetParent(lpte->hwnd), WM_COMMAND, MAKEWPARAM(GetWindowLong(lpte->hwnd, GWL_ID), EN_CHANGE), (LPARAM)lpte->hwnd);
}

void TextEdit_View_OnDelete(LPTEXTEDITVIEW lpte)
{
    POINT ptSelStart, ptSelEnd, ptCursorPos;
    BOOL fControl   = IsCtrlDown();
    BOOL fShift     = IsShiftDown();

    if (!TextEdit_View_QueryEditable(lpte) || lpte->lpes == NULL)
        return;

    if (fControl && fShift)
    {
        int nLineCount = TextEdit_View_GetLineCount(lpte);
        ptCursorPos = TextEdit_View_GetCursorPos(lpte);

        TextEdit_Buffer_BeginUndoGroup(lpte->lpes, FALSE);

        if ((ptCursorPos.y + 1) == nLineCount && nLineCount == 1)
            TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptCursorPos.y, 0, ptCursorPos.y, TextEdit_View_GetLineLength(lpte, ptCursorPos.y), PE_ACTION_DELETE);
        else if ((ptCursorPos.y + 1) == nLineCount)
            TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptCursorPos.y - 1, TextEdit_View_GetLineLength(lpte, ptCursorPos.y - 1), ptCursorPos.y, TextEdit_View_GetLineLength(lpte, ptCursorPos.y), PE_ACTION_DELETE);
        else
            TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptCursorPos.y, 0, ptCursorPos.y + 1, 0, PE_ACTION_DELETE);

        if ((ptCursorPos.y + 1) == nLineCount && nLineCount != 1)
            ptCursorPos.y--;

        ptCursorPos.x = 0;

        ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
        TextEdit_View_SetAnchor(lpte, ptCursorPos);
        TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
        TextEdit_View_SetCursorPos(lpte, ptCursorPos);
        TextEdit_View_EnsureVisible(lpte, ptCursorPos);

        TextEdit_Buffer_FlushUndoGroup(lpte, lpte->lpes);
    }
    else if (fControl)
    {
        if (!TextEdit_View_IsSelection(lpte))
            TextEdit_Move_MoveWordLeft(lpte, TRUE);

        if (TextEdit_View_IsSelection(lpte))
        {
            TextEdit_View_GetSelection(lpte, &ptSelStart, &ptSelEnd);
            
            TextEdit_Buffer_BeginUndoGroup(lpte->lpes, FALSE);
            TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, PE_ACTION_DELETE);

            ASSERT_VALIDTEXTPOS(lpte, ptSelStart);
            TextEdit_View_SetAnchor(lpte, ptSelStart);
            TextEdit_View_SetSelection(lpte, ptSelStart, ptSelStart);
            TextEdit_View_SetCursorPos(lpte, ptSelStart);
            TextEdit_View_EnsureVisible(lpte, ptSelStart);

            TextEdit_Buffer_FlushUndoGroup(lpte, lpte->lpes);
        }
    }
    else
    {
        TextEdit_View_GetSelection(lpte, &ptSelStart, &ptSelEnd);

        if (Point_Equal(&ptSelStart, &ptSelEnd))
        {
            if (ptSelEnd.x == TextEdit_View_GetLineLength(lpte, ptSelEnd.y))
            {
                if (ptSelEnd.y == TextEdit_View_GetLineCount(lpte) - 1)
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
        ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
        TextEdit_View_SetAnchor(lpte, ptCursorPos);
        TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
        TextEdit_View_SetCursorPos(lpte, ptCursorPos);
        TextEdit_View_EnsureVisible(lpte, ptCursorPos);

        TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, PE_ACTION_DELETE);
    }
}

void TextEdit_View_SetTextType(LPTEXTEDITVIEW lpte, LPCTSTR pszExt)
{
    int nEoln;

    lpte->lpsd = TextEdit_Internal_GetSyntaxDefintion(pszExt);

    TextEdit_View_SetTabSize(lpte, lpte->lpsd->dwTabSize);
    TextEdit_View_SetViewTabs(lpte, (lpte->lpsd->dwFlags & SRCOPT_SHOWTABS));
    TextEdit_View_SetSelectionMargin(lpte, (lpte->lpsd->dwFlags & SRCOPT_SELMARGIN));

    if (lpte->lpsd->dwFlags & SRCOPT_EOLNDOS)
        nEoln = CRLF_STYLE_DOS;
    else if (lpte->lpsd->dwFlags & SRCOPT_EOLNUNIX)
        nEoln = CRLF_STYLE_UNIX;
    else if (lpte->lpsd->dwFlags & SRCOPT_EOLNMAC)
        nEoln = CRLF_STYLE_MAC;
    else /* eoln auto */
        nEoln = CRLF_STYLE_DOS; //CRLF_STYLE_AUTOMATIC;

    TextEdit_Buffer_SetCRLFMode(lpte->lpes, nEoln);
    TextEdit_View_SetFlags(lpte, lpte->lpsd->dwFlags);
}

void TextEdit_View_SetFlags(LPTEXTEDITVIEW lpte, DWORD dwFlags)
{
    if (lpte->dwFlags != dwFlags)
    {
        lpte->dwFlags = dwFlags;

        if (IsWindow(lpte->hwnd))
            InvalidateRect(lpte->hwnd, NULL, FALSE);
    }
}

void TextEdit_View_OnMouseWheel(LPTEXTEDITVIEW lpte, UINT nFlags, short zDelta, POINT pt)
{
    int nLineCount  = TextEdit_View_GetLineCount(lpte);
    int nNewTopLine = lpte->iTopLine - zDelta / 40;

    if (nNewTopLine < 0)
    nNewTopLine = 0;
    if (nNewTopLine >= nLineCount)
        nNewTopLine = nLineCount - 1;
    
    if (lpte->iTopLine != nNewTopLine)
    {
        int nScrollLines = lpte->iTopLine - nNewTopLine;

        lpte->iTopLine = nNewTopLine;

        ScrollWindow(lpte->hwnd, 0, nScrollLines * TextEdit_View_GetLineHeight(lpte), NULL, NULL);
        UpdateWindow(lpte->hwnd);
    }

    TextEdit_View_RecalcVertScrollBar(lpte, TRUE);
}


DWORD TextEdit_View_GetFlags(LPTEXTEDITVIEW lpte)
{
    return (lpte->dwFlags);
}

void TextEdit_View_ReplaceSelection(LPTEXTEDITVIEW lpte, LPCTSTR pszText)
{
    int x, y;
    POINT ptCursorPos;

    if (!TextEdit_View_QueryEditable(lpte) || lpte->lpes == NULL)
        return;

    TextEdit_Buffer_BeginUndoGroup(lpte->lpes, FALSE);

    TextEdit_View_DeleteCurrentSelection(lpte);

    ptCursorPos = TextEdit_View_GetCursorPos(lpte);

    ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);

    TextEdit_Buffer_InsertText(lpte, lpte->lpes, ptCursorPos.y, ptCursorPos.x, pszText, &y, &x, PE_ACTION_PASTE);
    ptCursorPos.x = x;
    ptCursorPos.y = y;

    ASSERT_VALIDTEXTPOS(lpte, ptCursorPos);
    TextEdit_View_SetAnchor(lpte, ptCursorPos);
    TextEdit_View_SetSelection(lpte, ptCursorPos, ptCursorPos);
    TextEdit_View_SetCursorPos(lpte, ptCursorPos);
    TextEdit_View_EnsureVisible(lpte, ptCursorPos);

    TextEdit_Buffer_FlushUndoGroup(lpte, lpte->lpes);
}

/*
    if (TextEdit_Buffer_GetLastActionDescription(lpte->lpes) == PE_ACTION_AUTOINDENT)
    {
        POINT ptLastAction = TextEdit_Buffer_GetLastChangePos(lpte->lpes);

        if (ptLastAction.y != lpte->ptCursorPos.y)
        {
            LPCTSTR pszLine = TextEdit_View_GetLineChars(lpte, ptLastAction.y);
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
                TextEdit_Buffer_DeleteText(lpte, lpte->lpes, ptLastAction.y, 0, ptLastAction.y, TextEdit_View_GetLineLength(lpte, ptLastAction.y), PE_ACTION_DELETE);
        }
    }
*/

HCURSOR TextEdit_View_SetMarginCursor(LPTEXTEDITVIEW lpte, HCURSOR hCursor)
{
    HCURSOR hOldCursor = lpte->hCursor;

    lpte->hCursor = hCursor;

    return (hOldCursor);
}

HCURSOR TextEdit_View_GetMarginCursor(LPTEXTEDITVIEW lpte)
{
    return (lpte->hCursor);
}

BOOL TextEdit_View_GotoLine(LPTEXTEDITVIEW lpte, int nLine, BOOL bExtendSelection)
{
    int nLineCount = TextEdit_View_GetLineCount(lpte);
    POINT ptSelStart, ptSelEnd;

    if (nLine <= 0)
        nLine = 1;

    if (--nLine < nLineCount)
    {
        if (bExtendSelection)
        {
            TextEdit_View_GetSelection(lpte, &ptSelStart, &ptSelEnd);
            ptSelEnd.y = nLine;
            ptSelEnd.x = 0;
            TextEdit_View_SetSelection(lpte, ptSelStart, ptSelEnd);
        }
        else
        {
            ptSelStart.y = ptSelEnd.y = nLine;
            ptSelStart.x = ptSelEnd.x = 0;
            TextEdit_View_SetSelection(lpte, ptSelStart, ptSelEnd);
        }

        return (TRUE);
    }

    return (FALSE);
}

/*
DROPEFFECT TextEdit_View_OnDragEnter(HWND hwnd, IDataObject *pDataObject, DWORD dwKeyState, CPoint point)
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

void CCrystalTEXTEDITVIEW::
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

BOOL CCrystalTEXTEDITVIEW::
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
  m_pTextBuffer->InsertText (this, ptDropPos.y, ptDropPos.x, pszText, y, x, PE_ACTION_DRAGDROP);  //   [JRT]

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
