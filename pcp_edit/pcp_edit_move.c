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
#include "pcp_edit_view.h"
#include "pcp_edit_internal.h"
#include "pcp_edit_buffer.h"
#include "pcp_edit_move.h"

/* pcp_generic */
#include <pcp_dpa.h>
#include <pcp_point.h>
#include <pcp_mem.h>

#define CRYSTAL_TIMER_DRAGSEL   1001

void TextMove_MoveLeft(LPEDITVIEW lpev, BOOL bSelect)
{
    Edit_View_PrepareSelBounds(lpev);
    
    if (!Point_Equal(&lpev->ptDrawSelStart, &lpev->ptDrawSelEnd) && !bSelect)
    {
        lpev->ptCursorPos = lpev->ptDrawSelStart;
    }
    else
    {
        if (lpev->ptCursorPos.x == 0)
        {
            if (lpev->ptCursorPos.y > 0)
            {
                lpev->ptCursorPos.y--;
                lpev->ptCursorPos.x = Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y);
            }
        }
        else
        {
            lpev->ptCursorPos.x--;
        }
    }

    lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);
    
    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
    Edit_View_UpdateCaret(lpev);

    if (!bSelect)
        lpev->ptAnchor = lpev->ptCursorPos;

    Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
}

void TextMove_MoveRight(LPEDITVIEW lpev, BOOL bSelect)
{
    Edit_View_PrepareSelBounds(lpev);

    if (!Point_Equal(&lpev->ptDrawSelStart, &lpev->ptDrawSelEnd) && !bSelect)
    {
        lpev->ptCursorPos = lpev->ptDrawSelEnd;
    }
    else
    {
        if (lpev->ptCursorPos.x == Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y))
        {
            if (lpev->ptCursorPos.y < Edit_View_GetLineCount(lpev) - 1)
            {
                lpev->ptCursorPos.y++;
                lpev->ptCursorPos.x = 0;
            }
        }
        else
        {
            lpev->ptCursorPos.x++;
        }
    }
    
    lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);
    
    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
    Edit_View_UpdateCaret(lpev);

    if (!bSelect)
        lpev->ptAnchor = lpev->ptCursorPos;

    Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
}

void TextMove_MoveWordLeft(LPEDITVIEW lpev, BOOL bSelect)
{
    LPCTSTR pszChars;
    int nPos;

    Edit_View_PrepareSelBounds(lpev);

    if (!Point_Equal(&lpev->ptDrawSelStart, &lpev->ptDrawSelEnd) && !bSelect)
    {
        TextMove_MoveLeft(lpev, bSelect);
    
        return;
    }

    if (lpev->ptCursorPos.x == 0)
    {
        if (lpev->ptCursorPos.y == 0)
            return;

        lpev->ptCursorPos.y--;
        lpev->ptCursorPos.x = Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y);
    }

    pszChars = Edit_View_GetLineChars(lpev, lpev->ptCursorPos.y);
    nPos = lpev->ptCursorPos.x;
    
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

    lpev->ptCursorPos.x = nPos;
    lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);
    
    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
    Edit_View_UpdateCaret(lpev);

    if (!bSelect)
        lpev->ptAnchor = lpev->ptCursorPos;

    Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
}

void TextMove_MoveWordRight(LPEDITVIEW lpev, BOOL bSelect)
{
    int nLength, nPos;
    LPCTSTR pszChars;

    Edit_View_PrepareSelBounds(lpev);

    if (!Point_Equal(&lpev->ptDrawSelStart, &lpev->ptDrawSelEnd) && !bSelect)
    {
        TextMove_MoveRight(lpev, bSelect);
    
        return;
    }

    if (lpev->ptCursorPos.x == Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y))
    {
        if (lpev->ptCursorPos.y == Edit_View_GetLineCount(lpev) - 1)
            return;

        lpev->ptCursorPos.y++;
        lpev->ptCursorPos.x = 0;
    }

    nLength = Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y);

    if (lpev->ptCursorPos.x == nLength)
    {
        TextMove_MoveRight(lpev, bSelect);
    
        return;
    }

    pszChars = Edit_View_GetLineChars(lpev, lpev->ptCursorPos.y);
    nPos = lpev->ptCursorPos.x;

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

    lpev->ptCursorPos.x = nPos;
    lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);

    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
    Edit_View_UpdateCaret(lpev);

    if (!bSelect)
        lpev->ptAnchor = lpev->ptCursorPos;

    Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
}

