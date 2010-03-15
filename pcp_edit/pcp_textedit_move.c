/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_move.c
 * Created    : not known (before 01/24/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:39:42
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* windows */
#include <commctrl.h>

/* pcp_edit */
#include "pcp_textedit_view.h"

/* pcp_generic */
#include <pcp_dpa.h>
#include <pcp_point.h>
#include <pcp_mem.h>

#define PTE_TIMER_DRAGSEL   1001

void TextEdit_Move_MoveLeft(LPTEXTEDITVIEW lpte, BOOL bSelect)
{
    TextEdit_View_PrepareSelBounds(lpte);

    if (!Point_Equal(&lpte->ptDrawSelStart, &lpte->ptDrawSelEnd) && !bSelect)
    {
        lpte->ptCursorPos = lpte->ptDrawSelStart;
    }
    else
    {
        if (lpte->ptCursorPos.x == 0)
        {
            if (lpte->ptCursorPos.y > 0)
            {
                lpte->ptCursorPos.y--;
                lpte->ptCursorPos.x = TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y);
            }
        }
        else
        {
            lpte->ptCursorPos.x--;
        }
    }

    lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);
    
    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
    TextEdit_View_UpdateCaret(lpte);

    if (!bSelect)
        lpte->ptAnchor = lpte->ptCursorPos;

    TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
}

void TextEdit_Move_MoveRight(LPTEXTEDITVIEW lpte, BOOL bSelect)
{
    TextEdit_View_PrepareSelBounds(lpte);

    if (!Point_Equal(&lpte->ptDrawSelStart, &lpte->ptDrawSelEnd) && !bSelect)
    {
        lpte->ptCursorPos = lpte->ptDrawSelEnd;
    }
    else
    {
        if (lpte->ptCursorPos.x == TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y))
        {
            if (lpte->ptCursorPos.y < TextEdit_View_GetLineCount(lpte) - 1)
            {
                lpte->ptCursorPos.y++;
                lpte->ptCursorPos.x = 0;
            }
        }
        else
        {
            lpte->ptCursorPos.x++;
        }
    }
    
    lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);
    
    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
    TextEdit_View_UpdateCaret(lpte);

    if (!bSelect)
        lpte->ptAnchor = lpte->ptCursorPos;

    TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
}

void TextEdit_Move_MoveWordLeft(LPTEXTEDITVIEW lpte, BOOL bSelect)
{
    LPCTSTR pszChars;
    int nPos;

    TextEdit_View_PrepareSelBounds(lpte);

    if (!Point_Equal(&lpte->ptDrawSelStart, &lpte->ptDrawSelEnd) && !bSelect)
    {
        TextEdit_Move_MoveLeft(lpte, bSelect);
    
        return;
    }

    if (lpte->ptCursorPos.x == 0)
    {
        if (lpte->ptCursorPos.y == 0)
            return;

        lpte->ptCursorPos.y--;
        lpte->ptCursorPos.x = TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y);
    }

    pszChars = TextEdit_View_GetLineChars(lpte, lpte->ptCursorPos.y);
    nPos = lpte->ptCursorPos.x;
    
    while (nPos > 0 && isspace(pszChars[nPos - 1]))
        nPos--;

    if (nPos > 0)
    {
        nPos--;
    
        if (isalnum(pszChars[nPos]) || pszChars[nPos] == _T('_'))
        {
            while (nPos > 0 && (isalnum(pszChars[nPos - 1]) || pszChars[nPos - 1] == _T('_')))
                nPos--;
        }
        else
        {
            while (nPos > 0 && !isalnum(pszChars[nPos - 1]) && pszChars[nPos - 1] != _T('_') && !isspace(pszChars[nPos - 1]))
                nPos--;
        }
    }

    lpte->ptCursorPos.x = nPos;
    lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);
    
    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
    TextEdit_View_UpdateCaret(lpte);

    if (!bSelect)
        lpte->ptAnchor = lpte->ptCursorPos;

    TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
}

