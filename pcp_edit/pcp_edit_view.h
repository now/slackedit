/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_view.h
 * Created    : not known (before 01/24/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:42:00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_EDIT_VIEW_H
#define __PCP_EDIT_VIEW_H

/* windows */
#include <commctrl.h>

/* pcp_generic */
#include <pcp_dpa.h>
#include <pcp_regexp.h>

/* pcp_edit */
#include "pcp_edit.h"

typedef struct tagEDITSTATE
{
	BOOL	bInit;
	BOOL	bCursorHidden;
	BOOL	bReadOnly;
	BOOL	bReadOnlyButWriteable;
	BOOL	bModified;
	BOOL	bUndoBeginGroup;
	BOOL	bUndoGroup;
	POINT	ptLastChange;
	BITMAP	bmCache;
	int		iCRLFMode;
	int		nUndoPosition;
	int		nSyncPosition;
	int		nUndoBufSize;
	int		nTabSize;
	int		nLastUndoDescription;
	HDPA	hdpaLines;
	HDPA	hdpaUndos;
} EDITSTATE, FAR *LPEDITSTATE;

typedef struct tagEDITVIEW
{
	HWND		hwnd;
	BOOL		bSelMargin;
	BOOL		bSmoothScroll;
	BOOL		bViewTabs;
	BOOL		bFocused;
	BOOL		bShowInactiveSelection;
	BOOL		bDisableDragAndDrop;
	BOOL		bDragSelection;
	BOOL		bDraggingText;
	BOOL		bVertScrollBarLocked;
	BOOL		bHorzScrollBarLocked;
	BOOL		bLastSearch;
	BOOL		bPrintHeader;
	BOOL		bPrintFooter;
	BOOL		bBookmarkExist;
	BOOL		bMultipleSearch;
	BOOL		bCursorHidden;
	BOOL		bWordSelection;
	BOOL		bLineSelection;
	BOOL		bPreparingToDrag;
	BOOL		bOvrMode;
	BOOL		bIncrementalSearchBackwards;
	BOOL		bIncrementalSearchForwards;
	BOOL		bIncrementalFound;
	BOOL		bSelectCharMode;
	BOOL		bSelectLineMode;
	LOGFONT		lfBaseFont;
	POINT		ptSelStart;
	POINT		ptSelEnd;
	POINT		ptDrawSelStart;
	POINT		ptDrawSelEnd;
	POINT		ptAnchor;
	POINT		ptCursorPos;
	POINT		ptDraggedTextBegin;
	POINT		ptDraggedTextEnd;
	POINT		ptIncrementalSearchStartPos;
	POINT		ptCursorPosBeforeIncrementalSearch;
	POINT		ptSelStartBeforeIncrementalSearch;
	POINT		ptSelEndBeforeIncrementalSearch;
	LPINT		piActualLineLength;
	INT			iActualLengthArraySize;
	INT			iOffsetChar;
	INT			iTopLine;
	INT			iLineHeight;
	INT			iCharWidth;
	INT			iTabSize;
	INT			iMaxLineLength;
	INT			iScreenLines;
	INT			iScreenChars;
	INT			iIdealCharPos;
	INT			iLastFindWhatLen;
	INT			iLastReplaceLen;
	RxNode		*rxNode;
	RxMatchRes	rxMatch;
	LPTSTR		pszMatched;
	LPTSTR		pszLastFindWhat;
	LPTSTR		pszIncrementalSearch;
	LPTSTR		pszOldIncrementalSearch;
	PDWORD		pdwParseCookies;
	DWORD		dwFlags;
	DWORD		dwLastSearchFlags;
	INT			iParseArraySize;
	UINT		uDragSelTimer;
	LPEDITSTATE	lpes;
	HIMAGELIST	hilIcons;
	HBITMAP		*pCacheBitmap;
	HFONT		ahFonts[4];
	HCURSOR		hCursor;
	LPTEXTDEF	lpTextDef;
} EDITVIEW, FAR *LPEDITVIEW;

enum
{
	UPDATE_HORZRANGE	= 0x0001,
	UPDATE_VERTRANGE	= 0x0002,
	UPDATE_SINGLELINE	= 0x0100,
	UPDATE_FLAGSONLY	= 0x0200,

	UPDATE_RESET		= 0x1000
};

/* internal */
BOOL Edit_View_IsInsideSelBlock(LPEDITVIEW lpew, POINT ptTextPos);
BOOL Edit_View_IsInsideSelection(LPEDITVIEW lpew, const POINT ptTextPos);
void Edit_View_UpdateCaret(LPEDITVIEW lpew);
int Edit_View_GetScreenLines(LPEDITVIEW lpew);
void Edit_View_AdjustTextPoint(LPEDITVIEW lpew, POINT *lppt);
int Edit_View_CalculateActualOffset(LPEDITVIEW lpew, int iLineIndex, int iCharIndex);
int Edit_View_ApproxActualOffset(LPEDITVIEW lpew, int iLineIndex, int iOffset);
void Edit_View_UpdateView(LPEDITVIEW lpew, LPINSERTCONTEXT pContext, DWORD dwFlags, int iLineIndex);

/* interfaced */
void Edit_View_GetSelection(LPEDITVIEW lpew, LPPOINT lptStart, LPPOINT lptEnd);
int Edit_View_GetLineCount(LPEDITVIEW lpew);
int Edit_View_GetLineLength(LPEDITVIEW lpew, int iLineIndex);
LPCTSTR Edit_View_GetLineChars(LPEDITVIEW lpew, int iLineIndex);
void Edit_View_SetSelection(LPEDITVIEW lpew, POINT ptStart, POINT ptEnd);
void Edit_View_EnsureVisible(LPEDITVIEW lpew, POINT pt);
void Edit_View_GetText(LPEDITVIEW lpew, POINT ptStart, POINT ptEnd, LPTSTR *pszText);
void Edit_View_SetAnchor(LPEDITVIEW lpew, POINT ptNewAnchor);
POINT Edit_View_GetCursorPos(LPEDITVIEW lpew);
void Edit_View_SetCursorPos(LPEDITVIEW lpew, POINT lpptCursorPos);
HFONT Edit_View_GetFont(LPEDITVIEW lpew, BOOL bItalic, BOOL bBold);
void Edit_View_SetFont(LPEDITVIEW lpew, LPLOGFONT lf);
BOOL Edit_View_HighlightText(LPEDITVIEW lpew, POINT ptStartPos, int iLength, BOOL bReverse);
BOOL Edit_View_FindText(LPEDITVIEW lpew, LPCTSTR pszText, POINT ptStartPos, DWORD dwFlags, BOOL bWrapSearch, POINT *lpptFoundPos);
BOOL Edit_View_FindTextInBlock(LPEDITVIEW lpew, LPCTSTR pszText, POINT ptStartPosition,
							POINT ptBlockBegin, POINT ptBlockEnd,
							DWORD dwFlags, BOOL bWrapSearch, POINT *lpptFoundPos);
BOOL Edit_View_GetViewTabs(LPEDITVIEW lpew);
void Edit_View_SetViewTabs(LPEDITVIEW lpew, BOOL bViewTabs);
BOOL Edit_View_GetSelectionMargin(LPEDITVIEW lpew);
void Edit_View_SetSelectionMargin(LPEDITVIEW lpew, BOOL bSelMargin);
BOOL Edit_View_GetSmoothScroll(LPEDITVIEW lpew);
void Edit_View_SetSmoothScroll(LPEDITVIEW lpew, BOOL bSmoothScroll);
BOOL Edit_View_GetDisableDragAndDrop(LPEDITVIEW lpew);
void Edit_View_SetDisableDragAndDrop(LPEDITVIEW lpew, BOOL bDDAD);
BOOL Edit_View_FindReplaceSelection(LPEDITVIEW lpew, LPCTSTR pszNewText, DWORD dwFlags);
BOOL Edit_View_SetTextType(LPEDITVIEW lpew, LPCTSTR pszExt);
void Edit_View_LoadFile(LPEDITVIEW lpew, LPCTSTR pszFileName);
void Edit_View_OnClearBookmarks(LPEDITVIEW lpew);
void Edit_View_OnToggleBookmark(LPEDITVIEW lpew);
void Edit_View_OnNextBookmark(LPEDITVIEW lpew);
void Edit_View_OnPrevBookmark(LPEDITVIEW lpew);
void Edit_View_OnClearAllBookmarks(LPEDITVIEW lpew);
void Edit_View_FindIncremental(LPEDITVIEW lpew, BOOL bFindNext);
void Edit_View_FindIncrementalNext(LPEDITVIEW lpew, BOOL bForward);
void Edit_View_GotoLastChange(LPEDITVIEW lpew);
BOOL Edit_View_GetSelectionXY(LPEDITVIEW lpew, LPDWORD lpdwStartPos, LPDWORD lpdwEndPos);
BOOL Edit_View_SetSelectionXY(LPEDITVIEW lpew, DWORD dwStartPos, DWORD dwEndPos);
BOOL Edit_View_IsSelection(LPEDITVIEW lpew);
HCURSOR Edit_View_SetMarginCursor(LPEDITVIEW lpew, HCURSOR hCursor);
HCURSOR Edit_View_GetMarginCursor(LPEDITVIEW lpew);
void Edit_View_AssertValidTextPos(LPEDITVIEW lpew, POINT pt);
void Edit_View_ReplaceSelection(LPEDITVIEW lpew, LPTSTR pszText);
BOOL Edit_View_DeleteCurrentSelection(LPEDITVIEW lpew);
BOOL Edit_View_GotoLine(LPEDITVIEW lpew, int nLine, BOOL bExtendSelection);

void Edit_View_OnAutoComplete(LPEDITVIEW lpew);
POINT Edit_View_OnMatchBrace(LPEDITVIEW lpew, POINT ptStartPos);
BOOL Edit_View_OnFindPrev(LPEDITVIEW lpew);
BOOL Edit_View_OnFindNext(LPEDITVIEW lpew);
BOOL Edit_View_FindSelection(LPEDITVIEW lpew, BOOL fForward);
void Edit_View_OnTab(LPEDITVIEW lpew);
void Edit_View_OnEditUntab(LPEDITVIEW lpew);
void Edit_View_OnUndo(LPEDITVIEW lpew);
void Edit_View_OnRedo(LPEDITVIEW lpew);
void Edit_View_OnDelete(LPEDITVIEW lpew);

#ifdef _DEBUG
#define ASSERT_VALIDTEXTPOS(lpew, pt)	TextView_AssertValidTextPos(lpew, pt);
#else
#define ASSERT_VALIDTEXTPOS(lpew, pt)
#endif

#endif /* __PCP_EDIT_VIEW_H */
