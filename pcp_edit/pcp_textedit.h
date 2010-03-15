/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_textedit.h
 * Created    : 07/23/00
 * Owner      : pcppopper
 * Revised on : 07/23/00
 * Comments   : 
 *              
 *              
 ****************************************************************/

#ifndef __PCP_TEXTEDIT_H
#define __PCP_TEXTEDIT_H

#include "pcp_edit_dll.h" /* For: FOREXPORT */
#include "pcp_edit.h" /* For: LPEDITVTBL */

#define WC_PCP_TEXTEDIT _T("PCP_Edit_TextEditCtrl")

#define DEFAULT_PRINT_MARGIN		1000	//10 Millimeters

#define TE_MAX_LINE_LENGTH			2048

enum
{
	//	Base colors
	COLORINDEX_WHITESPACE,
	COLORINDEX_BKGND,
	COLORINDEX_NORMALTEXT,
	COLORINDEX_SELMARGIN,
	COLORINDEX_SELBKGND,
	COLORINDEX_SELTEXT,
	//	Syntax colors
	COLORINDEX_PREFIXED,
	COLORINDEX_KEYWORD,
	COLORINDEX_FUNCNAME,
	COLORINDEX_COMMENT,
	COLORINDEX_NUMBER,
	COLORINDEX_OPERATOR,
	COLORINDEX_STRING,
	COLORINDEX_PREPROCESSOR,
	COLORINDEX_USER1,
	COLORINDEX_USER2,
	//	Compiler/debugger colors
	COLORINDEX_ERRORBKGND,
	COLORINDEX_ERRORTEXT,
	COLORINDEX_EXECUTIONBKGND,
	COLORINDEX_EXECUTIONTEXT,
	COLORINDEX_BREAKPOINTBKGND,
	COLORINDEX_BREAKPOINTTEXT
};

enum tagCRLFSTYLE
{
	CRLF_STYLE_AUTOMATIC	= -1,
	CRLF_STYLE_DOS			= 0,
	CRLF_STYLE_UNIX			= 1,
	CRLF_STYLE_MAC			= 2
};

#define SRCOPT_INSERTTABS	0x00000001
#define SRCOPT_SHOWTABS		0x00000002
#define SRCOPT_BSATBOL		0x00000004
#define SRCOPT_SELMARGIN	0x00000008
#define SRCOPT_AUTOINDENT	0x00000010
#define SRCOPT_BRACEANSI	0x00000020
#define SRCOPT_BRACEGNU		0x00000040
#define SRCOPT_EOLNDOS		0x00000080
#define SRCOPT_EOLNUNIX		0x00000100
#define SRCOPT_EOLNMAC		0x00000200
#define SRCOPT_FNBRACE		0x00000400

typedef struct tagSYNTAXDEFINITION
{
	BOOL	bPlain;
	TCHAR	szName[32];
	TCHAR	szExts[MAX_PATH];
	DWORD	dwFlags;
	DWORD	dwTabSize;
	TCHAR	szOpenComment[8];
	TCHAR	szCloseComment[8];
	TCHAR	szOpenComment2[8];
	TCHAR	szCloseComment2[8];
	TCHAR	szLineComment[8];
	TCHAR	szLineComment2[8];
	TCHAR	szBrackets[16];
	TCHAR	cString;
	TCHAR	cChar;
	TCHAR	cEscape;
	TCHAR	cPreProcessor;
	TCHAR	cFunctionBegin;
	TCHAR	cLineContinuation;
	TCHAR	cLineEnd;
	TCHAR	cDisable;
	TCHAR	cEnable;
	INT		nOpenCommentLen;
	INT		nCloseCommentLen;
	INT		nOpenComment2Len;
	INT		nCloseComment2Len;
	INT		nLineCommentLen;
	INT		nLineComment2Len;
	BOOL	bCommentFirstRealChar;
	BOOL	bPrefixesInStrings;
	LPTSTR	pszOperators;
	LPTSTR	*apszPrefixes;
	LPTSTR	*apszKeywords;
	LPTSTR	*apszUser1Keywords;
	LPTSTR	*apszUser2Keywords;
	LPTSTR	*apszIndent;
	LPTSTR	*apszUnIndent;
	BOOL	bCase;
	COLORREF crWhiteSpace;
	COLORREF crBackground;
	COLORREF crNormalText;
	COLORREF crSelMargin;
	COLORREF crSelBackground;
	COLORREF crSelText;
	COLORREF crPrefixed;
	COLORREF crKeyword;
	COLORREF crFunctionName;
	COLORREF crComment;
	COLORREF crNumber;
	COLORREF crOperator;
	COLORREF crString;
	COLORREF crPreprocessor;
	COLORREF crUser1;
	COLORREF crUser2;
} SYNTAXDEFINITION, *LPSYNTAXDEFINITION;