void TextEdit_Move_MoveWordRight(LPTEXTEDITVIEW lpte, BOOL bSelect)
{
    int nLength, nPos;
    LPCTSTR pszChars;

    TextEdit_View_PrepareSelBounds(lpte);

    if (!Point_Equal(&lpte->ptDrawSelStart, &lpte->ptDrawSelEnd) && !bSelect)
    {
        TextEdit_Move_MoveRight(lpte, bSelect);
    
        return;
    }

    if (lpte->ptCursorPos.x == TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y))
    {
        if (lpte->ptCursorPos.y == TextEdit_View_GetLineCount(lpte) - 1)
            return;

        lpte->ptCursorPos.y++;
        lpte->ptCursorPos.x = 0;
    }

    nLength = TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y);

    if (lpte->ptCursorPos.x == nLength)
    {
        TextEdit_Move_MoveRight(lpte, bSelect);
    
        return;
    }

    pszChars = TextEdit_View_GetLineChars(lpte, lpte->ptCursorPos.y);
    nPos = lpte->ptCursorPos.x;

    if (isalnum(pszChars[nPos]) || pszChars[nPos] == _T('_'))
    {
        while (nPos < nLength && isalnum(pszChars[nPos]) || pszChars[nPos] == _T('_'))
            nPos++;
    }
    else
    {
        while (nPos < nLength && !isalnum(pszChars[nPos]) && pszChars[nPos] != _T('_') && !isspace(pszChars[nPos]))
            nPos++;
    }

    while (nPos < nLength && isspace(pszChars[nPos]))
        nPos++;

    lpte->ptCursorPos.x = nPos;
    lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);

    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
    TextEdit_View_UpdateCaret(lpte);

    if (!bSelect)
        lpte->ptAnchor = lpte->ptCursorPos;

    TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
}

void TextEdit_Move_MoveUp(LPTEXTEDITVIEW lpte, BOOL bSelect)
{
    TextEdit_View_PrepareSelBounds(lpte);

    if (!Point_Equal(&lpte->ptDrawSelStart, &lpte->ptDrawSelEnd) && !bSelect)
        lpte->ptCursorPos = lpte->ptDrawSelStart;

    if (lpte->ptCursorPos.y > 0)
    {
        if (lpte->iIdealCharPos == -1)
            lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);
        
        lpte->ptCursorPos.y--;
        lpte->ptCursorPos.x = TextEdit_View_ApproxActualOffset(lpte, lpte->ptCursorPos.y, lpte->iIdealCharPos);
        
        if (lpte->ptCursorPos.x > TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y))
            lpte->ptCursorPos.x = TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y);
    }

    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
    TextEdit_View_UpdateCaret(lpte);

    if (!bSelect)
        lpte->ptAnchor = lpte->ptCursorPos;

    TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
}

void TextEdit_Move_MoveDown(LPTEXTEDITVIEW lpte, BOOL bSelect)
{
    TextEdit_View_PrepareSelBounds(lpte);

    if (!Point_Equal(&lpte->ptDrawSelStart, &lpte->ptDrawSelEnd) && !bSelect)
        lpte->ptCursorPos = lpte->ptDrawSelEnd;

    if (lpte->ptCursorPos.y < TextEdit_View_GetLineCount(lpte) - 1)
    {
        if (lpte->iIdealCharPos == -1)
            lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);

        lpte->ptCursorPos.y++;
        lpte->ptCursorPos.x = TextEdit_View_ApproxActualOffset(lpte, lpte->ptCursorPos.y, lpte->iIdealCharPos);

        if (lpte->ptCursorPos.x > TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y))
            lpte->ptCursorPos.x = TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y);
    }
    else if (lpte->ptCursorPos.y == TextEdit_View_GetLineCount(lpte) - 1)
    {
        if (lpte->iIdealCharPos == -1)
            lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);

        lpte->ptCursorPos.x = TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y);
    }

    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
    TextEdit_View_UpdateCaret(lpte);

    if (!bSelect)
        lpte->ptAnchor = lpte->ptCursorPos;

    TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
}

void TextEdit_Move_MoveHome(LPTEXTEDITVIEW lpte, BOOL bSelect)
{
    int nLength = TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y);
    LPCTSTR pszChars = TextEdit_View_GetLineChars(lpte, lpte->ptCursorPos.y);
    int nHomePos = 0;
    
    while (nHomePos < nLength && isspace(pszChars[nHomePos]))
        nHomePos++;

    if (nHomePos == nLength || lpte->ptCursorPos.x == nHomePos)
        lpte->ptCursorPos.x = 0;
    else
        lpte->ptCursorPos.x = nHomePos;

    lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);

    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
    TextEdit_View_UpdateCaret(lpte);

    if (!bSelect)
        lpte->ptAnchor = lpte->ptCursorPos;

    TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
}

