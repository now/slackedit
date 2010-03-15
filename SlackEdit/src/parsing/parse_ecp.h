/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : ecp_parser.h
 * Created    : not known (before 12/27/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:17:56
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __ECP_PARSER_H
#define __ECP_PARSER_H

typedef struct tagECPCLIPBOOK
{
	LPTSTR	pszTitle;
	TCHAR	szFileName[MAX_PATH];
	BOOL	bSort;
	BOOL	bTclFile;
} ECPCLIPBOOK, *PECPCLIPBOOK;

typedef struct tagECPITEM
{
	LPTSTR	pszTitle;
	BOOL	bTcl;
	LPTSTR	pszBody;
} ECPITEM, *PECPITEM;

BOOL ECP_GetClipbook(LPCTSTR pszFileName, PECPCLIPBOOK pClipbook);
BOOL ECP_ReadItems(HWND hwndList, PECPCLIPBOOK pClipbook);
void ECP_ProcessItem(PECPITEM pItem, BOOL bSwitchFocus);
BOOL ECP_GetItems(HWND hwndList, PECPCLIPBOOK pClipbook);

#endif /* __ECP_PARSER_H */