/* text extension of the edit interface */

typedef struct tagTEXTEDITEXTENSIONVTBL
{
	/* base V-Table */
	LPEDITVTBL lpVtbl;

	/* buffer handling */
	int (*GetLineActualLength)(LPEDITINTERFACE lpInterface, int iLineIndex);
	int (*CalculateActualOffset)(LPEDITINTERFACE lpInterface, int iLineIndex, int iCharIndex);

	/* UI-Toggles */
	BOOL (*GetViewTabs)(LPEDITINTERFACE lpInterface);
	void (*SetViewTabs)(LPEDITINTERFACE lpInterface, BOOL bViewTabs);
	BOOL (*GetSelectionMargin)(LPEDITINTERFACE lpInterface);
	void (*SetSelectionMargin)(LPEDITINTERFACE lpInterface, BOOL bSelMargin);
	HCURSOR (*GetMarginCursor)(LPEDITINTERFACE lpInterface);
	HCURSOR (*SetMarginCursor)(LPEDITINTERFACE lpInterface, HCURSOR hCursor);

	/* searching */
	POINT (*FindWordBoundaryToLeft)(LPTEXTEDITVIEW lpte, POINT pt);
	POINT (*FindWordBoundaryToRight)(LPTEXTEDITVIEW lpte, POINT pt);

	/* file-i/o */
	void (*LoadFileEx)(LPEDITINTERFACE lpInterface, LPCTSTR pszFileName, int nCrLfStyle);
	BOOL (*SaveToFileEx)(LPEDITINTERFACE lpInterface, LPCTSTR pszFileName, int nCrLfStyle, BOOL bClearModifiedFlag, BOOL fBackup, LPCTSTR pszBackupFileName);
	int (*GetCRLFMode)(LPEDITINTERFACE lpInterface);
	void (*SetCRLFMode)(LPEDITINTERFACE lpInterface, int nCRLFMode);

	/* bookmarks */
	void (*GotoNumBookmark)(LPEDITINTERFACE lpInterface, int nBookmarkID)
	void (*ToggleNumBookmark)(LPEDITINTERFACE lpInterface, int nBookmarkID)
	void (*ClearAllNumBookmarks)(LPEDITINTERFACE lpInterface)

	/* other */
	void (*SetTextType)(LPEDITINTERFACE lpInterface, LPCTSTR pszExt);
	POINT (*MatchBrace)(LPEDITINTERFACE lpInterface, POINT ptStartPos);
	void (*Tab)(LPEDITINTERFACE lpInterface);
	void (*UnTab)(LPEDITINTERFACE lpInterface);
	void (*OnDelete)(LPEDITINTERFACE lpInterface);
} TEXTEDITVTBL, *LPTEXTEDITVTBL;

/* macros for nice access of the V-Table in C */

/* buffer handling */
#define PCP_TextEdit_GetLineActualLength(lpInterface, iLineIndex) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->GetLineActualLength((LPEDITINTERFACE)(lpInterface), (int)(iLineIndex)))

#define PCP_TextEdit_CalculateActualOffset(lpInterface, iLineIndex, iCharIndex) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->CalculateActualOffset((LPEDITINTERFACE)(lpInterface), (int)(iLineIndex), (int)(iCharIndex)))

/* Searching */
#define PCP_TextEdit_FindWordBoundaryToLeft(lpte, pt) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->FindWordBoundaryToLeft((LPEDITINTERFACE)(lpInterface), (POINT)(pt)))