void TextEdit_Move_MoveEnd(LPTEXTEDITVIEW lpte, BOOL bSelect)
{
    lpte->ptCursorPos.x = TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y);
    lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);

    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
    TextEdit_View_UpdateCaret(lpte);

    if (!bSelect)
        lpte->ptAnchor = lpte->ptCursorPos;

    TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
}

void TextEdit_Move_MovePgUp(LPTEXTEDITVIEW lpte, BOOL bSelect)
{
    int nNewTopLine = lpte->iTopLine - TextEdit_View_GetScreenLines(lpte) + 1;
    
    if (nNewTopLine < 0)
        nNewTopLine = 0;

    if (lpte->iTopLine != nNewTopLine)
    {
        TextEdit_View_ScrollToLine(lpte, nNewTopLine, FALSE, TRUE);
//      TextEdit_View_UpdateSiblingScrollPos(lpte, TRUE);
    }

    lpte->ptCursorPos.y -= TextEdit_View_GetScreenLines(lpte) - 1;

    if (lpte->ptCursorPos.y < 0)
        lpte->ptCursorPos.y = 0;

    if (lpte->ptCursorPos.x > TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y))
        lpte->ptCursorPos.x = TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y);

    lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);

    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);   //todo: no vertical scroll
    TextEdit_View_UpdateCaret(lpte);

    if (!bSelect)
        lpte->ptAnchor = lpte->ptCursorPos;

    TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
}

void TextEdit_Move_MovePgDn(LPTEXTEDITVIEW lpte, BOOL bSelect)
{
    int nNewTopLine = lpte->iTopLine + TextEdit_View_GetScreenLines(lpte) - 1;
    
    if (nNewTopLine >= TextEdit_View_GetLineCount(lpte))
        nNewTopLine = TextEdit_View_GetLineCount(lpte) - 1;

    if (lpte->iTopLine != nNewTopLine)
    {
        TextEdit_View_ScrollToLine(lpte, nNewTopLine, FALSE, TRUE);
//      TextEdit_View_UpdateSiblingScrollPos(lpte, TRUE);
    }

    lpte->ptCursorPos.y += TextEdit_View_GetScreenLines(lpte) - 1;

    if (lpte->ptCursorPos.y >= TextEdit_View_GetLineCount(lpte))
        lpte->ptCursorPos.y = TextEdit_View_GetLineCount(lpte) - 1;

    if (lpte->ptCursorPos.x > TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y))
        lpte->ptCursorPos.x = TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y);

    lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);

    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);   //todo: no vertical scroll
    TextEdit_View_UpdateCaret(lpte);

    if (!bSelect)
        lpte->ptAnchor = lpte->ptCursorPos;

    TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
}

void TextEdit_Move_MoveCtrlHome(LPTEXTEDITVIEW lpte, BOOL bSelect)
{
    lpte->ptCursorPos.x = 0;
    lpte->ptCursorPos.y = 0;
    lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);

    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
    TextEdit_View_UpdateCaret(lpte);

    if (!bSelect)
        lpte->ptAnchor = lpte->ptCursorPos;

    TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
}

void TextEdit_Move_MoveCtrlEnd(LPTEXTEDITVIEW lpte, BOOL bSelect)
{
    lpte->ptCursorPos.y = TextEdit_View_GetLineCount(lpte) - 1;
    lpte->ptCursorPos.x = TextEdit_View_GetLineLength(lpte, lpte->ptCursorPos.y);
    lpte->iIdealCharPos = TextEdit_View_CalculateActualOffset(lpte, lpte->ptCursorPos.y, lpte->ptCursorPos.x);

    TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
    TextEdit_View_UpdateCaret(lpte);

    if (!bSelect)
        lpte->ptAnchor = lpte->ptCursorPos;

    TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
}

void TextEdit_Move_ScrollUp(LPTEXTEDITVIEW lpte)
{
    if (lpte->iTopLine > 0)
    {
        TextEdit_View_ScrollToLine(lpte, lpte->iTopLine - 1, FALSE, TRUE);
//      TextEdit_View_UpdateSiblingScrollPos(lpte, TRUE);
    }
}