void TextMove_MoveUp(LPEDITVIEW lpev, BOOL bSelect)
{
    Edit_View_PrepareSelBounds(lpev);

    if (!Point_Equal(&lpev->ptDrawSelStart, &lpev->ptDrawSelEnd) && !bSelect)
        lpev->ptCursorPos = lpev->ptDrawSelStart;

    if (lpev->ptCursorPos.y > 0)
    {
        if (lpev->iIdealCharPos == -1)
            lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);
        
        lpev->ptCursorPos.y--;
        lpev->ptCursorPos.x = Edit_View_ApproxActualOffset(lpev, lpev->ptCursorPos.y, lpev->iIdealCharPos);
        
        if (lpev->ptCursorPos.x > Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y))
            lpev->ptCursorPos.x = Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y);
    }

    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
    Edit_View_UpdateCaret(lpev);

    if (!bSelect)
        lpev->ptAnchor = lpev->ptCursorPos;

    Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
}

void TextMove_MoveDown(LPEDITVIEW lpev, BOOL bSelect)
{
    Edit_View_PrepareSelBounds(lpev);

    if (!Point_Equal(&lpev->ptDrawSelStart, &lpev->ptDrawSelEnd) && !bSelect)
        lpev->ptCursorPos = lpev->ptDrawSelEnd;

    if (lpev->ptCursorPos.y < Edit_View_GetLineCount(lpev) - 1)
    {
        if (lpev->iIdealCharPos == -1)
            lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);

        lpev->ptCursorPos.y++;
        lpev->ptCursorPos.x = Edit_View_ApproxActualOffset(lpev, lpev->ptCursorPos.y, lpev->iIdealCharPos);

        if (lpev->ptCursorPos.x > Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y))
            lpev->ptCursorPos.x = Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y);
    }
    else if (lpev->ptCursorPos.y == Edit_View_GetLineCount(lpev) - 1)
    {
        if (lpev->iIdealCharPos == -1)
            lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);

        lpev->ptCursorPos.x = Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y);
    }

    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
    Edit_View_UpdateCaret(lpev);

    if (!bSelect)
        lpev->ptAnchor = lpev->ptCursorPos;

    Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
}

void TextMove_MoveHome(LPEDITVIEW lpev, BOOL bSelect)
{
    int nLength = Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y);
    LPCTSTR pszChars = Edit_View_GetLineChars(lpev, lpev->ptCursorPos.y);
    int nHomePos = 0;
    
    while (nHomePos < nLength && isspace(pszChars[nHomePos]))
        nHomePos++;

    if (nHomePos == nLength || lpev->ptCursorPos.x == nHomePos)
        lpev->ptCursorPos.x = 0;
    else
        lpev->ptCursorPos.x = nHomePos;

    lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);

    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
    Edit_View_UpdateCaret(lpev);

    if (!bSelect)
        lpev->ptAnchor = lpev->ptCursorPos;

    Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
}

void TextMove_MoveEnd(LPEDITVIEW lpev, BOOL bSelect)
{
    lpev->ptCursorPos.x = Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y);
    lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);

    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
    Edit_View_UpdateCaret(lpev);

    if (!bSelect)
        lpev->ptAnchor = lpev->ptCursorPos;

    Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
}

