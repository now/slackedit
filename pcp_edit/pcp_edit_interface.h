/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_interface.h
 * Created    : 07/22/00
 * Owner      : pcppopper
 * Revised on : 07/22/00
 * Comments   : 
 *              
 *              
 ****************************************************************/

#ifndef __PCP_EDIT_INTERFACE_H
#define __PCP_EDIT_INTERFACE_H

#define WC_EDITVIEW "Popping_Edit"

#define DEFAULT_PRINT_MARGIN		1000	//10 Millimeters

#define EB_MAX_LINE_LENGTH			2048

enum
{
	FIND_MATCH_CASE		= 0x0001,
	FIND_WHOLE_WORD		= 0x0002,
	FIND_REG_EXP		= 0x0004,
	FIND_DIRECTION_UP	= 0x0010,
	REPLACE_SELECTION	= 0x0100
};

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

enum CRLFSTYLE
{
	CRLF_STYLE_AUTOMATIC	= -1,
	CRLF_STYLE_DOS			= 0,
	CRLF_STYLE_UNIX			= 1,
	CRLF_STYLE_MAC			= 2
};

enum
{
	CE_ACTION_UNKNOWN			= 0,
	CE_ACTION_PASTE				= 1,
	CE_ACTION_DELSEL			= 2,
	CE_ACTION_CUT				= 3,
	CE_ACTION_TYPING			= 4,
	CE_ACTION_BACKSPACE			= 5,
	CE_ACTION_INDENT			= 6,
	CE_ACTION_DRAGDROP			= 7,
	CE_ACTION_REPLACE			= 8,
	CE_ACTION_DELETE			= 9,
	CE_ACTION_AUTOINDENT		= 10,
	CE_ACTION_AUTOCOMPLETE		= 11,
	CE_ACTION_INTERNALINSERT	= 12
	//	...
	//	Expandable: user actions allowed
};

/* Notifications */
#define PEN_FIRST					(31337)
#define PEN_SELCHANGE				(PEN_FIRST + 0)
#define PEN_INCREMENTALSEARCHUPDATE	(PEN_FIRST + 1)
#define PEN_OVERWRITEMODECHANGED	(PEN_FIRST + 2)

#define PE_INCREMENTAL_STATUS_FORWARD	0x0001
#define PE_INCREMENTAL_STATUS_REVERSE	0x0002
#define PE_INCREMENTAL_STATUS_FAILING	0x0004
#define PE_INCREMENTAL_STATUS_ENDED		0x0008

typedef struct tagNMINCREMENTALSEARCH
{
	NMHDR nmhdr;
	UINT uStatus;
	LPTSTR pszSearchText;
} NMINCREMENTALSTATUS, *LPNMINCREMENTALSTATUS;

#define PEM_FIRST				(WM_USER + 301)
#define PEM_GETSELTEXT			(PEM_FIRST + 0)
#define PEM_GETSEL				(PEM_FIRST + 1)
#define PEM_GETSELXY			(PEM_FIRST + 2)
#define PEM_SETSEL				(PEM_FIRST + 3)
#define PEM_SETSELXY			(PEM_FIRST + 4)
#define PEM_REPLACESEL			(PEM_FIRST + 5)
#define PEM_GETLINE				(PEM_FIRST + 6)
#define PEM_LINECOUNT			(PEM_FIRST + 7)
#define PEM_LINELENGTH			(PEM_FIRST + 8)
#define PEM_GETCURSORPOS		(PEM_FIRST + 9)
#define PEM_CANUNDO				(PEM_FIRST + 10)
#define PEM_CANREDO				(PEM_FIRST + 11)
#define PEM_UNDO				(PEM_FIRST + 12)
#define PEM_REDO				(PEM_FIRST + 13)
#define PEM_CANPASTE			(PEM_FIRST + 14)
#define PEM_GETMODIFY			(PEM_FIRST + 15)
#define PEM_SETMODIFY			(PEM_FIRST + 16)
#define PEM_LOADFILE			(PEM_FIRST + 17)
#define PEM_GOTOLINE			(PEM_FIRST + 18)
#define PEM_GETOVERWRITEMODE	(PEM_FIRST + 19)
#define PEM_SETOVERWRITEMODE	(PEM_FIRST + 20)