void TextEdit_Move_ScrollDown(LPTEXTEDITVIEW lpte)
{
    if (lpte->iTopLine < TextEdit_View_GetLineCount(lpte) - 1)
    {
        TextEdit_View_ScrollToLine(lpte, lpte->iTopLine + 1, FALSE, TRUE);
//      TextEdit_View_UpdateSiblingScrollPos(lpte, TRUE);
    }
}

void TextEdit_Move_ScrollLeft(LPTEXTEDITVIEW lpte)
{
    if (lpte->iOffsetChar > 0)
    {
        TextEdit_View_ScrollToChar(lpte, lpte->iOffsetChar - 1, FALSE, TRUE);
        TextEdit_View_UpdateCaret(lpte);
    }
}

void TextEdit_Move_ScrollRight(LPTEXTEDITVIEW lpte)
{
    if (lpte->iOffsetChar < TextEdit_View_GetMaxLineLength(lpte) - 1)
    {
        TextEdit_View_ScrollToChar(lpte, lpte->iOffsetChar + 1, FALSE, TRUE);
        TextEdit_View_UpdateCaret(lpte);
    }
}

POINT TextEdit_Move_WordToRight(LPTEXTEDITVIEW lpte, POINT pt)
{
    int nLength = TextEdit_View_GetLineLength(lpte, pt.y);
    LPCTSTR pszChars = TextEdit_View_GetLineChars(lpte, pt.y);

    ASSERT_VALIDTEXTPOS(lpte, pt);

    while (pt.x < nLength)
    {
        if (!isalnum(pszChars[pt.x]) && pszChars[pt.x] != _T('_'))
            break;

        pt.x++;
    }

    ASSERT_VALIDTEXTPOS(lpte, pt);

    return (pt);
}

POINT TextEdit_Move_WordToLeft(LPTEXTEDITVIEW lpte, POINT pt)
{
    LPCTSTR pszChars = TextEdit_View_GetLineChars(lpte, pt.y);

    ASSERT_VALIDTEXTPOS(lpte, pt);

    while (pt.x > 0)
    {
        if (!isalnum(pszChars[pt.x - 1]) && pszChars[pt.x - 1] != _T('_'))
            break;

        pt.x--;
    }

    ASSERT_VALIDTEXTPOS(lpte, pt);

    return (pt);
}

void TextEdit_View_SelectAll(LPTEXTEDITVIEW lpte)
{
    int nLineCount = TextEdit_View_GetLineCount(lpte);
    POINT pt = { 0, 0 };

    lpte->ptCursorPos.x = TextEdit_View_GetLineLength(lpte, nLineCount - 1);
    lpte->ptCursorPos.y = nLineCount - 1;

    TextEdit_View_SetSelection(lpte, pt, lpte->ptCursorPos);
    TextEdit_View_UpdateCaret(lpte);
}

