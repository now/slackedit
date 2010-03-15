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

#ifndef __PCP_TextEdit_View_H
#define __PCP_TextEdit_View_H

/* windows */
#include <commctrl.h>

/* pcp_generic */
#include <pcp_dpa.h>

/* pcp_edit */
#include "pcp_textedit.h" /* For: LPSYNTAXDEFINITION */
#include "pcp_textedit_internal.h" /* For: LPINSERTCONTEXT */

typedef struct tagTEXTEDITSTATE
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
} TEXTEDITSTATE, *LPTEXTEDITSTATE;

typedef struct tagTEXTEDITVIEW
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
	BOOL		bPrintHeader;
	BOOL		bPrintFooter;
	BOOL		bBookmarkExist;
	BOOL		bCursorHidden;
	BOOL		bWordSelection;
	BOOL		bLineSelection;
	BOOL		bPreparingToDrag;
	BOOL		bOvrMode;
	LOGFONT		lfBaseFont;
	POINT		ptSelStart;
	POINT		ptSelEnd;
	POINT		ptDrawSelStart;
	POINT		ptDrawSelEnd;
	POINT		ptAnchor;
	POINT		ptCursorPos;
	POINT		ptDraggedTextBegin;
	POINT		ptDraggedTextEnd;
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
	INT			nSelectionMode;
	PDWORD		pdwParseCookies;
	DWORD		dwFlags;
	INT			iParseArraySize;
	UINT		uDragSelTimer;
	LPTEXTEDITSTATE	lpes;
	HIMAGELIST	hilIcons;
	HBITMAP		*pCacheBitmap;
	HFONT		ahFonts[4];
	HCURSOR		hCursor;
	LPSYNTAXDEFINITION	lpsd;
} TEXTEDITVIEW, *LPTEXTEDITVIEW;

enum
{
	UPDATE_HORZRANGE	= 0x0001,
	UPDATE_VERTRANGE	= 0x0002,
	UPDATE_SINGLELINE	= 0x0100,
	UPDATE_FLAGSONLY	= 0x0200,

	UPDATE_RESET		= 0x1000
};

/* internal */
LRESULT CALLBACK TextEdit_View_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL TextEdit_View_IsInsideSelBlock(LPTEXTEDITVIEW lpte, POINT ptTextPos);
BOOL TextEdit_View_IsInsideSelection(LPTEXTEDITVIEW lpte, const POINT ptTextPos);
void TextEdit_View_UpdateCaret(LPTEXTEDITVIEW lpte);
int TextEdit_View_GetScreenLines(LPTEXTEDITVIEW lpte);
void TextEdit_View_AdjustTextPoint(LPTEXTEDITVIEW lpte, POINT *lppt);
int TextEdit_View_ApproxActualOffset(LPTEXTEDITVIEW lpte, int iLineIndex, int iOffset);
void TextEdit_View_UpdateView(LPTEXTEDITVIEW lpte, LPINSERTCONTEXT pContext, DWORD dwFlags, int iLineIndex);
void TextEdit_View_OnEditOperation(LPTEXTEDITVIEW lpte, int nAction, LPCTSTR pszText);
void TextEdit_View_PrepareSelBounds(LPTEXTEDITVIEW lpte);
void TextEdit_View_ScrollToLine(LPTEXTEDITVIEW lpte, int iNewTopLine, BOOL bNoSmoothScroll, BOOL bTrackScrollBar);
void TextEdit_View_ScrollToChar(LPTEXTEDITVIEW lpte, int iNewOffsetChar, BOOL bNoSmoothScroll, BOOL bTrackScrollBar /*= TRUE*/);
int TextEdit_View_GetMaxLineLength(LPTEXTEDITVIEW lpte);
int TextEdit_View_GetMarginWidth(LPTEXTEDITVIEW lpte);
POINT TextEdit_View_ClientToText(LPTEXTEDITVIEW lpte, POINT pt);
int TextEdit_View_GetLineHeight(LPTEXTEDITVIEW lpte);

/* interfaced */

/* selection */
void TextEdit_View_GetSelection(LPTEXTEDITVIEW lpte, LPPOINT lptStart, LPPOINT lptEnd);
void TextEdit_View_SetSelection(LPTEXTEDITVIEW lpte, POINT ptStart, POINT ptEnd);
BOOL TextEdit_View_GetSelectionPos(LPTEXTEDITVIEW lpte, LPDWORD lpdwStartPos, LPDWORD lpdwEndPos);
BOOL TextEdit_View_SetSelectionPos(LPTEXTEDITVIEW lpte, DWORD dwStartPos, DWORD dwEndPos);
void TextEdit_View_EnsureVisible(LPTEXTEDITVIEW lpte, POINT pt);
void TextEdit_View_SetAnchor(LPTEXTEDITVIEW lpte, POINT ptNewAnchor);
POINT TextEdit_View_GetCursorPos(LPTEXTEDITVIEW lpte);
void TextEdit_View_SetCursorPos(LPTEXTEDITVIEW lpte, POINT lpptCursorPos);