void TextMove_MovePgUp(LPEDITVIEW lpev, BOOL bSelect)
{
    int nNewTopLine = lpev->iTopLine - Edit_View_GetScreenLines(lpev) + 1;
    
    if (nNewTopLine < 0)
        nNewTopLine = 0;

    if (lpev->iTopLine != nNewTopLine)
    {
        Edit_View_ScrollToLine(lpev, nNewTopLine, FALSE, TRUE);
//      Edit_View_UpdateSiblingScrollPos(lpev, TRUE);
    }

    lpev->ptCursorPos.y -= Edit_View_GetScreenLines(lpev) - 1;

    if (lpev->ptCursorPos.y < 0)
        lpev->ptCursorPos.y = 0;

    if (lpev->ptCursorPos.x > Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y))
        lpev->ptCursorPos.x = Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y);

    lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);

    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);   //todo: no vertical scroll
    Edit_View_UpdateCaret(lpev);

    if (!bSelect)
        lpev->ptAnchor = lpev->ptCursorPos;

    Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
}

void TextMove_MovePgDn(LPEDITVIEW lpev, BOOL bSelect)
{
    int nNewTopLine = lpev->iTopLine + Edit_View_GetScreenLines(lpev) - 1;
    
    if (nNewTopLine >= Edit_View_GetLineCount(lpev))
        nNewTopLine = Edit_View_GetLineCount(lpev) - 1;

    if (lpev->iTopLine != nNewTopLine)
    {
        Edit_View_ScrollToLine(lpev, nNewTopLine, FALSE, TRUE);
//      Edit_View_UpdateSiblingScrollPos(lpev, TRUE);
    }

    lpev->ptCursorPos.y += Edit_View_GetScreenLines(lpev) - 1;

    if (lpev->ptCursorPos.y >= Edit_View_GetLineCount(lpev))
        lpev->ptCursorPos.y = Edit_View_GetLineCount(lpev) - 1;

    if (lpev->ptCursorPos.x > Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y))
        lpev->ptCursorPos.x = Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y);

    lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);

    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);   //todo: no vertical scroll
    Edit_View_UpdateCaret(lpev);

    if (!bSelect)
        lpev->ptAnchor = lpev->ptCursorPos;

    Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
}

void TextMove_MoveCtrlHome(LPEDITVIEW lpev, BOOL bSelect)
{
    lpev->ptCursorPos.x = 0;
    lpev->ptCursorPos.y = 0;
    lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);

    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
    Edit_View_UpdateCaret(lpev);

    if (!bSelect)
        lpev->ptAnchor = lpev->ptCursorPos;

    Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
}

void TextMove_MoveCtrlEnd(LPEDITVIEW lpev, BOOL bSelect)
{
    lpev->ptCursorPos.y = Edit_View_GetLineCount(lpev) - 1;
    lpev->ptCursorPos.x = Edit_View_GetLineLength(lpev, lpev->ptCursorPos.y);
    lpev->iIdealCharPos = Edit_View_CalculateActualOffset(lpev, lpev->ptCursorPos.y, lpev->ptCursorPos.x);

    Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
    Edit_View_UpdateCaret(lpev);

    if (!bSelect)
        lpev->ptAnchor = lpev->ptCursorPos;

    Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
}

void TextMove_ScrollUp(LPEDITVIEW lpev)
{
    if (lpev->iTopLine > 0)
    {
        Edit_View_ScrollToLine(lpev, lpev->iTopLine - 1, FALSE, TRUE);
//      Edit_View_UpdateSiblingScrollPos(lpev, TRUE);
    }
}

void TextMove_ScrollDown(LPEDITVIEW lpev)
{
    if (lpev->iTopLine < Edit_View_GetLineCount(lpev) - 1)
    {
        Edit_View_ScrollToLine(lpev, lpev->iTopLine + 1, FALSE, TRUE);
//      Edit_View_UpdateSiblingScrollPos(lpev, TRUE);
    }
}

void TextMove_ScrollLeft(LPEDITVIEW lpev)
{
    if (lpev->iOffsetChar > 0)
    {
        Edit_View_ScrollToChar(lpev, lpev->iOffsetChar - 1, FALSE, TRUE);
        Edit_View_UpdateCaret(lpev);
    }
}