void TextEdit_View_OnLButtonDown(LPTEXTEDITVIEW lpte, UINT nFlags, POINT point)
{
    BOOL bShift = GetKeyState(VK_SHIFT) & 0x8000;
    BOOL bControl = GetKeyState(VK_CONTROL) & 0x8000;

    DefWindowProc(lpte->hwnd, WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));

    if (point.x < TextEdit_View_GetMarginWidth(lpte))
    {
        TextEdit_View_AdjustTextPoint(lpte, &point);

        if (bControl)
        {
            TextEdit_View_SelectAll(lpte);
        }
        else
        {
            POINT ptStart, ptEnd;

            lpte->ptCursorPos = TextEdit_View_ClientToText(lpte, point);
            lpte->ptCursorPos.x = 0;                //  Force beginning of the line

            if (!bShift)
                lpte->ptAnchor = lpte->ptCursorPos;

            ptStart = lpte->ptAnchor;

            if (ptStart.y == TextEdit_View_GetLineCount(lpte) - 1)
            {
                ptStart.x = TextEdit_View_GetLineLength(lpte, ptStart.y);
            }
            else
            {
                ptStart.y++;
                ptStart.x = 0;
            }

            ptEnd = lpte->ptCursorPos;
            ptEnd.x = 0;

            lpte->ptCursorPos = ptEnd;

            TextEdit_View_UpdateCaret(lpte);
            TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
            TextEdit_View_SetSelection(lpte, ptStart, ptEnd);

            SetCapture(lpte->hwnd);

            lpte->uDragSelTimer = SetTimer(lpte->hwnd, PTE_TIMER_DRAGSEL, 100, NULL);

            ASSERT(lpte->uDragSelTimer != 0);

            lpte->bWordSelection = FALSE;
            lpte->bLineSelection = TRUE;
            lpte->bDragSelection = TRUE;
        }
    }
    else
    {
        POINT ptText = TextEdit_View_ClientToText(lpte, point);

        TextEdit_View_PrepareSelBounds(lpte);
        //  [JRT]:  Support For Disabling Drag and Drop...
        if ((TextEdit_View_IsInsideSelBlock(lpte, ptText)) &&               // If Inside Selection Area
                (!lpte->bDisableDragAndDrop))               // And D&D Not Disabled
        {
            lpte->bPreparingToDrag = TRUE;
        }
        else
        {
            POINT ptStart, ptEnd;

            TextEdit_View_AdjustTextPoint(lpte, &point);
            lpte->ptCursorPos = TextEdit_View_ClientToText(lpte, point);
            
            if (!bShift)
                lpte->ptAnchor = lpte->ptCursorPos;

            if (bControl)
            {
                if (lpte->ptCursorPos.y < lpte->ptAnchor.y ||
                    lpte->ptCursorPos.y == lpte->ptAnchor.y && lpte->ptCursorPos.x < lpte->ptAnchor.x)
                {
                    ptStart = TextEdit_Move_WordToLeft(lpte, lpte->ptCursorPos);
                    ptEnd = TextEdit_Move_WordToRight(lpte, lpte->ptAnchor);
                }
                else
                {
                    ptStart = TextEdit_Move_WordToLeft(lpte, lpte->ptAnchor);
                    ptEnd = TextEdit_Move_WordToRight(lpte, lpte->ptCursorPos);
                }
            }
            else
            {
                ptStart = lpte->ptAnchor;
                ptEnd = lpte->ptCursorPos;
            }

            lpte->ptCursorPos = ptEnd;
            
            TextEdit_View_UpdateCaret(lpte);
            TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
            TextEdit_View_SetSelection(lpte, ptStart, ptEnd);

            SetCapture(lpte->hwnd);

            lpte->uDragSelTimer = SetTimer(lpte->hwnd, PTE_TIMER_DRAGSEL, 100, NULL);

            ASSERT(lpte->uDragSelTimer != 0);

            lpte->bWordSelection = bControl;
            lpte->bLineSelection = FALSE;
            lpte->bDragSelection = TRUE;
        }
    }

    ASSERT_VALIDTEXTPOS(lpte, lpte->ptCursorPos);
}