/* buffer handling */
int TextEdit_View_GetLineCount(LPTEXTEDITVIEW lpte);
int TextEdit_View_GetLineLength(LPTEXTEDITVIEW lpte, int iLineIndex);
LPCTSTR TextEdit_View_GetLineChars(LPTEXTEDITVIEW lpte, int iLineIndex);
void TextEdit_View_GetText(LPTEXTEDITVIEW lpte, POINT ptStart, POINT ptEnd, LPTSTR *pszText);
int TextEdit_View_GetLineActualLength(LPTEXTEDITVIEW lpte, int iLineIndex);
int TextEdit_View_CalculateActualOffset(LPTEXTEDITVIEW lpte, int iLineIndex, int iCharIndex);

/* font */
HFONT TextEdit_View_GetFont(LPTEXTEDITVIEW lpte, BOOL bItalic, BOOL bBold);
void TextEdit_View_SetFont(LPTEXTEDITVIEW lpte, LPLOGFONT lf);

/* searching */
BOOL TextEdit_View_HighlightText(LPTEXTEDITVIEW lpte, POINT ptStartPos, int iLength, BOOL bReverse);

/* UI-Toggles */
BOOL TextEdit_View_GetViewTabs(LPTEXTEDITVIEW lpte);
void TextEdit_View_SetViewTabs(LPTEXTEDITVIEW lpte, BOOL bViewTabs);
BOOL TextEdit_View_GetSelectionMargin(LPTEXTEDITVIEW lpte);
void TextEdit_View_SetSelectionMargin(LPTEXTEDITVIEW lpte, BOOL bSelMargin);
BOOL TextEdit_View_GetSmoothScroll(LPTEXTEDITVIEW lpte);
void TextEdit_View_SetSmoothScroll(LPTEXTEDITVIEW lpte, BOOL bSmoothScroll);
BOOL TextEdit_View_GetDisableDragAndDrop(LPTEXTEDITVIEW lpte);
void TextEdit_View_SetDisableDragAndDrop(LPTEXTEDITVIEW lpte, BOOL bDDAD);

/* Bookmarks */
void TextEdit_View_ClearBookmarks(LPTEXTEDITVIEW lpte);
void TextEdit_View_ToggleBookmark(LPTEXTEDITVIEW lpte);
void TextEdit_View_NextBookmark(LPTEXTEDITVIEW lpte);
void TextEdit_View_PrevBookmark(LPTEXTEDITVIEW lpte);
void TextEdit_View_ClearAllBookmarks(LPTEXTEDITVIEW lpte);
void TextView_GotoNumBookmark(LPTEXTEDITVIEW lpte, int nBookmarkID)
void TextView_ToggleNumBookmark(LPTEXTEDITVIEW lpte, int nBookmarkID)
void TextEdit_View_ClearAllNumBookmarks(LPTEXTEDITVIEW lpte)

/* other */
void TextEdit_View_SetTextType(LPTEXTEDITVIEW lpte, LPCTSTR pszExt);
void TextEdit_View_GotoLastChange(LPTEXTEDITVIEW lpte);
BOOL TextEdit_View_IsSelection(LPTEXTEDITVIEW lpte);
HCURSOR TextEdit_View_SetMarginCursor(LPTEXTEDITVIEW lpte, HCURSOR hCursor);
HCURSOR TextEdit_View_GetMarginCursor(LPTEXTEDITVIEW lpte);
void TextEdit_View_AssertValidTextPos(LPTEXTEDITVIEW lpte, POINT pt);
void TextEdit_View_ReplaceSelection(LPTEXTEDITVIEW lpte, LPCTSTR pszText);
BOOL TextEdit_View_DeleteCurrentSelection(LPTEXTEDITVIEW lpte);
BOOL TextEdit_View_GotoLine(LPTEXTEDITVIEW lpte, int nLine, BOOL bExtendSelection);
POINT TextEdit_View_MatchBrace(LPTEXTEDITVIEW lpte, POINT ptStartPos);
void TextEdit_View_Tab(LPTEXTEDITVIEW lpte);
void TextEdit_View_Untab(LPTEXTEDITVIEW lpte);
void TextEdit_View_Undo(LPTEXTEDITVIEW lpte);
void TextEdit_View_Redo(LPTEXTEDITVIEW lpte);
void TextEdit_View_OnDelete(LPTEXTEDITVIEW lpte);

#ifdef _DEBUG
#define ASSERT_VALIDTEXTPOS(lpte, pt)	TextEdit_View_AssertValidTextPos(lpte, pt);
#else
#define ASSERT_VALIDTEXTPOS(lpte, pt)
#endif

#endif /* __PCP_TextEdit_View_H */