void TextMove_ScrollRight(LPEDITVIEW lpev)
{
    if (lpev->iOffsetChar < Edit_View_GetMaxLineLength(lpev) - 1)
    {
        Edit_View_ScrollToChar(lpev, lpev->iOffsetChar + 1, FALSE, TRUE);
        Edit_View_UpdateCaret(lpev);
    }
}

POINT TextMove_WordToRight(LPEDITVIEW lpev, POINT pt)
{
    int nLength = Edit_View_GetLineLength(lpev, pt.y);
    LPCTSTR pszChars = Edit_View_GetLineChars(lpev, pt.y);

    ASSERT_VALIDTEXTPOS(lpev, pt);

    while (pt.x < nLength)
    {
        if (!isalnum(pszChars[pt.x]) && pszChars[pt.x] != _T('_'))
            break;

        pt.x++;
    }

    ASSERT_VALIDTEXTPOS(lpev, pt);

    return (pt);
}

POINT TextMove_WordToLeft(LPEDITVIEW lpev, POINT pt)
{
    LPCTSTR pszChars = Edit_View_GetLineChars(lpev, pt.y);

    ASSERT_VALIDTEXTPOS(lpev, pt);

    while (pt.x > 0)
    {
        if (!isalnum(pszChars[pt.x - 1]) && pszChars[pt.x - 1] != _T('_'))
            break;

        pt.x--;
    }

    ASSERT_VALIDTEXTPOS(lpev, pt);

    return (pt);
}

void Edit_View_SelectAll(LPEDITVIEW lpev)
{
    int nLineCount = Edit_View_GetLineCount(lpev);
    POINT pt = { 0, 0 };

    lpev->ptCursorPos.x = Edit_View_GetLineLength(lpev, nLineCount - 1);
    lpev->ptCursorPos.y = nLineCount - 1;

    Edit_View_SetSelection(lpev, pt, lpev->ptCursorPos);
    Edit_View_UpdateCaret(lpev);
}