void TextEdit_View_OnMouseMove(LPTEXTEDITVIEW lpte, UINT nFlags, POINT point)
{
    DefWindowProc(lpte->hwnd, WM_MOUSEMOVE, nFlags, MAKELPARAM(point.x, point.y));

    if (lpte->bDragSelection)
    {
        BOOL bOnMargin = point.x < TextEdit_View_GetMarginWidth(lpte);
        POINT ptStart, ptEnd;
        POINT ptNewCursorPos;

        TextEdit_View_AdjustTextPoint(lpte, &point);

        ptNewCursorPos = TextEdit_View_ClientToText(lpte, point);

        if (lpte->bLineSelection)
        {
            if (bOnMargin)
            {
                if (ptNewCursorPos.y < lpte->ptAnchor.y ||
                    ptNewCursorPos.y == lpte->ptAnchor.y && ptNewCursorPos.x < lpte->ptAnchor.x)
                {
                    ptEnd = lpte->ptAnchor;
            
                    if (ptEnd.y == TextEdit_View_GetLineCount(lpte) - 1)
                    {
                        ptEnd.x = TextEdit_View_GetLineLength(lpte, ptEnd.y);
                    }
                    else
                    {
                        ptEnd.y++;
                        ptEnd.x = 0;
                    }

                    ptNewCursorPos.x = 0;
                    lpte->ptCursorPos = ptNewCursorPos;
                }
                else
                {
                    ptEnd = lpte->ptAnchor;
                    ptEnd.x = 0;
                    lpte->ptCursorPos = ptNewCursorPos;
            
                    if (ptNewCursorPos.y == TextEdit_View_GetLineCount(lpte) - 1)
                    {
                        ptNewCursorPos.x = TextEdit_View_GetLineLength(lpte, ptNewCursorPos.y);
                    }
                    else
                    {
                        ptNewCursorPos.y++;
                        ptNewCursorPos.x = 0;
                    }
            
                    lpte->ptCursorPos.x = 0;
                }
            
                TextEdit_View_UpdateCaret(lpte);
                TextEdit_View_SetSelection(lpte, ptNewCursorPos, ptEnd);
            
                return;
            }

            //  Moving to normal selection mode
            SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));

            lpte->bLineSelection = lpte->bWordSelection = FALSE;
        }

        if (lpte->bWordSelection)
        {
            if (ptNewCursorPos.y < lpte->ptAnchor.y ||
                ptNewCursorPos.y == lpte->ptAnchor.y && ptNewCursorPos.x < lpte->ptAnchor.x)
            {
                ptStart = TextEdit_Move_WordToLeft(lpte, ptNewCursorPos);
                ptEnd = TextEdit_Move_WordToRight(lpte, lpte->ptAnchor);
            }
            else
            {
                ptStart = TextEdit_Move_WordToLeft(lpte, lpte->ptAnchor);
                ptEnd = TextEdit_Move_WordToRight(lpte, ptNewCursorPos);
            }
        }
        else
        {
            ptStart = lpte->ptAnchor;
            ptEnd = ptNewCursorPos;
        }

        lpte->ptCursorPos = ptEnd;

        TextEdit_View_UpdateCaret(lpte);
        TextEdit_View_SetSelection(lpte, ptStart, ptEnd);
    }

    if (lpte->bPreparingToDrag)
    {
        /*
        HGLOBAL hData = TextEdit_View_PrepareDragData(lpte);

        lpte->bPreparingToDrag = FALSE;

        if (hData != NULL)
        {
            COleDataSource ds;
            DROPEFFECT de;
            
            if (lpte->pTextBuffer != NULL)
                Edit_Buffer_BeginUndoGroup(lpte->lpes);

            ds.CacheGlobalData(CF_TEXT, hData);
            lpte->bDraggingText = TRUE;
            de = ds.DoDragDrop(GetDropEffect(lpte));

            if (de != DROPEFFECT_NONE)
                OnDropSource(de);
*/
            lpte->bDraggingText = FALSE;
/*
            if (lpte->pTextBuffer != NULL)
                Edit_Buffer_FlushUndoGroup(lpte->lpes);
                */
//      }
    }

    ASSERT_VALIDTEXTPOS(lpte, lpte->ptCursorPos);
}

