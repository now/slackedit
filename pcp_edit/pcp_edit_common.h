/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_common.h
 * Created    : 08/13/00
 * Owner      : pcppopper
 * Revised on : 08/13/00
 * Comments   : 
 *              
 *              
 ****************************************************************/

#ifndef __PCP_EDIT_COMMON_H
#define __PCP_EDIT_COMMON_H

typedef struct tagEDITWINDOW
{
	DWORD		dwLastSearchFlags;
	BOOL		bLastSearchSuccessful;
	int			nLastSearchLen;
	int			nLastReplaceLen;
	LPTSTR		pszSearchString;
	LPTSTR		pszMatchedLine;
	RxNode		*rxNode;
	RxMatchRes	rxMatch;
	POINT		ptIncrementalSearchStartPos;
	POINT		ptCursorPosBeforeIncrementalSearch;
	POINT		ptSelStartBeforeIncrementalSearch;
	POINT		ptSelEndBeforeIncrementalSearch;
	BOOL		bIncrementalSearchActive;
} EDITWINDOW, *LPEDITWINDOW;

BOOL CommonEdit_Find_FindText(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, LPCTSTR pszText, POINT ptStartPos, DWORD dwFlags, POINT *lpptFoundPos)
BOOL CommonEdit_Find_FindTextInBlock(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, LPCTSTR pszText, POINT ptStartPosition,
							POINT ptBlockBegin, POINT ptBlockEnd, DWORD dwFlags, LPPOINT lpptFoundPos)

BOOL CommonEdit_Find_FindTextVisual(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, LPCTSTR pszText, POINT ptStartPos, DWORD dwFlags, LPPOINT lpptFoundPos);
BOOL CommonEdit_Find_FindNext(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, LPPOINT lpptFoundPos);
BOOL CommonEdit_Find_FindPrevious(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, LPPOINT lpptFoundPos);

BOOL CommonEdit_Find_ReplaceSelection(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, LPCTSTR pszNewText, DWORD dwFlags)

DWORD CommonEdit_Find_GetLastSearchFlags(LPEDITWINDOW lpEditWindow);
LPCTSTR CommonEdit_Find_GetSearchString(LPEDITWINDOW lpEditWindow);

void CommonEdit_IncrementalSearch_Start(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, BOOL bForward);
LPCTSTR CommonEdit_IncrementalSearch_AddChar(LPEDITWINDOW lpEditWindow, TCHAR cChar);
LPCTSTR CommonEdit_IncrementalSearch_RemoveChar(LPEDITWINDOW lpEditWindow);
LPCTSTR CommonEdit_IncrementalSearch_GetChars(LPEDITWINDOW lpEditWindow);
void CommonEdit_IncrementalSearch_End(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, BOOL bReturnToOldPlace);

BOOL CommonEdit_Find_FindSelection(LPEDITINTERFACE lpInterface, LPEDITWINDOW lpEditWindow, BOOL fForward, LPPOINT lpptFoundPos);

#endif /* __PCP_EDIT_COMMON_H */