void Edit_View_OnLButtonDown(LPEDITVIEW lpev, UINT nFlags, POINT point)
{
    BOOL bShift = GetKeyState(VK_SHIFT) & 0x8000;
    BOOL bControl = GetKeyState(VK_CONTROL) & 0x8000;

    DefWindowProc(lpev->hwnd, WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));

    if (point.x < Edit_View_GetMarginWidth(lpev))
    {
        Edit_View_AdjustTextPoint(lpev, &point);

        if (bControl)
        {
            Edit_View_SelectAll(lpev);
        }
        else
        {
            POINT ptStart, ptEnd;

            lpev->ptCursorPos = Edit_View_ClientToText(lpev, point);
            lpev->ptCursorPos.x = 0;                //  Force beginning of the line

            if (!bShift)
                lpev->ptAnchor = lpev->ptCursorPos;

            ptStart = lpev->ptAnchor;

            if (ptStart.y == Edit_View_GetLineCount(lpev) - 1)
            {
                ptStart.x = Edit_View_GetLineLength(lpev, ptStart.y);
            }
            else
            {
                ptStart.y++;
                ptStart.x = 0;
            }

            ptEnd = lpev->ptCursorPos;
            ptEnd.x = 0;

            lpev->ptCursorPos = ptEnd;

            Edit_View_UpdateCaret(lpev);
            Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
            Edit_View_SetSelection(lpev, ptStart, ptEnd);

            SetCapture(lpev->hwnd);

            lpev->uDragSelTimer = SetTimer(lpev->hwnd, CRYSTAL_TIMER_DRAGSEL, 100, NULL);

            ASSERT(lpev->uDragSelTimer != 0);

            lpev->bWordSelection = FALSE;
            lpev->bLineSelection = TRUE;
            lpev->bDragSelection = TRUE;
        }
    }
    else
    {
        POINT ptText = Edit_View_ClientToText(lpev, point);

        Edit_View_PrepareSelBounds(lpev);
        //  [JRT]:  Support For Disabling Drag and Drop...
        if ((Edit_View_IsInsideSelBlock(lpev, ptText)) &&               // If Inside Selection Area
                (!lpev->bDisableDragAndDrop))               // And D&D Not Disabled
        {
            lpev->bPreparingToDrag = TRUE;
        }
        else
        {
            POINT ptStart, ptEnd;

            Edit_View_AdjustTextPoint(lpev, &point);
            lpev->ptCursorPos = Edit_View_ClientToText(lpev, point);
            
            if (!bShift)
                lpev->ptAnchor = lpev->ptCursorPos;

            if (bControl)
            {
                if (lpev->ptCursorPos.y < lpev->ptAnchor.y ||
                    lpev->ptCursorPos.y == lpev->ptAnchor.y && lpev->ptCursorPos.x < lpev->ptAnchor.x)
                {
                    ptStart = TextMove_WordToLeft(lpev, lpev->ptCursorPos);
                    ptEnd = TextMove_WordToRight(lpev, lpev->ptAnchor);
                }
                else
                {
                    ptStart = TextMove_WordToLeft(lpev, lpev->ptAnchor);
                    ptEnd = TextMove_WordToRight(lpev, lpev->ptCursorPos);
                }
            }
            else
            {
                ptStart = lpev->ptAnchor;
                ptEnd = lpev->ptCursorPos;
            }

            lpev->ptCursorPos = ptEnd;
            
            Edit_View_UpdateCaret(lpev);
            Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
            Edit_View_SetSelection(lpev, ptStart, ptEnd);

            SetCapture(lpev->hwnd);

            lpev->uDragSelTimer = SetTimer(lpev->hwnd, CRYSTAL_TIMER_DRAGSEL, 100, NULL);

            ASSERT(lpev->uDragSelTimer != 0);

            lpev->bWordSelection = bControl;
            lpev->bLineSelection = FALSE;
            lpev->bDragSelection = TRUE;
        }
    }

    ASSERT_VALIDTEXTPOS(lpev, lpev->ptCursorPos);
}

