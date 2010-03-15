/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit.h
 * Created    : not known (before 01/24/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:43:16
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_EDIT_H
#define __PCP_EDIT_H

#include "pcp_edit_dll.h"

typedef struct tagEDITINTERFACE EDITINTERFACE, *LPEDITINTERFACE;

typedef struct tagEDITVTBL
{
	/* Selection, anchor, cursor */
	BOOL (*IsSelection)(LPEDITINTERFACE lpInterface);
	void (*GetSelection)(LPEDITINTERFACE lpInterface, LPPOINT lpptStart, LPPOINT lpptEnd);
	void (*SetSelection)(LPEDITINTERFACE lpInterface, POINT ptStart, POINT ptEnd);
	void (*GetSelectionPos)(LPEDITINTERFACE lpInterface, LPINT lpnStartPos, LPINT lpnEndPos);
	void (*SetSelectionPos)(LPEDITINTERFACE lpInterface, int nStartPos, int nEndPos);
	int (*GetSelectionMode)(LPEDITINTERFACE lpInterface);
	int (*SetSelectionMode)(LPEDITINTERFACE lpInterface, int nMode);
	BOOL (*HighlightText)(LPEDITINTERFACE lpInterface, POINT ptStartPos, int nLength, BOOL bReverse);
	void (*SetAnchor)(LPEDITINTERFACE lpInterface, POINT ptNewAnchor);
	POINT (*GetCursorPos)(LPEDITINTERFACE lpInterface);
	void (*SetCursorPos)(LPEDITINTERFACE lpInterface, POINT ptCursorPos);
	BOOL (*GotoLine)(LPEDITINTERFACE lpInterface, int nLineIndex, BOOL bExtendSelection);
	void (*ReplaceSelection)(LPEDITINTERFACE lpInterface, LPCTSTR pszNewText);
	POINT (*GetLastChangePos)(LPEDITINTERFACE lpInterface);
	void (*EnsureVisible)(LPEDITINTERFACE lpInterface, POINT pt);

	/* buffer handling */
	int (*GetLineCount)(LPEDITINTERFACE lpInterface);
	int (*GetLineLength)(LPEDITINTERFACE lpInterface, int nLineIndex);
	LPCTSTR (*GetLineChars)(LPEDITINTERFACE lpInterface, int nLineIndex);
	void (*GetText)(LPEDITINTERFACE lpInterface, POINT ptStart, POINT ptEnd, LPTSTR *pszText);
	BOOL (*InsertText)(LPEDITINTERFACE lpInterface, int nLine, int nPos, LPCTSTR pszText, int *nEndLine, int *nEndPos, int nAction);
	BOOL (*DeleteText)(LPEDITINTERFACE lpInterface, int nStartLine, int nStartPos, int nEndLine, int nEndPos, int nAction);
	BOOL (*InsertTextPos)(LPEDITINTERFACE lpInterface, int nPos, LPCTSTR pszText, int nAction);
	BOOL (*DeleteTextPos)(LPEDITINTERFACE lpInterface, int nStartPos, int nEndPos, int nAction);
	void (*SetLineFlag)(LPEDITINTERFACE lpInterface, int iLine, DWORD dwFlag, BOOL bSet, BOOL bRemoveFromPreviousLine);
	DWORD (*GetLineFlags)(LPEDITINTERFACE lpInterface, int iLine);
	int (*GetLineWithFlag)(LPEDITINTERFACE lpInterface, DWORD dwFlag);

	/* font */
	HFONT (*GetFont)(LPEDITINTERFACE lpInterface);
	void (*SetFont)(LPEDITINTERFACE lpInterface, LPLOGFONT lplf);

	/* searching */
	BOOL (*FindText)(LPEDITINTERFACE lpInterface, LPCTSTR pszText, POINT ptStartPos, DWORD dwFlags, LPPOINT lpptFoundPos);
	BOOL (*FindTextInBlock)(LPEDITINTERFACE lpInterface, LPCTSTR pszText, POINT ptStartPos, POINT ptBlockBegin, POINT ptBlockEnd, DWORD dwFlags, LPPOINT lpptFoundPos);
	BOOL (*FindReplaceSelection)(LPEDITINTERFACE lpInterface, LPCTSTR pszNewText, DWORD dwFlags);
	BOOL (*FindTextVisual)(LPEDITINTERFACE lpInterface, LPCTSTR pszText, POINT ptStartPos, DWORD dwFlags, LPPOINT lpptFoundPos);
	BOOL (*FindNext)(LPEDITINTERFACE lpInterface, LPPOINT lpptFoundPos);
	BOOL (*FindPrevious)(LPEDITINTERFACE lpInterface, LPPOINT lpptFoundPos);
	BOOL (*GetSearchInfo)(LPEDITINTERFACE lpInterface, LPEDITSEARCHINFO lpSearchInfo);
	BOOL (*IsIncrementalSearchActive)(LPEDITINTERFACE lpInterface);
	void (*IncrementalSearchStart)(LPEDITINTERFACE lpInterface, BOOL bForward);
	LPCTSTR (*IncrementalSearchAddChar)(LPEDITINTERFACE lpInterface, TCHAR cChar);
	LPCTSTR (*IncrementalSearchRemoveChar)(LPEDITINTERFACE lpInterface);
	void (*IncrementalSearchEnd)(LPEDITINTERFACE lpInterface, BOOL bReturnToOldPlace);
	BOOL (*FindSelection)(LPEDITINTERFACE lpInterface, BOOL fForward, LPPOINT lpptFoundPos)

	/* file-i/o */
	BOOL (*LoadFile)(LPEDITINTERFACE lpInterface, LPCTSTR pszFileName);
	BOOL (*SaveToFile)(LPEDITINTERFACE lpInterface, LPCTSTR pszFileName);

	/* UI-Toggles */
	BOOL (*GetDisableDragAndDrop)(LPEDITINTERFACE lpInterface);
	void (*SetDisableDragAndDrop)(LPEDITINTERFACE lpInterface, BOOL bDisable);

	/* bookmarks */
	void (*ToggleBookmark)(LPEDITINTERFACE lpInterface);
	void (*NextBookmark)(LPEDITINTERFACE lpInterface);
	void (*PreviousBookmark)(LPEDITINTERFACE lpInterface);
	void (*ClearAllBookmarks)(LPEDITINTERFACE lpInterface);

	/* undo/redo */
	void (*BeginUndoGroup)(LPEDITINTERFACE lpInterface);
	void (*FlushUndoGroup)(LPEDITINTERFACE lpInterface);
	int (*GetUndoDescription)(LPEDITINTERFACE lpInterface, LPTSTR pszDesc, int nPos);
	int (*GetRedoDescription)(LPEDITINTERFACE lpInterface, LPTSTR pszDesc, int nPos);
	BOOL (*CanUndo)(LPEDITINTERFACE lpInterface);
	BOOL (*CanRedo)(LPEDITINTERFACE lpInterface);
	void (*Undo)(LPEDITINTERFACE lpInterface);
	void (*Redo)(LPEDITINTERFACE lpInterface);
	void (*SetModified)(LPEDITINTERFACE lpInterface, BOOL bModified);
	BOOL (*GetModified)(LPEDITINTERFACE lpInterface);

	/* other */
	int (*GetLastActionDescription)(LPEDITINTERFACE lpInterface);
	void (*AssertValidTextPos)(LPEDITINTERFACE lpInterface, POINT pt);
	BOOL (*DeleteCurrentSelection)(LPEDITINTERFACE lpInterface);
	BOOL (*GetReadOnly)(LPEDITINTERFACE lpInterface);
	void (*SetReadOnly)(LPEDITINTERFACE lpInterface, BOOL bReadOnly);
} EDITVTBL, *LPEDITVTBL;

typedef struct tagEDITINTERFACE
{
	LPEDITVTBL	lpVtbl;
	LPARAM		lpUserData;
} EDITINTERFACE, *LPEDITINTERFACE;

/* macros for nice access of the V-Table in C */

/* Selection, anchor, cursor */
#define PCP_Edit_IsSelection(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->IsSelection((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_GetSelection(lpInterface, lpptStart, lpptEnd) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetSelection((LPEDITINTERFACE)(lpInterface), (LPPOINT)(lpptStart), (LPPOINT)(lpptEnd)))

#define PCP_Edit_SetSelection(lpInterface, ptStart, ptEnd) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->SetSelection((LPEDITINTERFACE)(lpInterface), (POINT)(ptStart), (POINT)(ptEnd)))

#define PCP_Edit_GetSelectionPos(lpInterface, lpnStartPos, lpnEndPos) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetSelectionPos((LPEDITINTERFACE)(lpInterface), (LPINT)(lpnStartPos), (LPINT)(lpnEndPos)))

#define PCP_Edit_SetSelectionPos(lpInterface, nStartPos, nEndPos) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->SetSelectionPos((LPEDITINTERFACE)(lpInterface), (int)(nStartPos), (int)(nEndPos)))

#define PCP_Edit_GetSelectionMode(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetSelectionMode((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_SetSelectionMode(lpInterface, nMode) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->SetSelectionMode((LPEDITINTERFACE)(lpInterface), (int)(nMode)))

#define PCP_Edit_HighlightText(lpInterface, ptStartPos, nLength, bReverse) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->HighlightText((LPEDITINTERFACE)(lpInterface), (POINT)(ptStartPos), (int)(nLength)))

#define PCP_Edit_SetAnchor(lpInterface, ptNewAnchor) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->SetAnchor((LPEDITINTERFACE)(lpInterface), (POINT)(ptNewAnchor)))

#define PCP_Edit_GetCursorPos(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetCursorPos((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_SetCursorPos(lpInterface, ptCursorPos) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->SetCursorPos((LPEDITINTERFACE)(lpInterface), (POINT)(ptCursorPos)))

#define PCP_Edit_GotoLine(lpInterface, nLineIndex, bExtendSelection) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GotoLine((LPEDITINTERFACE)(lpInterface), (int)(nLineIndex), (BOOL)(bExtendSelection)))

#define PCP_Edit_ReplaceSelection(lpInterface, pszNewText) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->ReplaceSelection((LPEDITINTERFACE)(lpInterface), (LPCTSTR)(pszNewText)))

#define PCP_Edit_GetLastChangePos(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetLastChangePos((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_EnsureVisible(lpInterface, pt) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->EnsureVisible((LPEDITINTERFACE)(lpInterface), (POINT)(pt)))

/* buffer handling */
#define PCP_Edit_GetLineCount(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetLineCount((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_GetLineLength(lpInterface, nLineIndex) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetLineLength((LPEDITINTERFACE)(lpInterface), (int)(nLineIndex)))

#define PCP_Edit_GetLineChars(lpInterface, nLineIndex) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetLineChars((LPEDITINTERFACE)(lpInterface), (int)(nLineIndex)))

#define PCP_Edit_GetText(lpInterface, ptStart, ptEnd, *pszText) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetText((LPEDITINTERFACE)(lpInterface), (POINT)(ptStart), (POINT)(ptEnd)))

#define PCP_Edit_InsertText(lpInterface, nLine, nPos, pszText, *nEndLine, int *nEndPos, int nAction) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->InsertText((LPEDITINTERFACE)(lpInterface), (int)(nLine), (int)(nPos)))

#define PCP_Edit_DeleteText(lpInterface, nStartLine, nStartPos, nEndLine, nEndPos, nAction) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->DeleteText((LPEDITINTERFACE)(lpInterface), (int)(nStartLine), (int)(nStartPos)))

#define PCP_Edit_InsertTextPos(lpInterface, nPos, pszText, nAction) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->InsertTextPos((LPEDITINTERFACE)(lpInterface), (int)(nPos), (LPCTSTR)(pszText)))

#define PCP_Edit_DeleteTextPos(lpInterface, nStartPos, nEndPos, nAction) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->DeleteTextPos((LPEDITINTERFACE)(lpInterface), (int)(nStartPos), (int)(nEndPos)))

#define PCP_Edit_SetLineFlag(lpInterface, iLine, dwFlag, bSet, bRemoveFromPreviousLine) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->SetLineFlag((LPEDITINTERFACE)(lpInterface), (int)(iLine), (DWORD)(dwFlag), (BOOL)(bSet), (BOOL)(bRemoveFromPreviousLine)))

#define PCP_Edit_GetLineFlags(lpInterface, iLine) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetLineFlags((LPEDITINTERFACE)(lpInterface), (int)(iLine)))

#define PCP_Edit_GetLineWithFlag(lpInterface, dwFlag) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetLineWithFlag((LPEDITINTERFACE)(lpInterface), (DWORD)(dwFlag)))

/* font */
#define PCP_Edit_GetFont(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetFont((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_SetFont(lpInterface, lplf) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->SetFont((LPEDITINTERFACE)(lpInterface), (LPLOGFONT)(lplf)))

/* searching */
#define PCP_Edit_FindText(lpInterface, pszText, ptStartPos, dwFlags, bWrapSearch, lpptFoundPos) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->FindText((LPEDITINTERFACE)(lpInterface), (LPCTSTR)(pszText), (POINT)(ptStartPos), (DWORD)(dwFlags), (BOOL)(bWrapSearch), (LPPOINT)(lpptFoundPos)))

#define PCP_Edit_FindTextInBlock(lpInterface, pszText, ptStartPos, ptBlockBegin, ptBlockEnd, dwFlags, bWrapSearch, lpptFoundPos) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->FindTextInBlock((LPEDITINTERFACE)(lpInterface), (LPCTSTR)(pszText), (POINT)(ptStartPos), (POINT)(ptBlockBegin), (POINT)(ptBlockEnd), (DWORD)(dwFlags), (BOOL)(bWrapSearch), (LPPOINT)(lpptFoundPos)))

#define PCP_Edit_FindReplaceSelection(lpInterface, pszNewText, dwFlags) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->FindReplaceSelection((LPEDITINTERFACE)(lpInterface), (LPCTSTR)(pszNewText), (DWORD)(dwFlags)))

#define PCP_Edit_FindTextVisual(lpInterface, pszText, ptStartPos, dwFlags, lpptFoundPos) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->FindTextVisual((LPEDITINTERFACE)(lpInterface), (LPCTSTR)(pszText), (POINT)(ptStartPos), (DWORD)(dwFlags), (LPPOINT)(lpptFoundPos)))

#define PCP_Edit_FindNext(lpInterface, lpptFoundPos) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->FindNext((LPEDITINTERFACE)(lpInterface), (LPPOINT)(lpptFoundPos)))

#define PCP_Edit_FindPrevious(lpInterface, lpptFoundPos) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->FindPrevious((LPEDITINTERFACE)(lpInterface), (LPPOINT)(lpptFoundPos)))

#define PCP_Edit_GetSearchInfo(lpInterface, lpSearchInfo) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetSearchInfo((LPEDITINTERFACE)(lpInterface), (LPEDITSEARCHINFO)(lpSearchInfo)))

#define PCP_Edit_IsIncrementalSearchActive(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->IsIncrementalSearchActive((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_IncrementalSearchStart(lpInterface, bForward) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->IncrementalSearchStart((LPEDITINTERFACE)(lpInterface), (BOOL)(bForward)))

#define PCP_Edit_IncrementalSearchAddChar(lpInterface, cChar) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->IncrementalSearchAddChar((LPEDITINTERFACE)(lpInterface), (TCHAR)(cChar)))

#define PCP_Edit_IncrementalSearchRemoveChar(pInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->IncrementalSearchRemoveChar((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_IncrementalSearchEnd(pInterface, bReturnToOldPlace) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->IncrementalSearchEnd((LPEDITINTERFACE)(lpInterface), (BOOL)(bReturnToOldPlace)))

#define PCP_Edit_FindSelection(lpInterface, fForward, lpptFoundPos \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->FindSelection((LPEDITINTERFACE)(lpInterface), (BOOL)(fForward), (LPPOINT)(lpptFoundPos)))

/* file-i/o */
#define PCP_Edit_LoadFile(lpInterface, pszFileName) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->LoadFile((LPEDITINTERFACE)(lpInterface), (LPCTSTR)(pszFileName)))

#define PCP_Edit_SaveToFile(lpInterface, pszFileName) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->SaveToFile((LPEDITINTERFACE)(lpInterface), (LPCTSTR)(pszFileName)))

/* UI-Toggles */
#define PCP_Edit_GetDisableDragAndDrop(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetDisableDragAndDrop((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_SetDisableDragAndDrop(lpInterface, bDisable) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->SetDisableDragAndDrop((LPEDITINTERFACE)(lpInterface), (BOOL)(bDisable)))

/* bookmarks */
#define PCP_Edit_ToggleBookmark(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->ToggleBookmark((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_NextBookmark(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->NextBookmark((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_PreviousBookmark(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->PreviousBookmark((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_ClearAllBookmarks(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->ClearAllBookmarks((LPEDITINTERFACE)(lpInterface)))

/* undo/redo */
#define PCP_Edit_BeginUndoGroup(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->BeginUndoGroup((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_FlushUndoGroup(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->FlushUndoGroup((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_GetUndoDescription(lpInterface, pszDesc, nPos) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetUndoDescription((LPEDITINTERFACE)(lpInterface), (LPTSTR)(pszDesc), (int)(nPos)))

#define PCP_Edit_GetRedoDescription(lpInterface, pszDesc, nPos) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetRedoDescription((LPEDITINTERFACE)(lpInterface), (LPTSTR)(pszDesc), (int)(nPos)))

#define PCP_Edit_CanUndo(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->CanUndo((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_CanRedo(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->CanRedo((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_Undo(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->Undo((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_Redo(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->Redo((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_SetModified(lpInterface, bModified) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->SetModified((LPEDITINTERFACE)(lpInterface), (BOOL)(bModified)))

#define PCP_Edit_GetModified(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetModified((LPEDITINTERFACE)(lpInterface)))

/* other */
#define PCP_Edit_GetLastActionDescription(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetLastActionDescription((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_AssertValidTextPos(lpInterface, pt) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->AssertValidTextPos((LPEDITINTERFACE)(lpInterface), (POINT)(pt)))

#define PCP_Edit_DeleteCurrentSelection(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->DeleteCurrentSelection((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_GetReadOnly(lpInterface) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->GetReadOnly((LPEDITINTERFACE)(lpInterface)))

#define PCP_Edit_SetReadOnly(lpInterface, bReadOnly) \
	((LPEDITINTERFACE)(lpInterface)->lpVtbl->SetReadOnly((LPEDITINTERFACE)(lpInterface), (BOOL)(bReadOnly)))


/* actions enum used for PCP_Edit_GetActionDescription among others */

enum tagEDITACTIONS
{
	PE_ACTION_UNKNOWN			= 0,
	PE_ACTION_PASTE				= 1,
	PE_ACTION_DELSEL			= 2,
	PE_ACTION_CUT				= 3,
	PE_ACTION_TYPING			= 4,
	PE_ACTION_BACKSPACE			= 5,
	PE_ACTION_INDENT			= 6,
	PE_ACTION_DRAGDROP			= 7,
	PE_ACTION_REPLACE			= 8,
	PE_ACTION_DELETE			= 9,
	PE_ACTION_AUTOINDENT		= 10,
	PE_ACTION_AUTOCOMPLETE		= 11,
	PE_ACTION_INTERNALINSERT	= 12
	//	...
	//	Expandable: user actions allowed
} EDITACTIONS, *LPEDITACTIONS;

/* enum values used for Get/SetSelectioMode */
enum tagSELECTIONMODES
{
	PE_SELECTION_MODE_NONE,
	PE_SELECTION_MODE_CHAR,
	PE_SELECTION_MODE_LINE,
	PE_SELECTION_MODE_BLOCK
} SELECTIONMODES, *LPSELECTIONMODES;

/* Notifications */
#define PEN_FIRST					(31337)
#define PEN_SELCHANGE				(PEN_FIRST + 0)
#define PEN_OVERWRITEMODECHANGED	(PEN_FIRST + 1)

/* search flags */

enum tagSEARCHFLAGS
{
	PE_FIND_MATCH_CASE		= 0x0001,
	PE_FIND_WHOLE_WORD		= 0x0002,
	PE_FIND_REG_EXP			= 0x0004,
	PE_FIND_DIRECTION_UP	= 0x0008,
	PE_FIND_WRAP_SEARCH		= 0x0010,
	PE_REPLACE_SELECTION	= 0x0100
} SEARCHFLAGS;

/* search info struct */
typedef struct tagEDITSEARCHINFO
{
	BOOL	bLastSearchSuccessful;
	LPCTSTR	pszSearchString;
	LPCTSTR	pszMatchedLine;
	int		nLastSearchLen;
	int		nLastReplaceLen;
	DWORD	dwLastSearchFlags;
	BOOL	bIncrementalSearchActive;
} EDITSEARCHINFO, *LPEDITSEARCHINFO;

/* exported functions */

FOREXPORT LPEDITINTERFACE PCP_Edit_GetInterface(HWND hwnd);
FOREXPORT BOOL PCP_Edit_GetActionDescription(int nAction, LPTSTR pszDesc);

#endif /* __PCP_EDIT_H */