#define PcpEdit_GetSelText(hwndCtl, lpstr)			((DWORD)SNDMSG((hwndCtl), PEM_GETSELTEXT, 0, (LPARAM)(LPTSTR)(lpstr)))
#define PcpEdit_GetSel(hwndCtl, ptStart, ptEnd)		((DWORD)SNDMSG((hwndCtl), PEM_GETSEL, (WPARAM)(LPPOINT)(ptStart), (LPARAM)(LPPOINT)(ptEnd)))
#define PcpEdit_GetSelXY(hwndCtl, ptStart, ptEnd)	((DWORD)SNDMSG((hwndCtl), PEM_GETSELXY, (WPARAM)(ptStart), (LPARAM)(ptEnd)))
#define PcpEdit_SetSel(hwndCtl, ptStart, ptEnd)		((DWORD)SNDMSG((hwndCtl), PEM_SETSEL, (WPARAM)(LPPOINT)(ptStart), (LPARAM)(LPPOINT)(ptEnd)))
#define PcpEdit_SetSelXY(hwndCtl, ptStart, ptEnd)	((DWORD)SNDMSG((hwndCtl), PEM_SETSELXY, (WPARAM)(ptStart), (LPARAM)(ptEnd)))
#define PcpEdit_ReplaceSel(hwndCtl, lpstr)			((DWORD)SNDMSG((hwndCtl), PEM_REPLACESEL, 0, (LPARAM)(LPTSTR)(lpstr)))
#define PcpEdit_GetLineCount(hwndCtl)				((DWORD)SNDMSG((hwndCtl), PEM_LINECOUNT, 0, 0L))
#define PcpEdit_GetLineLength(hwndCtl, line)		((DWORD)SNDMSG((hwndCtl), PEM_LINELENGTH, 0, (LPARAM)(line)))
#define PcpEdit_GetLine(hwndCtl, line, lpstr)		((DWORD)SNDMSG((hwndCtl), PEM_GETLINE, (WPARAM)(line), (LPARAM)(LPTSTR)(lpstr)))
#define PcpEdit_GetCursorPos(hwndCtl, lppt)			((DWORD)SNDMSG((hwndCtl), PEM_GETCURSORPOS, 0, (LPARAM)(LPPOINT)(lppt)))
#define PcpEdit_Undo(hwndCtl)						((DWORD)SNDMSG((hwndCtl), PEM_UNDO, 0, 0L))
#define PcpEdit_Redo(hwndCtl)						((DWORD)SNDMSG((hwndCtl), PEM_REDO, 0, 0L))
#define PcpEdit_GetModify(hwndCtl)					((DWORD)SNDMSG((hwndCtl), PEM_GETMODIFY, 0, 0L))
#define PcpEdit_CanRedo(hwndCtl)					((DWORD)SNDMSG((hwndCtl), PEM_CANREDO, 0, 0L))
#define PcpEdit_CanUndo(hwndCtl)					((DWORD)SNDMSG((hwndCtl), PEM_CANUNDO, 0, 0L))
#define PcpEdit_CanPaste(hwndCtl)					((DWORD)SNDMSG((hwndCtl), PEM_CANPASTE, 0, 0L))
#define PcpEdit_SetModify(hwndCtl, fMod)			((DWORD)SNDMSG((hwndCtl), PEM_SETMODIFY, 0, (LPARAM)(BOOL)(fMod)))
#define PcpEdit_DelSel(hwndCtl)						((DWORD)SNDMSG((hwndCtl), WM_CLEAR, 0, 0L))
#define PcpEdit_Cut(hwndCtl)						((VOID)SNDMSG((hwndCtl), WM_CUT, 0, 0L))
#define PcpEdit_Paste(hwndCtl)						((DWORD)SNDMSG((hwndCtl), WM_PASTE, 0, 0L))
#define PcpEdit_LoadFile(hwndCtl, lpstr)			((DWORD)SNDMSG((hwndCtl), PEM_LOADFILE, 0, (LPARAM)(LPCTSTR)(lpstr)))
#define PcpEdit_GotoLine(hwndCtl, nLine, bExtend)	((BOOL)SNDMSG((hwndCtl), PEM_GOTOLINE, (WPARAM)(INT)(nLine), (LPARAM)(BOOL)(bExtend)))
#define PcpEdit_GetOverWriteMode(hwndCtl)			((BOOL)SNDMSG((hwndCtl), PEM_GETOVERWRITEMODE, 0, 0L))
#define PcpEdit_SetOverWriteMode(hwndCtl, bOvr)		((BOOL)SNDMSG((hwndCtl), PEM_SETOVERWRITEMODE, 0, (LPARAM)(BOOL)(bOvr)))

struct Pcp_Edit_Class;

struct tagPcp_Edit_ClassVtbl
{

#endif /* __PCP_EDIT_INTERFACE_H */
