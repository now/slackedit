/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_buffer.h
 * Created    : 07/29/99
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:42:57
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_EDIT_BUFFER_H
#define __PCP_EDIT_BUFFER_H

#include "pcp_edit_view.h"

/* internal */
enum LINEFLAGS
{
	LF_BOOKMARK_FIRST			= 0x00000001L,
	LF_EXECUTION				= 0x00010000L,
	LF_BREAKPOINT				= 0x00020000L,
	LF_COMPILATION_ERROR		= 0x00040000L,
	LF_BOOKMARKS	        	= 0x00080000L,
	LF_INVALID_BREAKPOINT       = 0x00100000L
};

#define LF_BOOKMARK(id)		(LF_BOOKMARK_FIRST << id)

void Edit_Buffer_BeginUndoGroup(LPEDITSTATE lpes, BOOL bMergeWithPrevious);
void Edit_Buffer_FlushUndoGroup(LPEDITVIEW lpev, LPEDITSTATE lpes);
int Edit_Buffer_FindNextBookmarkLine(LPEDITSTATE lpes, int iCurrentLine);
int Edit_Buffer_FindPrevBookmarkLine(LPEDITSTATE lpes, int iCurrentLine);
void Edit_Buffer_RecalcPoint(INSERTCONTEXT ic, POINT *pt);
POINT Edit_Buffer_GetLastChangePos(LPEDITSTATE lpes);
void Edit_Buffer_SetLineFlag(LPEDITVIEW lpev, LPEDITSTATE lpes, int iLine, DWORD dwFlag, BOOL bSet, BOOL bRemoveFromPreviousLine);
int Edit_Buffer_FindLineWithFlag(LPEDITSTATE lpes, DWORD dwFlag);
int Edit_Buffer_GetLineWithFlag(LPEDITSTATE lpes, DWORD dwFlag);
DWORD Edit_Buffer_GetLineFlags(LPEDITSTATE lpes, int iLine);
void Edit_Buffer_FreeAll(LPEDITSTATE lpes);
BOOL Edit_Buffer_InitNew(LPEDITVIEW lpev, int iCrLfStyle);
int Edit_Buffer_GetLineCount(LPEDITSTATE lpes);
int Edit_Buffer_GetLineLength(LPEDITSTATE lpes, int iLine);
LPTSTR Edit_Buffer_GetLineChars(LPEDITSTATE lpes, int iLine);
void Edit_Buffer_GetText(LPEDITSTATE lpes, int iStartLine, int iStartChar, int iEndLine, int iEndChar, LPTSTR *pszText, LPCTSTR pszCRLF);

/* interfaced */
void Edit_Buffer_SetModified(LPEDITSTATE lpes, BOOL bModified);
int Edit_Buffer_GetLastActionDescription(LPEDITSTATE lpes);
BOOL Edit_Buffer_InsertText(LPEDITVIEW lpev, LPEDITSTATE lpes, int iLine, int iPos, LPCTSTR pszText, int *iEndLine, int *iEndChar, int iAction);
BOOL Edit_Buffer_DeleteText(LPEDITVIEW lpev, LPEDITSTATE lpes, int iStartLine, int iStartChar, int iEndLine, int iEndChar, int iAction);
BOOL Edit_Buffer_Undo(LPEDITVIEW lpev, LPEDITSTATE lpes, POINT *ptCursorPos);
BOOL Edit_Buffer_Redo(LPEDITVIEW lpev, LPEDITSTATE lpes, POINT *ptCursorPos);
int Edit_Buffer_GetUndoDescription(LPEDITSTATE lpes, LPTSTR pszDesc, int iPos);
int Edit_Buffer_GetRedoDescription(LPEDITSTATE lpes, LPTSTR pszDesc, int iPos);
BOOL Edit_Buffer_CanUndo(LPEDITSTATE lpes);
BOOL Edit_Buffer_CanRedo(LPEDITSTATE lpes);
int Edit_Buffer_GetCRLFMode(LPEDITSTATE lpes);
void Edit_Buffer_SetCRLFMode(LPEDITSTATE lpes, int iCRLFMode);
BOOL Edit_Buffer_GetReadOnly(LPEDITSTATE lpes);
void Edit_Buffer_SetReadOnly(LPEDITSTATE lpes, BOOL bReadOnly);
BOOL Edit_Buffer_LoadFromFile(LPEDITVIEW lpev, LPEDITSTATE lpes, LPCTSTR pszFileName, int iCrLfStyle);
BOOL Edit_Buffer_SaveToFile(LPEDITSTATE lpes, LPCTSTR pszFileName, int iCrLfStyle, BOOL bClearModifiedFlag, BOOL fBackup, LPCTSTR pszBackupFileName);

#endif /* __PCP_EDIT_BUFFER_H */