void Edit_View_OnMouseMove(LPEDITVIEW lpev, UINT nFlags, POINT point)
{
    DefWindowProc(lpev->hwnd, WM_MOUSEMOVE, nFlags, MAKELPARAM(point.x, point.y));

    if (lpev->bDragSelection)
    {
        BOOL bOnMargin = point.x < Edit_View_GetMarginWidth(lpev);
        POINT ptStart, ptEnd;
        POINT ptNewCursorPos;

        Edit_View_AdjustTextPoint(lpev, &point);

        ptNewCursorPos = Edit_View_ClientToText(lpev, point);

        if (lpev->bLineSelection)
        {
            if (bOnMargin)
            {
                if (ptNewCursorPos.y < lpev->ptAnchor.y ||
                    ptNewCursorPos.y == lpev->ptAnchor.y && ptNewCursorPos.x < lpev->ptAnchor.x)
                {
                    ptEnd = lpev->ptAnchor;
            
                    if (ptEnd.y == Edit_View_GetLineCount(lpev) - 1)
                    {
                        ptEnd.x = Edit_View_GetLineLength(lpev, ptEnd.y);
                    }
                    else
                    {
                        ptEnd.y++;
                        ptEnd.x = 0;
                    }

                    ptNewCursorPos.x = 0;
                    lpev->ptCursorPos = ptNewCursorPos;
                }
                else
                {
                    ptEnd = lpev->ptAnchor;
                    ptEnd.x = 0;
                    lpev->ptCursorPos = ptNewCursorPos;
            
                    if (ptNewCursorPos.y == Edit_View_GetLineCount(lpev) - 1)
                    {
                        ptNewCursorPos.x = Edit_View_GetLineLength(lpev, ptNewCursorPos.y);
                    }
                    else
                    {
                        ptNewCursorPos.y++;
                        ptNewCursorPos.x = 0;
                    }
            
                    lpev->ptCursorPos.x = 0;
                }
            
                Edit_View_UpdateCaret(lpev);
                Edit_View_SetSelection(lpev, ptNewCursorPos, ptEnd);
            
                return;
            }

            //  Moving to normal selection mode
            SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));

            lpev->bLineSelection = lpev->bWordSelection = FALSE;
        }

        if (lpev->bWordSelection)
        {
            if (ptNewCursorPos.y < lpev->ptAnchor.y ||
                ptNewCursorPos.y == lpev->ptAnchor.y && ptNewCursorPos.x < lpev->ptAnchor.x)
            {
                ptStart = TextMove_WordToLeft(lpev, ptNewCursorPos);
                ptEnd = TextMove_WordToRight(lpev, lpev->ptAnchor);
            }
            else
            {
                ptStart = TextMove_WordToLeft(lpev, lpev->ptAnchor);
                ptEnd = TextMove_WordToRight(lpev, ptNewCursorPos);
            }
        }
        else
        {
            ptStart = lpev->ptAnchor;
            ptEnd = ptNewCursorPos;
        }

        lpev->ptCursorPos = ptEnd;

        Edit_View_UpdateCaret(lpev);
        Edit_View_SetSelection(lpev, ptStart, ptEnd);
    }

    if (lpev->bPreparingToDrag)
    {
        /*
        HGLOBAL hData = Edit_View_PrepareDragData(lpev);

        lpev->bPreparingToDrag = FALSE;

        if (hData != NULL)
        {
            COleDataSource ds;
            DROPEFFECT de;
            
            if (lpev->pTextBuffer != NULL)
                Edit_Buffer_BeginUndoGroup(lpev->lpes);

            ds.CacheGlobalData(CF_TEXT, hData);
            lpev->bDraggingText = TRUE;
            de = ds.DoDragDrop(GetDropEffect(lpev));

            if (de != DROPEFFECT_NONE)
                OnDropSource(de);
*/
            lpev->bDraggingText = FALSE;
/*
            if (lpev->pTextBuffer != NULL)
                Edit_Buffer_FlushUndoGroup(lpev->lpes);
                */
//      }
    }

    ASSERT_VALIDTEXTPOS(lpev, lpev->ptCursorPos);
}

