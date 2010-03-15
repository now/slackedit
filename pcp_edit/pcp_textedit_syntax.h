/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_syntax.h
 * Created    : not known (before 01/24/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:41:34
 * Comments   : Generic syntax hiliting for pcp_edit 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_EDIT_SYNTAX_H
#define __PCP_EDIT_SYNTAX_H

#include "pcp_edit.h"

DWORD TextEdit_Syntax_ParseLineGeneric(LPTEXTEDITVIEW lpte, DWORD dwCookie, int nLineIndex, LPTEXTBLOCK pBuf, int *piActualItems);
BOOL TextEdit_Syntax_IsKeywordGeneric(LPTSTR apszKeywords[], LPCTSTR pszChars, int nLength, BOOL bCase);

#endif /* __PCP_EDIT_SYNTAX_H */