void TextEdit_View_OnLButtonUp(LPTEXTEDITVIEW lpte, UINT nFlags, POINT point)
{
    DefWindowProc(lpte->hwnd, WM_LBUTTONUP, nFlags, MAKELPARAM(point.x, point.y));

    if (lpte->bDragSelection)
    {
        POINT ptNewCursorPos;
        POINT ptStart, ptEnd;

        TextEdit_View_AdjustTextPoint(lpte, &point);
        ptNewCursorPos = TextEdit_View_ClientToText(lpte, point);

        if (lpte->bLineSelection)
        {
            POINT ptEnd;

            if (ptNewCursorPos.y < lpte->ptAnchor.y ||
                ptNewCursorPos.y == lpte->ptAnchor.y && ptNewCursorPos.x < lpte->ptAnchor.x)
            {
                ptEnd = lpte->ptAnchor;

                if (ptEnd.y == TextEdit_View_GetLineCount(lpte) - 1)
                {
                    ptEnd.x = TextEdit_View_GetLineLength(lpte, ptEnd.y);
                }
                else
                {
                    ptEnd.y++;
                    ptEnd.x = 0;
                }

                ptNewCursorPos.x = 0;
                lpte->ptCursorPos = ptNewCursorPos;
            }
            else
            {
                ptEnd = lpte->ptAnchor;
                ptEnd.x = 0;

                if (ptNewCursorPos.y == TextEdit_View_GetLineCount(lpte) - 1)
                {
                    ptNewCursorPos.x = TextEdit_View_GetLineLength(lpte, ptNewCursorPos.y);
                }
                else
                {
                    ptNewCursorPos.y++;
                    ptNewCursorPos.x = 0;
                }

                lpte->ptCursorPos = ptNewCursorPos;
            }

            TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
            TextEdit_View_UpdateCaret(lpte);
            TextEdit_View_SetSelection(lpte, ptNewCursorPos, ptEnd);
        }
        else
        {
            if (lpte->bWordSelection)
            {
                if (ptNewCursorPos.y < lpte->ptAnchor.y ||
                    ptNewCursorPos.y == lpte->ptAnchor.y && ptNewCursorPos.x < lpte->ptAnchor.x)
                {
                    ptStart = TextEdit_Move_WordToLeft(lpte, ptNewCursorPos);
                    ptEnd = TextEdit_Move_WordToRight(lpte, lpte->ptAnchor);
                }
                else
                {
                    ptStart = TextEdit_Move_WordToLeft(lpte, lpte->ptAnchor);
                    ptEnd = TextEdit_Move_WordToRight(lpte, ptNewCursorPos);
                }
            }
            else
            {
                ptStart = lpte->ptAnchor;
                ptEnd = lpte->ptCursorPos;
            }

            lpte->ptCursorPos = ptEnd;

            TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
            TextEdit_View_UpdateCaret(lpte);
            TextEdit_View_SetSelection(lpte, ptStart, ptEnd);
        }

        ReleaseCapture();
        KillTimer(lpte->hwnd, lpte->uDragSelTimer);

        lpte->bDragSelection = FALSE;
    }

    if (lpte->bPreparingToDrag)
    {
        lpte->bPreparingToDrag = FALSE;

        TextEdit_View_AdjustTextPoint(lpte, &point);

        lpte->ptCursorPos = TextEdit_View_ClientToText(lpte, point);

        TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
        TextEdit_View_UpdateCaret(lpte);
        TextEdit_View_SetSelection(lpte, lpte->ptCursorPos, lpte->ptCursorPos);
    }

    ASSERT_VALIDTEXTPOS(lpte, lpte->ptCursorPos);
}

void TextEdit_View_OnTimer(LPTEXTEDITVIEW lpte, UINT nIDEvent)
{

    if (nIDEvent == PTE_TIMER_DRAGSEL)
    {
        POINT pt;
        RECT rcClient;
        BOOL bChanged = FALSE;
        int nNewTopLine, nLineCount;
        int nNewOffsetChar;
        int nMaxLineLength;
        
        ASSERT(lpte->bDragSelection);

        GetCursorPos(&pt);
        ScreenToClient(lpte->hwnd, &pt);
        GetClientRect(lpte->hwnd, &rcClient);

        //  Scroll vertically, if necessary
        nNewTopLine = lpte->iTopLine;
        nLineCount = TextEdit_View_GetLineCount(lpte);
        
        if (pt.y < rcClient.top)
        {
            nNewTopLine--;

            if (pt.y < rcClient.top - TextEdit_View_GetLineHeight(lpte))
                nNewTopLine -= 2;
        }
        else if (pt.y >= rcClient.bottom)
        {
            nNewTopLine++;

            if (pt.y >= rcClient.bottom + TextEdit_View_GetLineHeight(lpte))
                nNewTopLine += 2;
        }

        if (nNewTopLine < 0)
            nNewTopLine = 0;

        if (nNewTopLine >= nLineCount)
            nNewTopLine = nLineCount - 1;

        if (lpte->iTopLine != nNewTopLine)
        {
            TextEdit_View_ScrollToLine(lpte, nNewTopLine, FALSE, TRUE);
//          TextEdit_View_UpdateSiblingScrollPos(lpte, TRUE);
            bChanged = TRUE;
        }

        //  Scroll horizontally, if necessary
        nNewOffsetChar = lpte->iOffsetChar;
        nMaxLineLength = TextEdit_View_GetMaxLineLength(lpte);

        if (pt.x < rcClient.left)
            nNewOffsetChar--;
        else if (pt.x >= rcClient.right)
            nNewOffsetChar++;

        if (nNewOffsetChar >= nMaxLineLength)
            nNewOffsetChar = nMaxLineLength - 1;

        if (nNewOffsetChar < 0)
            nNewOffsetChar = 0;

        if (lpte->iOffsetChar != nNewOffsetChar)
        {
            TextEdit_View_ScrollToChar(lpte, nNewOffsetChar, FALSE, TRUE);
            TextEdit_View_UpdateCaret(lpte);
//          TextEdit_View_UpdateSiblingScrollPos(lpte, FALSE);

            bChanged = TRUE;
        }

        //  Fix changes
        if (bChanged)
        {
            POINT ptNewCursorPos;
            TextEdit_View_AdjustTextPoint(lpte, &pt);

            ptNewCursorPos = TextEdit_View_ClientToText(lpte, pt);

            if (!Point_Equal(&ptNewCursorPos, &lpte->ptCursorPos))
            {
                lpte->ptCursorPos = ptNewCursorPos;

                TextEdit_View_UpdateCaret(lpte);
            }

            TextEdit_View_SetSelection(lpte, lpte->ptAnchor, lpte->ptCursorPos);
        }
    }
}

