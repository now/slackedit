/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_interface.c
 * Created    : 07/22/00
 * Owner      : pcppopper
 * Revised on : 07/22/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include <pcp_includes.h>

/* pcp_generic */
#include <pcp_mem.h>

/* pcp_edit */
#include "pcp_edit.h"
#include "pcp_edit_common.h"
#include "pcp_textedit.h"
#include "pcp_textedit_interface.h"
#include "pcp_textedit_buffer.h"

/****************************************************************
 * Type Definitions
 ****************************************************************/

/****************************************************************
 * Function Definitions
 ****************************************************************/

static BOOL TextEdit_Interface_IsSelection(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_GetSelection(LPEDITINTERFACE lpInterface, LPPOINT lpptStart, LPPOINT lpptEnd);
static void TextEdit_Interface_SetSelection(LPEDITINTERFACE lpInterface, POINT ptStart, POINT ptEnd);
static void TextEdit_Interface_GetSelectionPos(LPEDITINTERFACE lpInterface, LPINT lpnStartPos, LPINT lpnEndPos);
static void TextEdit_Interface_SetSelectionPos(LPEDITINTERFACE lpInterface, int nStartPos, int nEndPos);
static int TextEdit_Interface_GetSelectionMode(LPEDITINTERFACE lpInterface);
static int TextEdit_Interface_SetSelectionMode(LPEDITINTERFACE lpInterface, int nMode);
static BOOL TextEdit_Interface_HighlightText(LPEDITINTERFACE lpInterface, POINT ptStartPos, int nLength, BOOL bReverse);
static void TextEdit_Interface_SetAnchor(LPEDITINTERFACE lpInterface, POINT ptNewAnchor);
static POINT TextEdit_Interface_GetCursorPos(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_SetCursorPos(LPEDITINTERFACE lpInterface, POINT ptCursorPos);
static BOOL TextEdit_Interface_GotoLine(LPEDITINTERFACE lpInterface, int nLineIndex, BOOL bExtendSelection);;
static void TextEdit_Interface_ReplaceSelection(LPEDITINTERFACE lpInterface, LPCTSTR pszNewText);
static POINT TextEdit_Interface_GetLastChangePos(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_EnsureVisible(LPEDITINTERFACE lpInterface, POINT pt);

static int TextEdit_Interface_GetLineCount(LPEDITINTERFACE lpInterface);
static int TextEdit_Interface_GetLineLength(LPEDITINTERFACE lpInterface, int nLineIndex);
static LPCTSTR TextEdit_Interface_GetLineChars(LPEDITINTERFACE lpInterface, int nLineIndex);
static void TextEdit_Interface_GetText(LPEDITINTERFACE lpInterface, POINT ptStart, POINT ptEnd, LPTSTR *pszText);
static BOOL TextEdit_Interface_InsertText(LPEDITINTERFACE lpInterface, int nLine, int nPos, LPCTSTR pszText, int *pnEndLine, int *pnEndPos, int nAction);
static BOOL TextEdit_Interface_DeleteText(LPEDITINTERFACE lpInterface, int nStartLine, int nStartPos, int nEndLine, int nEndPos, int nAction);
static BOOL TextEdit_Interface_InsertTextPos(LPEDITINTERFACE lpInterface, int nPos, LPCTSTR pszText, int nAction);
static BOOL TextEdit_Interface_DeleteTextPos(LPEDITINTERFACE lpInterface, int nStartPos, int nEndPos, int nAction);
static void TextEdit_Interface_SetLineFlag(LPEDITINTERFACE lpInterface, int iLine, DWORD dwFlag, BOOL bSet, BOOL bRemoveFromPreviousLine);
static DWORD TextEdit_Interface_GetLineFlags(LPEDITINTERFACE lpInterface, int iLine);
static int TextEdit_Interface_GetLineWithFlag(LPEDITINTERFACE lpInterface, DWORD dwFlag);

static HFONT TextEdit_Interface_GetFont(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_SetFont(LPEDITINTERFACE lpInterface, LPLOGFONT lplf);

static BOOL TextEdit_Interface_FindText(LPEDITINTERFACE lpInterface, LPCTSTR pszText, POINT ptStartPos, DWORD dwFlags, LPPOINT lpptFoundPos);
static BOOL TextEdit_Interface_FindTextInBlock(LPEDITINTERFACE lpInterface, LPCTSTR pszText, POINT ptStartPos, POINT ptBlockBegin, POINT ptBlockEnd, DWORD dwFlags, LPPOINT lpptFoundPos);
static BOOL TextEdit_Interface_FindReplaceSelection(LPEDITINTERFACE lpInterface, LPCTSTR pszNewText, DWORD dwFlags);
static BOOL TextEdit_Interface_FindTextVisual(LPEDITINTERFACE lpInterface, LPCTSTR pszText, POINT ptStartPos, DWORD dwFlags, LPPOINT lpptFoundPos);
static BOOL TextEdit_Interface_FindNext(LPEDITINTERFACE lpInterface, LPPOINT lpptFoundPos);
static BOOL TextEdit_Interface_FindPrevious(LPEDITINTERFACE lpInterface, LPPOINT lpptFoundPos);
static BOOL TextEdit_Interface_IsIncrementalSearchActive(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_IncrementalSearchStart(LPEDITINTERFACE lpInterface, bForward);
static LPCTSTR TextEdit_Interface_IncrementalSearchAddChar(LPEDITINTERFACE lpInterface, TCHAR cChar);
static LPCTSTR TextEdit_Interface_IncrementalSearchRemoveChar(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_IncrementalSearchEnd(LPEDITINTERFACE lpInterface, BOOL bReturnToOldPlace);
static BOOL TextEdit_Interface_FindSelection(LPEDITINTERFACE lpInterface, BOOL fForward, LPPOINT lpptFoundPos);
static BOOL TextEdit_Interface_GetSearchInfo(LPEDITINTERFACE lpInterface, LPEDITSEARCHINFO lpSearchInfo)

static BOOL TextEdit_Interface_LoadFile(LPEDITINTERFACE lpInterface, LPCTSTR pszFileName);
static BOOL TextEdit_Interface_SaveToFile(LPEDITINTERFACE lpInterface, LPCTSTR pszFileName);

static BOOL TextEdit_Interface_GetDisableDragAndDrop(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_SetDisableDragAndDrop(LPEDITINTERFACE lpInterface, BOOL bDisable);

static void TextEdit_Interface_ToggleBookmark(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_NextBookmark(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_PreviousBookmark(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_ClearAllBookmarks(LPEDITINTERFACE lpInterface);

static void TextEdit_Interface_BeginUndoGroup(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_FlushUndoGroup(LPEDITINTERFACE lpInterface);
static int TextEdit_Interface_GetUndoDescription(LPEDITINTERFACE lpInterface, LPTSTR pszDesc, int nPos);
static int TextEdit_Interface_GetRedoDescription(LPEDITINTERFACE lpInterface, LPTSTR pszDesc, int nPos);
static BOOL TextEdit_Interface_CanUndo(LPEDITINTERFACE lpInterface);
static BOOL TextEdit_Interface_CanRedo(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_Undo(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_Redo(LPEDITINTERFACE lpInterface);

static void TextEdit_Interface_SetModified(LPEDITINTERFACE lpInterface, BOOL bModified);
static BOOL TextEdit_Interface_GetModified(LPEDITINTERFACE lpInterface);

static int TextEdit_Interface_GetLastActionDescription(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_AssertValidTextPos(LPEDITINTERFACE lpInterface, POINT pt);
static BOOL TextEdit_Interface_DeleteCurrentSelection(LPEDITINTERFACE lpInterface);
static BOOL TextEdit_Interface_GetReadOnly(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_SetReadOnly(LPEDITINTERFACE lpInterface, BOOL bReadOnly);

static void TextEdit_Interface_GotoNumBookmark(LPEDITINTERFACE lpInterface, int nBookmarkID
static void TextEdit_Interface_ToggleNumBookmark(LPEDITINTERFACE lpInterface, int nBookmarkID
static void TextEdit_Interface_ClearAllNumBookmarks(LPEDITINTERFACE lpInterface
static BOOL TextEdit_Interface_GetViewTabs(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_SetViewTabs(LPEDITINTERFACE lpInterface, BOOL bViewTabs);
static BOOL TextEdit_Interface_GetSelectionMargin(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_SetSelectionMargin(LPEDITINTERFACE lpInterface, BOOL bSelMargin);
static HCURSOR TextEdit_Interface_GetMarginCursor(LPEDITINTERFACE lpInterface);
static HCURSOR TextEdit_Interface_SetMarginCursor(LPEDITINTERFACE lpInterface, HCURSOR hCursor);
static void TextEdit_Interface_LoadFileEx(LPEDITINTERFACE lpInterface, LPCTSTR pszFileName, int nCrLfStyle);
static BOOL TextEdit_Interface_SaveToFileEx(LPEDITINTERFACE lpInterface, LPCTSTR pszFileName, int nCrLfStyle, BOOL bClearModifiedFlag, BOOL fBackup, LPCTSTR pszBackupFileName);
static int TextEdit_Interface_GetCRLFMode(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_SetCRLFMode(LPEDITINTERFACE lpInterface, int nCRLFMode);
static void TextEdit_Interface_SetTextType(LPEDITINTERFACE lpInterface, LPCTSTR pszExt);
static POINT TextEdit_Interface_MatchBrace(LPEDITINTERFACE lpInterface, POINT ptStartPos);
static void TextEdit_Interface_Tab(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_UnTab(LPEDITINTERFACE lpInterface);
static void TextEdit_Interface_OnDelete(LPEDITINTERFACE lpInterface);
static POINT TextEdit_Interface_FindWordBoundaryToLeft(LPTEXTEDITVIEW lpte, POINT pt);
static POINT TextEdit_Interface_FindWordBoundaryToRight(LPTEXTEDITVIEW lpte, POINT pt);

/****************************************************************
 * Global Variables
 ****************************************************************/

static ATOM s_hTextEditInterfaceAtom;
static EDITINTERFACE s_teInterface;

/****************************************************************
 * Function Implementations
 ****************************************************************/

void TextEdit_Internal_SetupInterface(void)
{
    /* add the property atom name used with *Prop functions */
    s_hTextEditInterfaceAtom = GlobalAddAtom(_T("TextEditInterfaceProp"));

    /* alloc the V-Table first of all */
    s_teInterface.lpVtbl = (LPEDITVTBL)Mem_Alloc(sizeof(TEXTEDITVTBL));

    /* base */
    s_teInterface.lpVtbl->IsSelection = TextEdit_Interface_IsSelection;
    s_teInterface.lpVtbl->GetSelection = TextEdit_Interface_GetSelection;
    s_teInterface.lpVtbl->SetSelection = TextEdit_Interface_SetSelection;
    s_teInterface.lpVtbl->GetSelectionPos = TextEdit_Interface_GetSelectionPos;
    s_teInterface.lpVtbl->SetSelectionPos = TextEdit_Interface_SetSelectionPos;
    s_teInterface.lpVtbl->GetSelectionMode = TextEdit_Interface_GetSelectionMode;
    s_teInterface.lpVtbl->SetSelectionMode = TextEdit_Interface_SetSelectionMode;
    s_teInterface.lpVtbl->HighlightText = TextEdit_Interface_HighlightText;
    s_teInterface.lpVtbl->SetAnchor = TextEdit_Interface_SetAnchor;
    s_teInterface.lpVtbl->GetCursorPos = TextEdit_Interface_GetCursorPos;
    s_teInterface.lpVtbl->SetCursorPos = TextEdit_Interface_SetCursorPos;
    s_teInterface.lpVtbl->GotoLine = TextEdit_Interface_GotoLine;
    s_teInterface.lpVtbl->ReplaceSelection = TextEdit_Interface_ReplaceSelection;
    s_teInterface.lpVtbl->GetLastChangePos = TextEdit_Interface_GetLastChangePos;
    s_teInterface.lpVtbl->EnsureVisible = TextEdit_Interface_EnsureVisible;
    s_teInterface.lpVtbl->GetLineCount = TextEdit_Interface_GetLineCount;
    s_teInterface.lpVtbl->GetLineLength = TextEdit_Interface_GetLineLength;
    s_teInterface.lpVtbl->GetLineChars = TextEdit_Interface_GetLineChars;
    s_teInterface.lpVtbl->GetText = TextEdit_Interface_GetText;
    s_teInterface.lpVtbl->InsertText = TextEdit_Interface_InsertText;
    s_teInterface.lpVtbl->DeleteText = TextEdit_Interface_DeleteText;
    s_teInterface.lpVtbl->InsertTextPos = TextEdit_Interface_InsertTextPos;
    s_teInterface.lpVtbl->DeleteTextPos = TextEdit_Interface_DeleteTextPos;
    s_teInterface.lpVtbl->GetFont = TextEdit_Interface_GetFont;
    s_teInterface.lpVtbl->SetFont = TextEdit_Interface_SetFont;
    s_teInterface.lpVtbl->FindText = TextEdit_Interface_FindText;
    s_teInterface.lpVtbl->FindTextInBlock = TextEdit_Interface_FindTextInBlock;
    s_teInterface.lpVtbl->FindReplaceSelection = TextEdit_Interface_FindReplaceSelection;
    s_teInterface->lpVtbl->FindTextVisual = TextEdit_Interface_FindTextVisual;
    s_teInterface->lpVtbl->FindNext = TextEdit_Interface_FindNext;
    s_teInterface->lpVtbl->FindPrevious = TextEdit_Interface_FindPrevious;

    s_teInterface->lpVtbl->IsIncrementalSearchActive = TextEdit_Interface_IsIncrementalSearchActive;
    s_teInterface->lpVtbl->IncrementalSearchStart = TextEdit_Interface_IncrementalSearchStart;
    s_teInterface->lpVtbl->IncrementalSearchAddChar = TextEdit_Interface_IncrementalSearchAddChar;
    s_teInterface->lpVtbl->IncrementalSearchRemoveChar = TextEdit_Interface_IncrementalSearchRemoveChar;
    s_teInterface->lpVtbl->IncrementalSearchEnd = TextEdit_Interface_IncrementalSearchEnd;
    s_teInterface->lpVtbl->FindSelection = TextEdit_Interface_FindSelection;
    s_teInterface->lpVtbl->GetSearchInfo = TextEdit_Interface_GetSearchInfo;

    s_teInterface.lpVtbl->LoadFile = TextEdit_Interface_LoadFile;
    s_teInterface.lpVtbl->SaveToFile = TextEdit_Interface_SaveToFile;
    s_teInterface.lpVtbl->GetDisableDragAndDrop = TextEdit_Interface_GetDisableDragAndDrop;
    s_teInterface.lpVtbl->SetDisableDragAndDrop = TextEdit_Interface_SetDisableDragAndDrop;
    s_teInterface.lpVtbl->ToggleBookmark = TextEdit_Interface_ToggleBookmark;
    s_teInterface.lpVtbl->NextBookmark = TextEdit_Interface_NextBookmark;
    s_teInterface.lpVtbl->PreviousBookmark = TextEdit_Interface_PreviousBookmark;
    s_teInterface.lpVtbl->ClearAllBookmarks = TextEdit_Interface_ClearAllBookmarks;
    s_teInterface.lpVtbl->BeginUndoGroup = TextEdit_Interface_BeginUndoGroup;
    s_teInterface.lpVtbl->FlushUndoGroup = TextEdit_Interface_FlushUndoGroup;
    s_teInterface.lpVtbl->GetUndoDescription = TextEdit_Interface_GetUndoDescription;
    s_teInterface.lpVtbl->GetRedoDescription = TextEdit_Interface_GetRedoDescription;
    s_teInterface.lpVtbl->CanUndo = TextEdit_Interface_CanUndo;
    s_teInterface.lpVtbl->CanRedo = TextEdit_Interface_CanRedo;
    s_teInterface.lpVtbl->Undo = TextEdit_Interface_Undo;
    s_teInterface.lpVtbl->Redo = TextEdit_Interface_Redo;

    s_teInterface.lpVtbl->SetModified = TextEdit_Interface_SetModified;
    s_teInterface.lpVtbl->GetModified = TextEdit_Interface_GetModified;

    s_teInterface.lpVtbl->GetLastActionDescription = TextEdit_Interface_GetLastActionDescription;
    s_teInterface.lpVtbl->AssertValidTextPos = TextEdit_Interface_AssertValidTextPos;
    s_teInterface.lpVtbl->DeleteCurrentSelection = TextEdit_Interface_DeleteCurrentSelection;
    s_teInterface.lpVtbl->GetReadOnly = TextEdit_Interface_GetReadOnly;
    s_teInterface.lpVtbl->SetReadOnly = TextEdit_Interface_SetReadOnly;

    /* extension */
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->GetLineActualLength = TextEdit_Interface_GetLineActualLength;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->CalculateActualOffset = TextEdit_Interface_CalculateActualOffset;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->GetViewTabs = TextEdit_Interface_GetViewTabs;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->SetViewTabs = TextEdit_Interface_SetViewTabs;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->GetSelectionMargin = TextEdit_Interface_GetSelectionMargin;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->SetSelectionMargin = TextEdit_Interface_SetSelectionMargin;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->GetMarginCursor = TextEdit_Interface_GetMarginCursor;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->SetMarginCursor = TextEdit_Interface_SetMarginCursor;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->LoadFileEx = TextEdit_Interface_LoadFileEx;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->SaveToFileEx = TextEdit_Interface_SaveToFileEx;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->GetCRLFMode = TextEdit_Interface_GetCRLFMode;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->SetCRLFMode = TextEdit_Interface_SetCRLFMode;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->SetTextType = TextEdit_Interface_SetTextType;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->MatchBrace = TextEdit_Interface_MatchBrace;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->Tab = TextEdit_Interface_Tab;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->UnTab = TextEdit_Interface_UnTab;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->OnDelete = TextEdit_Interface_OnDelete;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->FindWordBoundaryToLeft = TextEdit_Interface_FindWordBoundaryToLeft;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->FindWordBoundaryToRight = TextEdit_Interface_FindWordBoundaryToRight;
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->GotoNumBookmark = TextEdit_Interface_GotoNumBookmark
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->ToggleNumBookmark = TextEdit_Interface_ToggleNumBookmark
    ((LPTEXTEDITVTBL)s_teInterface.lpVtbl)->ClearAllNumBookmarks = TextEdit_Interface_ClearAllNumBookmarks
}

void TextEdit_Internal_DeleteInterface(void)
{
    GlobalDeleteAtom(s_hTextEditInterfaceAtom);
    Mem_Free(s_teInterface.lpVtbl);
}

BOOL TextEdit_Internal_CreateInterface(HWND hwnd)
{
    LPTEXTEDITINTERFACE lpteInterface = (LPTEXTEDITINTERFACE)Mem_Alloc(sizeof(TEXTEDITINTERFACE));

    if (lpteInterface == NULL)
        return (FALSE);

    lpteInterface->lpInterface = &s_teInterface;

    lpteInterface->lpte = (LPTEXTEDITVIEW)Mem_Alloc(sizeof(TEXTEDITVIEW));
    lpteInterface->lpEditWindow = (LPEDITWINDOW)Mem_Alloc(sizeof(EDITWINDOW));

    if (lpteInterface->lpte == NULL)
    {
        Mem_Free(lpteInterface);

        return (FALSE);
    }

    if (!SetProp(hwnd, (LPCTSTR)MAKEWORD(s_hTextEditInterfaceAtom, 0), (HANDLE)lpteInterface))
    {
        Mem_Free(lpteInterface->lpte);
        Mem_Free(lpteInterface);

        return (FALSE);
    }

    return (TRUE);
}

LPTEXTEDITINTERFACE TextEdit_Internal_GetInterface(HWND hwnd)
{
    LPTEXTEDITINTERFACE lpteInterface = (LPTEXTEDITINTERFACE)GetProp(hwnd, (LPCTSTR)MAKEWORD(s_hTextEditInterfaceAtom, 0));

    return (lpteInterface);
}

BOOL TextEdit_Internal_DestroyInterface(HWND hwnd)
{
    LPTEXTEDITINTERFACE lpteInterface = (LPTEXTEDITINTERFACE)GetProp(hwnd, (LPCTSTR)MAKEWORD(s_hTextEditInterfaceAtom, 0));

    if (lpteInterface == NULL)
        return (FALSE);

    if (lpteInterface->lpEditWindow->pszMatchedLine != NULL)
        Mem_Free(lpteInterface->lpEditWindow->pszMatchedLine);
    if (lpteInterface->lpEditWindow->pszSearchString != NULL)
        Mem_Free(lpteInterface->lpEditWindow->pszSearchString);
    if (lpteInterface->lpEditWindow->rxNode != NULL)
        free(lpteInterface->lpEditWindow->rxNode);
    Mem_Free(lpteInterface->lpEditWindow);

    Mem_Free(lpteInterface->lpte);
    Mem_Free(lpteInterface);

    RemoveProp(hwnd, (LPCTSTR)MAKEWORD(s_hTextEditInterfaceAtom, 0));

    return (TRUE);
}

/* common edit interface handlers */

/* selection */

static BOOL TextEdit_Interface_IsSelection(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_View_IsSelection(((LPTEXTEDITINTERFACE)lpInterface)->lpte));
}

static void TextEdit_Interface_GetSelection(LPEDITINTERFACE lpInterface, LPPOINT lpptStart, LPPOINT lpptEnd)
{
    TextEdit_View_GetSelection(((LPTEXTEDITINTERFACE)lpInterface)->lpte, lpptStart, lpptEnd);
}

static void TextEdit_Interface_SetSelection(LPEDITINTERFACE lpInterface, POINT ptStart, POINT ptEnd)
{
    TextEdit_View_SetSelection(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ptStart, ptEnd);
}

static void TextEdit_Interface_GetSelectionPos(LPEDITINTERFACE lpInterface, LPINT lpnStartPos, LPINT lpnEndPos)
{
    TextEdit_View_GetSelectionPos(((LPTEXTEDITINTERFACE)lpInterface)->lpte, lpnStartPos, lpnEndPos);
}

static void TextEdit_Interface_SetSelectionPos(LPEDITINTERFACE lpInterface, int nStartPos, int nEndPos)
{
    TextEdit_View_SetSelectionPos(((LPTEXTEDITINTERFACE)lpInterface)->lpte, nStartPos, nEndPos);
}

static int TextEdit_Interface_GetSelectionMode(LPEDITINTERFACE lpInterface)
{
    return (((LPTEXTEDITINTERFACE)lpInterface)->lpte->nSelectionMode);
}

static int TextEdit_Interface_SetSelectionMode(LPEDITINTERFACE lpInterface, int nMode)
{
    int nOldMode = ((LPTEXTEDITINTERFACE)lpInterface)->lpte->nSelectionMode;

    ((LPTEXTEDITINTERFACE)lpInterface)->lpte->nSelectionMode = nMode;

    return (nOldMode);
}

static BOOL TextEdit_Interface_HighlightText(LPEDITINTERFACE lpInterface, POINT ptStartPos, int nLength, BOOL bReverse)
{
    return (TextEdit_View_HighlightText(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ptStartPos, nLength, bReverse));
}

static void TextEdit_Interface_SetAnchor(LPEDITINTERFACE lpInterface, POINT ptNewAnchor)
{
    TextEdit_View_SetAnchor(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ptNewAnchor);
}

static POINT TextEdit_Interface_GetCursorPos(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_View_GetCursorPos(((LPTEXTEDITINTERFACE)lpInterface)->lpte));
}

static void TextEdit_Interface_SetCursorPos(LPEDITINTERFACE lpInterface, POINT ptCursorPos)
{
    TextEdit_View_SetCursorPos(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ptCursorPos);
}

static BOOL TextEdit_Interface_GotoLine(LPEDITINTERFACE lpInterface, int nLineIndex, BOOL bExtendSelection)
{
    return (TextEdit_View_GotoLine(((LPTEXTEDITINTERFACE)lpInterface)->lpte, nLineIndex, bExtendSelection));
}

static void TextEdit_Interface_ReplaceSelection(LPEDITINTERFACE lpInterface, LPCTSTR pszNewText)
{
    TextEdit_View_ReplaceSelection(((LPTEXTEDITINTERFACE)lpInterface)->lpte, pszNewText);
}

static POINT TextEdit_Interface_GetLastChangePos(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_Buffer_GetLastChangePos(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes));
}

static void TextEdit_Interface_EnsureVisible(LPEDITINTERFACE lpInterface, POINT pt)
{
    TextEdit_View_EnsureVisible(((LPTEXTEDITINTERFACE)lpInterface)->lpte, pt);
}

/* Linehandling */

static int TextEdit_Interface_GetLineCount(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_View_GetLineCount(((LPTEXTEDITINTERFACE)lpInterface)->lpte));
}

static int TextEdit_Interface_GetLineLength(LPEDITINTERFACE lpInterface, int nLineIndex)
{
    return (TextEdit_View_GetLineLength(((LPTEXTEDITINTERFACE)lpInterface)->lpte, nLineIndex));
}

static LPCTSTR TextEdit_Interface_GetLineChars(LPEDITINTERFACE lpInterface, int nLineIndex)
{
    return (TextEdit_View_GetLineChars(((LPTEXTEDITINTERFACE)lpInterface)->lpte, nLineIndex));
}

static void TextEdit_Interface_GetText(LPEDITINTERFACE lpInterface, POINT ptStart, POINT ptEnd, LPTSTR *pszText)
{
    TextEdit_View_GetText(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ptStart, ptEnd, pszText);
}

static BOOL TextEdit_Interface_InsertText(LPEDITINTERFACE lpInterface, int nLine, int nPos, LPCTSTR pszText, int *pnEndLine, int *pnEndPos, int nAction)
{
    return (TextEdit_Buffer_InsertText(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, nLine, nPos, pszText, pnEndLine, pnEndPos, nAction));
}

static BOOL TextEdit_Interface_DeleteText(LPEDITINTERFACE lpInterface, int nStartLine, int nStartPos, int nEndLine, int nEndPos, int nAction)
{
    return (TextEdit_Buffer_DeleteText(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, nStartLine, nStartPos, nEndLine, nEndPos, nAction));
}

static BOOL TextEdit_Interface_InsertTextPos(LPEDITINTERFACE lpInterface, int nPos, LPCTSTR pszText, int nAction)
{
    return (TextEdit_Buffer_InsertTextPos(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, nPos, pszText, nAction));
}

static BOOL TextEdit_Interface_DeleteTextPos(LPEDITINTERFACE lpInterface, int nStartPos, int nEndPos, int nAction)
{
    return (TextEdit_Buffer_DeleteTextPos(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, nStartPos, nEndPos, nAction));
}

static void TextEdit_Interface_SetLineFlag(LPEDITINTERFACE lpInterface, int iLine, DWORD dwFlag, BOOL bSet, BOOL bRemoveFromPreviousLine)
{
    TextEdit_Buffer_SetLineFlag(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, iLine, dwFlag, bSet, bRemoveFromPreviousLine);
}

static DWORD TextEdit_Interface_GetLineFlags(LPEDITINTERFACE lpInterface, int iLine)
{
    return (TextEdit_Buffer_GetLineFlags(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, iLine));
}

static int TextEdit_Interface_GetLineWithFlag(LPEDITINTERFACE lpInterface, DWORD dwFlag)
{
    return (TextEdit_Buffer_GetLineWithFlag(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, DWORD dwFlag));
}

/* font */


static HFONT TextEdit_Interface_GetFont(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_View_GetFont(((LPTEXTEDITINTERFACE)lpInterface)->lpte, FALSE, FALSE));
}

static void TextEdit_Interface_SetFont(LPEDITINTERFACE lpInterface, LPLOGFONT lplf)
{
    TextEdit_View_SetFont(((LPTEXTEDITINTERFACE)lpInterface)->lpte, lplf);
}

/* searching */

static BOOL TextEdit_Interface_FindText(LPEDITINTERFACE lpInterface, LPCTSTR pszText, POINT ptStartPos, DWORD dwFlags,, LPPOINT lpptFoundPos)
{
    return (TextEdit_View_FindText(lpInterface, ((LPTEXTEDITINTERFACE)lpInterface)->lpEditWindow, pszText, ptStartPos, dwFlags lpptFoundPos));
}

static BOOL TextEdit_Interface_FindTextInBlock(LPEDITINTERFACE lpInterface, LPCTSTR pszText, POINT ptStartPos, POINT ptBlockBegin, POINT ptBlockEnd, DWORD dwFlags, LPPOINT lpptFoundPos)
{
    return (CommonEdit_Find_FindTextInBlock(lpInterface, ((LPTEXTEDITINTERFACE)lpInterface)->lpEditWindow, pszText, ptStartPos, ptBlockBegin, ptBlockEnd, dwFlags, lpptFoundPos));
}

static BOOL TextEdit_Interface_FindReplaceSelection(LPEDITINTERFACE lpInterface, LPCTSTR pszNewText, DWORD dwFlags)
{
    return (CommonEdit_Find_ReplaceSelection(lpInterface, ((LPTEXTEDITINTERFACE)lpInterface)->lpEditWindow, pszNewText, dwFlags));
}

static BOOL TextEdit_Interface_FindTextVisual(LPEDITINTERFACE lpInterface, LPCTSTR pszText, POINT ptStartPos, DWORD dwFlags, LPPOINT lpptFoundPos)
{
    return (CommonEdit_Find_FindTextVisual(lpInterface, ((LPTEXTEDITINTERFACE)lpInterface)->lpEditWindow, pszText, ptStartPos, dwFlags, lpptFoundPos));
}

static BOOL TextEdit_Interface_FindNext(LPEDITINTERFACE lpInterface, LPPOINT lpptFoundPos)
{
    return (CommonEdit_Find_FindNext(lpInterface, ((LPTEXTEDITINTERFACE)lpInterface)->lpEditWindow, lpptFoundPos));
}

static BOOL TextEdit_Interface_FindPrevious(LPEDITINTERFACE lpInterface, LPPOINT lpptFoundPos)
{
    return (CommonEdit_Find_FindPrevious(lpInterface, ((LPTEXTEDITINTERFACE)lpInterface)->lpEditWindow, lpptFoundPos));
}

static BOOL TextEdit_Interface_GetSearchInfo(LPEDITINTERFACE lpInterface, LPEDITSEARCHINFO lpSearchInfo)
{
    return (CommonEdit_Find_GetSearchInfo(((LPEDITINTERFACE)lpInterface)->lpEditWindow, lpSearchInfo));
}

static BOOL TextEdit_Interface_IsIncrementalSearchActive(LPEDITINTERFACE lpInterface)
{
    return (((LPTEXTEDITINTERFACE)lpInterface)->lpEditWindow->bIncrementalSearchActive);
}

static void TextEdit_Interface_IncrementalSearchStart(LPEDITINTERFACE lpInterface, bForward)
{
    CommonEdit_IncrementalSearch_Start((lpInterface, ((LPTEXTEDITINTERFACE)lpInterface)->lpEditWindow, bForward);
}

static LPCTSTR TextEdit_Interface_IncrementalSearchAddChar(LPEDITINTERFACE lpInterface, TCHAR cChar)
{
    return (CommonEdit_IncrementalSearch_AddChar(lpInterface, ((LPTEXTEDITINTERFACE)lpInterface)->lpEditWindow, cChar);
}

static LPCTSTR TextEdit_Interface_IncrementalSearchRemoveChar(LPEDITINTERFACE lpInterface)
{
    return (CommonEdit_IncrementalSearch_RemoveChar(lpInterface, ((LPTEXTEDITINTERFACE)lpInterface)->lpEditWindow))
}

static void TextEdit_Interface_IncrementalSearchEnd(LPEDITINTERFACE lpInterface, BOOL bReturnToOldPlace)
{
    CommonEdit_IncrementalSearch_End(lpInterface, ((LPTEXTEDITINTERFACE)lpInterface)->lpEditWindow, bReturnToOldPlace);
}

static BOOL TextEdit_Interface_FindSelection(LPEDITINTERFACE lpInterface, BOOL fForward, LPPOINT lpptFoundPos)
{
    return (CommonEdit_Find_FindSelection(lpInterface, ((LPTEXTEDITINTERFACE)lpInterface)->lpEditWindow, fForward, lpptFoundPos))
}

/* file-i/o */

static BOOL TextEdit_Interface_LoadFile(LPEDITINTERFACE lpInterface, LPCTSTR pszFileName)
{
    LPTEXTEDITVIEW lpte = ((LPTEXTEDITINTERFACE)lpInterface)->lpte;
    BOOL bSuccess;

    TextEdit_Buffer_FreeAll(lpte->lpes);
    bSuccess = TextEdit_Buffer_LoadFromFile(lpte, lpte->lpes, pszFileName, lpte->lpes->iCRLFMode);
    RedrawWindow(lpte->hwnd, NULL, NULL, RDW_INVALIDATE|RDW_INTERNALPAINT|RDW_ERASE|RDW_ERASENOW|RDW_UPDATENOW|RDW_NOFRAME);

    return (bSuccess);
}

static BOOL TextEdit_Interface_SaveToFile(LPEDITINTERFACE lpInterface, LPCTSTR pszFileName)
{
    LPTEXTEDITVIEW lpte = ((LPTEXTEDITINTERFACE)lpInterface)->lpte;

    return (TextEdit_Buffer_SaveToFile(lpte->lpes, pszFileName, TextEdit_Buffer_GetCRLFMode(lpte->lpes), TRUE, FALSE, NULL));
}

/* UI-Toggles */
static BOOL TextEdit_Interface_GetDisableDragAndDrop(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_View_GetDisableDragAndDrop(((LPTEXTEDITINTERFACE)lpInterface)->lpte));
}

static void TextEdit_Interface_SetDisableDragAndDrop(LPEDITINTERFACE lpInterface, BOOL bDisable)
{
    TextEdit_View_SetDisableDragAndDrop(((LPTEXTEDITINTERFACE)lpInterface)->lpte, bDisable);
}

/* bookmarks */
static void TextEdit_Interface_ToggleBookmark(LPEDITINTERFACE lpInterface)
{
    TextEdit_View_ToggleBookmark(((LPTEXTEDITINTERFACE)lpInterface)->lpte);
}

static void TextEdit_Interface_NextBookmark(LPEDITINTERFACE lpInterface)
{
    TextEdit_View_NextBookmark(((LPTEXTEDITINTERFACE)lpInterface)->lpte);
}

static void TextEdit_Interface_PreviousBookmark(LPEDITINTERFACE lpInterface)
{
    TextEdit_View_PrevBookmark(((LPTEXTEDITINTERFACE)lpInterface)->lpte);
}

static void TextEdit_Interface_ClearAllBookmarks(LPEDITINTERFACE lpInterface)
{
    TextEdit_View_ClearAllBookmarks(((LPTEXTEDITINTERFACE)lpInterface)->lpte);
}

/* undo/redo */

static void TextEdit_Interface_BeginUndoGroup(LPEDITINTERFACE lpInterface)
{
    TextEdit_Buffer_BeginUndoGroup(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, FALSE);
}

static void TextEdit_Interface_FlushUndoGroup(LPEDITINTERFACE lpInterface)
{
    TextEdit_Buffer_FlushUndoGroup(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes);
}

static int TextEdit_Interface_GetUndoDescription(LPEDITINTERFACE lpInterface, LPTSTR pszDesc, int nPos)
{
    return (TextEdit_Buffer_GetUndoDescription(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, pszDesc, nPos));
}

static int TextEdit_Interface_GetRedoDescription(LPEDITINTERFACE lpInterface, LPTSTR pszDesc, int nPos)
{
    return (TextEdit_Buffer_GetRedoDescription(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, pszDesc, nPos));
}

static BOOL TextEdit_Interface_CanUndo(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_Buffer_CanUndo(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes));
}

static BOOL TextEdit_Interface_CanRedo(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_Buffer_CanRedo(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes));
}

static void TextEdit_Interface_Undo(LPEDITINTERFACE lpInterface)
{
    TextEdit_View_Undo(((LPTEXTEDITINTERFACE)lpInterface)->lpte);
}

static void TextEdit_Interface_Redo(LPEDITINTERFACE lpInterface)
{
    TextEdit_View_Redo(((LPTEXTEDITINTERFACE)lpInterface)->lpte);
}

static void TextEdit_Interface_SetModified(LPEDITINTERFACE lpInterface, BOOL bModified)
{
    TextEdit_Buffer_SetModified(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, bModified);
}

static BOOL TextEdit_Interface_GetModified(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_Buffer_GetModified(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes));
}

/* other */

static int TextEdit_Interface_GetLastActionDescription(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_Buffer_GetLastActionDescription(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes));
}

static void TextEdit_Interface_AssertValidTextPos(LPEDITINTERFACE lpInterface, POINT pt)
{
    TextEdit_View_AssertValidTextPos(((LPTEXTEDITINTERFACE)lpInterface)->lpte, pt);
}

static BOOL TextEdit_Interface_DeleteCurrentSelection(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_View_DeleteCurrentSelection(((LPTEXTEDITINTERFACE)lpInterface)->lpte));
}

static BOOL TextEdit_Interface_GetReadOnly(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_Buffer_GetReadOnly(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes));
}

static void TextEdit_Interface_SetReadOnly(LPEDITINTERFACE lpInterface, BOOL bReadOnly)
{
    TextEdit_Buffer_SetReadOnly(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, bReadOnly);
}

/* TextEdit extension interface functions */

/* buffer handling */

static int TextEdit_Interface_GetLineActualLength(LPEDITINTERFACE lpInterface, int iLineIndex)
{
    return (TextEdit_View_GetLineActualLength(((LPEDITINTERFACE)lpInterface)->lpte, iLineIndex));
}

static int TextEdit_Interface_CalculateActualOffset(LPEDITINTERFACE lpInterface, int iLineIndex, int iCharIndex)
{
    return (TextEdit_View_CalculateActualOffset(((LPEDITINTERFACE)lpInterface)->lpte, int iLineIndex, int iCharIndex));
}

/* UI-Toggles */

static BOOL TextEdit_Interface_GetViewTabs(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_View_GetViewTabs(((LPTEXTEDITINTERFACE)lpInterface)->lpte));
}

static void TextEdit_Interface_SetViewTabs(LPEDITINTERFACE lpInterface, BOOL bViewTabs)
{
    TextEdit_View_SetViewTabs(((LPTEXTEDITINTERFACE)lpInterface)->lpte, bViewTabs);
}

static BOOL TextEdit_Interface_GetSelectionMargin(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_View_GetSelectionMargin(((LPTEXTEDITINTERFACE)lpInterface)->lpte));
}

static void TextEdit_Interface_SetSelectionMargin(LPEDITINTERFACE lpInterface, BOOL bSelMargin)
{
    TextEdit_View_SetSelectionMargin(((LPTEXTEDITINTERFACE)lpInterface)->lpte, bSelMargin);
}

static HCURSOR TextEdit_Interface_GetMarginCursor(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_View_GetMarginCursor(((LPTEXTEDITINTERFACE)lpInterface)->lpte));
}

static HCURSOR TextEdit_Interface_SetMarginCursor(LPEDITINTERFACE lpInterface, HCURSOR hCursor)
{
    return (TextEdit_View_SetMarginCursor(((LPTEXTEDITINTERFACE)lpInterface)->lpte, hCursor));
}

/* searching */

static POINT TextEdit_Interface_FindWordBoundaryToLeft(LPTEXTEDITVIEW lpte, POINT pt)
{
    return (TextEdit_Move_WordToLeft(((LPTEXTEDITVIEW)lpte)->lpte, POINT pt));
}

static POINT TextEdit_Interface_FindWordBoundaryToRight(LPTEXTEDITVIEW lpte, POINT pt)
{
    return (TextEdit_Move_WordToRight(((LPTEXTEDITVIEW)lpte)->lpte, POINT pt));
}

/* file-i/o */

static void TextEdit_Interface_LoadFileEx(LPEDITINTERFACE lpInterface, LPCTSTR pszFileName, int nCrLfStyle)
{
    TextEdit_Buffer_LoadFromFile(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, pszFileName, nCrLfStyle);
}

static BOOL TextEdit_Interface_SaveToFileEx(LPEDITINTERFACE lpInterface, LPCTSTR pszFileName, int nCrLfStyle, BOOL bClearModifiedFlag, BOOL fBackup, LPCTSTR pszBackupFileName)
{
    return (TextEdit_Buffer_SaveToFile(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, pszFileName, nCrLfStyle, bClearModifiedFlag, fBackup, pszBackupFileName));
}

static int TextEdit_Interface_GetCRLFMode(LPEDITINTERFACE lpInterface)
{
    return (TextEdit_Buffer_GetCRLFMode(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes));
}

static void TextEdit_Interface_SetCRLFMode(LPEDITINTERFACE lpInterface, int nCRLFMode)
{
    TextEdit_Buffer_SetCRLFMode(((LPTEXTEDITINTERFACE)lpInterface)->lpte->lpes, nCRLFMode);
}

/* bookmarks */
static void TextEdit_Interface_GotoNumBookmark(LPEDITINTERFACE lpInterface, int nBookmarkID
{
    TextEdit_View_GotoNumBookmark(((LPEDITINTERFACE)lpInterface)->lpte, int nBookmarkID)
}

static void TextEdit_Interface_ToggleNumBookmark(LPEDITINTERFACE lpInterface, int nBookmarkID
{
    TextEdit_View_ToggleNumBookmark(((LPEDITINTERFACE)lpInterface)->lpte, int nBookmarkID)
}

static void TextEdit_Interface_ClearAllNumBookmarks(LPEDITINTERFACE lpInterface
{
    TextEdit_View_ClearAllNumBookmarks(((LPEDITINTERFACE)lpInterface)->lpte)
}

/* other */

static void TextEdit_Interface_SetTextType(LPEDITINTERFACE lpInterface, LPCTSTR pszExt)
{
    TextEdit_View_SetTextType(((LPTEXTEDITINTERFACE)lpInterface)->lpte, pszExt);
}


static POINT TextEdit_Interface_MatchBrace(LPEDITINTERFACE lpInterface, POINT ptStartPos)
{
    return (TextEdit_View_MatchBrace(((LPTEXTEDITINTERFACE)lpInterface)->lpte, ptStartPos));
}

static void TextEdit_Interface_Tab(LPEDITINTERFACE lpInterface)
{
    TextEdit_View_Tab(((LPTEXTEDITINTERFACE)lpInterface)->lpte);
}

static void TextEdit_Interface_UnTab(LPEDITINTERFACE lpInterface)
{
    TextEdit_View_Untab(((LPTEXTEDITINTERFACE)lpInterface)->lpte);
}

static void TextEdit_Interface_OnDelete(LPEDITINTERFACE lpInterface)
{
    TextEdit_View_OnDelete(((LPTEXTEDITINTERFACE)lpInterface)->lpte);
}
