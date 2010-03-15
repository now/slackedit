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

#ifndef __PCP_TEXTEDIT_BUFFER_H
#define __PCP_TEXTEDIT_BUFFER_H

#include "pcp_textedit_view.h"

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

int TextEdit_Buffer_FindNextBookmarkLine(LPTEXTEDITSTATE lpes, int iCurrentLine);
int TextEdit_Buffer_FindPrevBookmarkLine(LPTEXTEDITSTATE lpes, int iCurrentLine);
void TextEdit_Buffer_RecalcPoint(INSERTCONTEXT ic, POINT *pt);
POINT TextEdit_Buffer_GetLastChangePos(LPTEXTEDITSTATE lpes);
void TextEdit_Buffer_SetLineFlag(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, int iLine, DWORD dwFlag, BOOL bSet, BOOL bRemoveFromPreviousLine);
int TextEdit_Buffer_FindLineWithFlag(LPTEXTEDITSTATE lpes, DWORD dwFlag);
int TextEdit_Buffer_GetLineWithFlag(LPTEXTEDITSTATE lpes, DWORD dwFlag);
DWORD TextEdit_Buffer_GetLineFlags(LPTEXTEDITSTATE lpes, int iLine);
void TextEdit_Buffer_FreeAll(LPTEXTEDITSTATE lpes);
BOOL TextEdit_Buffer_InitNew(LPTEXTEDITVIEW lpte, int iCrLfStyle);
int TextEdit_Buffer_GetLineCount(LPTEXTEDITSTATE lpes);
int TextEdit_Buffer_GetLineLength(LPTEXTEDITSTATE lpes, int iLine);
LPTSTR TextEdit_Buffer_GetLineChars(LPTEXTEDITSTATE lpes, int iLine);
void TextEdit_Buffer_GetText(LPTEXTEDITSTATE lpes, int iStartLine, int iStartChar, int iEndLine, int iEndChar, LPTSTR *pszText, LPCTSTR pszCRLF);

/* interfaced */
void TextEdit_Buffer_BeginUndoGroup(LPTEXTEDITSTATE lpes, BOOL bMergeWithPrevious);
void TextEdit_Buffer_FlushUndoGroup(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes);
BOOL TextEdit_Buffer_GetModified(LPTEXTEDITSTATE lpes);
void TextEdit_Buffer_SetModified(LPTEXTEDITSTATE lpes, BOOL bModified);
int TextEdit_Buffer_GetLastActionDescription(LPTEXTEDITSTATE lpes);
BOOL TextEdit_Buffer_InsertText(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, int iLine, int iPos, LPCTSTR pszText, int *iEndLine, int *iEndChar, int iAction);
BOOL TextEdit_Buffer_DeleteText(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, int iStartLine, int iStartChar, int iEndLine, int iEndChar, int iAction);
BOOL TextEdit_Buffer_InsertTextPos(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, int nPos, LPCTSTR pszText, int nAction);
BOOL TextEdit_Buffer_DeleteTextPos(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, int nStartPos, int nEndPos, int nAction);
BOOL TextEdit_Buffer_Undo(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, POINT *ptCursorPos);
BOOL TextEdit_Buffer_Redo(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, POINT *ptCursorPos);
int TextEdit_Buffer_GetUndoDescription(LPTEXTEDITSTATE lpes, LPTSTR pszDesc, int iPos);
int TextEdit_Buffer_GetRedoDescription(LPTEXTEDITSTATE lpes, LPTSTR pszDesc, int iPos);
BOOL TextEdit_Buffer_CanUndo(LPTEXTEDITSTATE lpes);
BOOL TextEdit_Buffer_CanRedo(LPTEXTEDITSTATE lpes);
int TextEdit_Buffer_GetCRLFMode(LPTEXTEDITSTATE lpes);
void TextEdit_Buffer_SetCRLFMode(LPTEXTEDITSTATE lpes, int iCRLFMode);
BOOL TextEdit_Buffer_GetReadOnly(LPTEXTEDITSTATE lpes);
void TextEdit_Buffer_SetReadOnly(LPTEXTEDITSTATE lpes, BOOL bReadOnly);
BOOL TextEdit_Buffer_LoadFromFile(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, LPCTSTR pszFileName, int iCrLfStyle);
BOOL TextEdit_Buffer_SaveToFile(LPTEXTEDITSTATE lpes, LPCTSTR pszFileName, int iCrLfStyle, BOOL bClearModifiedFlag, BOOL fBackup, LPCTSTR pszBackupFileName);

#endif /* __PCP_TEXTEDIT_BUFFER_H */
