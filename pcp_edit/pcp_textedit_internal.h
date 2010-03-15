/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_editview_internal.h
 * Created    : not known (before 01/03/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:41:42
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __EDITVIEW_INTERNAL_H
#define __EDITVIEW_INTERNAL_H

#include "pcp_textedit.h" /* For: LPSYNTAXDEFINITION */

typedef struct tagINSERTCONTEXT
{
	BOOL bInsert;
	POINT ptStart;
	POINT ptEnd;
} INSERTCONTEXT, *LPINSERTCONTEXT;

typedef struct tagTEXTBLOCK
{
	int	iCharPos;
	int iColorIndex;
} TEXTBLOCK, *LPTEXTBLOCK;

BOOL TextEdit_Internal_RegisterControl(HMODULE hModule);
void TextEdit_Internal_UnregisterControl(HMODULE hModule);

void TextEdit_Internal_CreateSyntaxDefinitionsList(void);
void TextEdit_Internal_AddSyntaxDefinition(LPSYNTAXDEFINITION lpsd);
LPSYNTAXDEFINITION TextEdit_Internal_GetSyntaxDefintion(LPCTSTR pszExt);

#endif /* __EDITVIEW_INTERNAL_H */