void Edit_View_OnLButtonUp(LPEDITVIEW lpev, UINT nFlags, POINT point)
{
    DefWindowProc(lpev->hwnd, WM_LBUTTONUP, nFlags, MAKELPARAM(point.x, point.y));

    if (lpev->bDragSelection)
    {
        POINT ptNewCursorPos;
        POINT ptStart, ptEnd;

        Edit_View_AdjustTextPoint(lpev, &point);
        ptNewCursorPos = Edit_View_ClientToText(lpev, point);

        if (lpev->bLineSelection)
        {
            POINT ptEnd;

            if (ptNewCursorPos.y < lpev->ptAnchor.y ||
                ptNewCursorPos.y == lpev->ptAnchor.y && ptNewCursorPos.x < lpev->ptAnchor.x)
            {
                ptEnd = lpev->ptAnchor;

                if (ptEnd.y == Edit_View_GetLineCount(lpev) - 1)
                {
                    ptEnd.x = Edit_View_GetLineLength(lpev, ptEnd.y);
                }
                else
                {
                    ptEnd.y++;
                    ptEnd.x = 0;
                }

                ptNewCursorPos.x = 0;
                lpev->ptCursorPos = ptNewCursorPos;
            }
            else
            {
                ptEnd = lpev->ptAnchor;
                ptEnd.x = 0;

                if (ptNewCursorPos.y == Edit_View_GetLineCount(lpev) - 1)
                {
                    ptNewCursorPos.x = Edit_View_GetLineLength(lpev, ptNewCursorPos.y);
                }
                else
                {
                    ptNewCursorPos.y++;
                    ptNewCursorPos.x = 0;
                }

                lpev->ptCursorPos = ptNewCursorPos;
            }

            Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
            Edit_View_UpdateCaret(lpev);
            Edit_View_SetSelection(lpev, ptNewCursorPos, ptEnd);
        }
        else
        {
            if (lpev->bWordSelection)
            {
                if (ptNewCursorPos.y < lpev->ptAnchor.y ||
                    ptNewCursorPos.y == lpev->ptAnchor.y && ptNewCursorPos.x < lpev->ptAnchor.x)
                {
                    ptStart = TextMove_WordToLeft(lpev, ptNewCursorPos);
                    ptEnd = TextMove_WordToRight(lpev, lpev->ptAnchor);
                }
                else
                {
                    ptStart = TextMove_WordToLeft(lpev, lpev->ptAnchor);
                    ptEnd = TextMove_WordToRight(lpev, ptNewCursorPos);
                }
            }
            else
            {
                ptStart = lpev->ptAnchor;
                ptEnd = lpev->ptCursorPos;
            }

            lpev->ptCursorPos = ptEnd;

            Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
            Edit_View_UpdateCaret(lpev);
            Edit_View_SetSelection(lpev, ptStart, ptEnd);
        }

        ReleaseCapture();
        KillTimer(lpev->hwnd, lpev->uDragSelTimer);

        lpev->bDragSelection = FALSE;
    }

    if (lpev->bPreparingToDrag)
    {
        lpev->bPreparingToDrag = FALSE;

        Edit_View_AdjustTextPoint(lpev, &point);

        lpev->ptCursorPos = Edit_View_ClientToText(lpev, point);

        Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
        Edit_View_UpdateCaret(lpev);
        Edit_View_SetSelection(lpev, lpev->ptCursorPos, lpev->ptCursorPos);
    }

    ASSERT_VALIDTEXTPOS(lpev, lpev->ptCursorPos);
}

void Edit_View_OnTimer(LPEDITVIEW lpev, UINT nIDEvent)
{

    if (nIDEvent == CRYSTAL_TIMER_DRAGSEL)
    {
        POINT pt;
        RECT rcClient;
        BOOL bChanged = FALSE;
        int nNewTopLine, nLineCount;
        int nNewOffsetChar;
        int nMaxLineLength;
        
        ASSERT(lpev->bDragSelection);

        GetCursorPos(&pt);
        ScreenToClient(lpev->hwnd, &pt);
        GetClientRect(lpev->hwnd, &rcClient);

        //  Scroll vertically, if necessary
        nNewTopLine = lpev->iTopLine;
        nLineCount = Edit_View_GetLineCount(lpev);
        
        if (pt.y < rcClient.top)
        {
            nNewTopLine--;

            if (pt.y < rcClient.top - Edit_View_GetLineHeight(lpev))
                nNewTopLine -= 2;
        }
        else if (pt.y >= rcClient.bottom)
        {
            nNewTopLine++;

            if (pt.y >= rcClient.bottom + Edit_View_GetLineHeight(lpev))
                nNewTopLine += 2;
        }

        if (nNewTopLine < 0)
            nNewTopLine = 0;

        if (nNewTopLine >= nLineCount)
            nNewTopLine = nLineCount - 1;

        if (lpev->iTopLine != nNewTopLine)
        {
            Edit_View_ScrollToLine(lpev, nNewTopLine, FALSE, TRUE);
//          Edit_View_UpdateSiblingScrollPos(lpev, TRUE);
            bChanged = TRUE;
        }

        //  Scroll horizontally, if necessary
        nNewOffsetChar = lpev->iOffsetChar;
        nMaxLineLength = Edit_View_GetMaxLineLength(lpev);

        if (pt.x < rcClient.left)
            nNewOffsetChar--;
        else if (pt.x >= rcClient.right)
            nNewOffsetChar++;

        if (nNewOffsetChar >= nMaxLineLength)
            nNewOffsetChar = nMaxLineLength - 1;

        if (nNewOffsetChar < 0)
            nNewOffsetChar = 0;

        if (lpev->iOffsetChar != nNewOffsetChar)
        {
            Edit_View_ScrollToChar(lpev, nNewOffsetChar, FALSE, TRUE);
            Edit_View_UpdateCaret(lpev);
//          Edit_View_UpdateSiblingScrollPos(lpev, FALSE);

            bChanged = TRUE;
        }

        //  Fix changes
        if (bChanged)
        {
            POINT ptNewCursorPos;
            Edit_View_AdjustTextPoint(lpev, &pt);

            ptNewCursorPos = Edit_View_ClientToText(lpev, pt);

            if (!Point_Equal(&ptNewCursorPos, &lpev->ptCursorPos))
            {
                lpev->ptCursorPos = ptNewCursorPos;

                Edit_View_UpdateCaret(lpev);
            }

            Edit_View_SetSelection(lpev, lpev->ptAnchor, lpev->ptCursorPos);
        }
    }
}