void TextEdit_View_OnLButtonDblClk(LPTEXTEDITVIEW lpte, UINT nFlags, POINT point)
{
    DefWindowProc(lpte->hwnd, WM_LBUTTONDBLCLK, nFlags, MAKELPARAM(point.x, point.y));

    if (!lpte->bDragSelection)
    {
        POINT ptStart, ptEnd;

        TextEdit_View_AdjustTextPoint(lpte, &point);

        lpte->ptCursorPos = TextEdit_View_ClientToText(lpte, point);
        lpte->ptAnchor = lpte->ptCursorPos;

        if (lpte->ptCursorPos.y < lpte->ptAnchor.y ||
            lpte->ptCursorPos.y == lpte->ptAnchor.y && lpte->ptCursorPos.x < lpte->ptAnchor.x)
        {
            ptStart = TextEdit_Move_WordToLeft(lpte, lpte->ptCursorPos);
            ptEnd = TextEdit_Move_WordToRight(lpte, lpte->ptAnchor);
        }
        else
        {
            ptStart = TextEdit_Move_WordToLeft(lpte, lpte->ptAnchor);
            ptEnd = TextEdit_Move_WordToRight(lpte, lpte->ptCursorPos);
        }

        lpte->ptCursorPos = ptEnd;

        TextEdit_View_UpdateCaret(lpte);
        TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
        TextEdit_View_SetSelection(lpte, ptStart, ptEnd);

        SetCapture(lpte->hwnd);

        lpte->uDragSelTimer = SetTimer(lpte->hwnd, PTE_TIMER_DRAGSEL, 100, NULL);

        ASSERT(lpte->uDragSelTimer != 0);

        lpte->bWordSelection = TRUE;
        lpte->bLineSelection = FALSE;
        lpte->bDragSelection = TRUE;
    }
}

void TextEdit_View_OnRButtonDown(LPTEXTEDITVIEW lpte, UINT nFlags, POINT point)
{
    POINT pt = point;

    TextEdit_View_AdjustTextPoint(lpte, &pt);

    pt = TextEdit_View_ClientToText(lpte, pt);

    if (!TextEdit_View_IsInsideSelBlock(lpte, pt))
    {
        lpte->ptAnchor = lpte->ptCursorPos = pt;

        TextEdit_View_SetSelection(lpte, lpte->ptCursorPos, lpte->ptCursorPos);
        TextEdit_View_EnsureVisible(lpte, lpte->ptCursorPos);
        TextEdit_View_UpdateCaret(lpte);
    }

    DefWindowProc(lpte->hwnd, WM_RBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
}

BOOL TextEdit_View_TextInClipboard(LPTEXTEDITVIEW lpte)
{
    return (IsClipboardFormatAvailable(CF_TEXT));
}

BOOL TextEdit_View_PutToClipboard(LPTEXTEDITVIEW lpte, LPCTSTR pszText)
{
    BOOL bOK = FALSE;

    if (pszText == NULL || lstrlen(pszText) == 0)
        return (FALSE);


    if (OpenClipboard(lpte->hwnd))
    {
        HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, lstrlen(pszText) + 1);

        EmptyClipboard();

        if (hData != NULL)
        {
            LPTSTR pszData = (LPTSTR)GlobalLock(hData);

            _tcscpy(pszData, (LPTSTR)pszText);
            GlobalUnlock(hData);

            bOK = SetClipboardData(CF_TEXT, hData) != NULL;
        }

        CloseClipboard();
    }

    return (bOK);
}