#define PCP_TextEdit_FindWordBoundaryToRight(lpte, pt) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->FindWordBoundaryToRight((LPEDITINTERFACE)(lpInterface), (POINT)(pt)))

/* UI-Toggles */
#define PCP_TextEdit_GetViewTabs(lpInterface) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->GetViewTabs((LPEDITINTERFACE)(lpInterface)))

#define PCP_TextEdit_SetViewTabs(lpInterface, bViewTabs) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->SetViewTabs((LPEDITINTERFACE)(lpInterface), (BOOL)(bViewTabs)))

#define PCP_TextEdit_GetSelectionMargin(lpInterface) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->GetSelectionMargin((LPEDITINTERFACE)(lpInterface)))

#define PCP_TextEdit_SetSelectionMargin(lpInterface, bSelMargin) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->SetSelectionMargin((LPEDITINTERFACE)(lpInterface), (BOOL)(bSelMargin)))

#define PCP_TextEdit_GetMarginCursor(lpInterface) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->GetMarginCursor((LPEDITINTERFACE)(lpInterface)))

#define PCP_TextEdit_SetMarginCursor(lpInterface, hCursor) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->SetMarginCursor((LPEDITINTERFACE)(lpInterface), (HCURSOR)(hCursor)))

/* file-i/o */
#define PCP_TextEdit_LoadFileEx(lpInterface, pszFileName, nCrLfStyle) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->LoadFileEx((LPEDITINTERFACE)(lpInterface), (LPCTSTR)(pszFileName), (int)(nCrLfStyle)))

#define PCP_TextEdit_SaveToFileEx(lpInterface, pszFileName, nCrLfStyle, bClearModifiedFlag, fBackup, pszBackupFileName) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->SaveToFileEx((LPEDITINTERFACE)(lpInterface), (LPCTSTR)(pszFileName), (int)(nCrLfStyle), (BOOL)(bClearModifiedFlag), (BOOL)(fBackup), (LPCTSTR)(pszBackupFileName)))

#define PCP_TextEdit_GetCRLFMode(lpInterface) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->GetCRLFMode((LPEDITINTERFACE)(lpInterface)))

#define PCP_TextEdit_SetCRLFMode(lpInterface, nCRLFMode) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->SetCRLFMode((LPEDITINTERFACE)(lpInterface), (int)(nCRLFMode)))

/* bookmarks */
#define PCP_TextEdit_GotoNumBookmark(lpInterface, nBookmarkID \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->GotoNumBookmark((LPEDITINTERFACE)(lpInterface), (int)(nBookmarkID)))

#define PCP_TextEdit_ToggleNumBookmark(lpInterface, nBookmarkID \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->ToggleNumBookmark((LPEDITINTERFACE)(lpInterface), (int)(nBookmarkID)))

#define PCP_TextEdit_ClearAllNumBookmarks(lpInterface \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->ClearAllNumBookmarks((LPEDITINTERFACE)(lpInterface)))

/* other */
#define PCP_TextEdit_SetTextType(lpInterface, pszExt) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->SetTextType((LPEDITINTERFACE)(lpInterface), (LPCTSTR)(pszExt)))

#define PCP_TextEdit_MatchBrace(lpInterface, ptStartPos) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->MatchBrace((LPEDITINTERFACE)(lpInterface), (POINT)(ptStartPos)))

#define PCP_TextEdit_Tab(lpInterface) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->Tab((LPEDITINTERFACE)(lpInterface)))

#define PCP_TextEdit_UnTab(lpInterface) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->UnTab((LPEDITINTERFACE)(lpInterface)))

#define PCP_TextEdit_OnDelete(lpInterface) \
	(((LPTEXTEDITVTBL)(LPEDITINTERFACE)(lpInterface)->lpVtbl)->OnDelete((LPEDITINTERFACE)(lpInterface)))

/* functions */

FOREXPORT void PCP_Edit_TextEdit_AddSyntaxDefinition(LPSYNTAXDEFINITION lptd);
FOREXPORT LPEDITINTERFACE PCP_Edit_TextEdit_GetInterface(HWND hwnd);

#endif /* __PCP_TEXTEDIT_H */