void Edit_View_OnLButtonDblClk(LPEDITVIEW lpev, UINT nFlags, POINT point)
{
    DefWindowProc(lpev->hwnd, WM_LBUTTONDBLCLK, nFlags, MAKELPARAM(point.x, point.y));

    if (!lpev->bDragSelection)
    {
        POINT ptStart, ptEnd;

        Edit_View_AdjustTextPoint(lpev, &point);

        lpev->ptCursorPos = Edit_View_ClientToText(lpev, point);
        lpev->ptAnchor = lpev->ptCursorPos;

        if (lpev->ptCursorPos.y < lpev->ptAnchor.y ||
            lpev->ptCursorPos.y == lpev->ptAnchor.y && lpev->ptCursorPos.x < lpev->ptAnchor.x)
        {
            ptStart = TextMove_WordToLeft(lpev, lpev->ptCursorPos);
            ptEnd = TextMove_WordToRight(lpev, lpev->ptAnchor);
        }
        else
        {
            ptStart = TextMove_WordToLeft(lpev, lpev->ptAnchor);
            ptEnd = TextMove_WordToRight(lpev, lpev->ptCursorPos);
        }

        lpev->ptCursorPos = ptEnd;

        Edit_View_UpdateCaret(lpev);
        Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
        Edit_View_SetSelection(lpev, ptStart, ptEnd);

        SetCapture(lpev->hwnd);

        lpev->uDragSelTimer = SetTimer(lpev->hwnd, CRYSTAL_TIMER_DRAGSEL, 100, NULL);

        ASSERT(lpev->uDragSelTimer != 0);

        lpev->bWordSelection = TRUE;
        lpev->bLineSelection = FALSE;
        lpev->bDragSelection = TRUE;
    }
}

void Edit_View_OnRButtonDown(LPEDITVIEW lpev, UINT nFlags, POINT point)
{
    POINT pt = point;

    Edit_View_AdjustTextPoint(lpev, &pt);

    pt = Edit_View_ClientToText(lpev, pt);

    if (!Edit_View_IsInsideSelBlock(lpev, pt))
    {
        lpev->ptAnchor = lpev->ptCursorPos = pt;

        Edit_View_SetSelection(lpev, lpev->ptCursorPos, lpev->ptCursorPos);
        Edit_View_EnsureVisible(lpev, lpev->ptCursorPos);
        Edit_View_UpdateCaret(lpev);
    }

    DefWindowProc(lpev->hwnd, WM_RBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
}

BOOL Edit_View_TextInClipboard(LPEDITVIEW lpev)
{
    return (IsClipboardFormatAvailable(CF_TEXT));
}

BOOL Edit_View_PutToClipboard(LPEDITVIEW lpev, LPCTSTR pszText)
{
    BOOL bOK = FALSE;

    if (pszText == NULL || lstrlen(pszText) == 0)
        return (FALSE);


    if (OpenClipboard(lpev->hwnd))
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
