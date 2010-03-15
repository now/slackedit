/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_common.c
 * Created    : 08/13/00
 * Owner      : pcppopper
 * Revised on : 08/13/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "../pcp_generic/pcp_includes.h"

/* pcp_generic */
#include <pcp_regexp.h>

/* pcp_edit */
#include "pcp_edit.h"
#include "pcp_edit_common.h"

/****************************************************************
 * Type Definitions
 ****************************************************************/

/****************************************************************
 * Function Definitions
 ****************************************************************/

static int CommonEdit_Find_FindStringHelper(LPCTSTR pszFindWhere, LPCTSTR pszFindWhat, DWORD dwFlags, int *nLen, RxNode *rxNode, RxMatchRes *rxMatch)

/****************************************************************
 * Global Variables
 ****************************************************************/

/****************************************************************
 * Function Implementations
 ****************************************************************/

static int CommonEdit_Find_FindStringHelper(LPCTSTR pszFindWhere, LPCTSTR pszFindWhat, DWORD dwFlags, int *nLen, RxNode *rxNode, RxMatchRes *rxMatch)
{
    if (dwFlags & PE_FIND_REG_EXP)
    {
        int nPos;
        
        if (rxNode)
            RxFree(rxNode);

        rxNode = RxCompile(pszFindWhat);

        if (rxNode && RxExec(rxNode, pszFindWhere, _tcslen(pszFindWhere), pszFindWhere, rxMatch, (dwFlags & PE_FIND_MATCH_CASE) != 0 ? RX_CASE : 0))
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
            
            if ((dwFlags & PE_FIND_WHOLE_WORD) == 0)
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

BOOL CommonEdit_Find_FindText(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, LPCTSTR pszText, POINT ptStartPos, DWORD dwFlags, POINT *lpptFoundPos)
{
    int iLineCount = PCP_Edit_GetLineCount(lpInterface);
    POINT ptBlockBegin = { 0, 0 };
    POINT ptBlockEnd = { PCP_Edit_GetLineLength(lpInterface, iLineCount - 1), (iLineCount - 1) };

    return (CommonEdit_Find_FindTextInBlock(lpInterface, lpEditWindow, pszText, ptStartPos, ptBlockBegin,
            ptBlockEnd, dwFlags, lpptFoundPos));
}

BOOL CommonEdit_Find_FindTextInBlock(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, LPCTSTR pszText, POINT ptStartPosition,
                            POINT ptBlockBegin, POINT ptBlockEnd, DWORD dwFlags, LPPOINT lpptFoundPos)
{
    POINT ptCurrentPos = ptStartPosition;
    LPTSTR pszWhat;
    INT nEolns;
    BOOL bWrapSearch = (dwFlags & PE_FIND_WRAP_SEARCH) ? TRUE : FALSE;

    ASSERT(pszText != NULL && _tcslen(pszText) > 0);
    PCP_Edit_AssertValidTextPos(lpInterface, ptCurrentPos);
    PCP_Edit_AssertValidTextPos(lpInterface, ptBlockBegin);
    PCP_Edit_AssertValidTextPos(lpInterface, ptBlockEnd);
    ASSERT (ptBlockBegin.y < ptBlockEnd.y || ptBlockBegin.y == ptBlockEnd.y &&
            ptBlockBegin.x <= ptBlockEnd.x);

    if (Point_Equal(&ptBlockBegin, &ptBlockEnd))
        return (FALSE);

    if (ptCurrentPos.y < ptBlockBegin.y || ptCurrentPos.y == ptBlockBegin.y &&
        ptCurrentPos.x < ptBlockBegin.x)
    {
        ptCurrentPos = ptBlockBegin;
    }

    pszWhat = (LPTSTR)pszText;

    if (dwFlags & PE_FIND_REG_EXP)
    {
        nEolns = String_CountSubStrings(pszWhat, _T("\\n"));
    }
    else
    {
        nEolns = 0;

        if ((dwFlags & PE_FIND_MATCH_CASE) == 0)
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

    if (dwFlags & PE_FIND_DIRECTION_UP)
    {
        //  Let's check if we deal with whole text.
        //  At this point, we cannot search *up* in selection
        ASSERT(ptBlockBegin.x == 0 && ptBlockBegin.y == 0);
        ASSERT(ptBlockEnd.x == PCP_Edit_GetLineLength(lpInterface, PCP_Edit_GetLineCount(lpInterface) - 1) &&
                ptBlockEnd.y == PCP_Edit_GetLineCount(lpInterface) - 1);

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

                if (dwFlags & PE_FIND_REG_EXP)
                {
                    int i;

                    for (i = 0; i <= nEolns && ptCurrentPos.y >= i; i++)
                    {
                        LPCTSTR pszChars = PCP_Edit_GetLineChars(lpInterface, ptCurrentPos.y - i);

                        if (i)
                        {
                            iLineLength = PCP_Edit_GetLineLength(lpInterface, ptCurrentPos.y - i);
                            ptCurrentPos.x = 0;

                            pszLine = (LPTSTR)Mem_ReAllocStr(pszLine, Mem_SizeStr(pszLine) + 1 + SZ);

                            String_Insert(&pszLine, _T("\n"), 0);
                        }
                        else
                        {
                            iLineLength = (ptCurrentPos.x != -1) ? ptCurrentPos.x : TextEdit_View_GetLineLength(lpInterface, ptCurrentPos.y - i);
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
                    iLineLength = PCP_Edit_GetLineLength(lpInterface, ptCurrentPos.y);

                    if (ptCurrentPos.x == -1)
                    {
                        ptCurrentPos.x = iLineLength;
                    }
                    else
                    {
                        if (ptCurrentPos.x > iLineLength)
                            ptCurrentPos.x = iLineLength - 1;
                    }

                    pszChars = PCP_Edit_GetLineChars(lpInterface, ptCurrentPos.y);
                    pszLine = (LPTSTR)Mem_AllocStr(ptCurrentPos.x + 1 + SZ);
                    _tcsncpy(pszLine, pszChars, ptCurrentPos.x); // + 1

                    if ((dwFlags & PE_FIND_MATCH_CASE) == 0)
                        _tcsupr(pszLine);
                }

                nMatchLen   = _tcslen(pszWhat);
                nLineLen    = _tcslen(pszLine);

                do
                {
                    iPos = CommonEdit_Find_FindStringHelper(pszLine, pszWhat, dwFlags, &lpEditWindow->nLastSearchLen, lpEditWindow->rxNode, &lpEditWindow->rxMatch);

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

                    if (lpEditWindow->pszMatchedLine != NULL)
                        Mem_Free(lpEditWindow->pszMatchedLine);

                    lpEditWindow->pszMatchedLine = String_Duplicate(pszLine);

                    Mem_Free(pszLine);

                    return (TRUE);
                }
                else
                {
                    if (lpEditWindow->pszMatchedLine != NULL)
                        Mem_Free(lpEditWindow->pszMatchedLine);

                    lpEditWindow->pszMatchedLine = NULL;
                }

                ptCurrentPos.y--;

                if (ptCurrentPos.y >= 0)
                    ptCurrentPos.x = PCP_Edit_GetLineLength(lpInterface, ptCurrentPos.y);

                Mem_Free(pszLine);
            }
            
            //  Beginning of text reached
            if (!bWrapSearch)
                return (FALSE);

            //  Start again from the end of text
            bWrapSearch = FALSE;

            ptCurrentPos.x = -1;
            ptCurrentPos.y = PCP_Edit_GetLineCount(lpInterface) - 1;
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

                if (dwFlags & PE_FIND_REG_EXP)
                {
                    int i;

                    nLines = PCP_Edit_GetLineCount(lpInterface);

                    for (i = 0; i <= nEolns && ptCurrentPos.y + i < nLines; i++)
                    {
                        LPCTSTR pszChars = PCP_Edit_GetLineChars(lpInterface, ptCurrentPos.y + i);
                        nLineLength = PCP_Edit_GetLineLength(lpInterface, ptCurrentPos.y + i);

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

                    nLineLength = PCP_Edit_GetLineLength(lpInterface, ptCurrentPos.y) - ptCurrentPos.x;

                    if (nLineLength <= 0)
                    {
                        ptCurrentPos.x = 0;
                        ptCurrentPos.y++;

                        continue;
                    }
                    
                    pszChars = PCP_Edit_GetLineChars(lpInterface, ptCurrentPos.y);
                    pszChars += ptCurrentPos.x;

                    pszLine = (LPTSTR)Mem_AllocStr(nLineLength + SZ);
                    //  Prepare necessary part of line
                    _tcsncpy(pszLine, pszChars, nLineLength);

                    if ((dwFlags & PE_FIND_MATCH_CASE) == 0)
                        _tcsupr(pszLine);
                }
                
                //  Perform search in the line
                nPos = CommonEdit_Find_FindStringHelper(pszLine, pszWhat, dwFlags, &lpEditWindow->nLastSearchLen, lpEditWindow->rxNode, &lpEditWindow->rxMatch);

                if (nPos >= 0)
                {
                    if (lpEditWindow->pszMatchedLine != NULL)
                        Mem_Free(lpEditWindow->pszMatchedLine);

                    lpEditWindow->pszMatched = String_Duplicate(pszLine);

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
                    if (lpEditWindow->pszMatchedLine != NULL)
                        Mem_Free(lpEditWindow->pszMatchedLine);

                    lpEditWindow->pszMatchedLine = NULL;
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

BOOL CommonEdit_Find_FindTextVisual(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, LPCTSTR pszText, POINT ptStartPos, DWORD dwFlags, LPPOINT lpptFoundPos)
{
    lpEditWindow->dwLastSearchFlags = dwFlags;
    lpEditWindow->pszSearchText     = String_Duplicate(pszText);
    lpEditWindow->nLastSearchLen    = _tcslen(lpEditWindow->pszSearchText);

    lpEditWindow->bLastSearchSuccessful = PCP_Edit_FindText(lpInterface, pszText, ptCurrentPos, lpFileInfo->dwLastSearchFlags, lpEditWindow->dwLastSearchFlags & PE_FIND_WRAP_SEARCH, lpptFoundPos);

    if (!lpEditWindow->bLastSearchSuccessful)
        return (FALSE);

    PCP_Edit_HighlightText(lpInterface, *lpptFoundPos, lpEditWindow->nLastSearchLen, (lpEditWindow->dwLastSearchFlags & PE_FIND_DIRECTION_UP) != 0);

    return (TRUE);
}

BOOL CommonEdit_Find_FindNext(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, LPPOINT lpptFoundPos)
{
    if (lpEditWindow->bLastSearchSuccessful)
    {
        POINT ptSearchPos = PCP_Edit_GetCursorPos(lpInterface);

        if (PCP_Edit_IsSelection(lpInterface))
        {
            POINT ptDummy;

            if (lpFileInfo->dwLastSearchFlags & PE_FIND_DIRECTION_UP)
                PCP_Edit_GetSelection(lpInterface, &ptSearchPos, &ptDummy);
            else
                PCP_Edit_GetSelection(lpInterface, &ptDummy, &ptSearchPos);
        }

        lpEditWindow->bLastSearchSuccessful = PCP_Edit_FindText(lpInterface, lpEditWindow->pszSearchString, ptSearchPos, lpEditWindow->dwLastSearchFlags, lpEditWindow->dwLastSearchFlags & PE_FIND_WRAP_SEARCH, lpptFoundPos);
        
        if (!lpEditWindow->bLastSearchSuccessful)
            return (FALSE);

        PCP_Edit_HighlightText(lpInterface, *lpptFoundPos, lpEditWindow->nLastSearchLen, (lpEditWindow->dwLastSearchFlags & PE_FIND_DIRECTION_UP) != 0);

        return (TRUE);
    }

    return (FALSE);
}

BOOL CommonEdit_Find_FindPrevious(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, LPPOINT lpptFoundPos)
{
    DWORD dwSaveSearchFlags = lpEditWindow->dwLastSearchFlags;
    BOOL bSuccess;
    
    if ((lpEditWindow->dwLastSearchFlags & PE_FIND_DIRECTION_UP) != 0)
        lpEditWindow->dwLastSearchFlags &= ~PE_FIND_DIRECTION_UP;
    else
        lpEditWindow->dwLastSearchFlags |= PE_FIND_DIRECTION_UP;

    bSuccess = CommonEdit_Find_FindNext(lpInterface, lpEditWindow, lpptFoundPos);
    lpEditWindow->dwLastSearchFlags = dwSaveSearchFlags;

    return (bSuccess);
}

BOOL CommonEdit_Find_ReplaceSelection(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, LPCTSTR pszNewText, DWORD dwFlags)
{
    POINT ptCursorPos;
    POINT ptEndOfBlock;
    int x, y;

    ASSERT(pszNewText != NULL);

    if (!PCP_Edit_IsSelection(lpInterface))
        return (FALSE);

    PCP_Edit_BeginUndoGroup(lpInterface);
    
    PCP_Edit_DeleteCurrentSelection(lpInterface);

    ptCursorPos = PCP_Edit_GetCursorPos(lpInterface);
    PCP_Edit_AssertValidTextPos(lpInterface, ptCursorPos);

    if (dwFlags & PE_FIND_REG_EXP)
    {
        LPTSTR pszNewStr;

        if (lpEditWindow->pszMatchedLine != NULL && !RxReplace(pszNewText, lpEditWindow->pszMatchedLine, lpEditWindow->nLastSearchLen, lpEditWindow->rxMatch, &pszNewStr, &lpEditWindow->nLastReplaceLen))
        {
            LPTSTR pszText;

            if (pszNewStr && lpEditWindow->nLastReplaceLen > 0)
            {
                pszText = (LPTSTR)Mem_Alloc(lpEditWindow->nLastReplaceLen + 1);
                _tcscpy(pszText, pszNewStr);
            }
            else
            {
                pszText = _T("");
            }

            PCP_Edit_InsertText(lpInterface, ptCursorPos.y, ptCursorPos.x, pszText, &y, &x, PE_ACTION_REPLACE);

            if (pszNewStr != NULL)
                free(pszNewStr);
        }
    }
    else
    {
        PCP_Edit_InsertText(lpInterface, ptCursorPos.y, ptCursorPos.x, pszNewText, &y, &x, PE_ACTION_REPLACE);
        lpEditWindow->nLastReplaceLen = _tcslen(pszNewText);
    }

    ptEndOfBlock.x = x;
    ptEndOfBlock.y = y;

    PCP_Edit_AssertValidTextPos(lpInterface, ptCursorPos);
    PCP_Edit_AssertValidTextPos(lpInterface, ptEndOfBlock);
    PCP_Edit_SetAnchor(lpInterface, ptEndOfBlock);
    PCP_Edit_SetSelection(lpInterface, ptCursorPos, ptEndOfBlock);
    PCP_Edit_SetCursorPos(lpInterface, ptEndOfBlock);
    PCP_Edit_EnsureVisible(lpInterface, ptEndOfBlock);
    PCP_Edit_FlushUndoGroup(lpInterface);

    return (TRUE);
}


BOOL CommonEdit_Find_GetSearchInfo(LPEDITWINDOW lpEditWindow, LPEDITSEARCHINFO lpSearchInfo)
{
    ASSERT(lpSearchInfo != NULL);
    ASSERT(lpEditWindow != NULL);

    lpSearchInfo->bLastSearchSuccessful = lpEditWindow->bLastSearchSuccessful;
    lpSearchInfo->pszSearchString       = lpEditWindow->pszSearchString;
    lpSearchInfo->pszMatchedLine        = lpEditWindow->pszMatchedLine;
    lpSearchInfo->nLastSearchLen        = lpEditWindow->nLastSearchLen;
    lpSearchInfo->nLastReplaceLen       = lpEditWindow->nLastReplaceLen;
    lpSearchInfo->dwLastSearchFlags     = lpEditWindow->dwLastSearchFlags;

    return (TRUE);
}

void CommonEdit_IncrementalSearch_Start(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, BOOL bForward)
{
    lpEditWindow->ptIncrementalSearchStartPos =
            lpEditWindow->ptCursorPosBeforeIncrementalSearch =
                PCP_Edit_GetCursorPos(lpInterface);

    PCP_Edit_GetSelection(lpInterface, &lpEditWindow->ptSelStartBeforeIncrementalSearch,
                                    &lpEditWindow->ptSelEndBeforeIncrementalSearch);

    if (lpEditWindow->pszSearchString != NULL)
        Mem_Free(lpEditWindow->pszSearchString);

    lpEditWindow->pszSearchString = NULL;

    lpEditWindow->dwLastSearchFlags &= ~PE_FIND_DIRECTION_UP;

    if (!bForward)
        lpEditWindow->dwLastSearchFlags |= PE_FIND_DIRECTION_UP;

    lpEditWindow->bIncrementalSearchActive  = TRUE;
}

LPCTSTR CommonEdit_IncrementalSearch_AddChar(LPEDITWINDOW lpEditWindow, TCHAR cChar)
{
    TCHAR szChar[2] = { cChar, _T('\0') };

    ASSERT(lpEditWindow->bIncrementalSearchActive);

    if (lpEditWindow->pszSearchString == NULL)
        lpEditWindow->pszSearchString = (LPTSTR)Mem_AllocStr(1);
    else
        lpEditWindow->pszSearchString = (LPTSTR)Mem_ReAllocStr(lpEditWindow->pszSearchString, Mem_SizeStr(lpEditWindow->pszSearchString) + 1);

    _tcscat(lpEditWindow->pszSearchString, szChar);

    return (lpEditWindow->pszSearchString);
}
    
LPCTSTR CommonEdit_IncrementalSearch_RemoveChar(LPEDITWINDOW lpEditWindow)
{
    ASSERT(lpEditWindow->bIncrementalSearchActive);

    if (lpEditWindow->pszSearchString != NULL && _tcslen(lpEditWindow->pszSearchString) >= 1)
        lpEditWindow->pszSearchString[_tcslen(lpEditWindow->pszSearchString) - 1] = _T('\0');

    return (lpEditWindow->pszSearchString);
}

void CommonEdit_IncrementalSearch_End(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, BOOL bReturnToOldPlace)
{
    ASSERT(lpEditWindow->bIncrementalSearchActive);

    /* restore some old info if necessary */
    if (bReturnToOldPlace)
    {
        PCP_Edit_SetSelection(lpInterface, lpEditWindow->ptSelStartBeforeIncrementalSearch, lpEditWindow->ptSelEndBeforeIncrementalSearch);
        PCP_Edit_SetCursorPos(lpInterface, lpEditWindow->ptCursorPosBeforeIncrementalSearch);
        PCP_Edit_EnsureVisible(lpInterface, lpEditWindow->ptCursorPosBeforeIncrementalSearch);
    }

    lpEditWindow->bIncrementalSearchActive = FALSE;
}

BOOL CommonEdit_Find_FindSelection(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, BOOL fForward, LPPOINT lpptFoundPos)
{
    LPTSTR pszSearchText;
    int nLineCount;
    POINT ptStartPos;
    POINT ptDummy;
    DWORD dwFlags = PCP_Edit_GetLastSearchFlags(lpInterface);

    if (!PCP_Edit_IsSelection(lpInterface))
    {
        // Section: select current word
        POINT ptStart, ptEnd, ptAnchor, pt;
        ptAnchor = ptStart = pt = PCP_Edit_GetCursorPos(lpInterface);

        if (pt.y < ptAnchor.y || pt.y == ptAnchor.y && pt.x < ptAnchor.x)
        {
            ptStart = PCP_TextEdit_WordToLeft(lpInterface, pt);
            ptEnd = PCP_TextEdit_WordToRight(lpInterface, ptAnchor);
        }
        else
        {
            ptStart = PCP_TextEdit_WordToLeft(lpInterface, ptAnchor);
            ptEnd = PCP_TextEdit_WordToRight(lpInterface, pt);
        }

        pt = ptEnd;

        PCP_Edit_SetCursorPos(lpInterface, pt);
        PCP_Edit_SetSelection(lpInterface, ptStart, ptEnd);
        PCP_Edit_SetAnchor(lpInterface, ptAnchor);
        PCP_Edit_EnsureVisible(lpInterface, pt);
    }
    
    if (PCP_Edit_IsSelection(lpInterface))
    {
        POINT ptSelStart, ptSelEnd;

        PCP_Edit_GetSelection(lpInterface, &ptSelStart, &ptSelEnd);            

        if (ptSelStart.y == ptSelEnd.y)
        {
            LPCTSTR pszChars = PCP_Edit_GetLineChars(lpInterface, ptSelStart.y);
            int nChars = ptSelEnd.x - ptSelStart.x;

            pszSearchText = (LPTSTR)Mem_AllocStr(nChars);
            _tcsncpy(pszSearchText, (pszChars + ptSelStart.x), nChars);
        }
    }
    else
    {
        return (FALSE); // cannot auto-locate any selection
    }

    if (fForward)
    {
        dwFlags &= ~PE_FIND_DIRECTION_UP;
        PCP_Edit_GetSelection(lpInterface, &ptDummy, &ptStartPos);
    }
    else
    {
        dwFlags |= PE_FIND_DIRECTION_UP; // Search upwards
        PCP_Edit_GetSelection(lpInterface, &ptStartPos, &ptDummy);
    }

    return (PCP_Edit_FindTextVisual(lpInterface, pszSearchText, ptStartPos, dwFlags, lpptFoundPos));
}
